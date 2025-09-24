/*
 * Copyright (C) 2022-2025 Intel Corporation
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

#define LOG_TAG PipeLine

#include "src/core/processingUnit/PipeLine.h"
#include "PSysDevice.h"
#include "CBStage.h"
#include "IpuPacAdaptor.h"
#include "GraphUtils.h"
#include "iutils/CameraLog.h"
#include "StageDescriptor.h"
namespace icamera {

PipeLine::PipeLine(int cameraId, int streamId, std::shared_ptr<GraphConfig> gc,
                   std::shared_ptr<CameraScheduler> scheduler)
        : mCameraId(cameraId),
          mStreamId(streamId),
          mGraphConfig(gc),
          mScheduler(scheduler),
          mPSysDevice(nullptr),
          mPacAdaptor(nullptr) {
    LOG1("<id%d>@%s stream %d", mCameraId, __func__, mStreamId);
}

PipeLine::~PipeLine() {
    LOG1("<id%d>@%s stream %d", mCameraId, __func__, mStreamId);

    releasePipeStage();

    if (mPacAdaptor) {
        mPacAdaptor->reinitAic(mStreamId);
    }
}

void PipeLine::deinit() {
    if (mPSysDevice != nullptr) {
        mPSysDevice->deinit();
    }
}

void PipeLine::updateIspTuningMode(TuningMode tuningMode) {
    if (!PlatformData::supportUpdateTuning(mCameraId)) {
        return;
    }

    uint32_t ispTuningMode = 0;
    int ret = mGraphConfig->getIspTuningModeByStreamId(mStreamId, ispTuningMode);
    CheckAndLogError(ret != OK, VOID_VALUE, "Failed to get tuning mode");

    uint32_t aiqMode = IA_MKN_CHTOUL('D', 'F', 'L', 'T');
    ia_lard_input_params lardParam = {
        IA_MKN_CHTOUL('D', 'F', 'L', 'T'),
        aiqMode,
        ispTuningMode,
        IA_MKN_CHTOUL('D', 'F', 'L', 'T'),
    };
    cca::cca_nvm tmpNvm = {};

    IntelCca* intelCca = IntelCca::getInstance(mCameraId, tuningMode);
    CheckAndLogError(!intelCca, VOID_VALUE, "Failed to get IntelCca");

    ia_err iaErr = intelCca->updateTuning(cca::CCA_LARD_ISP, lardParam, tmpNvm, mStreamId);
    CheckAndLogError(iaErr != ia_err_none, VOID_VALUE, "Failed to update tuning %u",
                     ispTuningMode);

    LOG1("Update isp tuning mode %u, streamId %d", ispTuningMode, mStreamId);
}

int PipeLine::configure(TuningMode tuningMode, IpuPacAdaptor* adaptor) {
    LOG1("<id%d>@%s, tuningMode:%d", mCameraId, __func__, tuningMode);
    CheckAndLogError(!adaptor, UNKNOWN_ERROR, "%s: nullptr adaptor", __func__);

    releasePipeStage();
    mTuningMode = tuningMode;
    mPacAdaptor = adaptor;

    updateIspTuningMode(tuningMode);

    int ret = createPipeStages();
    CheckAndLogError(ret != OK, ret, "%s, Create pipeStages failed about stream %d", __func__,
                     mStreamId);

    // Get pipeStage connection and analyze connection
    std::vector<IGraphType::PipelineConnection> connVector;
    std::vector<IGraphType::PrivPortFormat> tnrPortFormat;
    ret = mGraphConfig->pipelineGetConnections(mStreamId, &connVector, &tnrPortFormat);
    CheckAndLogError(ret != OK, ret, "%s, Get pipeLine connections failed about stream %d",
                     __func__, mStreamId);
    analyzeConnections(connVector);

    // link internal PipeStage and bind internal terminals.
    linkPipeStages();

    setFrameInfoForPipeStage();

    ret = configurePipeStages();
    CheckAndLogError(ret != OK, ret, "Failed to configure connections on stream %d, ret = %d",
                     mStreamId, ret);

    dumpPipeStages();

    return OK;
}

int PipeLine::start() {
    LOG1("<id%d>@%s stream %d", mCameraId, __func__, mStreamId);

    for (auto& unit : mPSUnit) {
        int ret = unit.pipeStage->start();
        CheckAndLogError(ret != OK, ret, "%s, pipe stage %s start fails.", __func__,
                         unit.pipeStage->getName());
    }

    return mPSysDevice->addGraph(mPSysGraph);
}

int PipeLine::stop() {
    LOG1("<id%d>@%s stream %d", mCameraId, __func__, mStreamId);

    mPSysDevice->closeGraph();

    for (auto& unit : mPSUnit) {
        int ret = unit.pipeStage->stop();
        CheckAndLogError(ret != OK, ret, "%s, pipe stage %s stop fails.", __func__,
                         unit.pipeStage->getName());
    }

    return OK;
}

void PipeLine::setControl(int64_t sequence, const StageControl& control) {
    for (auto& unit : mPSUnit) {
        unit.pipeStage->setControl(sequence, control);
    }
}

// It supports that one pipe has only one input pipeStage now
IPipeStage* PipeLine::getInput() {
    for (auto& unit : mPSUnit) {
        if (unit.isInputEdge) {
            return unit.pipeStage;
        }
    }
    return nullptr;
}

std::vector<IPipeStage*> PipeLine::getOutput() {
    std::vector<IPipeStage*> outPutStage;
    for (auto& unit : mPSUnit) {
        if (unit.isOutputEdge) {
            outPutStage.push_back(unit.pipeStage);
        }
    }
    LOG1("@%s stream %d, size:%lu", __func__, mStreamId, outPutStage.size());

    return outPutStage;
}

void PipeLine::registerListener(EventType eventType, EventListener* enventListener) {
    for (auto& unit : mPSUnit) {
        if (eventType == EVENT_STAGE_BUF_READY && unit.isOutputEdge) {
            unit.pipeStage->registerListener(eventType, enventListener);
        }
        if (eventType == EVENT_PSYS_STATS_SIS_BUF_READY ||
            eventType == EVENT_PSYS_STATS_BUF_READY) {
            unit.pipeStage->registerListener(eventType, enventListener);
        }
    }
}

void PipeLine::removeListener(EventType eventType, EventListener* enventListener) {
    for (auto& unit : mPSUnit) {
        if (eventType == EVENT_STAGE_BUF_READY && unit.isOutputEdge) {
            unit.pipeStage->removeListener(eventType, enventListener);
        }
        if (eventType == EVENT_PSYS_STATS_SIS_BUF_READY ||
            eventType == EVENT_PSYS_STATS_BUF_READY) {
            unit.pipeStage->removeListener(eventType, enventListener);
        }
    }
}

void PipeLine::releasePipeStage() {
    for (auto& unit : mPSUnit) {
        if (unit.ipuStage) {
            unit.ipuStage->deInit();
        }
        if (unit.pipeStage) {
            delete unit.pipeStage;
        }
    }
    if (mPSysDevice) {
        delete mPSysDevice;
        mPSysDevice = nullptr;
    }
    mPSUnit.clear();
}

status_t PipeLine::createPipeStages() {
    std::map<int32_t, std::string> stages;
    status_t ret = mGraphConfig->getStagesByStreamId(mStreamId, &stages);
    CheckAndLogError(ret != OK, ret, "%s: Get pipeStages from graph failed", __func__);

    if (mPSysDevice) {
        delete mPSysDevice;
    }
    mPSysDevice = new PSysDevice(mCameraId);

    ret = mPSysDevice->init();
    CheckAndLogError(ret != OK, ret, "%s: failed to initialize psys device", __func__);

    std::map<int32_t, OuterNode*> outerNodes;
    ret = mGraphConfig->getOuterNodes(mStreamId, outerNodes);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Get outer node fail for stream %d", mStreamId);

    for (auto& stage : stages) {
        int32_t stageId = stage.first;
        std::string& stageName = stage.second;

        PipeStageUnit unit;
        unit.stageId = stageId;
        unit.stageUuid = STAGE_UID(mStreamId, stageId);

        const StageType type = mGraphConfig->getPGType(stageId);
        if (type == STAGE_IPU) {
            unit.contextId = GraphUtils::getContextId(stageId);
            CheckAndLogError(outerNodes.find(unit.contextId) == outerNodes.end(), UNKNOWN_ERROR,
                             "No outer node for psUnit contextId %d", unit.contextId);
            unit.psysContextId = mGraphConfig->getPSysContextId(mStreamId, unit.contextId);
            unit.node = outerNodes[unit.contextId];
            uint8_t resourceId = GraphUtils::getResourceId(stageId);
            unit.ipuStage = new CBStage(mCameraId, mStreamId, stageId, unit.contextId,
                                        unit.psysContextId, resourceId, stageName, mPSysDevice,
                                        mPacAdaptor);
            unit.pipeStage = unit.ipuStage;
        } else {
            LOGE("Not support stage type %d", type);
            return UNKNOWN_ERROR;
        }

        mPSUnit.push_back(unit);
        LOG1("%s, pipe stage name:%s, stage:%d, uuid:%x, context id %u", __func__,
             stageName.c_str(), stageId, unit.stageUuid, unit.contextId);

        if (unit.ipuStage) {
            ret = unit.ipuStage->init();
            CheckAndLogError(ret != OK, UNKNOWN_ERROR, "init stage %d error", stageId);
        }

        if (mScheduler) {
            mScheduler->registerNode(unit.pipeStage);
        }
    }

    return OK;
}

status_t PipeLine::configurePipeStages() {
    GraphLink** links = nullptr;
    int32_t numLinks = mGraphConfig->getGraphLinks(mStreamId, &links);
    CheckAndLogError(numLinks <= 0, UNKNOWN_ERROR, "%s:%d get links fail", __func__, mStreamId);

    status_t ret = createPSysGraph(numLinks, links);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s:%d create PSysGraph fail", __func__, mStreamId);

    for (auto& unit : mPSUnit) {
        if (!unit.ipuStage) {
            continue;
        }

        PSysNode* pNode = nullptr;
        for (auto& node : mPSysGraph.nodes) {
            if (node.nodeCtxId == unit.psysContextId) {
                pNode = &node;
                break;
            }
        }
        CheckAndLogError(!pNode, UNKNOWN_ERROR, "No PSys node for psys contextId %d",
                         unit.psysContextId);

        std::unordered_map<uint8_t, TerminalConfig> terminalConfig;
        ret = unit.ipuStage->configure(unit.node->nodeKernels,
                                       const_cast<const GraphLink**>(links), numLinks,
                                       pNode->terminalConfig);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Configure ipuStage %x fail", unit.stageId);

        // Save terminalConfig (for payload terminals) to PSysGraph
        for (auto& node : mPSysGraph.nodes) {
            if (node.nodeCtxId == unit.contextId) {
                node.terminalConfig.insert(terminalConfig.begin(), terminalConfig.end());
            }
        }
    }

    dumpPSysGraph();

    return OK;
}

status_t PipeLine::updateConfigurationSettingForPtz(bool isKeyResChanged) {
    cca::cca_aic_config aicConfig{};

    int32_t cbNum = 0;
    for (auto& unit : mPSUnit) {
        if (!unit.ipuStage) {
            continue;
        }

        cca::cca_cb_config& cb = aicConfig.cb_config[cbNum];
        cb.group_id = static_cast<int32_t>(unit.psysContextId);
        cb.kernel_group = const_cast<aic::ImagingKernelGroup*>(&unit.node->nodeKernels);
        cbNum++;
    }
    aicConfig.cb_num = cbNum;

    return mPacAdaptor->updateResolutionSettings(mStreamId, aicConfig, isKeyResChanged);
}

status_t PipeLine::createPSysGraph(int32_t numLinks, GraphLink** links) {
    mPSysGraph.links.clear();

    std::map<uint8_t, PSysNode> nodes;  // <contextId, node>
    for (int32_t i = 0; i < numLinks; i++) {
        const GraphLink* link = links[i];
        if (!link->isActive) {
            continue;
        }

        // Save CB nodes
        TerminalConfig cfg;
        for (int i = 0 ; i < 2; i++) {
            OuterNode* node = (i == 0) ? link->srcNode : link->destNode;
            if (!node || node->type == NodeTypes::Isys) {
                continue;
            }

            uint8_t termId = (i == 0) ? link->srcTerminalId : link->destTerminalId;
            PSysNode& pNode = nodes[node->contextId];  // Create empty node for the 1st access
            if (pNode.terminalConfig.empty()) {
                // Init for the 1st access
                pNode.nodeCtxId = mGraphConfig->getPSysContextId(mStreamId, node->contextId);
                pNode.nodeRsrcId = node->resourceId;
                MEMCPY_S(pNode.bitmaps.teb, sizeof(pNode.bitmaps.teb),
                         node->bitmaps.teb, sizeof(node->bitmaps.teb));
                MEMCPY_S(pNode.bitmaps.deb, sizeof(pNode.bitmaps.deb),
                         node->bitmaps.deb, sizeof(node->bitmaps.deb));
                MEMCPY_S(pNode.bitmaps.rbm, sizeof(pNode.bitmaps.rbm),
                         node->bitmaps.rbm, sizeof(node->bitmaps.rbm));
                MEMCPY_S(pNode.bitmaps.reb, sizeof(pNode.bitmaps.reb),
                         node->bitmaps.reb, sizeof(node->bitmaps.reb));
            }

            if (link->linkConfiguration) {
                cfg.payloadSize = link->linkConfiguration->bufferSize;
                pNode.terminalConfig[termId] = cfg;
            }
        }

        // Save links between CBs
        if (link->type != LinkType::Node2Node && link->type != LinkType::Node2Self) {
            continue;
        }
        if (link->srcNode->type == NodeTypes::Isys) {
            continue;
        }

        PSysLink pLink = {};
        pLink.srcNodeCtxId = mGraphConfig->getPSysContextId(mStreamId, link->srcNode->contextId);
        pLink.srcTermId = link->srcTerminalId;
        pLink.dstNodeCtxId = mGraphConfig->getPSysContextId(mStreamId,
                                                            link->destNode->contextId);
        pLink.dstTermId = link->destTerminalId;
        pLink.delayedLink = link->frameDelay;
        if (link->linkConfiguration)
            pLink.streamingMode = link->linkConfiguration->streamingMode;

#ifndef IPU_SYSVER_ipu8
        if ((mStreamId == VIDEO_STREAM_ID) && (link->dest == GraphElementType::BbpsWithTnr)) {
            LOG2("This video pipeline has enabled IPU TNR feature.");
        }
#endif
        mPSysGraph.links.push_back(pLink);
    }

    mPSysGraph.nodes.clear();
    for (auto& node : nodes) {
        mPSysGraph.nodes.push_back(node.second);
    }

    return OK;
}

void PipeLine::dumpPSysGraph() {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(PipeLine)) ||
        !Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_LEVEL3)) {
        return;
    }

    LOG3("Dump psys graph link for stream %d", mStreamId);
    for (auto& link : mPSysGraph.links)
        LOG3("    %u:%u -> %u:%u, delay %d, streamingMode %d", link.srcNodeCtxId, link.srcTermId,
             link.dstNodeCtxId, link.dstTermId, link.delayedLink, link.streamingMode);

    LOG3("Dump psys graph node for stream %d", mStreamId);
    for (auto& node : mPSysGraph.nodes) {
        LOG3("    c%d:r%u, has %zu terminals", node.nodeCtxId, node.nodeRsrcId,
             node.terminalConfig.size());
        LOG3("             ted %x%x, deb %x%x%x%x", node.bitmaps.teb[1], node.bitmaps.teb[0],
             node.bitmaps.deb[3], node.bitmaps.deb[2], node.bitmaps.deb[1], node.bitmaps.deb[0]);
        LOG3("             rbm %x%x%x%x, reb %x%x%x%x", node.bitmaps.rbm[3], node.bitmaps.rbm[2],
             node.bitmaps.rbm[1], node.bitmaps.rbm[0], node.bitmaps.reb[3], node.bitmaps.reb[2],
             node.bitmaps.reb[1], node.bitmaps.reb[0]);

        for (auto& term : node.terminalConfig)
            LOG3("        term %u, size %d", term.first, term.second.payloadSize);
    }
}

PipeStageUnit* PipeLine::findPipeStage(uuid stageUuid) {
    for (unsigned int i = 0; i < mPSUnit.size(); i++) {
        if (mPSUnit[i].stageUuid == stageUuid) {
            return &mPSUnit[i];
        }
    }
    return nullptr;
}

void PipeLine::storeTerminalInfo(const IGraphType::PipelineConnection& connection) {
    TerminalInfo desc;
    desc.terminal = 0;
    desc.stageUuid = 0;
    desc.frameDesc.mWidth = connection.portFormatSettings.width;
    desc.frameDesc.mHeight = connection.portFormatSettings.height;
    desc.frameDesc.mFormat = connection.portFormatSettings.format;
    desc.enabled = true;
    desc.usrStreamId = connection.stream ? connection.stream->streamId() : -1;

    const IGraphType::ConnectionConfig& connCfg = connection.connectionConfig;
    if (connection.portFormatSettings.enabled) {
        mSinkMapSource[connCfg.mSinkTerminal] = connCfg.mSourceTerminal;
        mSourceMapSink[connCfg.mSourceTerminal] = connCfg.mSinkTerminal;
    }

    // Check if there is new input terminal
    if ((connCfg.mSinkStage != INVALID_PORT) && (connCfg.mSinkStage != connCfg.mSourceStage) &&
        mTerminalsDesc.find(connCfg.mSinkTerminal) == mTerminalsDesc.end()) {
        PipeStageUnit* unit = findPipeStage(connCfg.mSinkStage);
        if (unit) {
            desc.terminal = connCfg.mSinkTerminal;
            desc.stageUuid = connCfg.mSinkStage;
            mTerminalsDesc[desc.terminal] = desc;
            unit->inputTerminals.push_back(desc.terminal);
        }
    }

    // Check if there is new output terminal
    if ((connCfg.mSourceStage != INVALID_PORT) && (connCfg.mSinkStage != connCfg.mSourceStage) &&
        mTerminalsDesc.find(connCfg.mSourceTerminal) == mTerminalsDesc.end()) {
        PipeStageUnit* unit = findPipeStage(connCfg.mSourceStage);
        if (unit) {
            desc.terminal = connCfg.mSourceTerminal;
            desc.stageUuid = connCfg.mSourceStage;
            mTerminalsDesc[desc.terminal] = desc;
            unit->outputTerminals.push_back(desc.terminal);
        }
    }

    if (mTerminalsDesc.find(connection.portFormatSettings.terminalId) != mTerminalsDesc.end()) {
        mTerminalsDesc[connection.portFormatSettings.terminalId].enabled =
            connection.portFormatSettings.enabled;
    }
}

void PipeLine::analyzeConnections(const std::vector<IGraphType::PipelineConnection>& connVector) {
    for (auto const& connection : connVector) {
        LOG2("streamId:%d: terminal %x (%d): %dx%d, 0x%x(%s)", mStreamId,
             connection.portFormatSettings.terminalId, connection.portFormatSettings.enabled,
             connection.portFormatSettings.width, connection.portFormatSettings.height,
             connection.portFormatSettings.fourcc,
             CameraUtils::fourcc2String(connection.portFormatSettings.fourcc).c_str());
        LOG2("streamId:%d:     connection source %x, %x, %d, has edge %d", mStreamId,
             connection.connectionConfig.mSourceStage, connection.connectionConfig.mSourceTerminal,
             connection.connectionConfig.mSourceIteration, connection.hasEdgePort);
        LOG2("streamId:%d:     connection sink %x, %x, %d, type %d", mStreamId,
             connection.connectionConfig.mSinkStage, connection.connectionConfig.mSinkTerminal,
             connection.connectionConfig.mSinkIteration,
             connection.connectionConfig.mConnectionType);

        storeTerminalInfo(connection);

        if (connection.portFormatSettings.enabled == 0) {
            // No actions are needed for the disabled connections.
            continue;
        }

        mTerminalStage[connection.connectionConfig.mSourceTerminal] =
            connection.connectionConfig.mSourceStage;
        mTerminalStage[connection.connectionConfig.mSinkTerminal] =
            connection.connectionConfig.mSinkStage;
        // If the connection's sink stage is same as the first stage/pg id in this executor,
        // then it means the connection belongs to input terminal pairs.
        if (connection.connectionConfig.mSourceStage == INVALID_PORT &&
            connection.connectionConfig.mSinkStage != INVALID_PORT && connection.hasEdgePort) {
            mEdgeConnections.push_back(connection);
            PipeStageUnit* unit = findPipeStage(connection.connectionConfig.mSinkStage);
            if (unit) {
                unit->isInputEdge = true;
            }
        }

        // If the connection's source stage is same as the last stage/pg id in this executor,
        // then it means the connection belongs to output terminal pairs.
        // SIS is output terminal but it doesn't belong to any stream, so it is not real edge
        // output.
        if (connection.connectionConfig.mSinkStage == INVALID_PORT &&
            connection.connectionConfig.mSourceStage != INVALID_PORT && connection.hasEdgePort &&
            connection.connectionConfig.mSourceTerminal !=
                connection.connectionConfig.mSinkTerminal) {
            mEdgeConnections.push_back(connection);
            PipeStageUnit* unit = findPipeStage(connection.connectionConfig.mSourceStage);
            if (unit) {
                unit->isOutputEdge = true;
            }
        }
    }
}

IPipeStage* PipeLine::findStageProducer(const PipeStageUnit& psUnit) {
    CheckAndLogError(psUnit.inputTerminals.size() == 0, nullptr, "@%s: invalid stage input",
                     __func__);
    // a stage should have only 1 producer
    uuid sourceStage = mTerminalStage[mSinkMapSource[psUnit.inputTerminals[0]]];

    for (auto& unit : mPSUnit) {
        if (unit.stageUuid == sourceStage) {
            return unit.pipeStage;
        }
    }

    return nullptr;
}

std::vector<PipeStageUnit*> PipeLine::findStageConsumer(const PipeStageUnit& psUnit) {
    std::vector<PipeStageUnit*> consumerUnits;
    std::set<uuid> sinkStages;
    for (auto& sourceTerminal : psUnit.outputTerminals) {
        sinkStages.insert(mTerminalStage[mSourceMapSink[sourceTerminal]]);
    }

    for (auto& unit : mPSUnit) {
        if (sinkStages.find(unit.stageUuid) != sinkStages.end()) {
            consumerUnits.push_back(&unit);
        }
    }

    return consumerUnits;
}

std::map<uuid, uuid> PipeLine::getConsumerPortMap(const PipeStageUnit* psUnitConsumer) {
    std::map<uuid, uuid> portMap;
    std::vector<uuid> intputTerminals = psUnitConsumer->inputTerminals;
    for (auto& sinkTerminal : intputTerminals) {
        if (mSinkMapSource.find(sinkTerminal) == mSinkMapSource.end()) {
            continue;
        }
        // <producer output port, consumer input port>
        portMap[mSinkMapSource[sinkTerminal]] = sinkTerminal;
        LOG1("@%s, %s port %x map producer port %x", __func__, psUnitConsumer->pipeStage->getName(),
             sinkTerminal, mSinkMapSource[sinkTerminal]);
    }
    return portMap;
}

void PipeLine::linkPipeStages() {
    for (auto& unit : mPSUnit) {
        // set producer for PipeStage, it has support only one producer now.
        if (!unit.isInputEdge) {
            IPipeStage* producer = findStageProducer(unit);
            if (producer) {
                unit.pipeStage->setBufferProducer(producer);
                LOG1("%s: find producer %s for consumer %s", __func__, producer->getName(),
                     unit.pipeStage->getName());
            }
        }

        // set consumer for PipeStage
        if (!unit.isOutputEdge) {
            std::vector<PipeStageUnit*> consumer = findStageConsumer(unit);
            if (consumer.size() != 0) {
                for (size_t i = 0; i < consumer.size(); i++) {
                    unit.pipeStage->addFrameAvailableListener(consumer[i]->pipeStage);
                    LOG1("%s: find consumer %s for producer %s", __func__,
                         consumer[i]->pipeStage->getName(), unit.pipeStage->getName());
                    // <producer output port, consumer input port>
                    auto portMap = getConsumerPortMap(consumer[i]);
                    if (portMap.size() != 0) {
                        consumer[i]->pipeStage->setInputTerminals(portMap);
                    }
                }
            }
        }
    }
}

void PipeLine::setFrameInfoForPipeStage() {
    for (auto& unit : mPSUnit) {
        // Set frame info for BufferQueue
        std::map<uuid, stream_t> inputInfo;
        std::map<uuid, stream_t> outputInfo;

        // Link internal pipeStage (sink stage accepts input ports arrangement from source stage
        // (output ports) source stage(output ports) -> (input ports)sink stage
        // Use its own input info due to no executor as producer
        for (auto terminal : unit.inputTerminals) {
            if (!mTerminalsDesc[terminal].enabled) {
                continue;
            }
            if (mSinkMapSource.find(terminal) == mSinkMapSource.end() && !unit.isInputEdge) {
                continue;
            }

            stream_t inputConfig;
            CLEAR(inputConfig);
            inputConfig.width = mTerminalsDesc[terminal].frameDesc.mWidth;
            inputConfig.height = mTerminalsDesc[terminal].frameDesc.mHeight;
            inputConfig.format = mTerminalsDesc[terminal].frameDesc.mFormat;
            inputConfig.id = mTerminalsDesc[terminal].usrStreamId;

            uuid tmpTerminal = terminal;
            if (!unit.isInputEdge) {
                tmpTerminal = mSinkMapSource.at(terminal);
            }
            inputInfo[tmpTerminal] = inputConfig;
            LOG1("%s, %s(%d) input terminal:%u, w:%d, h:%d, format:%x,%s", __func__,
                 unit.pipeStage->getName(), unit.isInputEdge, tmpTerminal, inputConfig.width,
                 inputConfig.height, inputConfig.format,
                 CameraUtils::pixelCode2String(inputConfig.format));
        }

        for (auto terminal : unit.outputTerminals) {
            if (!mTerminalsDesc[terminal].enabled) {
                continue;
            }
            if (mSourceMapSink.find(terminal) == mSourceMapSink.end()) {
                continue;
            }
            stream_t outputConfig;
            CLEAR(outputConfig);
            outputConfig.width = mTerminalsDesc[terminal].frameDesc.mWidth;
            outputConfig.height = mTerminalsDesc[terminal].frameDesc.mHeight;
            outputConfig.format = mTerminalsDesc[terminal].frameDesc.mFormat;
            outputConfig.id = mTerminalsDesc[terminal].usrStreamId;
            outputInfo[terminal] = outputConfig;

            LOG1("%s, pipe stage:%s output terminal:%u, w:%d, h:%d, format:%x:%s", __func__,
                 unit.pipeStage->getName(), terminal, outputConfig.width, outputConfig.height,
                 outputConfig.format, CameraUtils::pixelCode2String(outputConfig.format));
        }
        unit.pipeStage->setFrameInfo(inputInfo, outputInfo);
    }
}

void PipeLine::getTerminalFrameInfos(const std::vector<uuid>& terminals,
                                     std::map<uuid, FrameInfo>& infoMap) const {
    infoMap.clear();
    for (auto terminal : terminals) {
        const TerminalInfo& termDesc = mTerminalsDesc.at(terminal);
        if (termDesc.enabled) {
            infoMap[terminal] = termDesc.frameDesc;
        }
    }
}

void PipeLine::dumpPipeStages() const {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(PipeLine))) {
        return;
    }

    LOG3("============= dump PipeStage for stream %d =================", mStreamId);
    for (auto& unit : mPSUnit) {
        if (unit.isInputEdge) {
            LOG3("This is input edge");
        }
        if (unit.isOutputEdge) {
            LOG3("This is output edge");
        }
        uuid stageUuid = unit.stageUuid;
        LOG3("    PS: %d: %s, uuid %x", unit.stageId,
             unit.pipeStage ? unit.pipeStage->getName() : "GPU-TNR", stageUuid);

        LOG3("        InTerms: %zu", unit.inputTerminals.size());
        for (auto const& term : unit.inputTerminals) {
            const TerminalInfo& termDesc = mTerminalsDesc.at(term);
            if (termDesc.enabled) {
                LOG3("            %x: %dx%d (%s)", termDesc.terminal, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight,
                     CameraUtils::fourcc2String(termDesc.frameDesc.mFormat).c_str());
            } else {
                LOG3("            %x: %dx%d (%s), disabled", termDesc.terminal,
                     termDesc.frameDesc.mWidth, termDesc.frameDesc.mHeight,
                     CameraUtils::fourcc2String(termDesc.frameDesc.mFormat).c_str());
            }
        }

        LOG3("        OutTerms: %zu", unit.outputTerminals.size());
        for (auto const& term : unit.outputTerminals) {
            const TerminalInfo& termDesc = mTerminalsDesc.at(term);
            if (termDesc.enabled) {
                LOG3("            %x: %dx%d (%s)", termDesc.terminal, termDesc.frameDesc.mWidth,
                     termDesc.frameDesc.mHeight,
                     CameraUtils::fourcc2String(termDesc.frameDesc.mFormat).c_str());
            } else {
                LOG3("            %x: %dx%d (%s), disabled", termDesc.terminal,
                     termDesc.frameDesc.mWidth, termDesc.frameDesc.mHeight,
                     CameraUtils::fourcc2String(termDesc.frameDesc.mFormat).c_str());
            }
        }
    }
    LOG3("============= dump done for stream %d =================", mStreamId);
}

}  // namespace icamera
