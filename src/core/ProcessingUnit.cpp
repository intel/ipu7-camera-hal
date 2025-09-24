/*
 * Copyright (C) 2017-2025 Intel Corporation.
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

#define LOG_TAG ProcessingUnit

#include "ProcessingUnit.h"

#include <set>
#include <utility>

#include "3a/AiqResultStorage.h"
#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "CameraContext.h"
#include "src/core/processingUnit/IPipeManagerFactory.h"
#include "StageDescriptor.h"

#define EXTREME_STRENGTH_LEVEL4 (-120)
#define EXTREME_STRENGTH_LEVEL3 (-60)
#define EXTREME_STRENGTH_LEVEL2 (0)
#define EXTREME_STRENGTH_LEVEL1 (20)

using std::shared_ptr;
using std::unique_ptr;

namespace icamera {

//#define USE_SCHEDULER

ProcessingUnit::ProcessingUnit(int cameraId, std::shared_ptr<CameraScheduler> scheduler)
        : mCameraId(cameraId),
          mTuningMode(TUNING_MODE_MAX),
          mRawPort(INVALID_PORT),
          mStatus(PIPELINE_UNCREATED),
          mScheduler(scheduler),
          mLastStillTnrSequence(-1) {
    mProcessThread = new ProcessThread(this);

    CLEAR(mTnrTriggerInfo);
}

ProcessingUnit::~ProcessingUnit() {
    mProcessThread->wait();
    delete mProcessThread;
}

int ProcessingUnit::configure(const std::map<uuid, stream_t>& inputInfo,
                             const std::map<uuid, stream_t>& outputInfo,
                             const ConfigMode configModes) {
    CheckAndLogError(mStatus == PIPELINE_CREATED, -1,
                     "@%s mStatus is in wrong status: PIPELINE_CREATED", __func__);

    setFrameInfo(inputInfo, outputInfo);

    mConfigMode = configModes;
    mOpaqueRawPorts.clear();
    mRawPort = INVALID_PORT;

    std::map<uuid, stream_t> outputFrameInfo;
    stream_t stillStream = {}, videoStream = {};
    for (auto& outFrameInfo : BufferQueue::mOutputFrameInfo) {
        // Check if it's required to output raw image from ISYS
        if (outFrameInfo.second.format == V4L2_PIX_FMT_SGRBG12) {
            mRawPort = outFrameInfo.first;
        } else if (outFrameInfo.second.usage == CAMERA_STREAM_OPAQUE_RAW) {
            mOpaqueRawPorts.insert(outFrameInfo.first);
        } else if ((outFrameInfo.second.streamType == CAMERA_STREAM_INPUT) &&
                   ((outFrameInfo.second.usage == CAMERA_STREAM_PREVIEW) ||
                    (outFrameInfo.second.usage == CAMERA_STREAM_VIDEO_CAPTURE))) {
            mYuvInputInfo[outFrameInfo.first] = outFrameInfo.second;
        } else if (outFrameInfo.second.usage == CAMERA_STREAM_STILL_CAPTURE) {
            stillStream = outFrameInfo.second;
            outputFrameInfo[outFrameInfo.first] = outFrameInfo.second;
        } else {
            videoStream = outFrameInfo.second;
            outputFrameInfo[outFrameInfo.first] = outFrameInfo.second;
        }
    }

    TuningConfig tuningConfig;
    int ret = PlatformData::getTuningConfigByConfigMode(mCameraId, mConfigMode, tuningConfig);
    CheckAndLogError(ret != OK, ret, "%s: can't get config for mode %d", __func__, mConfigMode);

    LOG1("%s, Create PipeManager for ConfigMode %d", __func__, mConfigMode);
    mPipeManager = unique_ptr<IPipeManager>
                   (IPipeManagerFactory::createIPipeManager(mCameraId, this, mScheduler));

    ret = mPipeManager->configure(inputInfo, outputFrameInfo, tuningConfig.configMode,
                                  tuningConfig.tuningMode, &mYuvInputInfo);
    CheckAndLogError(ret != OK, ret, "@%s configure psys dag failed:%d", __func__, ret);

    std::shared_ptr<GraphConfig> graphConfig =
        CameraContext::getInstance(mCameraId)->getGraphConfig(mConfigMode);
    CheckAndLogError((graphConfig.get() == nullptr), UNKNOWN_ERROR,
                      "Failed to get GraphConfig in PipeManager!");

    mStreamIdToPipeId = graphConfig->getStreamIdToPipeId();

    mTuningMode = tuningConfig.tuningMode;

    (void)getTnrTriggerInfo();

    if (ret == OK) {
        mStatus = PIPELINE_CREATED;
    }

    return ret;
}

int ProcessingUnit::start() {
    PERF_CAMERA_ATRACE();
    AutoMutex l(mBufferQueueLock);

    const bool pendRaw = PlatformData::isHALZslSupported(mCameraId) || (!mOpaqueRawPorts.empty());
    const int rawBufferNum = pendRaw ? PlatformData::getMaxRawDataNum(mCameraId)
                                     : PlatformData::getPreferredBufQSize(mCameraId);

    /* Should use MIN_BUFFER_COUNT to optimize frame latency when PSYS processing
     * time is slower than ISYS
     */
    bool needProducerBuffer = PlatformData::isIsysEnabled(mCameraId);

    // FILE_SOURCE_S
    needProducerBuffer = needProducerBuffer || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E

    int ret = OK;
    if (needProducerBuffer) {
        ret = BufferQueue::allocProducerBuffers(mCameraId, rawBufferNum);
        CheckAndLogError(ret != OK, NO_MEMORY, "Allocating producer buffer failed:%d", ret);
    }

    IProcessingUnit::mThreadRunning = true;
    BufferQueue::setThreadWaiting(true);
    mProcessThread->start();

    ret = mPipeManager->start();
    CheckAndLogError(ret != OK, ret, "Failed to start pipemanager");

    auto cameraContext = CameraContext::getInstance(mCameraId);
    auto dataContext = cameraContext->getDataContextBySeq(0);
    (void)setParameters(dataContext);

    return OK;
}

