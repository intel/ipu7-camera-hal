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

#define LOG_TAG PipeManager

#include "PipeManager.h"

#include <set>
#include <algorithm>

#include "CameraContext.h"
#include "iutils/CameraLog.h"
#include "GraphUtils.h"

namespace icamera {

PipeManager::PipeManager(int cameraId, PipeManagerCallback* callback,
                         std::shared_ptr<CameraScheduler> scheduler)
        : mCameraId(cameraId),
          mScheduler(scheduler),
          mTuningMode(TUNING_MODE_MAX),
          mPMCallback(callback) {
    LOG1("<id%d>@%s ", mCameraId, __func__);

    mPacAdaptor = new IpuPacAdaptor(mCameraId);

    camera_coordinate_system_t array = PlatformData::getActivePixelArray(mCameraId);
    mActivePixels.width = array.right - array.left;
    mActivePixels.height = array.bottom - array.top;
    mZoom = {0, 0, mActivePixels.width, mActivePixels.height, 1.0, ROTATE_NONE};
}

PipeManager::~PipeManager() {
    LOG1("<id%d>@%s", mCameraId, __func__);
    releasePipeLines();

    mPacAdaptor->deinit();
    delete mPacAdaptor;
}

int PipeManager::configure(const std::map<uuid, stream_t>& inputInfo,
                           const std::map<uuid, stream_t>& outputInfo,
                           ConfigMode configMode, TuningMode tuningMode,
                           const std::map<uuid, stream_t>* yuvInputInfo) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mConfigMode = configMode;
    mTuningMode = tuningMode;
    mInputFrameInfo = inputInfo;
    mOutputFrameInfo = outputInfo;
    mDefaultMainInputPort = inputInfo.begin()->first;
    mOutputPortToStreamId.clear();

    status_t ret = OK;
    mGraphConfig = CameraContext::getInstance(mCameraId)->getGraphConfig(mConfigMode);
    CheckAndLogError(mGraphConfig == nullptr, UNKNOWN_ERROR,
                     "Failed to get GraphConfig in PipeManager!");

    std::vector<int32_t> activeStreamIds;
    ret = mGraphConfig->graphGetStreamIds(activeStreamIds);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get the streamIds");

    ret = mPacAdaptor->init(activeStreamIds);
    CheckAndLogError(ret != OK, ret, "Init pac Adaptor failed, tuningMode %d", mTuningMode);

    mPipeLines.clear();
    ret = createPipeLines(activeStreamIds);
    CheckAndLogError(ret != OK, ret, "@%s, create pipelines failed", __func__);

    if ((yuvInputInfo != nullptr) && (!mYuvPipeLine.empty())) {
        ret = bindYuvReprocessingPort(*yuvInputInfo);
    }

    return ret;
}

int PipeManager::start() {
    int ret = OK;

    for (auto& pipeLine : mPipeLines) {
        ret = pipeLine.second->start();
        CheckAndLogError(ret != OK, ret, "Failed to start pipeline");
    }
    return ret;
}

int PipeManager::stop() {
    for (auto& pipeLine : mPipeLines) {
        pipeLine.second->stop();
    }
    mPacAdaptor->clearAicResult();
    return icamera::OK;
}

int PipeManager::analyzeConnections() {
    // each pipeLine could have multi output but only 1 input

    // uuid of input edge, pipeLine input is ISYS
    std::vector<uint32_t> inputEdgePorts;
    // uuid of output edge, pipeLine output is USER
    std::vector<std::pair<uint32_t, uint32_t>> outputEdgePorts;

    // Get external connections
    std::map<int32_t, std::vector<IGraphType::PipelineConnection>> conVectors;
    for (auto& pipeLine : mPipeLines) {
        const int streamId = pipeLine.first;
        const auto& conVectors = pipeLine.second->getEdgeConnections();

        for (auto const& connection : conVectors) {
            if ((connection.portFormatSettings.enabled == 0) || (!connection.hasEdgePort)) {
                continue;
            }

            if (connection.connectionConfig.mSourceStage == 0U) {
                // In edge port: no source stage
                inputEdgePorts.push_back(connection.connectionConfig.mSinkTerminal);
            } else if (connection.stream != nullptr) {
                // Out edge: has user stream
                outputEdgePorts.push_back(
                    std::make_pair(streamId, connection.connectionConfig.mSourceTerminal));
            }
        }
    }

    // link pipeLines
    // TODO: Check connections between pipelines and link them

    // bind edge connections to external port
    return bindExternalPorts(inputEdgePorts, outputEdgePorts);
}

