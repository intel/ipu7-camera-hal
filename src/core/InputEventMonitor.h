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

#ifndef INPUT_EVENT_MONITOR_H
#define INPUT_EVENT_MONITOR_H

#include "CameraEvent.h"
#include "iutils/Thread.h"

#include <string>
#include <linux/input.h>

namespace icamera {

class InputEventMonitor : public EventSource {
 public:
    InputEventMonitor();
    virtual ~InputEventMonitor();
    virtual int configure(int eventType, int eventCode);
    virtual int start();
    virtual int stop();
    virtual int getValue();
    virtual bool isConfigured() const;

 private:
    bool check();
    bool checkDeviceCapabilities(const std::string &device, const int eventType, const int eventCode);
    int readRawValue();

 private:
    class WorkThread : public Thread {
        InputEventMonitor* mInputEventMonitor;

     public:
        explicit WorkThread(InputEventMonitor* _s) : mInputEventMonitor(_s) {}

        virtual void run() {
            bool ret = true;
            while (ret) {
                ret = threadLoop();
            }
        }

     private:
        virtual bool threadLoop() { return mInputEventMonitor->check(); }
    };

    WorkThread* mCheckThread;
    Mutex mLock;
    int32_t mEventType;
    int32_t mEventCode;
    uint32_t mEventIoctlModesIndex;
    int32_t mFd;
    int32_t mValue;
};

}  // namespace icamera

#endif // INPUT_EVENT_MONITOR_H
