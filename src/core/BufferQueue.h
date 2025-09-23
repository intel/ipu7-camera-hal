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

#pragma once

#include <map>
#include <vector>
#include <condition_variable>

#include "BufferInterface.h"
#include "CameraBuffer.h"
#include "CameraEvent.h"
#include "iutils/Errors.h"
#include "iutils/Thread.h"
#include "StageDescriptor.h"

namespace icamera {

class BufferQueue : public BufferConsumer, public BufferProducer, public EventListener {
 public:
    BufferQueue();
    virtual ~BufferQueue();

    /**
     * \brief the notify when poll one frame buffer
     *
     * Push the CameraBuffer to InputQueue and send a signal if needed
     */
    virtual int onBufferAvailable(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer);

    /**
     * \brief Register the BufferProducer
     *
     * Register the BufferProducer: Psys, software, or captureUnit
     */
    virtual void setBufferProducer(BufferProducer* producer);

    /**
     * \brief Queue one buffer to producer
     *
     * Push this buffer to output queue
     */
    virtual int qbuf(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer);

    /**
     * \brief allocate memory
     *
     * Not support this function in Psys and SWProcessor
     */
    virtual int allocateMemory(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
        return -1;
    }

    /**
     * \brief Add the get frame listener
     */
    virtual void addFrameAvailableListener(BufferConsumer* listener);

    /**
     * \brief Remove the get frame listener
     */
    virtual void removeFrameAvailableListener(BufferConsumer* listener);

    /**
     * \brief Set all frames configuration
     *
     * Must be called before configure which needs use frame configuration.
     */
    virtual void setFrameInfo(const std::map<uuid, stream_t>& inputInfo,
                              const std::map<uuid, stream_t>& outputInfo);

    /*
     * \brief Get all frames configuration
     */
    virtual void getFrameInfo(std::map<uuid, stream_t>& inputInfo,
                              std::map<uuid, stream_t>& outputInfo) const;

 protected:
    /**
     * \brief Clear and initialize input and output buffer queues.
     */
    void clearBufferQueues();
    /**
     * \brief Wait for available input and output buffers.
     *
     * should be called in a threadLoop, Only fetch buffer from the buffer queue, need pop buffer from
     * the queue after the buffer is used, and need to be protected by mBufferQueueLock.
     */
    int waitFreeBuffersInQueue(std::unique_lock<std::mutex>& lock,
                               std::map<uuid, std::shared_ptr<CameraBuffer> >& cInBuffer,
                               std::map<uuid, std::shared_ptr<CameraBuffer> >& cOutBuffer,
                               int64_t timeout = 0);
    int waitFreeBuffersInQueue(std::unique_lock<std::mutex>& lock,
                               std::map<uuid, std::shared_ptr<CameraBuffer> >& buffer,
                               std::map<uuid, CameraBufQ>& bufferQueue, int64_t timeout);
    /**
     * \brief Get available input and output buffers and pop them from buffer queue.
     *
     * should be called in a threadLoop, and need to be protected by mBufferQueueLock.
     */
    int getFreeBuffersInQueue(std::map<uuid, std::shared_ptr<CameraBuffer> >& inBuffers,
                              std::map<uuid, std::shared_ptr<CameraBuffer> >& outBuffers);
    /**
     * \brief Return buffers to producer and consumers
     */
    void returnBuffers(std::map<uuid, std::shared_ptr<CameraBuffer> >& inBuffers,
                       std::map<uuid, std::shared_ptr<CameraBuffer> >& outBuffers);

    static const nsecs_t kWaitDuration = 10000000000;  // 10000ms

    /**
     * \brief Buffers allocation for producer
     */
    int allocProducerBuffers(int camId, int bufNum);

   /**
   * \brief set thread waiting
   */
    void setThreadWaiting(bool waiting);

    BufferProducer* mBufferProducer;
    std::vector<BufferConsumer*> mBufferConsumerList;

    std::map<uuid, stream_t> mInputFrameInfo;
    std::map<uuid, stream_t> mOutputFrameInfo;

    std::map<uuid, CameraBufQ> mInputQueue;
    std::map<uuid, CameraBufQ> mOutputQueue;

    // For internal buffers allocation for producer
    std::map<uuid, CameraBufVector> mInternalBuffers;

    // Guard for BufferQueue public API
    Mutex mBufferQueueLock;
    std::condition_variable mFrameAvailableSignal;

    bool mThreadWaiting; // true if the thread is running, false if stopped

 private:
    int queueInputBuffer(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer);
};

}  // namespace icamera