int PipeManager::bindExternalPorts(
    const std::vector<uint32_t>& inputEdgePorts,
    const std::vector<std::pair<uint32_t, uint32_t>>& outputEdgePorts) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mInputMaps.clear();
    mOutputMaps.clear();

    std::map<uuid, stream_t> outputInfo;
    std::map<uuid, stream_t> inputInfo;

    // each output from user request should only bind to 1 pipeLine's output port
    std::map<uuid, stream_t> outputFrameInfo = mOutputFrameInfo;
    for (auto& pipeLine : mPipeLines) {
        int32_t streamId = pipeLine.first;

        // ProcessingUnit's input can bind to multi pipeLines input port
        std::map<uuid, stream_t> inputFrameInfo = mInputFrameInfo;
        LOG2("%s, start to bind the input port of pipeLine %d", __func__, streamId);
        /*
        ** Loop to bind multiple input ports to multiple frames one-to-one, remove the port and
        ** frame from map when bind succeed.
        */
        pipeLine.second->getInput()->getFrameInfo(inputInfo, outputInfo);
        for (auto& portInfo : inputInfo) {
            // Check if input port of pipeline is external input
            bool isInEdge = false;
            for (const auto port : inputEdgePorts) {
                if (portInfo.first == port) {
                    isInEdge = true;
                    break;
                }
            }
            if (!isInEdge) {
                continue;
            }

            // Link to external input
            bool bind = false;
            auto frameInfo = inputFrameInfo.begin();
            while (frameInfo != inputFrameInfo.end()) {
                bool match = isSameStreamConfig(portInfo.second, frameInfo->second, false);
                if (match) {
                    PortMapping portMap;
                    portMap.mPipeStage = pipeLine.second->getInput();
                    portMap.mExternalPort = frameInfo->first;
                    portMap.mStagePort = portInfo.first;
                    portMap.mStreamId = streamId;
                    mInputMaps.push_back(portMap);
                    LOG2("%s, external sourcePort %x, sinkPort %x", __func__, frameInfo->first,
                         portInfo.first);
                    inputFrameInfo.erase(frameInfo);
                    bind = true;
                    break;
                } else {
                    frameInfo++;
                }
            }
            CheckAndLogError(!bind, UNKNOWN_ERROR, "@%s, failed to bind input port %x", __func__,
                             portInfo.first);
        }
        // Then bind the output ports.
        LOG2("%s, start to bind the output port of pipeLine %d", __func__, streamId);
        std::vector<IPipeStage*> stages = pipeLine.second->getOutput();
        // Get all outputs of pipeline
        outputInfo.clear();
        for (auto& stage : stages) {
            stage->getFrameInfo(inputInfo, outputInfo);

            for (auto& portInfo : outputInfo) {
                bool isOutEdge = false;
                // Check if output port of pipeline is external output
                for (auto pair : outputEdgePorts) {
                    if (pair.first != static_cast<uint32_t>(streamId)) {
                        continue;
                    }
                    if (portInfo.first == pair.second) {
                        isOutEdge = true;
                        break;
                    }
                }
                if (!isOutEdge) {
                    continue;
                }

                // Link to external input
                bool bound = false;
                for (auto frameInfo = outputFrameInfo.begin();
                     frameInfo != outputFrameInfo.end();) {
                    if (isSameStreamConfig(portInfo.second, frameInfo->second, true)) {
                        PortMapping portMap;
                        portMap.mPipeStage = stage;
                        portMap.mExternalPort = frameInfo->first;
                        portMap.mStagePort = portInfo.first;
                        portMap.mStreamId = streamId;
                        mOutputMaps.push_back(portMap);
                        mOutputPortToStreamId[frameInfo->first] = streamId;
                        LOG2("%s, outputMap stage %s, source uuid %x, exPort %x", __func__,
                             portMap.mPipeStage->getName(), portInfo.first, frameInfo->first);
                        // Clear the external port because it is occupied
                        frameInfo = outputFrameInfo.erase(frameInfo);
                        bound = true;
                        break;
                    }
                    frameInfo++;
                }
                CheckAndLogError(!bound, BAD_VALUE, "Failed to bind output port:%x",
                                 portInfo.first);
            }
        }
    }
    // Each required port must be mapped to one of (edge) executor's port.
    // One input port may be mapped to more of (edge) executor's ports.
    CheckAndLogError(mInputMaps.size() < mInputFrameInfo.size(), BAD_VALUE,
                     "Failed to bind input ports");
    CheckAndLogError(mOutputMaps.size() < mOutputFrameInfo.size(), BAD_VALUE,
                     "Failed to bind output ports");

    return OK;
}

