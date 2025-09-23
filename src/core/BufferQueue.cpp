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

#define LOG_TAG BufferQueue

#include "BufferQueue.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"

namespace icamera {

BufferProducer::BufferProducer(int memType) : mMemType(memType) {
    LOG1("@%s BufferProducer %p created mMemType: %d", __func__, this, mMemType);
}

BufferQueue::BufferQueue()
        : mBufferProducer(nullptr),
          mThreadWaiting(true) {
    LOG1("@%s BufferQueue %p created", __func__, this);
}

BufferQueue::~BufferQueue() {}

int BufferQueue::queueInputBuffer(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    // If it's not in mInputQueue, then it's not for this processor.
    if (mInputQueue.find(port) == mInputQueue.end()) {
        return OK;
    }

    LOG2("%s CameraBuffer %p for port:%x", __func__, camBuffer.get(), port);

    CameraBufQ& input = mInputQueue[port];
    const bool needSignal = input.empty();
    input.push(camBuffer);
    if (needSignal) {
        mFrameAvailableSignal.notify_one();
    }

    return OK;
}

int BufferQueue::onBufferAvailable(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    AutoMutex l(mBufferQueueLock);

    return queueInputBuffer(port, camBuffer);
}

void BufferQueue::setBufferProducer(BufferProducer* producer) {
    LOG1("%s producer %p", __func__, producer);

    AutoMutex l(mBufferQueueLock);
    mBufferProducer = producer;

    if (producer == nullptr) {
        return;
    }

    mBufferProducer->addFrameAvailableListener(this);
}

void BufferQueue::addFrameAvailableListener(BufferConsumer* listener) {
    LOG1("%s listener %p", __func__, listener);
    AutoMutex l(mBufferQueueLock);
    bool isAlreadyAdded = false;
    for (const auto& consumer : mBufferConsumerList) {
        if (consumer == listener) {
            isAlreadyAdded = true;
            break;
        }
    }

    // If the listener has been already added, then we don't register it again.
    if (isAlreadyAdded) {
        return;
    }
    mBufferConsumerList.push_back(listener);
}

void BufferQueue::removeFrameAvailableListener(BufferConsumer* listener) {
    LOG1("%s listener %p", __func__, listener);
    AutoMutex l(mBufferQueueLock);

    for (auto it = mBufferConsumerList.begin(); it != mBufferConsumerList.end(); ++it) {
        if ((*it) == listener) {
            mBufferConsumerList.erase(it);
            break;
        }
    }
}

int BufferQueue::qbuf(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    LOG2("%s CameraBuffer %p for port:%x", __func__, camBuffer.get(), port);

    // Enqueue buffer to internal pool
    AutoMutex l(mBufferQueueLock);
    CheckAndLogError(mOutputQueue.find(port) == mOutputQueue.end(), BAD_VALUE,
                     "Not supported port:%x", port);

    CameraBufQ& output = mOutputQueue[port];
    output.push(camBuffer);

    return OK;
}

void BufferQueue::clearBufferQueues() {
    AutoMutex l(mBufferQueueLock);

    mInputQueue.clear();
    for (const auto& input : mInputFrameInfo) {
        mInputQueue[input.first] = CameraBufQ();
    }

    mOutputQueue.clear();
    for (const auto& output : mOutputFrameInfo) {
        mOutputQueue[output.first] = CameraBufQ();
    }
}

void BufferQueue::setFrameInfo(const std::map<uuid, stream_t>& inputInfo,
                               const std::map<uuid, stream_t>& outputInfo) {
    mInputFrameInfo = inputInfo;
    mOutputFrameInfo = outputInfo;

    clearBufferQueues();
}

void BufferQueue::getFrameInfo(std::map<uuid, stream_t>& inputInfo,
                               std::map<uuid, stream_t>& outputInfo) const {
    inputInfo = mInputFrameInfo;
    outputInfo = mOutputFrameInfo;
}

void BufferQueue::setThreadWaiting(bool waiting) {
    mThreadWaiting = waiting;
}

int BufferQueue::waitFreeBuffersInQueue(std::unique_lock<std::mutex>& lock,
                                        std::map<uuid, std::shared_ptr<CameraBuffer> >& buffer,
                                        std::map<uuid, CameraBufQ>& bufferQueue,
                                        int64_t timeout) {
    timeout = (timeout != 0 ? timeout : kWaitDuration) * SLOWLY_MULTIPLIER;

    for (auto& queue : bufferQueue) {
        const uuid port = queue.first;
        CameraBufQ& cameraBufQ = queue.second;
        if (cameraBufQ.empty()) {
            LOG2("%s: wait port %x", __func__, port);
            const std::cv_status status = mFrameAvailableSignal.wait_for(
                lock, std::chrono::nanoseconds(timeout));

            if (!mThreadWaiting) {
                return -1;  // Already stopped
            }

            if (status == std::cv_status::timeout) {
                return TIMED_OUT;
            }
        }
        if (cameraBufQ.empty()) {
            return NOT_ENOUGH_DATA;
        }

        // Wake up from the buffer available
        buffer[port] = cameraBufQ.front();
    }

    return OK;
}

int BufferQueue::waitFreeBuffersInQueue(std::unique_lock<std::mutex>& lock,
                                        std::map<uuid, std::shared_ptr<CameraBuffer> >& cInBuffer,
                                        std::map<uuid, std::shared_ptr<CameraBuffer> >& cOutBuffer,
                                        int64_t timeout) {
    int ret = OK;
    timeout = (timeout != 0 ? timeout : kWaitDuration) * SLOWLY_MULTIPLIER;

    LOG2("@%s start waiting the input and output buffers", __func__);
    ret = waitFreeBuffersInQueue(lock, cInBuffer, mInputQueue, timeout);
    if (ret != OK) {
        return ret;
    }

    return waitFreeBuffersInQueue(lock, cOutBuffer, mOutputQueue, timeout);
}

int BufferQueue::getFreeBuffersInQueue(std::map<uuid, std::shared_ptr<CameraBuffer> >& inBuffers,
                                       std::map<uuid, std::shared_ptr<CameraBuffer> >& outBuffers) {
    for (auto& input : mInputQueue) {
        const uuid port = input.first;
        CameraBufQ& inputQueue = input.second;
        if (inputQueue.empty()) {
            inBuffers.clear();
            return NOT_ENOUGH_DATA;
        }
        inBuffers[port] = inputQueue.front();
    }

    for (auto& output : mOutputQueue) {
        const uuid port = output.first;
        CameraBufQ& outputQueue = output.second;
        if (outputQueue.empty()) {
            inBuffers.clear();
            outBuffers.clear();
            return NOT_ENOUGH_DATA;
        }
        outBuffers[port] = outputQueue.front();
    }

    for (auto& input : mInputQueue) {
        input.second.pop();
    }
    for (auto& output : mOutputQueue) {
        output.second.pop();
    }
    return OK;
}

void BufferQueue::returnBuffers(std::map<uuid, std::shared_ptr<CameraBuffer> >& inBuffers,
                                std::map<uuid, std::shared_ptr<CameraBuffer> >& outBuffers) {
    // Return buffers
    if (mBufferProducer != nullptr) {
        for (auto const& portBufferPair : inBuffers) {
            mBufferProducer->qbuf(portBufferPair.first, portBufferPair.second);
        }
    }
    for (auto const& portBufferPair : outBuffers) {
        std::shared_ptr<CameraBuffer> outBuf = portBufferPair.second;
        const uuid port = portBufferPair.first;
        // If the output buffer is nullptr, that means user doesn't request that buffer,
        // so it doesn't need to be handled here.
        if (outBuf == nullptr) {
            continue;
        }
        for (auto& it : mBufferConsumerList) {
            it->onBufferAvailable(port, outBuf);
        }
    }
}

int BufferQueue::allocProducerBuffers(int camId, int bufNum) {
    LOG1("%s: buffer queue size %d", __func__, bufNum);

    mInternalBuffers.clear();

    CheckAndLogError(mBufferProducer == nullptr, BAD_VALUE, "@%s: Buffer Producer is nullptr",
                     __func__);

    for (const auto& item : mInputFrameInfo) {
        uuid port = item.first;
        const int srcFmt = item.second.format;
        const int srcWidth = item.second.width;
        const int srcHeight = item.second.height;

        LOG1("%s fmt:%s (%dx%d)", __func__, CameraUtils::format2string(srcFmt).c_str(), srcWidth,
             srcHeight);

        const int32_t size = CameraUtils::getFrameSize(srcFmt, srcWidth, srcHeight);
        int memType = mBufferProducer->getMemoryType();

        for (int i = 0; i < bufNum; i++) {
            std::shared_ptr<CameraBuffer> camBuffer;
            switch (memType) {
                case V4L2_MEMORY_USERPTR:
                    camBuffer =
                        CameraBuffer::create(V4L2_MEMORY_USERPTR, size, i, srcFmt, srcWidth,
                                             srcHeight);
                    CheckAndLogError(camBuffer == nullptr, NO_MEMORY,
                                     "Allocate producer userptr buffer failed");
                    break;

                case V4L2_MEMORY_MMAP:
                    camBuffer = std::make_shared<CameraBuffer>(V4L2_MEMORY_MMAP, size, i);
                    CheckAndLogError(camBuffer == nullptr, NO_MEMORY,
                                     "Allocate producer mmap buffer failed");
                    camBuffer->setUserBufferInfo(srcFmt, srcWidth, srcHeight);
                    mBufferProducer->allocateMemory(port, camBuffer);
                    break;

                default:
                    LOGE("Not supported v4l2 memory type:%d", memType);
                    return BAD_VALUE;
            }

            mInternalBuffers[port].push_back(camBuffer);
            mBufferProducer->qbuf(port, camBuffer);
        }
    }

    return OK;
}

}  // namespace icamera