void ProcessingUnit::stop() {
    PERF_CAMERA_ATRACE();
    mPipeManager->stop();

    IProcessingUnit::mThreadRunning = false;
    BufferQueue::setThreadWaiting(false);
    mProcessThread->exit();

    {
        AutoMutex l(mBufferQueueLock);
        mFrameAvailableSignal.notify_one();
    }

    {
        AutoMutex lMeta(mMetaQueueLock);
        mMetaAvailableSignal.notify_one();
    }

    mProcessThread->wait();

    mRawBufferMap.clear();
    // Thread is not running. It is safe to clear the Queue
    BufferQueue::clearBufferQueues();
}

status_t ProcessingUnit::getTnrTriggerInfo() {
    IntelCca* intelCca = IntelCca::getInstance(mCameraId, mTuningMode);
    CheckAndLogError(intelCca == nullptr, UNKNOWN_ERROR, "cca is nullptr, mode:%d", mTuningMode);
    cca::cca_cmc cmc;
    const ia_err ret = intelCca->getCMC(&cmc);
    CheckAndLogError(ret != ia_err_none, BAD_VALUE, "Get cmc data failed");
    mTnrTriggerInfo = cmc.tnr7us_trigger_info;
    LOG2("%s tnr trigger info: gain num: %d threshold: %f", __func__, mTnrTriggerInfo.num_gains,
         mTnrTriggerInfo.tnr7us_threshold_gain);
    for (unsigned int i = 0U; i < mTnrTriggerInfo.num_gains; i++) {
        LOG2("  %u: gain %f, frame count: %d", i, mTnrTriggerInfo.trigger_infos[i].gain,
             mTnrTriggerInfo.trigger_infos[i].frame_count);
    }
    return OK;
}

int ProcessingUnit::setParameters(const DataContext* dataContext) {
    // Process image enhancement related settings.
    const camera_image_enhancement_t& enhancement = dataContext->mIspParams.enhancement;
    AutoWMutex wl(mIspSettingsLock);
    mIspSettings.manualSettings.manualSharpness = static_cast<char>(enhancement.sharpness);
    mIspSettings.manualSettings.manualBrightness = static_cast<char>(enhancement.brightness);
    mIspSettings.manualSettings.manualContrast = static_cast<char>(enhancement.contrast);
    mIspSettings.manualSettings.manualHue = static_cast<char>(enhancement.hue);
    mIspSettings.manualSettings.manualSaturation = static_cast<char>(enhancement.saturation);
    mIspSettings.eeSetting.strength = enhancement.sharpness;

    mIspSettings.eeSetting.feature_level = ia_isp_feature_level_high;
    const camera_edge_mode_t manualEdgeMode = dataContext->mIspParams.edgeMode;
    LOG2("%s: manual edge mode set: %d", __func__, manualEdgeMode);
    switch (manualEdgeMode) {
        case EDGE_MODE_LEVEL_4:
            mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL4);
            break;
        case EDGE_MODE_LEVEL_3:
            mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL3);
            break;
        case EDGE_MODE_LEVEL_2:
            mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
            break;
        case EDGE_MODE_LEVEL_1:
            mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL1);
            break;
        default:
            mIspSettings.eeSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
            break;
    }

    LOG2("%s: ISP EE setting, level: %d, strength: %d", __func__,
         static_cast<int>(mIspSettings.eeSetting.feature_level),
         static_cast<int>(mIspSettings.eeSetting.strength));

    mIspSettings.nrSetting.feature_level = ia_isp_feature_level_high;
    mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
    const camera_nr_mode_t manualNrMode = dataContext->mIspParams.nrMode;
    LOG2("%s: manual NR mode set: %d", __func__, manualNrMode);
    switch (manualNrMode) {
        case NR_MODE_LEVEL_4:
            mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL4);
            break;
        case NR_MODE_LEVEL_3:
            mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL3);
            break;
        case NR_MODE_LEVEL_2:
            mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
            break;
        case NR_MODE_LEVEL_1:
            mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL1);
            break;
        default:
            mIspSettings.nrSetting.strength = static_cast<char>(EXTREME_STRENGTH_LEVEL2);
    }

    if (dataContext->mIspParams.nrLevel.set) {
        mIspSettings.nrSetting.strength
            = static_cast<char>(dataContext->mIspParams.nrLevel.nrLevel.overall);
    }

    LOG2("%s: ISP NR setting, level: %d, strength: %d", __func__,
         static_cast<int>(mIspSettings.nrSetting.feature_level),
         static_cast<int>(mIspSettings.nrSetting.strength));

    mIspSettings.videoStabilization
        = (dataContext->mAiqParams.videoStabilizationMode == VIDEO_STABILIZATION_MODE_ON);
    LOG2("%s: Video stabilization enabled:%d", __func__, mIspSettings.videoStabilization);

    mIspSettings.zoom = dataContext->zoomRegion;

    return OK;
}