int PipeManager::bindYuvReprocessingPort(const std::map<uuid, stream_t>& yuvInputInfo) {
    CheckAndLogError(mYuvPipeLine.find(YUV_REPROCESSING_STREAM_ID) == mYuvPipeLine.end(),
                     UNKNOWN_ERROR, "It doesn't have yuv pipe line");
    std::map<uuid, stream_t> outputInfo;
    std::map<uuid, stream_t> inputInfo;

    // YUV reprocessing pipe line has only one pipe stage
    IPipeStage* mPipeStage = mYuvPipeLine[YUV_REPROCESSING_STREAM_ID]->getInput();
    mPipeStage->getFrameInfo(inputInfo, outputInfo);

    // Bind the input ports
    for (auto& portInfo : inputInfo) {
        auto frameInfo = yuvInputInfo.begin();
        while (frameInfo != yuvInputInfo.end()) {
            if (isSameStreamConfig(portInfo.second, frameInfo->second, false)) {
                PortMapping portMap;
                portMap.mPipeStage = mPipeStage;
                portMap.mExternalPort = frameInfo->first;
                portMap.mStagePort = portInfo.first;
                portMap.mStreamId = YUV_REPROCESSING_STREAM_ID;
                mYuvInputMaps.push_back(portMap);
                LOG2("%s, YUV external sourcePort %d, sinkPort %d", __func__, frameInfo->first,
                     portInfo.first);
                // Keep external input port because it can be shared among pipelines
                break;
            }
            frameInfo++;
        }
        CheckAndLogError(frameInfo == yuvInputInfo.end(), UNKNOWN_ERROR,
                         "@%s, failed to bind input port %d", __func__, portInfo.first);
    }

    bool bound = false;
    std::map<uuid, stream_t> yuvOutputInfo = mOutputFrameInfo;
    for (auto& portInfo : outputInfo) {
        for (auto frameInfo = yuvOutputInfo.begin(); frameInfo != yuvOutputInfo.end();) {
            if (isSameStreamConfig(portInfo.second, frameInfo->second, true)) {
                PortMapping portMap;
                portMap.mPipeStage = mPipeStage;
                portMap.mExternalPort = frameInfo->first;
                portMap.mStagePort = portInfo.first;
                portMap.mStreamId = YUV_REPROCESSING_STREAM_ID;
                mYuvOutputMaps.push_back(portMap);
                LOG2("%s, YUV outputMap stage %s, source uuid %d, exPort %d", __func__,
                     portMap.mPipeStage->getName(), portInfo.first, frameInfo->first);
                // Clear the external port because it is occupied
                frameInfo = yuvOutputInfo.erase(frameInfo);
                bound = true;
                break;
            }
            frameInfo++;
        }
        CheckAndLogError(!bound, BAD_VALUE, "Failed to bind YUV output port:%d", portInfo.first);
    }

    return OK;
}

bool PipeManager::isSameStreamConfig(const stream_t& internal, const stream_t& external,
                                     bool checkStreamId) const {
    const int internalStride = CameraUtils::getStride(internal.format, internal.width);
    const int externalStride = CameraUtils::getStride(external.format, external.width);

    LOG1("%s: internal: %s(%dx%d: %d)(id %d), external: %s(%dx%d: %d) (id %d) usage:%d", __func__,
         CameraUtils::format2string(internal.format).c_str(), internal.width, internal.height,
         internalStride, internal.id, CameraUtils::format2string(external.format).c_str(),
         external.width, external.height, externalStride, external.id, external.usage);

    if (checkStreamId) {
        return internal.id == external.id;
    }

    /*
     * WA: PG accept GRBG format but actual input data is of RGGB format,
     *     PG use its kernel to crop to GRBG
     */
    if (((internal.format == V4L2_PIX_FMT_SGRBG10) || (internal.format == V4L2_PIX_FMT_SGRBG12)) &&
        ((external.format == V4L2_PIX_FMT_SRGGB10) || (external.format == V4L2_PIX_FMT_SRGGB12))) {
        return true;
    }

    const bool sameHeight =
        (internal.height == external.height) || (internal.height == ALIGN_32(external.height));
    if ((internal.format == external.format) && sameHeight &&
        ((internal.width == external.width) || (internalStride == externalStride))) {
        return true;
    }

    return false;
}

