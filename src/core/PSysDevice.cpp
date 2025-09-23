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

#define LOG_TAG PSysDevice

#include "PSysDevice.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/eventfd.h>

#include "CameraLog.h"
#include "Errors.h"
#include "Utils.h"

namespace icamera {

static const char* DRIVER_NAME = "/dev/ipu7-psys0";

PSysDevice::PSysDevice(int cameraId)
        : mPollThread(nullptr),
          mExitPending(false),
          mCameraId(cameraId),
          mFd(-1),
          mGraphId(INVALID_GRAPH_ID),
          mEventFd(-1) {
    LOG1("<%id> Construct PSysDevice", mCameraId);

    CLEAR(mFrameId);
    (void)memset(&mFrameIdToSeqMap, -1, sizeof(mFrameIdToSeqMap));
    mGraphNode = new graph_node[MAX_GRAPH_NODES];
    for (uint8_t i = 0U; i < MAX_GRAPH_NODES; i++) {
        mTaskBuffers[i] = new ipu_psys_term_buffers[MAX_GRAPH_TERMINALS];
    }

    mPollThread = new PollThread<PSysDevice>(this);

    mEventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
}

PSysDevice::~PSysDevice() {
    LOG1("Destroy PSysDevice");

    // Unregister PSYS buffer
    while (!mPtrToTermBufMap.empty()) {
        auto it = mPtrToTermBufMap.begin();
        PSysDevice::unregisterBuffer(&it->second);
    }

    if (mFd >= 0) {
        const int ret = ::close(mFd);
        if (ret < 0) {
            LOGE("Failed to close psys device %s, ret %d", strerror(errno), ret);
        }
    }

    mPollThread->wait();

    delete mPollThread;

    delete[] mGraphNode;
    for (uint8_t i = 0U; i < MAX_GRAPH_NODES; i++) {
        delete[] mTaskBuffers[i];
    }

    if (mEventFd >= 0) {
        close(mEventFd);
    }
}

void PSysDevice::deinit() {
    mPollThread->exit();
    mExitPending = true;

    // Wake up poll thread
    if (mEventFd >= 0) {
        const uint64_t value = 1U;
        const ssize_t ret = write(mEventFd, &value, sizeof(value));
        if (ret < 0) {
            LOGW("Failed to interrupt event %s", strerror(errno));
        }
    }
}

int PSysDevice::init() {
    mFd = open(DRIVER_NAME, O_RDWR | O_NONBLOCK, 0);
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "Failed to open psys device %s", strerror(errno));

    mPollThread->start();

    return OK;
}