/**
 * Get available setting sequence from outBuf
 */
int64_t ProcessingUnit::getSettingSequence(const CameraBufferPortMap& outBuf) const {
    int64_t settingSequence = -1;
    for (auto& output : outBuf) {
        if (output.second != nullptr) {
            settingSequence = output.second->getSettingSequence();
            break;
        }
    }
    return settingSequence;
}

/**
 * Check if the input frame should be skipped
 *
 * If the corresponding mSkip of AiqResult gotten from sequence is true,
 * return true; otherwise return false.
 */
bool ProcessingUnit::needSkipOutputFrame(int64_t sequence) {
    // Check if need to skip output frame
    auto cameraContext = CameraContext::getInstance(mCameraId);
    AiqResultStorage* resultStorage = cameraContext->getAiqResultStorage();
    const AiqResult* aiqResults = resultStorage->getAiqResult(sequence);
    if ((aiqResults != nullptr) && aiqResults->mSkip) {
        LOG1("<seq:%ld>@%s", sequence, __func__);
        return true;
    }
    return false;
}

/**
 * Check if 'inBuffer' can be used for 'settingSequence' to run PSys pipe.
 *
 * If 'settingSequence' is -1, it means the output buffer doesn't require particular
 * input buffer, so it can run the pipe.
 * If 'inputSequence' larger than 'settingSequence', the pipeline needs to
 * run as well, otherwise the pipe doesn't need to run and this input buffer needs to
 * be skipped.
 */
bool ProcessingUnit::needExecutePipe(int64_t settingSequence, int64_t inputSequence) const {
    if ((settingSequence == -1) || (inputSequence >= settingSequence)) {
        return true;
    }

    return false;
}

/**
 * Check if the input buffer need to be reused
 *
 * If 'settingSequence' is -1, it means the output buffer doesn't require particular
 * input buffer, so the input buffer doesn't need to be reused.
 * If 'inputSequence' larger than 'settingSequence', means the input buffer
 * may be required by following output buffer, so it may be reused later.
 */
bool ProcessingUnit::needHoldOnInputFrame(int64_t settingSequence, int64_t inputSequence) const {
    if ((settingSequence == -1) || (inputSequence <= settingSequence)) {
        return false;
    }

    return true;
}

// ProcessingUnit ThreadLoop
int ProcessingUnit::processNewFrame() {
    LOG3("<id%d>@%s", mCameraId, __func__);
    CheckAndLogError(mBufferProducer == nullptr, INVALID_OPERATION, "No available producer");

    /* Will trigger the Scheduler in this Loop. trigger chance:
    ** 1. ISys buffer done and request output buffer ready, trigger the Scheduler after prepareTask
    ** run PAL and stage parallel
    ** 2. ISys buffer done but output is empty, always happens at sequence 1 and 2, should trigger
    ** scheduler if there is any task remain
    ** 3. Wait ISys buffer timeout, always happens in reprocessing, when no new request
    ** there will no ISys buffer done event, should trigger scheduler if there is any task remain
    */
    int ret = OK;
    CameraBufferPortMap srcBuffers, dstBuffers;
    int64_t inputSequence = -1;
    bool taskReady = true;
    {
        std::unique_lock<std::mutex> lock(mBufferQueueLock);
        if (!this->mThreadRunning) {
            return -1;  // Already stopped
          }
        // set timeout only when there are already pending tasks in the Queue
        int64_t timeout = mSequencesInflight.size() > 0 ? kQueueTimeout : 0;
        ret = BufferQueue::waitFreeBuffersInQueue(lock, srcBuffers, BufferQueue::mInputQueue,
                                                  timeout);

        if (!this->mThreadRunning) {
            return -1;  // Already stopped
        }
        if (ret == NOT_ENOUGH_DATA) {
            return OK;
        }
        if (ret == TIMED_OUT) {
            LOG1("<id%d>@%s, timeout happen, wait recovery", mCameraId, __func__);
            taskReady = false;
        }

        if (!srcBuffers.empty()) {
            const uuid defaultPort = srcBuffers.begin()->first;
            shared_ptr<CameraBuffer> mainBuf = srcBuffers[defaultPort];
            inputSequence = mainBuf->getSequence();
        }
        // check the output request
        for (auto& output : BufferQueue::mOutputQueue) {
            const uuid port = output.first;
            CameraBufQ& outputQueue = output.second;
            if (outputQueue.empty()) {
                taskReady = false;
                LOG3("<id%d>@%s, port %d, output buffer not ready", mCameraId, __func__, port);
                break;
            }
            dstBuffers[port] = outputQueue.front();
        }
    }

    if (taskReady) {
        ret = prepareTask(&srcBuffers, &dstBuffers);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to process frame", __func__);
    }

    bool execute = true;
    {
        std::lock_guard<std::mutex> lock(mBufferQueueLock);
        // Do not trigger scheduler if no pending task
        execute = !mSequencesInflight.empty();
    }

    if (execute) {
        std::string source;
        mScheduler->executeNode(source, inputSequence);
    }

    return OK;
}