int PipeManager::createPipeLines(const std::vector<int32_t>& activeStreamIds) {
    for (auto& id : activeStreamIds) {
        LOG1("create PipeLine for stream id :%d", id);
        if (id == YUV_REPROCESSING_STREAM_ID) {
            mYuvPipeLine[YUV_REPROCESSING_STREAM_ID] =
                std::shared_ptr<PipeLine>(new PipeLine(mCameraId, id, mGraphConfig, mScheduler));
            mYuvPipeLine[YUV_REPROCESSING_STREAM_ID]->configure(mTuningMode, mPacAdaptor);
            mYuvPipeLine[YUV_REPROCESSING_STREAM_ID]->registerListener(EVENT_STAGE_BUF_READY, this);
            continue;
        }
        mPipeLines[id] =
            std::shared_ptr<PipeLine>(new PipeLine(mCameraId, id, mGraphConfig, mScheduler));
        const int ret = mPipeLines[id]->configure(mTuningMode, mPacAdaptor);
        CheckAndLogError(ret != OK, ret, "Failed to configure pipeline");
        mPipeLines[id]->registerListener(EVENT_PSYS_STATS_BUF_READY, this);
        mPipeLines[id]->registerListener(EVENT_PSYS_STATS_SIS_BUF_READY, this);
        mPipeLines[id]->registerListener(EVENT_STAGE_BUF_READY, this);
    }

    return analyzeConnections();
}

void PipeManager::releasePipeLines() {
    for (auto& pipeLine : mPipeLines) {
        pipeLine.second->removeListener(EVENT_PSYS_STATS_BUF_READY, this);
        pipeLine.second->removeListener(EVENT_PSYS_STATS_SIS_BUF_READY, this);
        pipeLine.second->removeListener(EVENT_STAGE_BUF_READY, this);
        pipeLine.second->deinit();
    }
    mPipeLines.clear();

    for (auto& pipeLine : mYuvPipeLine) {
        pipeLine.second->removeListener(EVENT_STAGE_BUF_READY, this);
    }
    mYuvPipeLine.clear();
}

void PipeManager::setControl(int64_t sequence, const PipeControl& control) {
    for (auto pipeline : mPipeLines) {
        if (control.find(pipeline.first) != control.end()) {
            pipeline.second->setControl(sequence, control.at(pipeline.first));
        }
    }
}

void PipeManager::addTask(PipeTaskData taskParam) {
    LOG2("<id%d>@%s", mCameraId, __func__);

    TaskInfo task = {};
    // Save the task data into mOngoingTasks
    task.mTaskData = taskParam;
    // Count how many valid output buffers need to be returned.
    for (auto& outBuf : taskParam.mOutputBuffers) {
        if (outBuf.second != nullptr) {
            task.mNumOfValidBuffers++;
        }
    }

    uuid port = mDefaultMainInputPort;
    if (taskParam.mYuvTask) {
        port = YUV_REPROCESSING_INPUT_PORT_ID;
    }

    const int64_t sequence = taskParam.mInputBuffers.at(port)->getSequence();
    LOG2("%s:<id%d:seq%ld> push task with %d output buffers", __func__, mCameraId,
         sequence, task.mNumOfValidBuffers);
    {
        AutoMutex taskLock(mTaskLock);
        std::pair<int64_t, TaskInfo> p(sequence, task);
        mOngoingTasks.insert(p);
    }

    if (taskParam.mYuvTask) {
        queueBuffers(taskParam, mYuvInputMaps, mYuvOutputMaps, YUV_REPROCESSING_INPUT_PORT_ID);
    } else {
        // Normally run AIC before execute psys
        std::vector<int32_t> activeStreamIds;
        (void)getActiveStreamIds(taskParam, &activeStreamIds);
        LOG2("%s, <seq%ld> run AIC before execute psys, active stream Ids: %zu",
             __func__, sequence, activeStreamIds.size());

        TRACE_LOG_PROCESS("run PAC", __func__, MAKE_COLOR(sequence), sequence);
        for (const auto& id : activeStreamIds) {
            (void)prepareIpuParams(&taskParam.mIspSettings, sequence, id);
        }
        queueBuffers(taskParam, mInputMaps, mOutputMaps, mDefaultMainInputPort);
    }
}

