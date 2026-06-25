/*
 * Copyright (C) 2022-2024 Intel Corporation
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

/**
 * @file IPipeManager.h
 * @brief IPipeManager, interface for Processing Unit to control stream pipes
 */
#pragma once

#include <map>
#include <set>
#include <unordered_map>

#include "CameraBuffer.h"
#include "CameraEvent.h"
#include "IspSettings.h"
#include "ParamDataType.h"
#include "PlatformData.h"
#include "IPipeStage.h"

namespace icamera {

typedef std::map<uuid, std::shared_ptr<CameraBuffer>> CameraBufferPortMap;

struct PipeTaskData {
    IspSettings mIspSettings;
    TuningMode mTuningMode;
    bool mFakeTask;
    bool mYuvTask;
    bool mCallbackRgbs;

    CameraBufferPortMap mInputBuffers;
    CameraBufferPortMap mOutputBuffers;
    PipeTaskData() {
        mTuningMode = TUNING_MODE_MAX;
        mFakeTask = false;
        mYuvTask = false;
        mCallbackRgbs = false;
    }
};

class PipeManagerCallback {
 public:
    virtual void onTaskDone(const PipeTaskData& result) = 0;
    virtual void onBufferDone(int64_t sequence, uuid port,
                              const std::shared_ptr<CameraBuffer>& camBuffer) = 0;
    virtual void onMetadataReady(int64_t sequence, const CameraBufferPortMap& outBuf) = 0;
    virtual void onStatsReady(EventData& eventData) = 0;
};

// Used to save all on-processing tasks.
struct TaskInfo {
    TaskInfo() : mNumOfValidBuffers(0), mNumOfReturnedBuffers(0), mMetadataDone(false) {}
    PipeTaskData mTaskData;
    int mNumOfValidBuffers;
    int mNumOfReturnedBuffers;
    bool mMetadataDone;
};

// <stream id, control>
typedef std::map<int, StageControl> PipeControl;

class IPipeManager : public EventListener {
 public:
    IPipeManager() {}
    virtual ~IPipeManager() {}
    virtual int configure(const std::map<uuid, stream_t>& inputInfo,
                          const std::map<uuid, stream_t>& outputInfo, ConfigMode configMode,
                          TuningMode tuningMode,
                          const std::map<uuid, stream_t>* yuvInputInfo = nullptr) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;

    virtual void setControl(int64_t sequence, const PipeControl& control) {}

    virtual void addTask(const PipeTaskData& taskParam) = 0;
    virtual int prepareIpuParams(IspSettings* settings, int64_t sequence = 0,
                                 int streamId = VIDEO_STREAM_ID) {
        return OK;
    }

 protected:
    // Use to handle the frame done event from the pipeline.
    virtual int onBufferDone(uuid port, const std::shared_ptr<CameraBuffer>& buffer) = 0;
    virtual void onMetadataReady(int64_t sequence) = 0;
};
}  // namespace icamera