int ProcessingUnit::handleYuvReprocessing(const CameraBufferPortMap* buffersMap) {
    CheckAndLogError(buffersMap->empty(), UNKNOWN_ERROR,
                     "%s, the input or output buffer is empty", __func__);

    CameraBufferPortMap srcBuffers, dstBuffers;
    std::shared_ptr<CameraBuffer> inputBuffer = nullptr;
    uint64_t timestamp = 0U;
    int64_t bufSequence = -1;

    for (const auto& item : *buffersMap) {
        if (item.first == YUV_REPROCESSING_INPUT_PORT_ID) {
            srcBuffers[item.first] = item.second;
            inputBuffer = item.second;
            timestamp = TIMEVAL2NSECS(item.second->getTimestamp());
            bufSequence = item.second->getSequence();
            LOG2("%s, YUV reprocessing input buf port id %d, usage %d, timestamp %lu, sequence %ld",
                 __func__, item.first, item.second->getStreamUsage(), timestamp, bufSequence);
        } else {
            dstBuffers[item.first] = item.second;
            if (item.second.get() != nullptr) {
                LOG2("%s, YUV output buf port id %d, usage %d, timestamp %ld, sequence %u",
                     __func__, item.first, item.second->getStreamUsage(),
                     TIMEVAL2USECS(item.second->getTimestamp()), item.second->getSequence());
            }
        }
     }

    for (auto& output: BufferQueue::mOutputQueue) {
        output.second.pop();
    }

    // handle buffer done for normal YUV output
    sendPsysRequestEvent(&dstBuffers, bufSequence, timestamp, EVENT_PSYS_REQUEST_BUF_READY);

    // Prepare the task input parameters including input and output buffers, settings etc.
    PipeTaskData taskParam;
    taskParam.mTuningMode = mTuningMode;
    taskParam.mInputBuffers = srcBuffers;
    taskParam.mOutputBuffers = dstBuffers;
    taskParam.mFakeTask = false;
    taskParam.mCallbackRgbs = false;
    taskParam.mYuvTask = true;

    if (!IProcessingUnit::mThreadRunning) {
        return -1;
    }

    mPipeManager->addTask(taskParam);

    // handle metadata event after running pal(update metadata from pal result)
    sendPsysRequestEvent(&dstBuffers, bufSequence, timestamp, EVENT_REQUEST_METADATA_READY);

    return OK;
}

void ProcessingUnit::handleRawReprocessing(CameraBufferPortMap* srcBuffers,
                                           CameraBufferPortMap* dstBuffers, bool* allBufDone,
                                           bool* hasRawOutput, bool* hasRawInput) {
    std::shared_ptr<CameraBuffer> rawOutputBuffer = nullptr;
    int64_t settingSequence = -1;
    CameraBufferPortMap videoBuf, stillBuf;
    uuid rawPort = 0U;

    for (const auto& item : *dstBuffers) {
        if (item.second != nullptr) {
            LOG2("%s, usage %d, timestamp %ld, sequence %u", __func__,
                 item.second->getStreamUsage(), TIMEVAL2USECS(item.second->getTimestamp()),
                 item.second->getSequence());
            if (item.second->getStreamUsage() == CAMERA_STREAM_OPAQUE_RAW) {
                rawOutputBuffer = item.second;
                rawPort = item.first;
            } else if (item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE) {
                stillBuf[item.first] = item.second;
            } else {
                videoBuf[item.first] = item.second;
            }
            if (item.second->getSettingSequence() >= 0) {
                settingSequence = item.second->getSettingSequence();
            }
        }
    }

    const uuid defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    const int64_t inputSequence = mainBuf->getSequence();
    uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());

    if ((rawOutputBuffer != nullptr) && TIMEVAL2USECS(rawOutputBuffer->getTimestamp()) > 0U) {
        timestamp = 0U;

        // handle Shutter first if has raw input
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_PSYS_REQUEST_BUF_READY);
        {
            AutoMutex lock(mBufferMapLock);
            // Find Raw buffer in mRawBufferMap
            if (mRawBufferMap.find(settingSequence) != mRawBufferMap.end()) {
                CameraBufferPortMap& mapBuf = mRawBufferMap[settingSequence];
                // Update source buffers
                for (const auto& bufPortMap : mapBuf) {
                    (*srcBuffers)[bufPortMap.first] = bufPortMap.second;
                }
                *hasRawInput = true;
            }
        }

        // Return opaque RAW buffer
        for (auto& it : BufferQueue::mBufferConsumerList) {
            it->onBufferAvailable(rawPort, rawOutputBuffer);
        }

        // Remove input stream from dstBuffers map
        dstBuffers->erase(rawPort);
    } else if (rawOutputBuffer != nullptr) {
        if (!needExecutePipe(settingSequence, inputSequence)) {
            LOG2("%s, inputSequence %ld is smaller than settingSequence %ld, skip sensor frame.",
                 __func__, inputSequence, settingSequence);
            return;
        }

        rawOutputBuffer->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());

        LOG2("%s, timestamp %ld, inputSequence %ld, dstBufferSize %d, addr %p", __func__, timestamp,
             inputSequence, rawOutputBuffer->getBufferSize(), rawOutputBuffer->getBufferAddr());

        // handle Shutter first if has raw output
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_PSYS_REQUEST_BUF_READY);

        // Return opaque RAW buffer
        for (auto& it : BufferQueue::mBufferConsumerList) {
            it->onBufferAvailable(rawPort, rawOutputBuffer);
        }
        *hasRawOutput = true;

        if (stillBuf.empty() && videoBuf.empty()) {
            *allBufDone = true;
        } else {
            // Remove raw stream from dstBuffers map
            dstBuffers->erase(rawPort);
        }
    }

    // handle Metadata ready event if all buffer done here(psys doesn't run)
    if (*allBufDone) {
        sendPsysRequestEvent(dstBuffers, settingSequence, timestamp, EVENT_REQUEST_METADATA_READY);
    }

    LOG2("%s, RAW input %d, RAW output %d, allBufDone %d, settingSequence %ld, inputSequence %ld",
         __func__, *hasRawInput, *hasRawOutput, *allBufDone, settingSequence, inputSequence);
}