int PipeManager::getActiveStreamIds(const PipeTaskData& taskData,
                                    std::vector<int32_t>* activeStreamIds) {
    LOG2("<id%d>@%s", mCameraId, __func__);

    // According to the output port to filter the valid executor stream Ids, and then run AIC
    for (auto& outputFrame : taskData.mOutputBuffers) {
        if (outputFrame.second.get() == nullptr) {
            continue;
        }

        CheckAndLogError(
            mOutputPortToStreamId.find(outputFrame.first) == mOutputPortToStreamId.end(),
            UNKNOWN_ERROR, "%s, failed to find streamIds for output port: %x", __func__,
            outputFrame.first);

        const int32_t streamId = mOutputPortToStreamId[outputFrame.first];
        if (std::find(activeStreamIds->begin(), activeStreamIds->end(), streamId) ==
            activeStreamIds->end()) {
            activeStreamIds->push_back(streamId);
        }
    }

    return OK;
}

int PipeManager::prepareIpuParams(IspSettings* settings, int64_t sequence, int streamId) {
    if (mPipeLines.find(streamId) == mPipeLines.end()) {
        LOGW("<id%d>@%s Invalid stream id:%d", mCameraId, __func__, streamId);
        return BAD_VALUE;
    }

    PtzInfo ptz;
    const bool zoomChanged = updateZoomSettings(settings->zoom, &ptz);
    if (zoomChanged) {
        LOG2("<seq%ld>Update graph for ptz", sequence);
        LOG3("zoom region: (%d, %d, %d, %d), ratio %f", settings->zoom.left, settings->zoom.right,
             settings->zoom.top, settings->zoom.bottom, settings->zoom.ratio);
        LOG3("convert to ptz: start point (%f, %f), region size (%f, %f), ratio %f, centered? %d",
             ptz.x, ptz.y, ptz.xSize, ptz.ySize, ptz.zoomRatio, ptz.zoomCentered);
        bool isKeyResChanged = false;
        const status_t status =
            mGraphConfig->updateGraphSettingForPtz(ptz, mCurPtz, &isKeyResChanged);
        if (status == OK ) {
            for (auto& pipe : mPipeLines) {
                pipe.second->updateConfigurationSettingForPtz(isKeyResChanged);
            }
            mCurPtz = ptz;
            mZoom = settings->zoom;
        }
    }

    const int ret = mPacAdaptor->runAIC(settings, sequence, streamId);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, <seq%ld> Failed to run AIC: streamId: %d",
                     __func__, sequence, streamId);

    return ret;
}