int PSysDevice::addGraph(const PSysGraph& graph) {
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "psys device wasn't opened");

    ipu_psys_graph_info graphDrv;
    CLEAR(graphDrv);

    graphDrv.graph_id = INVALID_GRAPH_ID;
    graphDrv.num_nodes = 0U;
    graphDrv.nodes = mGraphNode;
    (void)memset(mGraphNode, 0, sizeof(graph_node) * MAX_GRAPH_NODES);

    for (const auto& node : graph.nodes) {
        graph_node& drvNode = graphDrv.nodes[node.nodeCtxId];

        drvNode.node_ctx_id = static_cast<uint8_t>(node.nodeCtxId);
        drvNode.node_rsrc_id = static_cast<uint8_t>(node.nodeRsrcId);
        MEMCPY_S(&drvNode.profiles[0].teb, sizeof(drvNode.profiles[0].teb), &node.bitmaps.teb,
                 sizeof(node.bitmaps.teb));
        MEMCPY_S(&drvNode.profiles[0].deb, sizeof(drvNode.profiles[0].deb), &node.bitmaps.deb,
                 sizeof(node.bitmaps.deb));
        MEMCPY_S(&drvNode.profiles[0].rbm, sizeof(drvNode.profiles[0].rbm), &node.bitmaps.rbm,
                 sizeof(node.bitmaps.rbm));
        MEMCPY_S(&drvNode.profiles[0].reb, sizeof(drvNode.profiles[0].reb), &node.bitmaps.reb,
                 sizeof(node.bitmaps.reb));

        uint8_t termIndex = 0U;
        for (const auto& term : node.terminalConfig) {
            const uint8_t termId = term.first;
            drvNode.terminals[termIndex].term_id = termId;
            drvNode.terminals[termIndex].buf_size = term.second.payloadSize;
            termIndex++;
        }

        drvNode.num_terms = termIndex;
        graphDrv.num_nodes++;
    }

    uint8_t linkIndex = 0U;
    for (const auto& link : graph.links) {
        graph_link& drvLink = graphDrv.links[linkIndex];

        drvLink.ep_src.node_ctx_id = static_cast<uint8_t>(link.srcNodeCtxId);
        drvLink.ep_src.term_id = static_cast<uint8_t>(link.srcTermId);
        drvLink.ep_dst.node_ctx_id = static_cast<uint8_t>(link.dstNodeCtxId);
        drvLink.ep_dst.term_id = static_cast<uint8_t>(link.dstTermId);

        drvLink.foreign_key = IPU_PSYS_FOREIGN_KEY_NONE;
        drvLink.streaming_mode = link.streamingMode;
        drvLink.pbk_id = IPU_PSYS_LINK_PBK_ID_NONE;
        drvLink.pbk_slot_id = IPU_PSYS_LINK_PBK_SLOT_ID_NONE;
        drvLink.delayed_link = link.delayedLink;

        linkIndex++;
    }

    int ret = ::ioctl(mFd, static_cast<int>(IPU_IOC_GRAPH_OPEN), &graphDrv);
    CheckAndLogError((ret != 0) || (graphDrv.graph_id == INVALID_GRAPH_ID), INVALID_OPERATION,
                     "Failed to open graph %s", strerror(errno));

    mGraphId = graphDrv.graph_id;

    return OK;
}

int PSysDevice::closeGraph() {
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "psys device wasn't opened");

    if (mGraphId != INVALID_GRAPH_ID) {
        int ret = ::ioctl(mFd, static_cast<int>(IPU_IOC_GRAPH_CLOSE), &mGraphId);
        CheckAndLogError(ret != 0, INVALID_OPERATION, "Failed to close graph %s", strerror(errno));
        mGraphId = INVALID_GRAPH_ID;
    }
    CLEAR(mFrameId);
    return OK;
}

int PSysDevice::addTask(const PSysTask& task) {
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "psys device wasn't opened");

    ipu_psys_task_request taskData;
    CLEAR(taskData);

    taskData.graph_id = mGraphId;
    taskData.node_ctx_id = task.nodeCtxId;
    taskData.frame_id = mFrameId[task.nodeCtxId];
    taskData.task_buffers = mTaskBuffers[task.nodeCtxId];
    (void)memset(mTaskBuffers[task.nodeCtxId], 0,
                 sizeof(ipu_psys_term_buffers) * MAX_GRAPH_TERMINALS);
    taskData.term_buf_count = 0U;

    for (const auto& item : task.terminalBuffers) {
        taskData.task_buffers[taskData.term_buf_count].term_id = item.first;
        taskData.task_buffers[taskData.term_buf_count].term_buf = item.second.psysBuf;
        taskData.term_buf_count++;
    }

    {
        std::lock_guard<std::mutex> l(mDataLock);
        const uint8_t idx = taskData.frame_id % MAX_TASK_NUM;
        CheckWarningNoReturn(mFrameIdToSeqMap[task.nodeCtxId][idx] >= 0,
                             "context %d sequence %lld not done", task.nodeCtxId,
                             mFrameIdToSeqMap[task.nodeCtxId][idx]);

        mFrameIdToSeqMap[task.nodeCtxId][idx] = task.sequence;
        if (mFrameId[task.nodeCtxId] >= MAX_DRV_FRAME_ID) {
            mFrameId[task.nodeCtxId] = 0U;
        } else {
            ++mFrameId[task.nodeCtxId];
        }
    }

    const int ret = ioctl(mFd, static_cast<int>(IPU_IOC_TASK_REQUEST), &taskData);
    CheckAndLogError(ret != 0, INVALID_OPERATION, "Failed to add task %s", strerror(errno));

    return OK;
}