bool ProcessingUnit::isBufferHoldForRawReprocess(int64_t sequence) {
    AutoMutex lock(mBufferMapLock);
    if (mRawBufferMap.find(sequence) == mRawBufferMap.end()) {
        return false;
    }

    return true;
}

void ProcessingUnit::saveRawBuffer(CameraBufferPortMap* srcBuffers) {
    // Save buffer into mRawBufferMap
    CameraBufferPortMap mapBuf;
    for (const auto& src : *srcBuffers) {
        mapBuf[src.first] = src.second;
    }

    const uuid defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    const int64_t inputSequence = mainBuf->getSequence();

    LOG2("<id%d:seq%ld>@%s", mCameraId, inputSequence, __func__);
    {
        AutoMutex lock(mBufferMapLock);
        mRawBufferMap[inputSequence] = mapBuf;
    }

    returnRawBuffer();
}

void ProcessingUnit::returnRawBuffer() {
    AutoMutex lock(mBufferMapLock);
    // If too many buffers are holden in mRawQueue, return back to producer
    if (mRawBufferMap.size() > (PlatformData::getMaxRawDataNum(mCameraId) -
                                PlatformData::getMaxPipelineDepth(mCameraId))) {
        auto it = mRawBufferMap.cbegin();
        {
            AutoMutex l(mBufferQueueLock);
            if (mSequencesInflight.find(it->first) != mSequencesInflight.end()) {
                // Raw buffer is in used and return it later
                return;
            }
        }

        if (mBufferProducer != nullptr) {
            const CameraBufferPortMap& bufferPortMap = it->second;
            for (auto& item : bufferPortMap) {
                 mBufferProducer->qbuf(item.first, item.second);
            }
        }
        LOG2("@%s, returned sequence %ld", __func__, it->first);
        mRawBufferMap.erase(it);
    }
}

status_t ProcessingUnit::prepareTask(CameraBufferPortMap* srcBuffers,
                                     CameraBufferPortMap* dstBuffers) {
    CheckAndLogError(srcBuffers->empty() || dstBuffers->empty(), UNKNOWN_ERROR,
                     "%s, the input or output buffer is empty", __func__);

    // Used for RAW reprocessing
    bool allBufDone = false;
    bool hasRawOutput = false;
    bool hasRawInput = false;

    // Used for HAL ZSL
    bool reprocess = false;
    CameraBufferPortMap videoBuf, stillBuf;
    int64_t zslSequence = -1;

    if (!mOpaqueRawPorts.empty()) {
        handleRawReprocessing(srcBuffers, dstBuffers, &allBufDone, &hasRawOutput, &hasRawInput);
        saveRawBuffer(srcBuffers);

        if (allBufDone) {
            sendPsysFrameDoneEvent(dstBuffers);
            // If all buffers are handled
            AutoMutex l(mBufferQueueLock);
            if (hasRawOutput) {
                for (auto& input : BufferQueue::mInputQueue) {
                    input.second.pop();
                }
            }
            for (auto& output : BufferQueue::mOutputQueue) {
                output.second.pop();
            }
            return OK;
        }
    } else if (!mYuvInputInfo.empty()) {
        auto iter = dstBuffers->find(YUV_REPROCESSING_INPUT_PORT_ID);
        if ((iter != dstBuffers->end()) && (iter->second.get() != nullptr)) {
            return handleYuvReprocessing(dstBuffers);
        }
    } else if (PlatformData::isHALZslSupported(mCameraId)) {
        extractZslInfo(dstBuffers, reprocess, videoBuf, stillBuf, zslSequence);
        saveRawBuffer(srcBuffers);
    }

    const uuid defaultPort = srcBuffers->begin()->first;
    shared_ptr<CameraBuffer> mainBuf = (*srcBuffers)[defaultPort];
    const int64_t inputSequence = mainBuf->getSequence();
    TRACE_LOG_POINT("ProcessingUnit", "input output buffer ready", MAKE_COLOR(inputSequence),
                    inputSequence);
    const uint64_t timestamp = TIMEVAL2NSECS(mainBuf->getTimestamp());
    LOG2("%s: input buffer sequence %ld timestamp %ld", __func__, inputSequence, timestamp);

    // Output raw image
    if (mRawPort != INVALID_PORT) {
        shared_ptr<CameraBuffer> dstBuf = nullptr;

        // Get output buffer and remove it from dstBuffers
        for (auto& buffer : *dstBuffers) {
            if (buffer.first == mRawPort) {
                dstBuf = buffer.second;
                CheckAndLogError(dstBuf == nullptr, UNKNOWN_ERROR, "%s, dstBuf for output raw is null",
                                 __func__);
                dstBuf->updateV4l2Buffer(*mainBuf->getV4L2Buffer().Get());
                dstBuffers->erase(mRawPort);
                break;
            }
        }
        outputRawImage(mainBuf, dstBuf);
    }

    const int64_t settingSequence = getSettingSequence(*dstBuffers);
    const bool needRunPipe = needExecutePipe(settingSequence, inputSequence);
    const bool holdOnInput = needHoldOnInputFrame(settingSequence, inputSequence);
    LOG2("%s: dst sequence = %ld, src sequence = %ld, needRunPipe = %d, needReuseInput = %d",
         __func__, settingSequence, inputSequence, needRunPipe, holdOnInput);

    {
        AutoMutex l(mBufferQueueLock);
        if (needRunPipe && (!needSkipOutputFrame(inputSequence))) {
            for (auto& output : BufferQueue::mOutputQueue) {
                output.second.pop();
            }
        }

        // If input buffer will be used later, don't pop it from the queue.
        if ((!holdOnInput) && (!hasRawInput) && (!reprocess)) {
            for (auto& input : BufferQueue::mInputQueue) {
                input.second.pop();
            }
        }
    }

    if (needRunPipe) {
        // Raw output already has been returned back, and don't need to handle again.
        if (!hasRawOutput) {
            // handle buffer done for normal YUV output
            sendPsysRequestEvent(dstBuffers, settingSequence, timestamp,
                                 EVENT_PSYS_REQUEST_BUF_READY);
        }

        auto cameraContext = CameraContext::getInstance(mCameraId);
        auto dataContext = cameraContext->getDataContextBySeq(inputSequence);

        bool callbackRgbs = false;
        AiqResultStorage* resultStorage = cameraContext->getAiqResultStorage();
        AiqResult* aiqResult = const_cast<AiqResult*>(resultStorage->getAiqResult(inputSequence));
        if ((aiqResult != nullptr) && dataContext->mAiqParams.callbackRgbs) {
            callbackRgbs = true;
        }

        if (aiqResult != nullptr) {
            if (mTnrTriggerInfo.num_gains > 0U) {
                const int64_t sequence = zslSequence >= 0U ? zslSequence : inputSequence;
                handleExtraTasksForTnr(sequence, dstBuffers, aiqResult);
            }

            bool allBufDone = false;
            if (reprocess) {
                handleZslReprocessing(zslSequence, videoBuf, stillBuf, allBufDone, dstBuffers);
            }
            if (allBufDone) {
                if (!callbackRgbs) {
                    // handle metadata event after running pal(update metadata from pal result)
                    sendPsysRequestEvent(dstBuffers, settingSequence, timestamp,
                                         EVENT_REQUEST_METADATA_READY);
                }
                return OK;
            }
        }

        dispatchTask(*srcBuffers, *dstBuffers, false, callbackRgbs);

        if (!callbackRgbs) {
            // handle metadata event after running pal(update metadata from pal result)
            sendPsysRequestEvent(dstBuffers, settingSequence, timestamp,
                                 EVENT_REQUEST_METADATA_READY);
        }
    } else if ((!holdOnInput) && (!isBufferHoldForRawReprocess(inputSequence))) {
        for (const auto& src : *srcBuffers) {
            mBufferProducer->qbuf(src.first, src.second);
        }
    }

    return OK;
}