bool PipeManager::updateZoomSettings(const camera_zoom_region_t& zoom, PtzInfo* ptz) {
    if ((mActivePixels.width == 0) || (mActivePixels.height == 0)) {
        return false;
    }

#define RATIO_TOL 0.005
    // Check centered zoom (user zoom ratio > 1.0, don't support zoom ratio < 1.0)
    const bool ratioChanged = fabs(zoom.ratio - mZoom.ratio) > RATIO_TOL ? true : false;
    if (zoom.ratio - 1.0 > RATIO_TOL) {
        ptz->zoomCentered = true;
        ptz->zoomRatio = 1.0 / zoom.ratio;
        ptz->xSize = ptz->ySize = 1.0 / zoom.ratio;
        ptz->x = ptz->y = (1.0 - ptz->xSize) / 2.0;
        return ratioChanged;
    }

    // Handle crop region setting when user zoom ratio = 1.0
    if ((zoom.left == mZoom.left) && (zoom.right == mZoom.right) &&
        (zoom.top == mZoom.top) && (zoom.bottom == mZoom.bottom) && (!ratioChanged)) {
        return false;
    }

    // Support region with same aspect ratio as sensor only. Re-calc appropriate region
    if ((zoom.left >= 0) && (zoom.right > zoom.left) &&
        (zoom.top >= 0) && (zoom.bottom > zoom.top) &&
        (zoom.right <= mActivePixels.width) && (zoom.bottom <= mActivePixels.height)) {
        // Calc user region
        const float left   = static_cast<float>(zoom.left) / mActivePixels.width;
        const float right  = static_cast<float>(zoom.right) / mActivePixels.width;
        const float top    = static_cast<float>(zoom.top) / mActivePixels.height;
        const float bottom = static_cast<float>(zoom.bottom) / mActivePixels.height;
        const float xSize  = right - left;
        const float ySize  = bottom - top;

        ptz->x = left;
        ptz->y = top;
        ptz->xSize = xSize;
        ptz->ySize = ySize;
        ptz->zoomCentered = (fabs(left + right - 1.0) < RATIO_TOL) ||
                            (fabs(top + bottom - 1.0) < RATIO_TOL);
        /*
         * Only support same aspect ratio as active array crrently, extend region if need
         * original region 16:9, active 4:3: increase h
         * --------------------
         * |        new       |
         * |------------------|
         * |     original     |
         * |                  |
         * |                  |
         * |------------------|
         * |                  |
         * --------------------
         * original region 4:3, active 16:9: increase w
         * ---------------------
         * |  |             |  |
         * |  |             |  |
         * |  |   original  |  |
         * |  |             |  |
         * |  |             |  |
         * |--------------------
         */
        if (ptz->xSize > ptz->ySize) {
            // Increase h
            ptz->y -= (ptz->xSize - ptz->ySize) / 2;
            ptz->ySize = xSize;
            // Check the boundary
            if (ptz->y < 0.0) {
                ptz->y = 0.0;
            } else {
                if ((ptz->y + ptz->ySize) > 1.0) {
                    ptz->y = 1.0 - ptz->ySize;
                }
            }
        } else if (ptz->xSize < ptz->ySize) {
            // Increase w
            ptz->x -= (ptz->ySize - ptz->xSize) / 2;
            ptz->xSize = ySize;
            // Check the boundary
            if (ptz->x < 0.0) {
                ptz->x = 0.0;
            } else {
                if ((ptz->x + ptz->xSize) > 1.0) {
                    ptz->x = 1.0 - ptz->xSize;
                }
            }
        }

        // ptz zoom ratio is percentage of crop region
        ptz->zoomRatio = ptz->xSize;
        return true;
    }

    // Ignore invalid crop region
    return ratioChanged;
}

void PipeManager::queueBuffers(const PipeTaskData& task, const std::vector<PortMapping>& inputMaps,
                               const std::vector<PortMapping>& outputMaps, uuid inputPort) {
    LOG2("<id%d>@%s", mCameraId, __func__);

    std::vector<int32_t> activeStreamIds;
    (void)getActiveStreamIds(task, &activeStreamIds);

    int64_t sequence = task.mInputBuffers.at(inputPort)->getSequence();
    // Provide the output buffers for the output edge.
    for (auto& outputMap : outputMaps) {
        for (auto& outputFrame : task.mOutputBuffers) {
            if (outputMap.mExternalPort == outputFrame.first) {
                if (std::find(activeStreamIds.begin(), activeStreamIds.end(),
                    outputMap.mStreamId) == activeStreamIds.end()) {
                    continue;
                }

                LOG2("<seq%ld>queue output for stream:%d stage uuid: %u, external:%d", sequence,
                     outputMap.mStreamId, outputMap.mStagePort, outputFrame.first);
                outputMap.mPipeStage->qbuf(outputMap.mStagePort, outputFrame.second);
                break;
            }
        }
    }

    for (auto& inputMap : inputMaps) {
        for (auto& inputFrame : task.mInputBuffers) {
            if (inputMap.mExternalPort == inputFrame.first) {
                if (std::find(activeStreamIds.begin(), activeStreamIds.end(),
                    inputMap.mStreamId) == activeStreamIds.end()) {
                    continue;
                }

                LOG2("<seq%ld>queue input buffer for stream:%d stage uuid: %u, external:%d",
                     sequence, inputMap.mStreamId, inputMap.mStagePort, inputFrame.first);
                inputMap.mPipeStage->onBufferAvailable(inputMap.mStagePort, inputFrame.second);
            }
        }
    }
}

