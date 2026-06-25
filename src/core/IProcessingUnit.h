/*
 * Copyright (C) 2022 Intel Corporation.
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
#include <memory>
#include <vector>

#include "BufferQueue.h"

namespace icamera {

class IProcessingUnit : public BufferQueue {
 public:
    IProcessingUnit() : mProcessThread(nullptr), mThreadRunning(false) {}
    virtual ~IProcessingUnit() = default;

    virtual int configure(const std::map<uuid, stream_t>& inputInfo,
                          const std::map<uuid, stream_t>& outputInfo,
                          const ConfigMode configModes) {
        return OK;
    }

    virtual int start() = 0;
    virtual void stop() = 0;
    virtual int registerUserOutputBufs(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
        return OK;
    }

 protected:
    virtual int processNewFrame() = 0;

    /**
     * \brief The process new frame buffer thread
     *
     * Use this thread listen to the input queue and output queue.
     * And do process if these two queues are not empty
     */
    class ProcessThread : public Thread {
        IProcessingUnit* mProcessor;

     public:
        explicit ProcessThread(IProcessingUnit* p) : mProcessor(p) {}
        void run() {
            bool ret = true;
            while (ret) {
                ret = threadLoop();
            }
        }
        virtual bool threadLoop() {
            int ret = mProcessor->processNewFrame();
            return (ret == 0);
        }
    };

    // for the thread loop
    ProcessThread* mProcessThread;
    bool mThreadRunning;  // state of the processor. true after start and false after stop

 private:
    IProcessingUnit(const IProcessingUnit&) = delete;
    IProcessingUnit(IProcessingUnit&&) = delete;
    IProcessingUnit& operator=(const IProcessingUnit&) = delete;
    IProcessingUnit& operator=(IProcessingUnit&&) = delete;
};

}  // namespace icamera
