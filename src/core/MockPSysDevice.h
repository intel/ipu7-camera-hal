/*
 * Copyright (C) 2024-2025 Intel Corporation.
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

#include <unistd.h>

#include <list>
#include <map>
#include <mutex>
#include <set>
#include <unordered_map>

#include "Errors.h"
#include "FileSource.h"
#include "PSysDevice.h"
#include "iutils/Thread.h"
#include "modules/ipu_desc/ipu-psys.h"

namespace icamera {

/**
 * PSYS uAPI Mock
 */
class MockPSysDevice : public PSysDevice {
 public:
    explicit MockPSysDevice(int cameraId);
    virtual ~MockPSysDevice();

    virtual int init() override;
    virtual void deinit() override {}
    virtual void registerPSysDeviceCallback(uint8_t contextId,
                                            IPSysDeviceCallback* callback) override;

    virtual int addGraph(const PSysGraph& graph) override { return OK; }
    virtual int closeGraph() override { return OK; }

    virtual int addTask(const PSysTask& task) override;

    virtual int registerBuffer(TerminalBuffer* buf) override {
        buf->psysBuf.base.fd = ++mFd;
        return OK;
    }
    virtual void unregisterBuffer(TerminalBuffer* buf) override {}

    virtual int poll() override;

 private:
    static const int kStartingFrameCount = 20;
    icamera::FileSourceFromDir* mFileSource;
    const char* PNP_INJECTION_NAME = "/data/cameraPnp/libcamera";

    PollThread<MockPSysDevice>* mPollThread;
    std::condition_variable mTaskReadyCondition;
    bool mExitPending = false;

    int mFd = 0;
    std::mutex mDataLock;
    std::unordered_map<uint8_t, IPSysDeviceCallback*> mPSysDeviceCallbackMap;
    std::map<int64_t, std::set<uint8_t>> mTasksMap;
}; /* MockPSysDevice */

} /* namespace icamera */