void PipeManager::onMetadataReady(int64_t sequence) {
    LOG2("<seq%ld> %s", sequence, __func__);

    AutoMutex taskLock(mTaskLock);
    auto range = mOngoingTasks.equal_range(sequence);
    while (range.first != range.second) {
        auto& task = range.first->second;

        if (!task.mMetadataDone && task.mTaskData.mCallbackRgbs) {
            mPMCallback->onMetadataReady(sequence, task.mTaskData.mOutputBuffers);
            task.mMetadataDone = true;
            return;
        }
        ++range.first;
    }
}

/**
 * Use to handle the buffer done event from the pipeLine.
 *
 * This is for returning output buffers to ProcessingUnit. And it'll check if all the valid
 * output buffer returned, if so, then it'll return the whole corresponding task data to
 * ProcessingUnit.
 */
int PipeManager::onBufferDone(uuid port, const std::shared_ptr<CameraBuffer>& buffer) {
    if (buffer == nullptr) {
        return OK;  // No need to handle if the buffer is nullptr.
    }

    int64_t sequence = buffer->getSequence();
    LOG2("<id%d:seq%ld>@%s buffer=%p, port %u", mCameraId, sequence, __func__,
         buffer->getUserBuffer(), port);

    bool needReturn = false;
    uuid outputPort = INVALID_PORT;
    PipeTaskData result;
    {
        AutoMutex taskLock(mTaskLock);
        for (auto iter = mOngoingTasks.begin(); iter != mOngoingTasks.end(); iter++) {
            auto& task = iter->second;

            // Check if buffer belongs to the task because input buffer maybe reused
            for (auto& buf : task.mTaskData.mOutputBuffers) {
                if ((buf.second != nullptr) &&
                    (buffer->getUserBuffer() == buf.second->getUserBuffer())) {
                    outputPort = buf.first;
                }
            }
            /* an edge stage may have multi output ports, these ports would be edge port connect
            ** to user buffer or none edge port connect to post stage. all these ports will send
            ** buffer ready event, should ignore the none edge port event
            */
            if (outputPort == INVALID_PORT) {
                continue;
            }

            task.mNumOfReturnedBuffers++;
            if (task.mNumOfReturnedBuffers >= task.mNumOfValidBuffers) {
                result = task.mTaskData;
                needReturn = true;
                LOG2("<Id%d:seq%ld> finish task with %d returned output buffers, ", mCameraId,
                     sequence, task.mNumOfReturnedBuffers);
                // Remove the task data from mOngoingTasks since it's already processed.
                mOngoingTasks.erase(iter);
            }
            break;
            // No need check other if other tasks are matched with the returned buffer since
            // we already found one.
        }
    }

    if (outputPort == INVALID_PORT) {
        return OK;
    }

    if (mPMCallback != nullptr) {
        // Return buffer
        mPMCallback->onBufferDone(sequence, outputPort, buffer);
    }

    if (needReturn && (mPMCallback != nullptr)) {
        mPMCallback->onTaskDone(result);
    }

    return OK;
}

void PipeManager::handleEvent(EventData eventData) {
    // Process registered events
    LOG2("%s  event %d", __func__, eventData.type);
    switch (eventData.type) {
        case EVENT_PSYS_STATS_SIS_BUF_READY:
            mPMCallback->onStatsReady(eventData);
            break;
        case EVENT_PSYS_STATS_BUF_READY:
            /**
             * Stats from PAC are saved for Video pipe or Still pipe only cases.
             * So only handle the stats in those cases.
             */
            if ((eventData.pipeType == VIDEO_STREAM_ID) ||
                PlatformData::isStillOnlyPipeEnabled(mCameraId)) {
                mPMCallback->onStatsReady(eventData);
            }
            // Handle the metadata event in stats Done
            onMetadataReady(eventData.data.statsReady.sequence);
            break;
        case EVENT_STAGE_BUF_READY:
            (void)onBufferDone(eventData.data.stageBufReady.uuid, eventData.buffer);
            break;
        default:
            // Do not need to callback
            break;
    }
}
}  // namespace icamera
