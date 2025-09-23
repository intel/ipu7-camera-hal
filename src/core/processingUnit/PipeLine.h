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
#include "CameraScheduler.h"
#include "IPipeStage.h"
#include "PlatformData.h"
#include "PSysDevice.h"

namespace icamera {

struct TerminalInfo {
    uuid terminal;
    uuid stageUuid;
    FrameInfo frameDesc;
    bool enabled;
    int usrStreamId;
};

class CBStage;
class PSysDevice;
class IpuPacAdaptor;

struct PipeStageUnit {
    // Initialized during creation/configuration
    int stageId;
    uuid stageUuid;
    CBStage* ipuStage;  // Only for ipu stage
    OuterNode* node;
    uint8_t contextId;  // ContextId in static graph
    uint8_t psysContextId;  // For psys graph
    IPipeStage* pipeStage;

    // Initialized during connection analysis
    std::vector<uuid> inputTerminals;  // including disabled terminals
    std::vector<uuid> outputTerminals;
    bool isInputEdge;
    bool isOutputEdge;

    PipeStageUnit() {
        stageId = -1;
        stageUuid = INVALID_PORT;
        ipuStage = nullptr;
        node = nullptr;
        contextId =  uint8_t(-1);
        psysContextId =  uint8_t(-1);
        pipeStage= nullptr;
        isInputEdge = false;
        isOutputEdge = false;
    }
};

class PipeLine {
 public:
    PipeLine(int cameraId, int streamId, std::shared_ptr<GraphConfig> gc,
             std::shared_ptr<CameraScheduler> scheduler);
    virtual ~PipeLine();

    void deinit();
    int configure(TuningMode tuningMode, IpuPacAdaptor* adaptor);
    int start();
    int stop();

    void setControl(int64_t sequence, const StageControl& control);

    IPipeStage* getInput();
    std::vector<IPipeStage*> getOutput();
    std::vector<IGraphType::PipelineConnection>& getEdgeConnections() { return mEdgeConnections; }
    void registerListener(EventType eventType, EventListener* enventListener);
    void removeListener(EventType eventType, EventListener* enventListener);

    status_t updateConfigurationSettingForPtz(bool isKeyResChanged);

 private:
    void releasePipeStage();

    status_t createPipeStages();
    status_t configurePipeStages();
    status_t createPSysGraph(int32_t numLinks, GraphLink** links);

    void dumpPSysGraph();

    PipeStageUnit* findPipeStage(uuid stageUuid);
    void storeTerminalInfo(const IGraphType::PipelineConnection& connection);
    void analyzeConnections(const std::vector<IGraphType::PipelineConnection>& connVector);

    IPipeStage* findStageProducer(const PipeStageUnit& psUnit);
    std::vector<PipeStageUnit*> findStageConsumer(const PipeStageUnit& psUnit);
    std::map<uuid, uuid> getConsumerPortMap(const PipeStageUnit* psUnitConsumer);
    void linkPipeStages();

    void setFrameInfoForPipeStage();

    void getTerminalFrameInfos(const std::vector<uuid>& terminals,
                               std::map<uuid, FrameInfo>& infoMap) const;

    void updateIspTuningMode(TuningMode tuningMode);
    void dumpPipeStages() const;

 private:
    int mCameraId;
    int mStreamId;
    std::shared_ptr<GraphConfig> mGraphConfig;
    std::vector<PipeStageUnit> mPSUnit;
    std::vector<IGraphType::PipelineConnection> mEdgeConnections;

    std::shared_ptr<CameraScheduler> mScheduler;

    // For internal connections (between pipe stages)
    std::map<uuid, uuid> mSinkMapSource;  // <sink, source>
    std::map<uuid, uuid> mSourceMapSink;  // <source, sink>
    // for IPU6, there will be a directly map for IPU7 port uuid to stage
    std::map<uuid, uuid> mTerminalStage;  // <terminalId, stageId>
    std::map<uuid, TerminalInfo> mTerminalsDesc;

    TuningMode mTuningMode;

    PSysDevice* mPSysDevice;
    PSysGraph mPSysGraph;
    IpuPacAdaptor* mPacAdaptor;
};

}  // namespace icamera