int PSysDevice::wait(ipu_psys_event& event) {
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "psys device wasn't opened");

    int ret = ioctl(mFd, static_cast<int>(IPU_IOC_DQEVENT), &event);
    CheckAndLogError(ret != 0, INVALID_OPERATION, "Failed to dequeue event %s", strerror(errno));

    return OK;
}

void PSysDevice::updatePsysBufMap(TerminalBuffer* buf) {
    std::lock_guard<std::mutex> l(mDataLock);
    if ((buf->flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
        mPtrToTermBufMap[buf->userPtr] = *buf;
    } else if ((buf->flags & IPU_BUFFER_FLAG_DMA_HANDLE) != 0U) {
        mFdToTermBufMap[static_cast<int>(buf->handle)] = *buf;
    }
}

void PSysDevice::erasePsysBufMap(const TerminalBuffer* buf) {
    std::lock_guard<std::mutex> l(mDataLock);
    if ((buf->flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
        if (mPtrToTermBufMap.find(buf->userPtr) != mPtrToTermBufMap.end()) {
            mPtrToTermBufMap.erase(buf->userPtr);
        }
    } else if ((buf->flags & IPU_BUFFER_FLAG_DMA_HANDLE) != 0U) {
        if (mFdToTermBufMap.find(static_cast<int>(buf->handle)) != mFdToTermBufMap.end()) {
            mFdToTermBufMap.erase(static_cast<int>(buf->handle));
        }
    }
}

bool PSysDevice::getPsysBufMap(TerminalBuffer* buf) {
    std::lock_guard<std::mutex> l(mDataLock);
    if ((buf->flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
        if (mPtrToTermBufMap.find(buf->userPtr) != mPtrToTermBufMap.end()) {
            buf->psysBuf = mPtrToTermBufMap[buf->userPtr].psysBuf;
            return true;
        }
    } else if ((buf->flags & IPU_BUFFER_FLAG_DMA_HANDLE) != 0U) {
        if (mFdToTermBufMap.find(static_cast<int>(buf->handle)) != mFdToTermBufMap.end()) {
            buf->psysBuf = mFdToTermBufMap[static_cast<int>(buf->handle)].psysBuf;
            return true;
        }
    }

    return false;
}

int PSysDevice::registerBuffer(TerminalBuffer* buf) {
    CheckAndLogError(mFd < 0, INVALID_OPERATION, "psys device wasn't opened");
    CheckAndLogError(buf == nullptr, INVALID_OPERATION, "buf is nullptr");

    // If already registered, just return
    if (getPsysBufMap(buf)) {
        return OK;
    }

    int ret = OK;
    buf->psysBuf.len = buf->size;
    if ((buf->flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
        buf->psysBuf.base.userptr = buf->userPtr;
        buf->psysBuf.flags |= IPU_BUFFER_FLAG_USERPTR;

        ret = ioctl(mFd, static_cast<int>(IPU_IOC_GETBUF), &buf->psysBuf);
        CheckAndLogError(ret != 0, INVALID_OPERATION, "Failed to get buffer %s", strerror(errno));

        if ((buf->psysBuf.flags & IPU_BUFFER_FLAG_DMA_HANDLE) == 0U) {
            LOGW("IOC_GETBUF succeed but did not return dma handle");
            return INVALID_OPERATION;
        } else if ((buf->psysBuf.flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
            LOGW("IOC_GETBUF succeed but did not consume the userptr flag");
            return INVALID_OPERATION;
        }
    } else if ((buf->flags & IPU_BUFFER_FLAG_DMA_HANDLE) != 0U) {
        buf->psysBuf.base.fd = static_cast<int>(buf->handle);
        buf->psysBuf.flags |= IPU_BUFFER_FLAG_DMA_HANDLE;
    }

    if ((buf->flags & IPU_BUFFER_FLAG_NO_FLUSH) != 0U) {
        buf->psysBuf.flags |= IPU_BUFFER_FLAG_NO_FLUSH;
    }

    buf->psysBuf.data_offset = 0U;
    buf->psysBuf.bytes_used = buf->psysBuf.len;

    ret = ioctl(mFd, static_cast<int>(IPU_IOC_MAPBUF),
                reinterpret_cast<void*>(static_cast<intptr_t>(buf->psysBuf.base.fd)));
    CheckAndLogError(ret != 0, INVALID_OPERATION, "Failed to map buffer %s", strerror(errno));

    // Save PSYS buf
    updatePsysBufMap(buf);

    LOG2("%s, mapbuffer flags %x, ptr %p, fd %d, size %d", __func__, buf->flags, buf->userPtr,
         buf->psysBuf.base.fd, buf->size);

    return OK;
}

void PSysDevice::unregisterBuffer(const TerminalBuffer* buf) {
    if (mFd < 0) {
        LOGE("psys device wasn't opened");
        return;
    }
    if (buf == nullptr) {
        LOGE("buf is nullptr");
        return;
    }

    if (((buf->flags & IPU_BUFFER_FLAG_DMA_HANDLE) != 0U) && (!buf->isExtDmaBuf)) {
        LOGW("cannot unmap buffer fd %d", buf->psysBuf.base.fd);
        return;
    }

    int ret = ioctl(mFd, static_cast<int>(IPU_IOC_UNMAPBUF),
                    reinterpret_cast<void*>(static_cast<intptr_t>(buf->psysBuf.base.fd)));
    if (ret != 0) {
        LOGW("Failed to unmap buffer %s", strerror(errno));
    }

    if ((buf->flags & IPU_BUFFER_FLAG_USERPTR) != 0U) {
        ret = close(buf->psysBuf.base.fd);
        if (ret < 0) {
            LOGE("Failed to close fd %d, error %s", buf->psysBuf.base.fd, strerror(errno));
        }
    }

    // erase PSYS buf
    erasePsysBufMap(buf);
}

void PSysDevice::registerPSysDeviceCallback(uint8_t contextId, IPSysDeviceCallback* callback) {
    mPSysDeviceCallbackMap[contextId] = callback;
}

int PSysDevice::poll(short events, int timeout)
{
    std::vector<struct pollfd> pollfds;
    pollfds.reserve(1 + ((mEventFd >= 0) ? 1 : 0));

    pollfds.push_back({ mFd, events, 0 });

    if (mEventFd >= 0) {
        pollfds.push_back({ mEventFd, POLLIN, 0 });
    }

    return ::poll(pollfds.data(), pollfds.size(), timeout);
}

void PSysDevice::handleEvent(const ipu_psys_event& event) {
    int64_t sequence = -1;
    const uint8_t idx = event.frame_id % MAX_TASK_NUM;;

    {
        std::lock_guard<std::mutex> l(mDataLock);
        if (mFrameIdToSeqMap[event.node_ctx_id][idx] < 0) {
            LOGW("frame id %u isn't found", event.frame_id);
            return;
        }

        sequence = mFrameIdToSeqMap[event.node_ctx_id][idx];
    }

    if (mPSysDeviceCallbackMap.find(event.node_ctx_id) == mPSysDeviceCallbackMap.end()) {
        LOGW("context id %u isn't found", event.node_ctx_id);
        return;
    }
    mPSysDeviceCallbackMap[event.node_ctx_id]->bufferDone(sequence);

    {
        std::lock_guard<std::mutex> l(mDataLock);
        mFrameIdToSeqMap[event.node_ctx_id][idx] = -1;
    }

    LOG2("context id %u, frame id %u is done", event.node_ctx_id, event.frame_id);
}

int PSysDevice::poll() {
    if (mExitPending) {
        return NO_INIT;
    }

    int ret = poll(static_cast<int16_t>(POLLIN | POLLHUP | POLLERR),
                    kEventTimeout * SLOWLY_MULTIPLIER);

    if (mExitPending) {
        return NO_INIT;
    }

    if (ret == POLLIN) {
        ipu_psys_event event;
        CLEAR(event);

        ret = wait(event);
        if (ret == OK) {
            handleEvent(event);
        }
    } else {
        LOG2("%s, device poll timeout", __func__);
    }

    return OK;
}

}  // namespace icamera