void ProcessingUnit::extractZslInfo(const CameraBufferPortMap* dstBuffers, bool& reprocess,
                                    CameraBufferPortMap& videoBuf, CameraBufferPortMap& stillBuf,
                                    int64_t& zslSequence) {
    for (const auto& item : *dstBuffers) {
        if ((item.second != nullptr) &&
            (item.second->getStreamUsage() == CAMERA_STREAM_STILL_CAPTURE)) {
            stillBuf[item.first] = item.second;
        } else if ((item.second != nullptr) &&
            (item.second->getStreamUsage() != CAMERA_STREAM_OPAQUE_RAW)) {
            videoBuf[item.first] = item.second;
        }
    }

    if ((!stillBuf.empty()) && (TIMEVAL2USECS(stillBuf.begin()->second->getTimestamp()) > 0U)) {
        zslSequence = stillBuf.begin()->second->getSettingSequence();
        reprocess = true;
        LOG2("Handle HAL based ZSL, change target %ld", zslSequence);
    }
}

void ProcessingUnit::handleZslReprocessing(int64_t sequence, const CameraBufferPortMap& videoBuf,
                                           CameraBufferPortMap stillBuf, bool& allBufDone,
                                           CameraBufferPortMap* dstBuffers) {
    // Set reprocess task for still
    CameraBufferPortMap srcBuf;
    {
        AutoMutex lock(mBufferMapLock);
        if (mRawBufferMap.find(sequence) != mRawBufferMap.end()) {
            for (const auto& item : mRawBufferMap[sequence]) {
                srcBuf[item.first] = item.second;
            }
        }
        if (!srcBuf.empty()) {
            dispatchTask(srcBuf, stillBuf);

            if (videoBuf.empty()) {
                allBufDone = true;
            } else {
                dstBuffers->erase(stillBuf.begin()->first);
            }
        }
    }

    LOG2("%s, allBufDone %d, sequence %ld", __func__, allBufDone, sequence);
}

