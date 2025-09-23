/*
 * Copyright (C) 2024 Intel Corporation.
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

#define LOG_TAG MockPSysDevice

#include "MockPSysDevice.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CameraLog.h"
#include "Errors.h"

namespace icamera {

MockPSysDevice::MockPSysDevice(int cameraId) : PSysDevice(cameraId) {
    mPollThread = new PollThread<MockPSysDevice>(this);
    mFileSource = new icamera::FileSourceFromDir(PNP_INJECTION_NAME);
}

MockPSysDevice::~MockPSysDevice() {
    mExitPending = true;
    mPollThread->exit();
    mTaskReadyCondition.notify_one();
    mPollThread->wait();

    delete mPollThread;
    delete mFileSource;
}

int MockPSysDevice::init() {
    mPollThread->start();
    return OK;
}

void MockPSysDevice::registerPSysDeviceCallback(uint8_t contextId, IPSysDeviceCallback* callback) {
    mPSysDeviceCallbackMap[contextId] = callback;
}

int MockPSysDevice::addTask(const PSysTask& task) {
    for (const auto& item : task.terminalBuffers) {
        if (task.sequence < kStartingFrameCount && item.second.handle > 0) {
            char* addr =
                reinterpret_cast<char*>(::mmap(nullptr, item.second.size, PROT_READ | PROT_WRITE,
                                               MAP_SHARED, item.second.handle, 0));
            if (mFileSource) {
                mFileSource->fillFrameBuffer(addr, item.second.size, task.sequence);
            } else {
                memset(addr, 0x99, item.second.size);
            }
            munmap(addr, item.second.size);
        }
    }

    {
        std::unique_lock<std::mutex> lock(mDataLock);
        mTasksMap[task.sequence].insert(task.nodeCtxId);

        mTaskReadyCondition.notify_one();
    }
    return OK;
}

int MockPSysDevice::poll() {
    if (mExitPending) {
        return -1;
    }

    auto task = mTasksMap.begin();
    {
        std::unique_lock<std::mutex> lock(mDataLock);
        if (mTasksMap.empty()) {
            const std::cv_status ret =
                mTaskReadyCondition.wait_for(lock, std::chrono::nanoseconds(2000000000));
            if (mTasksMap.empty() || (ret == std::cv_status::timeout)) {
                return 0;
            }
        }
        task = mTasksMap.begin();
    }

    auto sec = task->second.begin();
    LOG2("%s, task.nodeCtxId %u, task.sequence %ld", __func__, *sec, task->first);
    mPSysDeviceCallbackMap[*sec]->bufferDone(task->first);

    {
        std::unique_lock<std::mutex> lock(mDataLock);
        task->second.erase(sec);
        if (task->second.empty()) {
            mTasksMap.erase(task);
        }
    }

    return 0;
}

}  // namespace icamera
