/*
 * Copyright (C) 2022-2025 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG CBStage

#include "CBStage.h"

#include "CBLayoutUtils.h"
#include "PlatformData.h"
#include "StageDescriptor.h"
#include "ia_pal_types_isp_ids_autogen.h"
#include "iutils/CameraLog.h"

namespace icamera {

CBStage::CBStage(int cameraId, int streamId, int stageId, uint8_t contextId, uint8_t psysContextId,
                 uint8_t resourceId, const std::string& cbName, PSysDevice* psysDevice,
                 IpuPacAdaptor* pacAdapt)
        : IPipeStage(cbName.c_str(), stageId),
          mCameraId(cameraId),
          mStreamId(streamId),
          mOuterNodeCtxId(contextId),
          mContextId(psysContextId),
          mResourceId(resourceId),
          mPSysDevice(psysDevice),
          mHasStatsTerminal(false),
          mPacAdapt(pacAdapt),
          mLinkStreamMode(LINK_STREAMING_MODE_SOFF),
          sPayloadDesc(nullptr),
          mPayloadDescCount(0),
          sTerminalDesc(nullptr),
          mTerminalDescCount(0),
          mKernelOffsetBuf(nullptr),
          mIaAicBuf(nullptr),
          mNode2SelfBufIndex(0) {
    LOG1("%s, graph ctxId %d, psys ctxId %d, mPSysDevice %p", __func__, mContextId, mOuterNodeCtxId,
         mPSysDevice);

    psysDevice->registerPSysDeviceCallback(mContextId, this);
}

CBStage::~CBStage() {}

int CBStage::init() {
    for (uint8_t i = 0U; i < kMaxTerminalBufArray; i++) {
        TerminalBufferInfo terminalBufferInfo;
        mTerminalBufferMaps[i] = terminalBufferInfo;
    }

    int ret = CBLayoutUtils::getCbPayloadDescriptor(mResourceId, &sPayloadDesc, mPayloadDescCount);
    CheckAndLogError(ret != OK, ret, "Failed to get payload descriptor");

    ret = CBLayoutUtils::getCbTerminalDescriptors(mResourceId, sTerminalDesc, mTerminalDescCount);
    CheckAndLogError(ret != OK, ret, "Failed to get terminal descriptor");

    // Use share memory buffers for sandboxing
    const size_t size =
        sizeof(uint32_t) * (mTerminalDescCount + kMaxSectionCount * 2 * kMaxTerminalBufArray);
    mKernelOffsetBuf =
        static_cast<uint32_t*>(mPacAdapt->allocateBuffer(mStreamId, mContextId, -1, size));

    mIaAicBuf = new aic::IaAicBuffer[mTerminalDescCount * kMaxTerminalBufArray];
    memset(mIaAicBuf, 0, sizeof(aic::IaAicBuffer) * mTerminalDescCount * kMaxTerminalBufArray);

    return OK;
}

void CBStage::deInit() {
    delete[] mIaAicBuf;

    // free all buffers allocated from PAC
    for (auto bufmap : mTerminalBufferMaps) {
        auto& terminalBufMap = bufmap.second.mMetadataBufferMap;
        for (auto it : terminalBufMap) {
            mPacAdapt->releaseBuffer(mStreamId, mContextId, it.first, it.second.userPtr);
        }
    }
    for (auto bufmap : mTerminalBufferMaps) {
        auto& terminalBufMap = bufmap.second.mPayloadBufferMap;
        for (auto it : terminalBufMap) {
            mPacAdapt->releaseBuffer(mStreamId, mContextId, it.first, it.second.userPtr);
        }
    }
    mTerminalBufferMaps.clear();

    // kernelOffset buffer do not have terminal id
    mPacAdapt->releaseBuffer(mStreamId, 0, 0, mKernelOffsetBuf);

    for (auto& it : mNode2SelfBuffers) {
        for (auto& buf: it.second) {
            free(buf.userPtr);
        }
    }
    mNode2SelfBuffers.clear();
    mNode2SelfLinks.clear();

    mUserToTerminalBuffer.clear();
}

// config the data terminals, init, config and prepare PAC.
int CBStage::configure(const StaticGraphNodeKernels& kernelGroup, const GraphLink** links,
                       uint32_t numOfLink,
                       std::unordered_map<uint8_t, TerminalConfig>& terminalConfig) {
    int ret = setTerminalLinkAndAllocNode2SelfBuffers(links, numOfLink);
    CheckAndLogError(ret != OK, ret, "Failed to alloc Node2Self buffers ret %d", ret);

    // Alloc stats buffers
    ret = allocMetadataBuffer(links, numOfLink, terminalConfig);
    CheckAndLogError(ret != OK, ret, "Failed to alloc metadata buffers ret %d", ret);

    PacTerminalBufMap termBufMap;
    aic::IaAicBuffer* iaAicBuf = mIaAicBuf;

    // Config PAC
    ret = pacConfig(kernelGroup, &iaAicBuf, terminalConfig, termBufMap);
    CheckAndLogError(ret != OK, ret, "Failed to config PAC ret %d", ret);

    // Register buffers into driver
    ret = registerMetadataBuffer(&iaAicBuf, termBufMap);
    CheckAndLogError(ret != OK, ret, "Failed to register metadata buffers ret %d", ret);

    ret = mPacAdapt->setPacTerminalData(mStreamId, mContextId, termBufMap);
    CheckAndLogError(ret != OK, ret, "Failed to set PAC terminal data, ret %d", ret);

    return OK;
}

void CBStage::registerListener(EventType eventType, EventListener* eventListener) {
    LOG1(" %s : %s  register %d", __func__, getName(), eventType);

    EventSource::registerListener(eventType, eventListener);
}

void CBStage::removeListener(EventType eventType, EventListener* eventListener) {
    LOG1(" %s : %s  unregister %d", __func__, getName(), eventType);

    EventSource::removeListener(eventType, eventListener);
}

bool CBStage::process(int64_t triggerId) {
    PERF_CAMERA_ATRACE_PARAM1(getName(), triggerId);

    {
        std::lock_guard<std::mutex> l(mDataLock);
        if (mStageTaskList.size() >= MAX_FRAME_NUM) {
            return true;
        }
    }

    StageTask task;
    if (fetchTask(&task) != OK) {
        return true;
    }

    // Check if the stage needs to run for valid output buffer
    bool needRun = false;
    for (const auto& item : task.outBuffers) {
        if (item.second != nullptr) {
            needRun = true;
            break;
        }
    }
    LOG2("<seq%ld>%s: process @ %ld, needRun %d", task.sequence, getName(), triggerId, needRun);
    if (!needRun) {
        // Return buffers to producer.
        if (mBufferProducer != nullptr) {
            for (const auto& item : task.inBuffers) {
                mBufferProducer->qbuf(item.first, item.second);
            }
        }
        return true;
    }

    return processTask(&task) == OK ? true : false;
}

int32_t CBStage::processTask(StageTask* task) {
    PacTerminalBufMap bufferMap;
    int ret = mPacAdapt->getAllBuffers(mStreamId, mContextId, task->sequence, bufferMap);
    if (ret != OK) {
        LOG2("%s, no PAC results and not run task for seq %ld", __func__, task->sequence);
        returnBuffers(task->inBuffers, task->outBuffers);
        return OK;
    }

    // Fill real buffer to run pipe
    for (auto& item : task->outBuffers) {
        if (item.second.get() == nullptr) {
            item.second = mInternalOutputBuffers[item.first];
        }
        item.second->setSequence(task->sequence);
    }

    std::unordered_map<uint8_t, TerminalBuffer> terminalBuffers;

    if (mInputPortTerminals.empty()) {
        ret = addFrameTerminals(&terminalBuffers, task->inBuffers);
        CheckAndLogError(ret != OK, ret, "Failed to add terminals for task->inBuffers");
    } else {
        std::map<uuid, std::shared_ptr<CameraBuffer>> inBuffers;
        // Map input port to terminal uuid
        for (auto& item : task->inBuffers) {
            CheckAndLogError(mInputPortTerminals.find(item.first) == mInputPortTerminals.end(),
                             UNKNOWN_ERROR, "%s: wrong input port %d", getName(), item.first);
            inBuffers[mInputPortTerminals[item.first]] = item.second;
        }
        ret = addFrameTerminals(&terminalBuffers, inBuffers);
        CheckAndLogError(ret != OK, ret, "Failed to add terminals for inBuffers");
    }

    ret = addFrameTerminals(&terminalBuffers, task->outBuffers, task->sequence);
    CheckAndLogError(ret != OK, ret, "Failed to add terminals for  task->outBuffers");

    {
        std::lock_guard<std::mutex> l(mDataLock);
        mStageTaskList.push_back(*task);
    }

    ret = addTask(&terminalBuffers, bufferMap, task->sequence);
    CheckAndLogError(ret != OK, ret, "Failed to add task ret %d", ret);

    if (mLinkStreamMode == LINK_STREAMING_MODE_BCLM) {
        std::map<uuid, std::shared_ptr<CameraBuffer> > inBuffers;
        returnBuffers(inBuffers, task->outBuffers);
    }

    return ret;
}

int CBStage::bufferDone(int64_t sequence) {
    std::lock_guard<std::mutex> l(mDataLock);

    if (mStageTaskList.size() > 0) {
        auto task = mStageTaskList.front();

        // Remove internal output buffers
        for (auto& item : task.outBuffers) {
            if (item.second.get() == mInternalOutputBuffers[item.first].get()) {
                item.second = nullptr;
            }
        }

        updateInfoAndSendEvents(&task);

        if (mLinkStreamMode == LINK_STREAMING_MODE_BCLM) {
            std::map<uuid, std::shared_ptr<CameraBuffer> > outBuffers;
            returnBuffers(task.inBuffers, outBuffers);
        } else {
            returnBuffers(task.inBuffers, task.outBuffers);
        }

        if (sequence != task.sequence) {
            LOGW("Buffer returned out of order");
        }

        mStageTaskList.pop_front();
    } else {
        LOGW("%s, sequence %ld wasn't missing", __func__, sequence);
    }

    // Unregister external DMA buffers for XE driver
    unregisterExtDmaBuf(sequence);

    return OK;
}

int32_t CBStage::fetchTask(StageTask* task) {
    AutoMutex l(mBufferQueueLock);

    const int32_t ret = getFreeBuffersInQueue(task->inBuffers, task->outBuffers);
    if (ret != OK) {
        return ret;
    }

    task->sequence = task->inBuffers.begin()->second->getSequence();
    return OK;
}

void CBStage::updateInfoAndSendEvents(StageTask* task) {
    std::shared_ptr<CameraBuffer> inBuf = task->inBuffers.begin()->second;
    v4l2_buffer_t inV4l2Buf = *inBuf->getV4L2Buffer().Get();

    EventData bufferEvent;
    bufferEvent.type = EVENT_STAGE_BUF_READY;
    bufferEvent.data.stageBufReady.sequence = task->sequence;
    for (auto& item : task->outBuffers) {
        if (item.second == nullptr) {
            continue;
        }

        item.second->updateV4l2Buffer(inV4l2Buf);
        bufferEvent.data.stageBufReady.uuid = item.first;
        bufferEvent.buffer = item.second;

        if ((CameraDump::isDumpTypeEnable(DUMP_PSYS_OUTPUT_BUFFER) &&
             (mResourceId == NODE_RESOURCE_ID_BBPS)) ||
            (CameraDump::isDumpTypeEnable(DUMP_PSYS_INTERM_BUFFER) &&
             (mResourceId == NODE_RESOURCE_ID_LBFF))) {
            CameraDump::dumpImage(mCameraId, item.second, M_PSYS, item.first);
        }

        notifyListeners(bufferEvent);
    }
    if (mHasStatsTerminal) {
        const unsigned long long timestamp = TIMEVAL2USECS(inBuf->getTimestamp());

        // Decode stats before send out event
        mPacAdapt->decodeStats(mStreamId, mContextId, task->sequence, timestamp);

        EventData statsEvent;
        statsEvent.type = EVENT_PSYS_STATS_BUF_READY;
        statsEvent.data.statsReady.sequence = task->sequence;
        statsEvent.data.statsReady.timestamp.tv_sec = inBuf->getTimestamp().tv_sec;
        statsEvent.data.statsReady.timestamp.tv_usec = inBuf->getTimestamp().tv_usec;
        statsEvent.pipeType = mStreamId;  // get the stream id from uuid in the future
        notifyListeners(statsEvent);
    }
}

int32_t CBStage::allocateFrameBuffers() {
    mInternalOutputBuffers.clear();
    // Allocate internal output buffers to support pipe execution without user output buffer
    for (auto const& item : mOutputFrameInfo) {
        const int fmt = item.second.format;
        const int width = item.second.width;
        const int height = item.second.height;
        const int size = CameraUtils::getFrameSize(fmt, width, height, true);
        std::shared_ptr<CameraBuffer> buf =
            CameraBuffer::create(V4L2_MEMORY_USERPTR, size, 0, fmt, width, height);
        CheckAndLogError(buf == nullptr, NO_MEMORY, "@%s: Allocate internal output buffer failed",
                         __func__);
        mInternalOutputBuffers[item.first] = buf;
    }

    const int bufCount = PlatformData::getMaxRequestsInflight(mCameraId);
    if (mBufferProducer != nullptr) {
        return allocProducerBuffers(mCameraId, bufCount);
    }

    return OK;
}

int CBStage::start() {
    return allocateFrameBuffers();
}

int CBStage::stop() {
    mInternalOutputBuffers.clear();
    return OK;
}

int CBStage::allocateNode2SelfBuffers(const PSysLink& psysLink, uint32_t bufferSize) {
    mNode2SelfLinks[psysLink.srcTermId].push_back(psysLink);
    // Buffer allocated
    if (mNode2SelfBuffers.find(psysLink.srcTermId) != mNode2SelfBuffers.end()) {
        return OK;
    }

    std::vector<TerminalBuffer>& bufV = mNode2SelfBuffers[psysLink.srcTermId];
    for (uint8_t i = 0U; i < kMaxNode2SelfBufArray; i++) {
        TerminalBuffer terminalBuf;
        CLEAR(terminalBuf);
        terminalBuf.userPtr = nullptr;
        terminalBuf.size = ALIGN_64(bufferSize);
        int ret = posix_memalign(&terminalBuf.userPtr, PAGE_SIZE_U, PAGE_ALIGN(terminalBuf.size));
        CheckAndLogError(ret != 0, NO_MEMORY, "Failed to alloc buffer");
        memset(terminalBuf.userPtr, 0, PAGE_ALIGN(terminalBuf.size));

        terminalBuf.flags |= IPU_BUFFER_FLAG_USERPTR | IPU_BUFFER_FLAG_NO_FLUSH;

        ret = mPSysDevice->registerBuffer(&terminalBuf);
        if (ret != OK) {
            LOGE("Failed to register node2self buffer ret %d", ret);
            free(terminalBuf.userPtr);
            return ret;
        }

        mUserToTerminalBuffer[terminalBuf.userPtr] = terminalBuf;
        bufV.push_back(terminalBuf);
    }

    return OK;
}

int CBStage::setTerminalLinkAndAllocNode2SelfBuffers(const GraphLink** links, uint8_t numOfLink) {
    for (uint8_t i = 0U; i < numOfLink; i++) {
        const GraphLink* link = links[i];

        if (!link->isActive) {
            continue;
        }
        const bool related = ((link->srcNode != nullptr) &&
                        (link->srcNode->contextId == mOuterNodeCtxId)) ||
                       ((link->destNode != nullptr) &&
                        (link->destNode->contextId == mOuterNodeCtxId));
        if (!related) {
            continue;
        }

        PSysLink psysLink;
        switch (link->type) {
            case LinkType::Source2Node:
                psysLink.srcNodeCtxId = 0xFF;
                psysLink.srcTermId = 0xFF;
                psysLink.dstNodeCtxId = mContextId;
                psysLink.dstTermId = link->destTerminalId;
                break;
            case LinkType::Node2Node:
                if (link->destNode->contextId == mOuterNodeCtxId) {
                    psysLink.srcNodeCtxId = 0xFF;  // can't know psys ctx id of other node
                    psysLink.srcTermId = 0xFF;
                    psysLink.dstNodeCtxId = mContextId;
                    psysLink.dstTermId = link->destTerminalId;
                } else if (link->srcNode->contextId == mOuterNodeCtxId) {
                    psysLink.srcNodeCtxId = mContextId;
                    psysLink.srcTermId = link->srcTerminalId;
                    psysLink.dstNodeCtxId = 0xFF;
                    psysLink.dstTermId = 0xFF;
                }
                break;
            case LinkType::Node2Self:
                psysLink.srcNodeCtxId = mContextId;
                psysLink.srcTermId = link->srcTerminalId;
                psysLink.dstNodeCtxId = mContextId;
                psysLink.dstTermId = link->destTerminalId;
                break;
            case LinkType::Node2Sink:
                psysLink.srcNodeCtxId = mContextId;
                psysLink.srcTermId = link->srcTerminalId;
                psysLink.dstNodeCtxId = 0xFF;
                psysLink.dstTermId = 0xFF;
                break;
            default:
                LOGW("unsupported type %d", link->type);
                break;
        }

        psysLink.streamingMode = link->linkConfiguration->streamingMode;
        psysLink.delayedLink = link->frameDelay;

        if (link->type == LinkType::Node2Self) {
            const int ret = allocateNode2SelfBuffers(psysLink, link->linkConfiguration->bufferSize);
            CheckAndLogError(ret != OK, NO_MEMORY, "Failed to alloc node2self buffer");
        }

        if ((psysLink.srcNodeCtxId == mContextId) &&
            (psysLink.streamingMode == LINK_STREAMING_MODE_BCLM) &&
            (mResourceId == NODE_RESOURCE_ID_LBFF)) {
            mLinkStreamMode = static_cast<uint8_t>(LINK_STREAMING_MODE_BCLM);
        }

        mTerminalLink.push_back(psysLink);
    }

    return OK;
}

int CBStage::allocMetadataBuffer(const GraphLink** links, uint8_t numOfLink,
                                 std::unordered_map<uint8_t, TerminalConfig>& terminalConfig) {
    for (uint8_t i = 0U; i < numOfLink; i++) {
        const GraphLink* link = links[i];
        CheckAndLogError(link == nullptr, BAD_VALUE, "link is nullptr");
        if (!link->isActive) {
            continue;
        }
        if (link->type == LinkType::Node2Self) {
            continue;
        }

        if (link->srcNode != nullptr) {
            if (link->srcNode->contextId != mOuterNodeCtxId) {
                continue;
            }
        } else {
            if ((link->destNode != nullptr) && (link->destNode->contextId != mOuterNodeCtxId)) {
                continue;
            }
        }

        const uint8_t terminalId = link->srcNode != nullptr ? link->srcTerminalId
                                                            : link->destTerminalId;
        const uint32_t size = ALIGN_64(link->linkConfiguration->bufferSize);
        if (CBLayoutUtils::isMetaDataTerminal(mResourceId, terminalId)) {
            for (auto& bufmap : mTerminalBufferMaps) {
                auto& terminalBufMap = bufmap.second.mMetadataBufferMap;
                TerminalBuffer terminalBuf;
                CLEAR(terminalBuf);

                terminalBuf.userPtr =
                    mPacAdapt->allocateBuffer(mStreamId, mContextId, terminalId, size);
                CheckAndLogError(terminalBuf.userPtr == nullptr, NO_MEMORY,
                                 "Failed to alloc metadata buffer");
                terminalBuf.size = size;
                terminalBuf.flags |= IPU_BUFFER_FLAG_USERPTR | IPU_BUFFER_FLAG_NO_FLUSH;

                terminalBufMap[terminalId] = terminalBuf;

                int ret = mPSysDevice->registerBuffer(&terminalBuf);
                CheckAndLogError(ret != OK, ret, "Failed to register metadata ret %d", ret);
                mUserToTerminalBuffer[terminalBuf.userPtr] = terminalBuf;
            }

            terminalConfig[terminalId].payloadSize = size;
        }
    }

    return OK;
}

int CBStage::registerMetadataBuffer(aic::IaAicBuffer** iaAicBuf, PacTerminalBufMap& termBufMap) {
    for (auto bufmap : mTerminalBufferMaps) {
        cca::cca_aic_terminal_config termCfg;
        CLEAR(termCfg);

        termCfg.cb_num = 1;
        cca::cca_cb_termal_buf* bufs = &termCfg.cb_terminal_buf[0];

        const auto& terminalBufMap = bufmap.second.mMetadataBufferMap;
        for (auto buf : terminalBufMap) {
            if (CBLayoutUtils::isMetaDataTerminal(mResourceId, buf.first)) {
                const uint32_t index = bufs->num_terminal;
                bufs->terminal_buf[index].terminal_index = buf.first;
                bufs->terminal_buf[index].payload = *iaAicBuf;
                *iaAicBuf += 1;
                bufs->terminal_buf[index].buf_size = buf.second.size;
                bufs->num_terminal++;
                bufs->group_id = mContextId;
                aic::IaAicBuffer* payload = bufs->terminal_buf[index].payload;
                payload->size = buf.second.size;
                payload->id = CBLayoutUtils::getTerminalPacBufferType(mResourceId, buf.first);
                payload->payloadPtr = buf.second.userPtr;

                PacTerminalBuf termBuf;
                termBuf.size = payload->size;
                termBuf.payloadPtr = payload->payloadPtr;
                termBufMap[buf.first] = termBuf;
            }
        }
        if (bufs->num_terminal > 0) {
            int ret = mPacAdapt->registerBuffer(mStreamId, termCfg);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to register metadata %d", ret);

            mHasStatsTerminal = true;
        }
    }

    return OK;
}

int CBStage::getKernelOffsetFromTerminalDesc(
    cca::cca_cb_kernel_offset& offsets, uint32_t** offsetPtr,
    std::unordered_map<uint8_t, TerminalConfig>& terminalConfig) {
    for (uint32_t i = 0U; i < mTerminalDescCount; i++) {
        const TerminalDescriptor& terminalDesc = sTerminalDesc[i];
        if ((terminalDesc.TerminalBufferType != TERMINAL_BUFFER_TYPE_METADATA) ||
            ((terminalDesc.TerminalType != TERMINAL_TYPE_CONNECT) &&
             (terminalDesc.TerminalDirection != TERMINAL_DIR_IN))) {
            continue;
        }

        if (terminalDesc.PacBufferType == PAC_BUFFER_TYPE_SPATIAL_IN) {
            if (terminalConfig.find(terminalDesc.TerminalId) == terminalConfig.end()) {
                LOG1("Skip register kernel offset on terminal %u", terminalDesc.TerminalId);
                continue;
            }

            const uint32_t idx = offsets.num_kernels++;
            if (strcmp(terminalDesc.TerminalName, "TERMINAL_CONNECT_LSC_INPUT") == 0) {
                offsets.kernels_offset[idx].uuid = static_cast<int32_t>(ia_pal_uuid_isp_lsc_1_2);
            } else if (strcmp(terminalDesc.TerminalName, "TERMINAL_CONNECT_GMV_INPUT") == 0) {
#ifdef IPU_SYSVER_ipu8
                offsets.kernels_offset[idx].uuid =
                    static_cast<int32_t>(ia_pal_uuid_isp_gmv_statistics_1_1);
#else
                offsets.kernels_offset[idx].uuid =
                    static_cast<int32_t>(ia_pal_uuid_isp_gmv_statistics_1_0);
#endif
            } else {
                offsets.kernels_offset[idx].uuid = terminalDesc.TerminalLinkedKernel;
            }

            offsets.kernels_offset[idx].fragment = 0;
            offsets.kernels_offset[idx].terminal_index =
                static_cast<uint32_t>(terminalDesc.TerminalId);
            offsets.kernels_offset[idx].terminal_type = static_cast<aic::IaAicBufferTypes>(
                CBLayoutUtils::getTerminalPacBufferType(mResourceId, terminalDesc.TerminalId));
            offsets.kernels_offset[idx].num_offsets = 1;
            offsets.kernels_offset[idx].offsets = *offsetPtr;
            *offsetPtr += 1;
            offsets.kernels_offset[idx].offsets[0] = 0;
            offsets.kernels_offset[idx].sizes = *offsetPtr;
            *offsetPtr += 1;
            offsets.kernels_offset[idx].sizes[0] =
                terminalConfig[terminalDesc.TerminalId].payloadSize;

            LOG1("%s, terminalId %u, uuid %d, offset %u, sizes %u", __func__,
                 terminalDesc.TerminalId, offsets.kernels_offset[idx].uuid,
                 offsets.kernels_offset[idx].offsets[0], offsets.kernels_offset[idx].sizes[0]);
        }
    }

    return OK;
}

bool CBStage::kernelExist(const StaticGraphNodeKernels& kernelGroup, uint32_t kernelUuid) {
    for (uint32_t i = 0U; i < kernelGroup.kernelCount; i++) {
        if (kernelGroup.kernelList[i].run_kernel.kernel_uuid == kernelUuid) {
            return true;
        }
    }

    return false;
}

int CBStage::getKernelOffsetFromPayloadDesc(const StaticGraphNodeKernels& kernelGroup,
                                            cca::cca_cb_kernel_offset& offsets) {
    uint32_t* kernelOffsets = mKernelOffsetBuf + mTerminalDescCount;
    uint32_t* sizes = mKernelOffsetBuf + mTerminalDescCount + kMaxSectionCount;

    std::multimap<std::pair<uint8_t, uint32_t>, std::pair<uint32_t, uint32_t>> offsetAndSizeMap;
    std::set<std::pair<uint8_t, uint32_t>> keySet;

    for (uint32_t terminalIdx = 0U; terminalIdx < mPayloadDescCount; terminalIdx++) {
        const payload_descriptor_t* payloadDesc = sPayloadDesc[terminalIdx];
        if (payloadDesc == nullptr) {
            continue;
        }

        for (uint32_t sectionIdx = 0U; sectionIdx < payloadDesc->number_of_sections; sectionIdx++) {
            const cb_payload_descriptor_t& section = payloadDesc->sections[sectionIdx];
            const int kernelUuid = CBLayoutUtils::cbDeviceId2Uuid(mResourceId, section.device_id);
            LOG1("%s, terminalId %u, uuid %d, section.device_id %d, sectionIdx %d",
                 __func__, terminalIdx, kernelUuid, section.device_id, sectionIdx);
            if (!kernelExist(kernelGroup, kernelUuid)) {
                continue;
            }

            std::pair<uint8_t, uint32_t> key = std::make_pair(terminalIdx, section.device_id);
            std::pair<uint32_t, uint32_t> value = std::make_pair(section.offset_in_payload,
                                                                 section.payload_size);
            offsetAndSizeMap.insert(std::make_pair(key, value));
            keySet.insert(key);
        }
    }

    for (auto iter = keySet.begin(); iter != keySet.end(); iter++) {
        const uint8_t terminalIdx = iter->first;
        const uint32_t deviceId = iter->second;
        const uint32_t idx = offsets.num_kernels++;
        const int kernelUuid = CBLayoutUtils::cbDeviceId2Uuid(mResourceId, deviceId);

        offsets.kernels_offset[idx].uuid = kernelUuid;
        offsets.kernels_offset[idx].fragment = 0;
        offsets.kernels_offset[idx].terminal_index = terminalIdx;
        offsets.kernels_offset[idx].terminal_type = static_cast<aic::IaAicBufferTypes>(
            CBLayoutUtils::getTerminalPacBufferType(mResourceId, terminalIdx));
        offsets.kernels_offset[idx].offsets = kernelOffsets;
        offsets.kernels_offset[idx].sizes = sizes;

        auto range = offsetAndSizeMap.equal_range(*iter);
        for (auto mapIter = range.first; mapIter != range.second; mapIter++) {
            *kernelOffsets = mapIter->second.first;
            *sizes = mapIter->second.second;

            LOG1("%s, terminalId %u, uuid %d, offset %u, sizes %u", __func__,
                 terminalIdx, kernelUuid, *kernelOffsets, *sizes);

            kernelOffsets++;
            sizes++;

            offsets.kernels_offset[idx].num_offsets++;
        }
    }

    return OK;
}

int CBStage::pacConfig(const StaticGraphNodeKernels& kernelGroup, aic::IaAicBuffer** iaAicPtr,
                       std::unordered_map<uint8_t, TerminalConfig>& terminalConfig,
                       PacTerminalBufMap& termBufMap) {
    uint32_t* offsetPtr = mKernelOffsetBuf;

    cca::cca_aic_config aicConfig{};

    aicConfig.cb_num = 1;
    aicConfig.cb_config[0].group_id = static_cast<int32_t>(mContextId);
    aicConfig.cb_config[0].fragment_count = 0;  // TODO: calculate fragment later
    aicConfig.cb_config[0].kernel_group = const_cast<aic::ImagingKernelGroup*>(&kernelGroup);

    cca::cca_cb_kernel_offset offsets;
    CLEAR(offsets);

    offsets.group_id = static_cast<int32_t>(mContextId);
    offsets.num_kernels = 0;

    cca::cca_aic_kernel_offset offset;
    offset.cb_num = 1;

    cca::cca_aic_terminal_config pacConfig;
    pacConfig.cb_num = 1;
    pacConfig.cb_terminal_buf[0].group_id = mContextId;
    pacConfig.cb_terminal_buf[0].num_terminal = 0;
    for (uint32_t i = 0U; i < mTerminalDescCount; i++) {
        const TerminalDescriptor& terminalDesc = sTerminalDesc[i];
        if (terminalDesc.TerminalType != TERMINAL_TYPE_LOAD) {
            continue;
        }

        const uint32_t idx = pacConfig.cb_terminal_buf[0].num_terminal;
        pacConfig.cb_terminal_buf[0].terminal_buf[idx].terminal_index = terminalDesc.TerminalId;
        pacConfig.cb_terminal_buf[0].terminal_buf[idx].payload = *iaAicPtr;
        *iaAicPtr += 1;
        pacConfig.cb_terminal_buf[0].num_terminal++;
    }

    (void)getKernelOffsetFromPayloadDesc(kernelGroup, offsets);

    (void)getKernelOffsetFromTerminalDesc(offsets, &offsetPtr, terminalConfig);

    offset.cb_kernel_offset[0] = offsets;
    int ret = mPacAdapt->pacConfig(mStreamId, aicConfig, offset, mKernelOffsetBuf, &pacConfig,
                                   CBLayoutUtils::getStatsBufToTermIds());
    CheckAndLogError(ret != OK, ret, "Failed to config PAC");

    ret = allocPayloadBuffer(pacConfig, terminalConfig);
    CheckAndLogError(ret != OK, ret, "Failed to alloc payload buffer %d", ret);

    ret = registerPayloadBuffer(iaAicPtr, termBufMap);
    CheckAndLogError(ret != OK, ret, "Failed to register buffers %d", ret);

    return OK;
}

int CBStage::allocPayloadBuffer(const cca::cca_aic_terminal_config& pacConfig,
                                std::unordered_map<uint8_t, TerminalConfig>& terminalConfig) {
    for (uint32_t i = 0U; i < pacConfig.cb_terminal_buf[0].num_terminal; i++) {
        if (pacConfig.cb_terminal_buf[0].terminal_buf[i].buf_size == 0U) {
            continue;
        }

        uint32_t size = ALIGN_64(pacConfig.cb_terminal_buf[0].terminal_buf[i].buf_size);
        const uint8_t terminalId = pacConfig.cb_terminal_buf[0].terminal_buf[i].terminal_index;

        bool inplaceBufAllocated = false;
        for (auto& bufmap : mTerminalBufferMaps) {
            if (isInPlaceTerminal(mResourceId, terminalId)) {
                if (!inplaceBufAllocated) {
                    inplaceBufAllocated = true;
                } else {
                    // inplace terminal allocates buffer only once
                    continue;
                }
            }

            auto& terminalBufMap = bufmap.second.mPayloadBufferMap;
            TerminalBuffer terminalBuf;
            CLEAR(terminalBuf);

            terminalBuf.userPtr =
                mPacAdapt->allocateBuffer(mStreamId, mContextId, terminalId, size);
            CheckAndLogError(terminalBuf.userPtr == nullptr, NO_MEMORY,
                             "Failed to alloc stats buffer");
            terminalBuf.size = size;
            terminalBuf.flags |= IPU_BUFFER_FLAG_USERPTR | IPU_BUFFER_FLAG_NO_FLUSH;

            terminalBufMap[terminalId] = terminalBuf;

            int ret = mPSysDevice->registerBuffer(&terminalBuf);
            CheckAndLogError(ret != OK, ret, "Failed to register payload ret %d", ret);
            mUserToTerminalBuffer[terminalBuf.userPtr] = terminalBuf;
        }

        terminalConfig[terminalId].payloadSize = size;
    }

    return OK;
}

bool CBStage::isInPlaceTerminal(uint8_t resourceId, uint8_t terminalId) {
    if ((PAC_BUFFER_TYPE_SR_FRAME_IN ==
         CBLayoutUtils::getTerminalPacBufferType(resourceId, terminalId)) ||
        (PAC_BUFFER_TYPE_SR_FRAG_SEQUENCER ==
         CBLayoutUtils::getTerminalPacBufferType(resourceId, terminalId))) {
        return true;
    }

    return false;
}

int CBStage::registerPayloadBuffer(aic::IaAicBuffer** iaAicBuf, PacTerminalBufMap& termBufMap) {
    for (auto bufmap : mTerminalBufferMaps) {
        cca::cca_aic_terminal_config termCfg;
        CLEAR(termCfg);

        termCfg.cb_num = 1;
        cca::cca_cb_termal_buf* bufs = &termCfg.cb_terminal_buf[0];

        const auto& terminalBufMap = bufmap.second.mPayloadBufferMap;
        for (auto buf : terminalBufMap) {
            if (CBLayoutUtils::isMetaDataTerminal(mResourceId, buf.first)) {
                continue;
            }

            const uint32_t index = bufs->num_terminal;
            bufs->terminal_buf[index].terminal_index = buf.first;
            bufs->terminal_buf[index].payload = *iaAicBuf;
            *iaAicBuf += 1;
            bufs->terminal_buf[index].buf_size = buf.second.size;
            bufs->num_terminal++;
            bufs->group_id = mContextId;
            aic::IaAicBuffer* payload = bufs->terminal_buf[index].payload;
            payload->size = buf.second.size;
            payload->id = CBLayoutUtils::getTerminalPacBufferType(mResourceId, buf.first);
            payload->payloadPtr = buf.second.userPtr;

            PacTerminalBuf termBuf;
            termBuf.size = payload->size;
            termBuf.payloadPtr = payload->payloadPtr;
            termBufMap[buf.first] = termBuf;
        }

        if (termCfg.cb_terminal_buf[0].num_terminal > 0) {
            int ret = mPacAdapt->registerBuffer(mStreamId, termCfg);
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to register payload buffer %d", ret);
        }
    }

    return OK;
}

int CBStage::addFrameTerminals(std::unordered_map<uint8_t, TerminalBuffer>* terminalBuffers,
                               const std::map<uuid, std::shared_ptr<CameraBuffer>>& buffers,
                               int64_t sequence) {
    for (auto it : buffers) {
        const uint8_t terminalId = GET_TERMINAL_ID(it.first);
        std::shared_ptr<CameraBuffer> buf = it.second;
        TerminalBuffer terminalBuf;
        CLEAR(terminalBuf);
        terminalBuf.size = buf->getBufferSize();
        if (buf->getMemory() == V4L2_MEMORY_DMABUF) {
            terminalBuf.handle = buf->getFd();
            terminalBuf.flags |= IPU_BUFFER_FLAG_DMA_HANDLE;
            if (PlatformData::unregisterExtDmaBuf(mCameraId)) {
                terminalBuf.isExtDmaBuf = true;
            }

            LOG2("%s, mStreamId %d, mContextId %u, terminalId %u, fd %lu, size %d", __func__,
                 mStreamId, mContextId, terminalId, terminalBuf.handle, terminalBuf.size);
        } else {
            terminalBuf.userPtr = buf->getBufferAddr();
            terminalBuf.flags |= IPU_BUFFER_FLAG_USERPTR;
            LOG2("%s, mStreamId %d, mContextId %u, terminalId %u, ptr %p, size %d", __func__,
                 mStreamId, mContextId, terminalId, terminalBuf.userPtr, terminalBuf.size);
        }

        bool flush = (!buf->isInternalBuffer()) ? true : false;
        if ((buf->getMemory() == V4L2_MEMORY_DMABUF) &&
            (PlatformData::removeCacheFlushOutputBuffer(mCameraId) ||
             (!buf->isNeedFlush()))) {
            flush = false;
        }
        if (!flush) {
            terminalBuf.flags |= IPU_BUFFER_FLAG_NO_FLUSH;
        }

        int ret = mPSysDevice->registerBuffer(&terminalBuf);
        CheckAndLogError(ret != OK, ret, "Failed to register outBuffers ret %d", ret);

        (*terminalBuffers)[terminalId] = terminalBuf;

        if (terminalBuf.isExtDmaBuf) {
            std::lock_guard<std::mutex> l(mDataLock);
            mSeqToTerminalBufferMaps.emplace(sequence, terminalBuf);
        }
    }

    return OK;
}

void CBStage::unregisterExtDmaBuf(int64_t sequence) {
    auto range = mSeqToTerminalBufferMaps.equal_range(sequence);
    bool found = false;

    for (auto& it = range.first; it != range.second; ++it) {
        mPSysDevice->unregisterBuffer(&it->second);
        found = true;
    }

    if (found) {
        mSeqToTerminalBufferMaps.erase(sequence);
    }
}

int CBStage::addTask(std::unordered_map<uint8_t, TerminalBuffer>* terminalBuffers,
                     const PacTerminalBufMap& bufferMap, int64_t sequence) {
    PSysTask psysTask;

    psysTask.nodeCtxId = mContextId;
    psysTask.sequence = sequence;
    psysTask.terminalBuffers = *terminalBuffers;

    for (auto buf : bufferMap) {
        if (mUserToTerminalBuffer.find(buf.second.payloadPtr) == mUserToTerminalBuffer.end()) {
            LOGE("Unknown buffer %p from PAC", buf.second.payloadPtr);
            return UNKNOWN_ERROR;
        }

        psysTask.terminalBuffers[buf.first] = mUserToTerminalBuffer[buf.second.payloadPtr];
    }

    if (mNode2SelfBuffers.size() > 0) {
        const uint8_t referInIdx = mNode2SelfBufIndex;
        const uint8_t referOutIdx = (referInIdx + 1) % kMaxNode2SelfBufArray;
        mNode2SelfBufIndex = referOutIdx;
        for (auto it : mNode2SelfBuffers) {
            TerminalBuffer& outBuf = it.second[referOutIdx];
            const TerminalBuffer& inBuf = it.second[referInIdx];
            psysTask.terminalBuffers[it.first] = mUserToTerminalBuffer[outBuf.userPtr];

            for (auto link : mNode2SelfLinks[it.first]) {
                if (link.delayedLink > 0) {
                    // Use output of the last frame as input
                    psysTask.terminalBuffers[link.dstTermId] = inBuf;
                } else {
                    // Use output of the current frame as input (buffer chasing)
                    psysTask.terminalBuffers[link.dstTermId] = mUserToTerminalBuffer[outBuf.userPtr];
                }
            }
        }
    }

    dumpTerminalData(bufferMap, sequence);

    int ret = mPSysDevice->addTask(psysTask);
    CheckAndLogError(ret != OK, ret, "Failed to add task ret %d", ret);

    return OK;
}

void CBStage::dumpTerminalData(const PacTerminalBufMap& bufferMap, int64_t sequence) {
    if (!CameraDump::isDumpTypeEnable(DUMP_PSYS_CB)) {
        return;
    }

    for (auto buf : bufferMap) {
        PacBufferType pacType = CBLayoutUtils::getTerminalPacBufferType(mResourceId, buf.first);
        if (pacType == PAC_BUFFER_TYPE_SPATIAL_OUT) {
            continue;
        }

        const char* typeStr =
            (pacType == PacBufferType::PAC_BUFFER_TYPE_PARAM_IN)             ? "PARAM_IN"
            : (pacType == PacBufferType::PAC_BUFFER_TYPE_PROGRAM)            ? "PROGRAM"
            : (pacType == PacBufferType::PAC_BUFFER_TYPE_SPATIAL_IN)         ? "SPATIAL_IN"
            : (pacType == PacBufferType::PAC_BUFFER_TYPE_SYS_FRAG_SEQUENCER) ? "SYS_FRAG_SEQUENCER"
            : (pacType == PacBufferType::PAC_BUFFER_TYPE_SR_FRAME_IN)        ? "SR_FRAME_IN"
            : (pacType == PacBufferType::PAC_BUFFER_TYPE_SR_FRAG_SEQUENCER)  ? "SR_FRAG_SEQUENCER"
            : "UNKNOWN";

        char fileName[MAX_NAME_LEN] = {'\0'};
        snprintf(fileName, (MAX_NAME_LEN - 1), "%s/cam%d_cb%u_resource%u_termId%u_%s_%ld.bin",
                 CameraDump::getDumpPath(), mCameraId, mContextId, mResourceId,
                 buf.first, typeStr, sequence);

        LOGI("<id%d:seq%ld> filename %s, ctx %u, resource %d, ptr %p, size %zu, pac %d, termId %u",
             mCameraId, sequence, fileName, mContextId, mResourceId, buf.second.payloadPtr,
             buf.second.size, pacType, buf.first);

        CameraDump::writeData(buf.second.payloadPtr, buf.second.size, fileName);
    }
}

}  // namespace icamera
