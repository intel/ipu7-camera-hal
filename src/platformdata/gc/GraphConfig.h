/*
 * Copyright (C) 2019-2025 Intel Corporation
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

#include <memory>
#include <utility>
#include <vector>
#include <map>

#include "HalStream.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "IGraphType.h"
#include "iutils/Thread.h"

#include "GraphResolutionConfigurator.h"

namespace icamera {

/**
 * \class GraphConfig
 *
 * \brief This is a wrapper of GraphConfigImpl class and it provides the
 * public APIs to get the graph config data.
 *
 * It maintains one static area and GraphConfigImpl object, user get graph
 * config data from the local structure or GraphConfigImpl object through
 * the public APIs
 */
class GraphConfig {
 public:
    GraphConfig();
    GraphConfig(int32_t camId, ConfigMode mode);
    ~GraphConfig();
    void releaseGraphNodes();

    // These public methods called by GraphConfigManager
    status_t configStreams(const std::vector<HalStream*>& halStreams,
                           const std::vector<HalStream*>& extraOutStreams);

    // fullPipes: workaround for IspParamAdaptor/AiqUnit in ipu6.
    // In ipu7 YUV reprocessing pipe (cpu pipe) is added, the caller should handle it.
    status_t graphGetStreamIds(std::vector<int32_t>& streamIds, bool fullPipes = true);
    status_t graphGetEdgeConnections(
                         std::vector<IGraphType::PipelineConnection> *confVector);

    status_t parse(int cameraId, const char* settingsXmlFile);

    int32_t getGraphId(void);
    status_t getStagesByStreamId(int32_t streamId, std::map<int32_t, std::string>* stages);
    // return <contextId, outerNode>
    status_t getOuterNodes(int32_t streamId, std::map<int32_t, OuterNode*>& outerNodes);
    int32_t getGraphLinks(int32_t streamId, GraphLink*** links);

    status_t updateGraphSettingForPtz(const PtzInfo& cur, const PtzInfo& prev,
                                      bool* isKeyResChanged);
    status_t getIspRawCropInfo(IspRawCropInfo& info);
    status_t getIspTuningModeByStreamId(int32_t streamId, uint32_t& ispTuningMode);

    // Not support the below API in ipu7, keep them only for ipu7 build
    int getSelectedMcId() { return -1; }
    void getCSIOutputResolution(camera_resolution_t &reso) {}
    status_t getGdcKernelSetting(uint32_t *kernelId,
                                 ia_isp_bxt_resolution_info_t *resolution) {
        return OK;
    }

    uint8_t getPSysContextId(int32_t streamId, uint8_t outerNodeCtxId);

    status_t getPgNames(std::vector<std::string>* pgNames) { return OK; }
    status_t getPgNamesByStreamId(int32_t streamId, std::vector<std::string>* pgNames) {
        return OK;
    }
    status_t pipelineGetConnections(
                         const std::vector<std::string> &pgList,
                         std::vector<IGraphType::PipelineConnection> *confVector,
                         std::vector<IGraphType::PrivPortFormat> *tnrPortFormat = nullptr) {
        return OK;
    }

    std::map<int32_t, int32_t> getStreamIdToPipeId() {
        return mStreamIdToPipeId;
    }

    StageType getPGType(int32_t pgId);

    status_t pipelineGetConnections(int32_t streamId,
        std::vector<IGraphType::PipelineConnection> *confVector,
        std::vector<IGraphType::PrivPortFormat> *tnrPortFormat = nullptr);