void ProcessingUnit::handleExtraTasksForTnr(int64_t sequence, CameraBufferPortMap* dstBuffers,
                                            const AiqResult* aiqResult) {
    bool hasStill = false;
    CameraBufferPortMap fakeTaskBuffers = *dstBuffers;
    // Extra tasks only for ipu still pipe
    for (const auto& item : *dstBuffers) {
        if (item.second == nullptr) {
            continue;
        }

        const int32_t streamId = item.second->getStreamId();
        const auto it = mStreamIdToPipeId.find(streamId);
        if ((it != mStreamIdToPipeId.end()) &&
            (mStreamIdToPipeId[streamId] == STILL_STREAM_ID)) {
            hasStill = true;
        } else {
            fakeTaskBuffers.erase(item.first);
        }
    }
    if (!hasStill) {
        return;
    }

    LOG2("<seq%ld>: still tnr task start", sequence);
    int64_t startSequence = sequence - (getTnrFrameCount(aiqResult) - 1);
    StageControl ctl;
    ctl.stillTnrReferIn = true;
    PipeControl control;
    control[STILL_STREAM_ID] = ctl;

    if (startSequence < 0) {
        startSequence = 0;
    }
    if (startSequence > mLastStillTnrSequence) {
        LOG2("<seq%ld>: still tnr task start from seq %ld, run %ld frames to generate data",
             sequence, startSequence, (sequence + 1 - startSequence));
        while (startSequence < sequence) {
            mPipeManager->setControl(startSequence, control);

            CameraBufferPortMap srcBuf;
            {
                AutoMutex lock(mBufferMapLock);
                if (mRawBufferMap.find(startSequence) != mRawBufferMap.end()) {
                    for (const auto& item : mRawBufferMap[startSequence]) {
                        srcBuf[item.first] = item.second;
                    }
                }
            }
            if (!srcBuf.empty()) {
                dispatchTask(srcBuf, fakeTaskBuffers, true, false);
            }
            startSequence++;
        }
    }

    mLastStillTnrSequence = sequence;
}

int ProcessingUnit::getTnrFrameCount(const AiqResult* aiqResult) {
    if (mTnrTriggerInfo.num_gains == 0U) {
        return 1;
    }

    const float totalGain = (aiqResult->mAeResults.exposures[0].exposure->analog_gain *
                  aiqResult->mAeResults.exposures[0].exposure->digital_gain);
    if (totalGain < mTnrTriggerInfo.tnr7us_threshold_gain) {
        return 1;
    }

    unsigned int index = 0U;
    for (unsigned int i = 1; i < mTnrTriggerInfo.num_gains; i++) {
        if (fabs(mTnrTriggerInfo.trigger_infos[i].gain - totalGain) <
            fabs(mTnrTriggerInfo.trigger_infos[i - 1].gain - totalGain)) {
            index = i;
        }
    }

    LOG2("TNR trigger frame count %d, the current total gain %f",
         mTnrTriggerInfo.trigger_infos[index].frame_count, totalGain);

    return mTnrTriggerInfo.trigger_infos[index].frame_count;
}

void ProcessingUnit::dispatchTask(CameraBufferPortMap& inBuf, CameraBufferPortMap& outBuf,
                                  bool fakeTask, bool callbackRgbs) {
    const int64_t currentSequence = inBuf.begin()->second->getSequence();
    TRACE_LOG_POINT("ProcessingUnit", "start run PSYS", MAKE_COLOR(currentSequence),
                    currentSequence);
    PERF_CAMERA_ATRACE_PARAM1("Task Sequence", currentSequence);
    {
        std::unique_lock<std::mutex> lock(mBufferQueueLock);
        mSequencesInflight.insert(currentSequence);
        LOG2("<id%d:seq:%ld>@%s, fake task %d, pending task: %zu", mCameraId, currentSequence,
             __func__, fakeTask, mSequencesInflight.size());
    }  // End of lock mBufferQueueLock

    // Prepare the task input parameters including input and output buffers, settings etc.
    PipeTaskData taskParam;
    taskParam.mTuningMode = mTuningMode;
    taskParam.mInputBuffers = inBuf;
    taskParam.mOutputBuffers = outBuf;
    taskParam.mFakeTask = fakeTask;
    taskParam.mCallbackRgbs = callbackRgbs;

    const int64_t settingSequence = getSettingSequence(outBuf);
    // Handle per-frame settings if output buffer requires
    if (settingSequence > -1) {
        auto cameraContext = CameraContext::getInstance(mCameraId);
        auto dataContext = cameraContext->getDataContextBySeq(currentSequence);

        (void)setParameters(dataContext);

        // Dump raw image if makernote mode is MAKERNOTE_MODE_JPEG or fake task for IQ tune
        if (((dataContext->mAiqParams.makernoteMode == MAKERNOTE_MODE_JPEG)
             || fakeTask) &&
            CameraDump::isDumpTypeEnable(DUMP_JPEG_BUFFER)) {
            CameraDump::dumpImage(mCameraId, inBuf.begin()->second, M_PSYS, inBuf.begin()->first);
        }
    }
    {
        AutoRMutex rl(mIspSettingsLock);
        mIspSettings.palOverride = nullptr;
        taskParam.mIspSettings = mIspSettings;
    }

    if (!IProcessingUnit::mThreadRunning) {
        return;
    }

    mPipeManager->addTask(taskParam);
}

void ProcessingUnit::onBufferDone(int64_t sequence, uuid port,
                                 const std::shared_ptr<CameraBuffer>& camBuffer) {
    LOG2("<id%d:seq%ld>@%s, port %d", mCameraId, sequence, __func__, port);

    if (CameraDump::isDumpTypeEnable(DUMP_PIPELINE_OUTPUT)) {
        CameraDump::dumpImage(mCameraId, camBuffer, M_PIPELINE, port);
    }

    if (!needSkipOutputFrame(sequence)) {
        for (auto& it : BufferQueue::mBufferConsumerList) {
            it->onBufferAvailable(port, camBuffer);
        }
    }
}

