/*
 * Copyright (C) 2015-2025 Intel Corporation.
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

#define LOG_TAG SwImageProcessor

#include "SwImageProcessor.h"

#include "CameraBuffer.h"
#include "PlatformData.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/SwImageConverter.h"
#include "iutils/Utils.h"

namespace icamera {

SwImageProcessor::SwImageProcessor(int cameraId) : mCameraId(cameraId) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mProcessThread = new ProcessThread(this);
}

SwImageProcessor::~SwImageProcessor() {
    mProcessThread->wait();
    delete mProcessThread;
}

int SwImageProcessor::start() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s", mCameraId, __func__);
    AutoMutex l(mBufferQueueLock);

    const int memType = mOutputFrameInfo.begin()->second.memType;
    CheckAndLogError(memType == V4L2_MEMORY_DMABUF, BAD_VALUE,
                     "@%s: DMABUF is not supported in SwProcessor as output", __func__);

    int ret = BufferQueue::allocProducerBuffers(mCameraId, MAX_BUFFER_COUNT);
    CheckAndLogError(ret != OK, ret, "@%s: Allocate Buffer failed", __func__);
    IProcessingUnit::mThreadRunning = true;
    mProcessThread->start();

    return 0;
}

void SwImageProcessor::stop() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s", mCameraId, __func__);

    IProcessingUnit::mThreadRunning = false;
    mProcessThread->exit();

    {
        AutoMutex l(mBufferQueueLock);
        mFrameAvailableSignal.notify_one();
    }

    mProcessThread->wait();

    // Thread is not running. It is safe to clear the Queue
    BufferQueue::clearBufferQueues();
}

int SwImageProcessor::processNewFrame() {
    PERF_CAMERA_ATRACE();

    int ret = OK;
    std::map<uuid, std::shared_ptr<CameraBuffer> > srcBuffers, dstBuffers;
    std::shared_ptr<CameraBuffer> cInBuffer;
    uuid inputPort = INVALID_PORT;
    LOG1("<id%d>@%s", mCameraId, __func__);

    {
        std::unique_lock<std::mutex> lock(mBufferQueueLock);
        if (!this->mThreadRunning) {
            return -1;  // Already stopped
        }

        ret = BufferQueue::waitFreeBuffersInQueue(lock, srcBuffers, dstBuffers);

        if (!this->mThreadRunning) {
            return -1;  // Already stopped
        }
        if (ret == NOT_ENOUGH_DATA) {
            return OK;
        }

        // Wait frame buffer time out should not involve thread exit.
        if (ret == TIMED_OUT) {
            LOG1("<id%d>@%s, timeout happen, wait recovery", mCameraId, __func__);
            return OK;
        }

        inputPort = srcBuffers.begin()->first;
        cInBuffer = srcBuffers[inputPort];

        for (auto& output : BufferQueue::mOutputQueue) {
            output.second.pop();
        }

        for (auto& input : BufferQueue::mInputQueue) {
            input.second.pop();
        }
    }
    CheckAndLogError(cInBuffer == nullptr, BAD_VALUE, "Invalid input buffer.");

    for (auto& dst : dstBuffers) {
        const uuid port = dst.first;
        std::shared_ptr<CameraBuffer> cOutBuffer = dst.second;
        // If the output buffer is nullptr, that means user doesn't request that buffer,
        // so it doesn't need to be handled here.
        if (cOutBuffer == nullptr) {
            continue;
        }

        // No Lock for this function make sure buffers are not freed before the stop
        ret = SwImageConverter::convertFormat(
            cInBuffer->getWidth(), cInBuffer->getHeight(),
            static_cast<unsigned char*>(cInBuffer->getBufferAddr()), cInBuffer->getBufferSize(),
            cInBuffer->getFormat(), static_cast<unsigned char*>(cOutBuffer->getBufferAddr()),
            cOutBuffer->getBufferSize(), cOutBuffer->getFormat());
        CheckAndLogError((ret < 0), ret, "format conversion failed with %d", ret);

        if (CameraDump::isDumpTypeEnable(DUMP_SW_IMG_PROC_OUTPUT)) {
            CameraDump::dumpImage(mCameraId, cOutBuffer, M_SWIPOP);
        }

        // update the interlaced field, sequence, and timestamp  from the src buf to dst buf
        cOutBuffer->updateV4l2Buffer(*cInBuffer->getV4L2Buffer().Get());

        // Notify listener: No lock here: mBufferConsumerList will not updated in this state
        for (auto& it : BufferQueue::mBufferConsumerList) {
            it->onBufferAvailable(port, cOutBuffer);
        }
    }

    // Return the buffers to the producer
    if (mBufferProducer != nullptr) {
        mBufferProducer->qbuf(inputPort, cInBuffer);
    }

    return OK;
}

}  // namespace icamera
