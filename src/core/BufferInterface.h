/*
 * Copyright (C) 2025 Intel Corporation.
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

#include "CameraBuffer.h"
#include "CameraEvent.h"

/**
 * These are the abstract Classes for buffer communication between different class of HAL
 */
namespace icamera {

class BufferProducer;

/**
 * BufferConsumer listens on the onBufferAvailable event from the producer by
 * calling setBufferProducer
 */
class BufferConsumer {
 public:
    virtual ~BufferConsumer() {}
    virtual int onBufferAvailable(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) = 0;
    virtual void setBufferProducer(BufferProducer* producer) = 0;
};

/**
 * BufferProcuder get the buffers from consumer by "qbuf".
 * Notify the consumer by calling the onBufferAvailable interface of consumer.
 * The consumer must be registered by "addFrameAvailableListener" before getting
 * any buffer done notification.
 */
class BufferProducer : public EventSource {
 public:
    explicit BufferProducer(int memType = V4L2_MEMORY_USERPTR);
    virtual int qbuf(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) = 0;
    virtual int allocateMemory(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) = 0;
    virtual void addFrameAvailableListener(BufferConsumer* listener) = 0;
    virtual void removeFrameAvailableListener(BufferConsumer* listener) = 0;
    int getMemoryType(void) const { return mMemType; }

 private:
    int mMemType;
};

}  // namespace icamera