void ProcessingUnit::onStatsReady(EventData& eventData) {
    if ((eventData.type == EVENT_PSYS_STATS_BUF_READY) ||
        (eventData.type == EVENT_PSYS_STATS_SIS_BUF_READY)) {
        notifyListeners(eventData);
    }
}

void ProcessingUnit::sendPsysFrameDoneEvent(const CameraBufferPortMap* dstBuffers) {
    for (auto& dst : *dstBuffers) {
        shared_ptr<CameraBuffer> outBuf = dst.second;
        if (outBuf == nullptr) {
            continue;
        }

        EventData frameData;
        frameData.type = EVENT_PSYS_FRAME;
        frameData.buffer = nullptr;
        frameData.data.frame.sequence = outBuf->getSequence();
        frameData.data.frame.timestamp.tv_sec = outBuf->getTimestamp().tv_sec;
        frameData.data.frame.timestamp.tv_usec = outBuf->getTimestamp().tv_usec;
        notifyListeners(frameData);

        LOG2("%s, frame done for sequence: %ld", __func__, frameData.data.frame.sequence);
        break;
    }
}

void ProcessingUnit::sendPsysRequestEvent(const CameraBufferPortMap* dstBuffers, int64_t sequence,
                                         uint64_t timestamp, EventType eventType) {
    for (const auto& output : *dstBuffers) {
        if ((output.second != nullptr) && (!output.second->isInternalBuffer())) {
            EventData event;
            event.type = eventType;
            event.buffer = nullptr;
            event.data.requestReady.timestamp =
                timestamp > 0U ? timestamp : output.second->getUserBuffer()->timestamp;
            event.data.requestReady.sequence = sequence;
            event.data.requestReady.frameNumber = output.second->getUserBuffer()->frameNumber;

            notifyListeners(event);
            break;
        }
    }
}

void ProcessingUnit::onTaskDone(const PipeTaskData& result) {
    const int64_t sequence = result.mInputBuffers.begin()->second->getSequence();
    LOG2("<id%d:seq%ld>@%s", mCameraId, sequence, __func__);
    TRACE_LOG_POINT("ProcessingUnit", __func__, MAKE_COLOR(sequence), sequence);
    PERF_CAMERA_ATRACE_PARAM1("Task Done Sequence", sequence);

    // If it is YUV reprocessing, its request doesn't have extra processing
    if (result.mYuvTask) {
        auto iter = result.mInputBuffers.find(YUV_REPROCESSING_INPUT_PORT_ID);
        if (iter == result.mInputBuffers.end()) {
            LOGW("<id%d:seq%ld>@%s can't find YUV reprocessing input buffer",
                 mCameraId, sequence, __func__);
            return;
        }
        // Return YUV reprocessing input buffer
        for (auto& it : BufferQueue::mBufferConsumerList) {
            it->onBufferAvailable(YUV_REPROCESSING_INPUT_PORT_ID, iter->second);
        }
        return;
    }

    if (!result.mFakeTask) {
        if (!needSkipOutputFrame(sequence)) {
            sendPsysFrameDoneEvent(&result.mOutputBuffers);
        }

        const int64_t settingSequence = getSettingSequence(result.mOutputBuffers);
        const bool holdOnInput = needHoldOnInputFrame(settingSequence, sequence);
        const bool hasRawOutput = isBufferHoldForRawReprocess(sequence);

        LOG2("%s, dst sequence: %ld, src sequence: %ld, hasRawOutput: %d, holdOnInput: %d",
             __func__, settingSequence, sequence, hasRawOutput, holdOnInput);
        // Return buffer only if the buffer is not used in the future.
        if ((!holdOnInput) && (mBufferProducer != nullptr) && (!hasRawOutput)) {
            for (const auto& src : result.mInputBuffers) {
                if ((src.second->getStreamUsage() != CAMERA_STREAM_OPAQUE_RAW) &&
                    (src.second->getStreamType() == CAMERA_STREAM_INPUT)) {
                    for (auto& it : BufferQueue::mBufferConsumerList) {
                        it->onBufferAvailable(src.first, src.second);
                    }
                } else {
                    mBufferProducer->qbuf(src.first, src.second);
                }
            }
        }
    }

    {
        AutoMutex l(mBufferQueueLock);
        std::multiset<int64_t>::iterator it = mSequencesInflight.find(sequence);
        if (it != mSequencesInflight.end()) {
            mSequencesInflight.erase(it);
        }
    }

    returnRawBuffer();
}

void ProcessingUnit::onMetadataReady(int64_t sequence, const CameraBufferPortMap& outBuf) {
    LOG2("<seq%ld> %s", sequence, __func__);

    // handle metadata event after decoding stats
    sendPsysRequestEvent(&outBuf, sequence, 0U, EVENT_REQUEST_METADATA_READY);
}

void ProcessingUnit::outputRawImage(shared_ptr<CameraBuffer>& srcBuf,
                                   shared_ptr<CameraBuffer>& dstBuf) {
    if ((srcBuf == nullptr) || (dstBuf == nullptr)) {
        return;
    }

    // Copy from source buffer
    CameraBufferMapper srcMapper(srcBuf);
    CameraBufferMapper dstMapper(dstBuf);

    MEMCPY_S(dstMapper.addr(), dstMapper.size(), srcMapper.addr(), srcMapper.size());

    // Send output buffer to its consumer
    for (auto& it : BufferQueue::mBufferConsumerList) {
        it->onBufferAvailable(mRawPort, dstBuf);
    }
}
}  // namespace icamera