 private:
    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfig);

 private:
    struct PostStageInfo {
        PostStageInfo() : stageId(-1), streamId(-1), enabled(false) {}
        std::string stageName;
        int32_t stageId;
        int32_t streamId;  // pipe stream id
        HalStream inputStream;
        std::vector<const HalStream*> outputStreams;
        bool enabled;
    };

    struct IpuStageInfo {
        IpuStageInfo() : streamId(0), stageId(-1), node(nullptr) {}
        int32_t streamId;
        std::string stageName;
        int32_t stageId;
        OuterNode* node;
    };

    struct IpuGraphLink {
        IpuGraphLink(int32_t streamId, const GraphLink* link)
            : streamId(streamId),
              graphLink(link),
              isEdge(false),
              stream(nullptr) {}
        int32_t streamId;  // pipe stream id
        const GraphLink* graphLink;
        bool isEdge;
        const HalStream* stream;  // valid if the sink is for user stream
    };

    struct StaticGraphInfo {
        StaticGraphInfo() : staticGraph(nullptr), graphResolutionConfig(nullptr) {}
        void clear() {
            if (graphResolutionConfig) {
                delete graphResolutionConfig;
            }
            graphResolutionConfig = nullptr;
            if (staticGraph) {
                delete staticGraph;
            }
            staticGraph = nullptr;
        }
        IStaticGraphConfig* staticGraph;
        std::vector<IpuGraphLink> links;
        // ISYS contextId is 0 in static graph, but psys cb context id should start at 0 (in FW).
        // W/A: save in context id order, to recode context id (index) for psys cbs.
        std::vector<IpuStageInfo> stageInfos;
        // Zoom support
        GraphResolutionConfigurator* graphResolutionConfig;
    };

 private:
    int32_t loadPipeConfig(const std::string& fileName);
    int32_t loadStaticGraphConfig(const std::string& name);
    void getStaticGraphConfigData(const std::map<VirtualSink, const HalStream*>& streams);

    void saveOuterNode(const GraphLink* link, StaticGraphInfo* graph);
    void saveLink(int32_t streamId, const GraphLink* link,
                  std::map<HwSink, const HalStream*>* streams, StaticGraphInfo* graph);

    status_t fillConnectionFormat(const IpuGraphLink& ipuLink, const OuterNode* node,
                                  IGraphType::PortFormatSettings* fmtSettings);
    void fillConnectionConfig(const IpuGraphLink& ipuLink, int32_t terminalId,
                              IGraphType::ConnectionConfig* conn);

    OuterNode* findFrameTerminalOwner(const GraphLink* link);
    const StaticGraphRunKernel* findKernelForFrameTerminal(const OuterNode* node,
                                                           int32_t terminalId);

    uint32_t createQueryKeyAttribute(int cameraId);
    status_t queryGraphSettings(const std::vector<HalStream*>& outStreams);

    status_t createPipeGraphConfigData(const std::vector<HalStream*>& outStreams,
                                       const std::vector<int>& outStreamIpuFlags,
                                       const HalStream* inStream);

    void chooseIpuOutputStreams(const std::vector<HalStream*>& halStreams,
                                std::vector<int>* ipuStreamFlags);
    void chooseIpuStreams(std::map<int, const HalStream*>& streams, int avaSlot,
                          std::vector<int>* ipuStreamFlags);

    void createPostStages(const std::vector<HalStream*>& outStreams,
                          const std::vector<int>& outStreamIpuFlags,
                          const HalStream* inStream);
    void fillOutputToPostProcessor(int32_t ipuStreamId, const HalStream* stream);
    void checkAndUpdatePostConnection(int32_t streamId, IGraphType::PipelineConnection* conn,
                                      std::vector<IGraphType::PipelineConnection>* postVector,
                                      std::map<int32_t, PostStageInfo>& postStageInfos);
    void dumpPostStageInfo();
    void dumpLink(const GraphLink* link);
    void dumpLink(const IpuGraphLink& ipuLink);
    void dumpNodes(const StaticGraphInfo& graph);

 private:
    int32_t mCameraId;

    // <camera id, binary>
    // TODO: Save different bin data (depends on use case, ...) for one camera?
    static Mutex sLock;
    static std::map<int32_t, StaticReaderBinaryData> mGraphConfigBinaries;

    StaticGraphReader mGraphReader;

    // <stream id, graph>
    std::map<int32_t, StaticGraphInfo> mStaticGraphs;
    float mSensorRatio;
    // <HalStream id, info>, HalStream could be user input stream or output stream (use ipu output)
    std::map<int32_t, PostStageInfo> mPostStageInfos;
    // GPU post stage, currently use same format as IPU post stage
    std::map<int32_t, PostStageInfo> mGPUStageInfos;

    // <stream id, pipe streamId>
    std::map<int32_t, int32_t> mStreamIdToPipeId;
};

}  // namespace icamera
