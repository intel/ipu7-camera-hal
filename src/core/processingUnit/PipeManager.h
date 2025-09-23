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

#pragma once

#include <map>
#include <set>
#include <unordered_map>

#include "CameraBuffer.h"
#include "CameraEvent.h"
#include "IPipeManager.h"
#include "IpuPacAdaptor.h"
#include "PipeLine.h"
#include "PlatformData.h"

namespace icamera {

class PipeManager : public IPipeManager {
 public:
    PipeManager(int cameraId, PipeManagerCallback* callback,
                std::shared_ptr<CameraScheduler> scheduler);
    virtual ~PipeManager();
    virtual int configure(const std::map<uuid, stream_t>& inputInfo,
                          const std::map<uuid, stream_t>& outputInfo,
                          ConfigMode configMode, TuningMode tuningMode,
                          const std::map<uuid, stream_t>* yuvInputInfo = nullptr);
    virtual int start();
    virtual int stop();

    virtual void setControl(int64_t sequence, const PipeControl& control);

    virtual void addTask(PipeTaskData taskParam);
    virtual int prepareIpuParams(IspSettings* settings, int64_t sequence = 0,
                                 int streamId = VIDEO_STREAM_ID);

 protected:
    /**
     * @brief handle bufferDone and metadata event from pipeStage
     */
    virtual void handleEvent(EventData eventData);
    // handle bufferDone
    virtual int onBufferDone(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer);
    // metadata handler
    virtual void onMetadataReady(int64_t sequence);

 public:
    /**
     * The relationship mapping between frameInfo and stage.
     */
    struct PortMapping {
        PortMapping()
                : mPipeStage(nullptr),
                  mExternalPort(INVALID_PORT),
                  mStagePort(INVALID_PORT),
                  mStreamId(0) {}
        IPipeStage* mPipeStage; /* state instance */
        uuid mExternalPort; /* port of the input frame or output frame connected with this stage */
        uuid mStagePort;    /* in or out port of the stage which connected with external frame*/
        int32_t mStreamId;  /* PipeLine stream id this stage belongs to*/
    };

 private:
    int mCameraId;
    std::shared_ptr<CameraScheduler> mScheduler;
    std::map<uuid, stream_t> mInputFrameInfo;
    std::map<uuid, stream_t> mOutputFrameInfo;
    ConfigMode mConfigMode;
    TuningMode mTuningMode;
    uuid mDefaultMainInputPort;
    IpuPacAdaptor* mPacAdaptor;
    std::shared_ptr<GraphConfig> mGraphConfig;
    Mutex mTaskLock;
    // map of sequence and taskinfo
    std::multimap<int64_t, TaskInfo> mOngoingTasks;

    // map of output stream to stream id
    std::map<uuid, int32_t> mOutputPortToStreamId;
    PipeManagerCallback* mPMCallback;
    // pair of streamid and pipeLine
    std::map<int32_t, std::shared_ptr<PipeLine>> mPipeLines;

    std::vector<PortMapping> mInputMaps;
    std::vector<PortMapping> mOutputMaps;

    // pair of streamid and pipeLine for YUV reprocessing
    std::map<int32_t, std::shared_ptr<PipeLine>> mYuvPipeLine;
    std::vector<PortMapping> mYuvInputMaps;
    std::vector<PortMapping> mYuvOutputMaps;

    camera_resolution_t mActivePixels;
    PtzInfo mCurPtz;
    camera_zoom_region_t mZoom;

 private:
    /**
     * @brief create pipeLine object for each stream id.
     *
     * @return ICamera::OK if everything goes ok.
     */
    int createPipeLines(const std::vector<int32_t>& activeStreamIds);
    /**
     * @brief destroy pipeLine objects.
     *
     */
    void releasePipeLines();

    /**
     * @brief analyze the input and output stage connections. link the stages connected in same
     * connection
     */
    int analyzeConnections();

    /**
     * @brief bind the external frame port to the input and output edge stages
     * @param inputEdgeStream: pipeLine's input edge stages
     * @param outputEdgeStream: pipeLine's output edge stages
     */
    int bindExternalPorts(const std::vector<uint32_t>& inputEdgeStream,
                          const std::vector<std::pair<uint32_t, uint32_t> >& outputEdgeStream);
    int bindYuvReprocessingPort(const std::map<uuid, stream_t>& yuvInputInfo);
    bool isSameStreamConfig(const stream_t& internal, const stream_t& external,
                            bool checkStreamId) const;
    /**
     * @brief queue the task to active pipeLines.
     * @return ICamera::OK if everything goes ok.
     */
    void queueBuffers(const PipeTaskData& task, const std::vector<PortMapping>& inputMaps,
                      const std::vector<PortMapping>& outputMaps, uuid inputPort);

    /**
     * @according to the task to get the active stream ids
     * @return ICamera::OK if everything goes ok.
     */
    int getActiveStreamIds(const PipeTaskData& taskData, std::vector<int32_t>* activeStreamIds);

    bool updateZoomSettings(const camera_zoom_region_t& zoom, PtzInfo* ptz);
};
}  // namespace icamera
