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

#define LOG_TAG GraphConfig

#include "src/platformdata/gc/GraphConfig.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "GraphUtils.h"
#include "CBLayoutUtils.h"

using std::map;
using std::string;
using std::vector;

namespace icamera {

static const float RATIO_TOLERANCE = 0.1f;

static bool isSameRatio(const HalStream* s1, const HalStream* s2) {
    return fabs((float)(s1->width()) / s1->height() - (float)(s2->width()) / s2->height())
           <= RATIO_TOLERANCE;
}
static bool isSameRatio(const HalStream* s1, float ratio) {
    return fabs((float)(s1->width()) / s1->height() - ratio) <= RATIO_TOLERANCE;
}

#define CHECK_STREAM_ID(id) \
    do { \
        CheckAndLogError(mStaticGraphs.find(id) == mStaticGraphs.end(), NO_ENTRY, \
                         "%s: no graph for stream %d", __func__, id); \
    } while (0)

Mutex GraphConfig::sLock;
std::map<int32_t, StaticReaderBinaryData> GraphConfig::mGraphConfigBinaries;

GraphConfig::GraphConfig(int32_t camId, ConfigMode mode) : mCameraId(camId), mSensorRatio(0.0f) {
    AutoMutex l(sLock);
    CheckAndLogError(mGraphConfigBinaries.find(mCameraId) == mGraphConfigBinaries.end(), VOID_VALUE,
                     "<id%d>No graph bin loaded", mCameraId);

    const StaticGraphStatus sRet = mGraphReader.Init(mGraphConfigBinaries[mCameraId]);
    CheckAndLogError(sRet != StaticGraphStatus::SG_OK, VOID_VALUE,
                      "%s: failed to init graph reader", __func__);
}

GraphConfig::GraphConfig() : mCameraId(-1) { }

GraphConfig::~GraphConfig() {
    for (auto& graph : mStaticGraphs) graph.second.clear();
    mStaticGraphs.clear();
}

status_t GraphConfig::parse(int cameraId, const char* settingsXmlFile) {
    //CheckAndLogError(settingsXmlFile, BAD_VALUE, "file name is null");
    mCameraId = cameraId;
    // TODO: get file name according to usecase, ipu streaming mode, ...... in configStream()?
    return loadPipeConfig(settingsXmlFile);
}

void GraphConfig::releaseGraphNodes() {
    for (auto& item : mGraphConfigBinaries) {
        free(item.second.data);
        item.second.data = nullptr;
    }
    mGraphConfigBinaries.clear();
}

uint32_t GraphConfig::createQueryKeyAttribute(int cameraId) {
    uint32_t attributes = 0U;
    return attributes;
}

/*
 * Query graph setting according to activeStreams
 */
status_t GraphConfig::queryGraphSettings(const vector<HalStream*>& outStreams) {
    status_t ret = OK;
    for (auto& graph : mStaticGraphs) graph.second.clear();
    mStaticGraphs.clear();

    GraphConfigurationKey queryVideoKey = {};
    GraphConfigurationKey queryStillKey = {};
    // TODO: depends config mode/sensor type/......?
    queryVideoKey.fps = 30;
    queryStillKey.fps = 30;
    int32_t videoCount = 0;
    int32_t stillCount = 0;
    map<VirtualSink, const HalStream*> streams;
    for (auto& stream : outStreams) {
        if ((stream->useCase() & USE_CASE_VIDEO) != 0) {
            if (videoCount == 0) {
                queryVideoKey.preview.width = stream->width();
                queryVideoKey.preview.height = stream->height();
                queryVideoKey.preview.bpp = 8;  // TODO: depends on format
                queryVideoKey.attributes = createQueryKeyAttribute(mCameraId);
                streams[VirtualSink::PreviewSink] = stream;
                LOG2("%s: video stream %d, vSink %d", __func__, stream->streamId(),
                     VirtualSink::PreviewSink);
            } else if (videoCount == 1) {
                queryVideoKey.video.width = stream->width();
                queryVideoKey.video.height = stream->height();
                queryVideoKey.video.bpp = 8;  // TODO: depends on format
                streams[VirtualSink::VideoSink] = stream;
                LOG2("%s: video stream %d, vSink %d", __func__, stream->streamId(),
                     VirtualSink::VideoSink);
            }
            videoCount++;
        } else if ((stream->useCase() & USE_CASE_STILL) != 0) {
            if (stillCount == 0) {
                queryStillKey.stills.width = stream->width();
                queryStillKey.stills.height = stream->height();
                queryStillKey.stills.bpp = 8;  // TODO: depends on format
                queryStillKey.attributes = createQueryKeyAttribute(mCameraId);
                streams[VirtualSink::StillsSink] = stream;
                LOG2("%s: still stream %d, vSink %d", __func__, stream->streamId(),
                     VirtualSink::StillsSink);
            }
            stillCount++;
        }
    }
    // Currently Static Graph doesn't support > 2 outputs per stream.
    CheckAndLogError(videoCount > 2, UNKNOWN_ERROR, "Too more video streams %d", videoCount);
    CheckAndLogError(stillCount > 2, UNKNOWN_ERROR, "Too more still streams %d", stillCount);

    if (videoCount) {
        IStaticGraphConfig* staticGraph = nullptr;
        const StaticGraphStatus sRet =
                    mGraphReader.GetStaticGraphConfig(queryVideoKey, &staticGraph);
        CheckAndLogError(sRet != StaticGraphStatus::SG_OK, NO_ENTRY, "%s: no graph for video",
                         __func__);
        mStaticGraphs[VIDEO_STREAM_ID].staticGraph = staticGraph;
    }
    if (stillCount) {
        IStaticGraphConfig* staticGraph = nullptr;
        const StaticGraphStatus sRet =
                    mGraphReader.GetStaticGraphConfig(queryStillKey, &staticGraph);
        CheckAndLogError(sRet != StaticGraphStatus::SG_OK, NO_ENTRY, "%s: no graph for still",
                         __func__);
        mStaticGraphs[STILL_STREAM_ID].staticGraph = staticGraph;
    }

    // TODO: might to get after reconfigured by the HAL
    getStaticGraphConfigData(streams);

    return ret;
}

status_t GraphConfig::configStreams(const vector<HalStream*>& halStreams,
                                    const vector<HalStream*>& extraOutStreams) {
    LOG1("@%s", __func__);
    if (mSensorRatio < RATIO_TOLERANCE) {
        mSensorRatio = PlatformData::getSensorRatio(mCameraId);
    }

    vector<HalStream*> outStreams;
    vector<HalStream*> inStreams;
    for (size_t i = 0U; i < halStreams.size(); i++) {
        if (static_cast<stream_t*>(halStreams[i]->mPrivate)->streamType != CAMERA_STREAM_INPUT) {
            outStreams.push_back(halStreams[i]);
        } else {
            inStreams.push_back(halStreams[i]);
        }
    }

    // ipu stream: value = index; listener stream: value = index of ipu stream owner
    vector<int> outStreamIpuFlags;
    chooseIpuOutputStreams(outStreams, &outStreamIpuFlags);
    vector<HalStream*> ipuStreams;
    int videoIpuStreamNum = 0;
    for (size_t i = 0U; i < outStreamIpuFlags.size(); i++) {
        int mapStreamIndex = i;

        if (static_cast<int>(i) == outStreamIpuFlags[i]) {
            ipuStreams.push_back(outStreams[i]);
            if (outStreams[i]->useCase() == USE_CASE_VIDEO) {
                videoIpuStreamNum++;
            }
        } else {
            mapStreamIndex = outStreamIpuFlags[i];
        }

        // Map the stream ID to a pipe streamId. If a stream is bound to another stream,
        // use the pipe streamId of the bound stream instead.
        const int streamId = outStreams[i]->streamId();
        if (outStreams[mapStreamIndex]->useCase() == USE_CASE_VIDEO) {
            mStreamIdToPipeId[streamId] = VIDEO_STREAM_ID;
        } else if(outStreams[mapStreamIndex]->useCase() == USE_CASE_STILL) {
            mStreamIdToPipeId[streamId] = STILL_STREAM_ID;
        } else {
            LOGW("%s: stream %d use case %d is not supported", __func__, streamId,
                 outStreams[mapStreamIndex]->useCase());
        }

        LOG3("%s: %zu: stream %d, %dx%d, usecase %d, owner stream idx %d", __func__, i,
             outStreams[i]->streamId(), outStreams[i]->width(), outStreams[i]->height(),
             outStreams[i]->useCase(), outStreamIpuFlags[i]);
    }

    // Check if extra stream is supported.
    int32_t ret = UNKNOWN_ERROR;  // Not query yet
    int configuredStreamNum = halStreams.size();
    // Only one video stream is supported currently.
    if ((extraOutStreams.size() == 1) &&
        (extraOutStreams.front()->useCase() == USE_CASE_VIDEO) &&
        (videoIpuStreamNum < icamera::PlatformData::getVideoStreamNum())) {
        ipuStreams.push_back(extraOutStreams.front());
        configuredStreamNum++;
        ret = queryGraphSettings(ipuStreams);
        if (ret != OK) {
            ipuStreams.pop_back();
            configuredStreamNum--;
        } else {
            LOG3("%s: support extra stream %d, %dx%d, usecase %d", __func__,
                 ipuStreams.back()->streamId(), ipuStreams.back()->width(),
                 ipuStreams.back()->height(), ipuStreams.back()->useCase());
        }
    }

    // Re-query if extraStreams is not supported
    if (ret != OK) {
        ret = queryGraphSettings(ipuStreams);
    }
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to config streams", __func__);

    // Get the whole graph (ipu + post processor)
    // Ignore extra stream because it must be from ipu stream and don't need any postprocessing.
    ret = createPipeGraphConfigData(outStreams, outStreamIpuFlags,
                                    inStreams.empty() ? nullptr : inStreams.front());
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, Failed to get the static graph config data",
                     __func__);

    return configuredStreamNum;
}

void GraphConfig::chooseIpuOutputStreams(const vector<HalStream*>& halStreams,
                                         vector<int>* ipuStreamFlags) {
    map<int, const HalStream*> videoStreams;
    map<int, const HalStream*> stillStreams;

    for (uint32_t i = 0U; i < halStreams.size(); i++) {
        if (halStreams[i]->useCase() == USE_CASE_VIDEO) {
            videoStreams[i] = halStreams[i];
        } else {
            stillStreams[i] = halStreams[i];
        }
        ipuStreamFlags->push_back(i);  // all streams use IPU outputs if ipu stream slots are enough
    }

    size_t avaVideoSlot = icamera::PlatformData::getVideoStreamNum();
    size_t avaStillSlot = 1U;
    if (videoStreams.size() > avaVideoSlot)
        chooseIpuStreams(videoStreams, avaVideoSlot, ipuStreamFlags);
    if (stillStreams.size() > avaStillSlot)
        chooseIpuStreams(stillStreams, avaStillSlot, ipuStreamFlags);
}

void GraphConfig::chooseIpuStreams(map<int, const HalStream*>& streams, int avaSlot,
                                   vector<int>* ipuStreamFlags) {
    // Find all possible ipu streams (with different ratio) and put them into the front of vector.
    vector<int> ipuStreams;
    int selected = 0;
    int32_t bigStreamSameRatio = -1;  // Selected stream with same ratio as sensor's
    for (auto item = streams.begin(); item != streams.end(); ++item) {
        int idx = item->first;
        const HalStream* stream = item->second;

        // Select one ipu stream with same ratio as sensor's, it has high priority.
        // Only one with biggest size is chosen
        if (isSameRatio(stream, mSensorRatio)) {
            if (bigStreamSameRatio < 0) {
                bigStreamSameRatio = idx;
            } else {
                ipuStreams.push_back(idx);
            }
            continue;
        }

        bool found = false;
        for (auto ipuSIdx : ipuStreams) {
            if (isSameRatio(stream, streams[ipuSIdx])) {
                found = true;
            }
        }
        if (found) {
            ipuStreams.push_back(idx);
        } else {
            ipuStreams.insert(ipuStreams.begin() + selected, idx);
            selected++;
        }
    }

    /*
     * For some special streams which aren't from IPU, and their ratio aren't same as any ipu
     * stream, we assume to use the biggest stream for them.
     * This stream with the biggest size will be default ipu stream for those special streams.
     */
    int bigIpuStreamIdx = streams.begin()->first;
    if (bigStreamSameRatio >= 0) {
        ipuStreams.insert(ipuStreams.begin(), bigStreamSameRatio);
    }
    // Limit the number of ipu streams
    while (ipuStreams.size() > static_cast<size_t>(avaSlot)) ipuStreams.pop_back();
    // Reselect default ipu stream if there is only one ipu stream which might be stream with same
    // ratio as sensor's
    if (avaSlot == 1) {
        bigIpuStreamIdx = ipuStreams[0];
    }

    // Find own ipu streams for others
    for (auto& stream : streams) {
        int idx = stream.first;

        // Use default ipu stream with bigest size
        int finalIpuIdx = bigIpuStreamIdx;
        for (auto ipuSIdx : ipuStreams) {
            // It is ipu stream
            if (idx == ipuSIdx) {
                finalIpuIdx = idx;
                break;
            }
            // It is listener stream, find own ipu stream
            if (isSameRatio(streams[idx], streams[ipuSIdx])) {
                finalIpuIdx = ipuSIdx;
                break;
            }
        }

        ipuStreamFlags->at(idx) = finalIpuIdx;
    }
}

// Create PostStage info
status_t GraphConfig::createPipeGraphConfigData(const vector<HalStream*>& outStreams,
                                                const vector<int>& outStreamIpuFlags,
                                                const HalStream* inStream) {
    // TODO: Check if ipu stream number > max stream number
    // Create for all ipu streams
    createPostStages(outStreams, outStreamIpuFlags, inStream);

    // Handle other streams
    for (size_t i = 0U; i < outStreamIpuFlags.size(); i++) {
        int32_t ipuStreamId = outStreams[outStreamIpuFlags[i]]->streamId();
        fillOutputToPostProcessor(ipuStreamId, outStreams[i]);
    }

    // Remove disabled post processors
    for (size_t i = 0U; i < outStreamIpuFlags.size(); i++) {
        if (static_cast<int>(i) != outStreamIpuFlags[i]) {
            continue;
        }
        int32_t streamId = outStreams[i]->streamId();
        if ((mPostStageInfos.find(streamId) != mPostStageInfos.end()) &&
            (!mPostStageInfos[streamId].enabled))
            mPostStageInfos.erase(streamId);
        if ((mGPUStageInfos.find(streamId) != mGPUStageInfos.end()) &&
            (!mGPUStageInfos[streamId].enabled))
            mGPUStageInfos.erase(streamId);
    }

    dumpPostStageInfo();
    return OK;
}

void GraphConfig::createPostStages(const vector<HalStream*>& outStreams,
                                   const vector<int>& outStreamIpuFlags,
                                   const HalStream* inStream) {
    int32_t videoPostCount = 1;  // 0 is post processor of still

    for (size_t i = 0U; i < outStreamIpuFlags.size(); i++) {
        if (static_cast<int>(i) != outStreamIpuFlags[i]) {
            continue;
        }

        const HalStream* ipuStream = outStreams[i];
        if ((ipuStream->useCase() == USE_CASE_STILL) &&
            PlatformData::isGpuTnrEnabled(mCameraId)) {
            PostStageInfo gpuPost;
            gpuPost.stageName = GPU_POST_STAGE_NAME_BASE;
            gpuPost.stageId = GPU_TNR_STAGE_ID;
            gpuPost.streamId = STILL_STREAM_ID;
            gpuPost.stageName.append(std::to_string(0));
            gpuPost.inputStream = *ipuStream;
            // GPU TNR only supports NV12 format
            gpuPost.inputStream.mFormat = V4L2_PIX_FMT_NV12;
            gpuPost.enabled = true;
            mGPUStageInfos[ipuStream->streamId()] = gpuPost;
            LOG1("Create GPU stage: %s, id %x for stream %d", gpuPost.stageName.c_str(),
                 gpuPost.stageId, ipuStream->streamId());
        }

        PostStageInfo post;
        post.stageName = SW_POST_STAGE_NAME_BASE;
        if (ipuStream->useCase() == USE_CASE_STILL) {
            post.stageId = SW_POST_STAGE_ID_BASE;
            post.streamId = STILL_STREAM_ID;
            post.stageName.append(std::to_string(0));
        } else {
            post.stageId = SW_POST_STAGE_ID_BASE + videoPostCount;
            post.streamId = VIDEO_STREAM_ID;
            post.stageName.append(std::to_string(videoPostCount));
            videoPostCount++;
        }

        if ((ipuStream->format() == V4L2_PIX_FMT_JPEG) ||
            (PlatformData::useGPUProcessor() && (ipuStream->useCase() != USE_CASE_STILL))) {
            post.enabled = true;
            LOG1("Create post stage: %s, id %x for stream %d", post.stageName.c_str(), post.stageId,
                 ipuStream->streamId());
        }

        post.inputStream = *ipuStream;
        if (ipuStream->format() == V4L2_PIX_FMT_JPEG) {
            post.inputStream.mFormat = V4L2_PIX_FMT_NV12;
        }
        mPostStageInfos[ipuStream->streamId()] = post;
    }

    if (inStream) {
        // Create stage to handle user input and outputs directly
        PostStageInfo post;
        post.stageName = SW_POST_REPROCESSING_STAGE_NAME;
        post.stageId = SW_POST_REPROCESSING_STAGE_ID;
        post.streamId = YUV_REPROCESSING_STREAM_ID;
        post.inputStream = *inStream;
        for (auto stream : outStreams) {
            post.outputStreams.push_back(stream);
        }
        post.enabled = true;
        LOG1("Create post stage: %s, id %x for input stream %d", post.stageName.c_str(),
             post.stageId, inStream->streamId());
        mPostStageInfos[inStream->streamId()] = post;
    }
}

void GraphConfig::fillOutputToPostProcessor(int32_t ipuStreamId, const HalStream* stream) {
    if (mPostStageInfos.find(ipuStreamId) != mPostStageInfos.end()) {
        PostStageInfo& post = mPostStageInfos[ipuStreamId];
        post.outputStreams.push_back(stream);

        // Enable because it has other streams
        if (ipuStreamId != stream->streamId()) {
            post.enabled = true;
            LOG1("Create post stage %s, id %d for stream %d", post.stageName.c_str(), post.stageId,
                 ipuStreamId);
        }
    }

    if (mGPUStageInfos.find(ipuStreamId) != mGPUStageInfos.end()) {
        PostStageInfo& post = mGPUStageInfos[ipuStreamId];
        const HalStream* outStream = stream;
        // use SW post stage's input 0 as GPU stage's output
        if(mPostStageInfos.find(ipuStreamId) != mPostStageInfos.end()) {
            outStream = &(mPostStageInfos[ipuStreamId].inputStream);
        }
        post.outputStreams.push_back(outStream);
        post.enabled = true;
    }
}

status_t GraphConfig::graphGetStreamIds(vector<int32_t>& streamIds, bool fullPipes) {
    CheckAndLogError(mStaticGraphs.empty(), UNKNOWN_ERROR,
                     "%s, The streamIds vector is empty", __func__);
    for (auto& graph : mStaticGraphs) streamIds.push_back(graph.first);
    if (!fullPipes) {
        return OK;
    }

    for (auto& gpuPost : mGPUStageInfos) {
        bool found = false;
        for (auto id : streamIds) {
            if (gpuPost.second.streamId == id) {
                found = true;
                break;
            }
        }
        if (!found) {
            streamIds.push_back(gpuPost.second.streamId);
        }
    }

    for (auto& post : mPostStageInfos) {
        bool found = false;
        for (auto id : streamIds) {
            if (post.second.streamId == id) {
                found = true;
                break;
            }
        }
        if (!found) {
            streamIds.push_back(post.second.streamId);
        }
    }
    return OK;
}

int32_t GraphConfig::getGraphId() {
    CheckAndLogError(mStaticGraphs.empty(), NO_ENTRY, "%s: no graph", __func__);

    int32_t id = 0;
    // Assume static graphs have same graph id
    if (mStaticGraphs.begin()->second.staticGraph->getGraphId(&id) != StaticGraphStatus::SG_OK)
        LOGE("%s: failed for stream %d", __func__, mStaticGraphs.begin()->first);

    return id;
}

status_t GraphConfig::getStagesByStreamId(int32_t streamId,
                                          std::map<int32_t, std::string>* stages) {
    CheckAndLogError(!stages, BAD_VALUE, "%s: nullptr", __func__);

    if (mStaticGraphs.find(streamId) != mStaticGraphs.end()) {
        for (auto& stage : mStaticGraphs[streamId].stageInfos)
            stages->insert(std::pair<int32_t, std::string>(stage.stageId, stage.stageName));
    }

    for (auto& gpuStage : mGPUStageInfos) {
        PostStageInfo& info = gpuStage.second;
        if (info.streamId == streamId)
            stages->insert(std::pair<int32_t, std::string>(info.stageId, info.stageName));
    }

    for (auto& stage : mPostStageInfos) {
        PostStageInfo& info = stage.second;
        if (info.streamId == streamId)
            stages->insert(std::pair<int32_t, std::string>(info.stageId, info.stageName));
    }

    return OK;
}

status_t GraphConfig::getOuterNodes(int32_t streamId,
                                    std::map<int32_t, OuterNode*>& outerNodes) {
    CHECK_STREAM_ID(streamId);
    outerNodes.clear();
    for (auto& stage : mStaticGraphs[streamId].stageInfos) {
        outerNodes[stage.node->contextId] = stage.node;
    }

    return outerNodes.empty() ? NO_ENTRY : OK;
}

uint8_t GraphConfig::getPSysContextId(int32_t streamId, uint8_t outerNodeCtxId) {
    for (auto &gc : mStaticGraphs) {
        if (streamId != gc.first) {
            continue;
        }
        for (uint32_t i = 0U; i < gc.second.stageInfos.size(); i++) {
            IpuStageInfo& info = gc.second.stageInfos[i];
            if (info.node->contextId == outerNodeCtxId) {
                return static_cast<uint8_t>(i);
            }
        }
    }

    LOGE("%s: Can't find node, stream %d, outerNode ctxId %u", __func__, streamId, outerNodeCtxId);
    return 0xFF;
}

int32_t GraphConfig::getGraphLinks(int32_t streamId, GraphLink*** links) {
    CheckAndLogError(!links, BAD_VALUE, "%s: nullptr", __func__);
    CHECK_STREAM_ID(streamId);

    GraphTopology* pGraphTopology = nullptr;
    StaticGraphStatus status =
                    mStaticGraphs[streamId].staticGraph->getGraphTopology(&pGraphTopology);
    CheckAndLogError(status != StaticGraphStatus::SG_OK, 0, "%s: no links for stream %d",
                      __func__, streamId);

    *links = pGraphTopology->links;
    return pGraphTopology->numOfLinks;
}

int32_t GraphConfig::loadPipeConfig(const string& fileName) {
    {
        AutoMutex l(sLock);
        if (mGraphConfigBinaries.find(mCameraId) != mGraphConfigBinaries.end())
            return OK;
    }

    string settingsFile = PlatformData::getGraphSettingFilePath() + fileName;
    int32_t ret = loadStaticGraphConfig(settingsFile);
    LOG1("Load file %s, result %d", settingsFile.c_str(), ret);
    return ret;
}

int32_t GraphConfig::loadStaticGraphConfig(const std::string& name) {
    const char* fileName = name.c_str();
    struct stat statBuf;
    int32_t ret = stat(fileName, &statBuf);
    CheckAndLogError(ret != OK, ret, "Failed to query the size of file: %s!", fileName);

    FILE* file = fopen(fileName, "rb");
    CheckAndLogError(!file, NAME_NOT_FOUND, "%s, Failed to open file: %s", __func__, fileName);

    StaticReaderBinaryData binData;
    binData.size = static_cast<uint32_t>(statBuf.st_size);
    binData.data = malloc(binData.size);
    if (binData.data == nullptr) {
        LOGE("%s: inputBinary.data is null", __func__);
        (void)fclose(file);
        return NO_MEMORY;
    }

    size_t len = fread(binData.data, 1, binData.size, file);
    (void)fclose(file);
    if (len != binData.size) {
        LOGE("%s, read data %zu from file %s, should be %u", __func__, len, fileName, binData.size);
        free(binData.data);
        binData.data = nullptr;
        return BAD_VALUE;
    }

    AutoMutex l(sLock);
    mGraphConfigBinaries[mCameraId] = binData;
    return OK;
}

void GraphConfig::getStaticGraphConfigData(const std::map<VirtualSink, const HalStream*>& streams) {
    for (auto& item : mStaticGraphs) {
        int32_t streamId = item.first;
        StaticGraphInfo& graph = item.second;
        GraphTopology* pGraphTopology = nullptr;
        StaticGraphStatus status = graph.staticGraph->getGraphTopology(&pGraphTopology);
        if (status != StaticGraphStatus::SG_OK) {
            continue;
        }

        std::map<HwSink, const HalStream*> streamsSinkMap;
        for (auto& stream : streams) {
            VirtualSink vSink = stream.first;
            HwSink hSink = HwSink::Disconnected;
            if (StaticGraphStatus::SG_OK ==
                graph.staticGraph->getVirtualSinkConnection(vSink, &hSink))
                streamsSinkMap[hSink] = stream.second;
            LOG3("%s: hal stream %d, vSink %d, hSink %d", __func__, stream.second->streamId(),
                 vSink, hSink);
        }

        // Disable redundant outputs that are enabled by StaticGraph but no user stream related to.
        SubGraphPublicInnerNodeConfiguration imageInnerOptions;
        if (streamsSinkMap.find(HwSink::ImageMpSink) == streamsSinkMap.end())
            imageInnerOptions.noMp = true;
        if (streamsSinkMap.find(HwSink::ImageDpSink) == streamsSinkMap.end())
            imageInnerOptions.noDp = true;
        SubGraphInnerNodeConfiguration innerConfig;
        innerConfig.imageInnerOptions = &imageInnerOptions;
        if (StaticGraphStatus::SG_OK != pGraphTopology->configInnerNodes(innerConfig)) {
            LOGW("%s: configInnerNodes failed for pipe %d", __func__, streamId);
        }

        for (int32_t i = 0; i < pGraphTopology->numOfLinks; i++) {
            const GraphLink* link = pGraphTopology->links[i];
            dumpLink(link);
            saveOuterNode(link, &graph);
            saveLink(streamId, link, &streamsSinkMap, &graph);
        }

        graph.graphResolutionConfig = new GraphResolutionConfigurator(graph.staticGraph);
        dumpNodes(graph);
    }
}

void GraphConfig::saveOuterNode(const GraphLink* link, StaticGraphInfo* graph) {
    // Save nodes, except ISYS
    if ((!link->isActive) || (link->destNode == nullptr) ||
        (link->destNode->type == NodeTypes::Isys)) {
        return;
    }

    int32_t stageId = GraphUtils::createStageId(link->destNode->resourceId,
                                                link->destNode->contextId);
    for (auto& stage : graph->stageInfos)
        if (stage.stageId == stageId) {
            return;
        }

    IpuStageInfo info;
    info.streamId = link->destNode->nodeKernels.streamId;
    info.stageId = stageId;
    info.stageName = GraphUtils::getStageName(info.stageId, info.streamId);
    info.node = link->destNode;

    bool saved = false;
    for (uint32_t i = 0U; i < graph->stageInfos.size(); i++) {
        uint8_t refId= graph->stageInfos[i].node->contextId;
        if (info.node->contextId < refId) {
            graph->stageInfos.insert(graph->stageInfos.begin() + i, info);
            saved = true;
        }
    }
    if (!saved) {
        graph->stageInfos.push_back(info);
    }

    LOG3("%s: stream %d, node %s:%d, context %d", __func__, info.streamId, info.stageName.c_str(),
         info.stageId, info.node->contextId);
}

void GraphConfig::saveLink(int32_t streamId, const GraphLink* link,
                           std::map<HwSink, const HalStream*>* streams, StaticGraphInfo* graph) {
    if (!link->isActive) {
        return;
    }
    // Ignore link: src="-1:Sensor:0" dest="2:Isys:0" type="Source2Node"
    if ((link->type == LinkType::Source2Node) && (link->destNode->type == NodeTypes::Isys)) {
        return;
    }

    IpuGraphLink ipuLink(streamId, link);
    bool hasNecessaryNode = true;
    if (link->type == LinkType::Source2Node) {
        // src="-1:LscBuffer:0" dest="0:LbffBayer:4" type="Source2Node"
        ipuLink.isEdge = true;
        hasNecessaryNode = link->destNode;
    } else if ((link->type == LinkType::Node2Node) && (link->srcNode->type == NodeTypes::Isys)) {
        // src="2:Isys:1" dest="0:LbffBayer:3" type="Node2Node"
        ipuLink.isEdge = true;
        hasNecessaryNode = link->destNode;
    } else if (link->type == LinkType::Node2Sink) {
        // src="0:LbffBayer:6" dest="-1:AeOut:0" type="Node2Sink"
        // src="1:BbpsNoTnr:14" dest="-1:ImageMp:0" type="Node2Sink"
        ipuLink.isEdge = true;
        hasNecessaryNode = link->srcNode;
        // Find output stream
        for (auto& s : *streams) {
            if (((link->dest == GraphElementType::ImageMp) && (s.first == HwSink::ImageMpSink)) ||
                ((link->dest == GraphElementType::ImageDp) && (s.first == HwSink::ImageDpSink))) {
                ipuLink.stream = s.second;
                streams->erase(s.first);
                break;
            }
        }
    }

    LOG3("%s: save to ipuLink? %s, isEdge? %s, hal stream id %d", __func__,
          hasNecessaryNode ? "yes" : "no", ipuLink.isEdge ? "yes" : "no",
          ipuLink.stream ? ipuLink.stream->streamId() : -1);

    if (hasNecessaryNode) {
        graph->links.push_back(ipuLink);
    }
}

// Check which node in link has frame terminal. Return source node if both have.
OuterNode* GraphConfig::findFrameTerminalOwner(const GraphLink* link) {
    if (link->destNode &&
        CBLayoutUtils::isFrameTerminal(link->destNode->resourceId, link->destTerminalId))
        return link->destNode;

    if (link->srcNode &&
        CBLayoutUtils::isFrameTerminal(link->srcNode->resourceId, link->srcTerminalId))
        return link->srcNode;

    return nullptr;
}

status_t GraphConfig::fillConnectionFormat(const IpuGraphLink& ipuLink, const OuterNode* node,
                                           IGraphType::PortFormatSettings* fmtSettings) {
    const GraphLink* link = ipuLink.graphLink;

    bool useDest = (node == link->destNode) ? true : false;
    int32_t terminal = useDest ? link->destTerminalId : link->srcTerminalId;
    const StaticGraphRunKernel* kernel = nullptr;
    kernel = findKernelForFrameTerminal(node, terminal);
    CheckAndLogError((ipuLink.stream == nullptr) && (kernel == nullptr), NO_ENTRY,
                     "%s: Can't find kernel for link", __func__);

    int32_t stageId = GraphUtils::createStageId(node->resourceId, node->contextId);
    fmtSettings->enabled = true;
    fmtSettings->terminalId = PORT_UID(ipuLink.streamId, stageId, terminal);
    // Use user stream instead of srcNode kernel info if it is valid (link type: Node2Sink)
    if (ipuLink.stream) {
        fmtSettings->width = ipuLink.stream->width();
        fmtSettings->height = ipuLink.stream->height();
    } else {
        if (kernel->resolution_info) {
            fmtSettings->width = useDest ? kernel->resolution_info->input_width
                                          : kernel->resolution_info->output_width;
            fmtSettings->height = useDest ? kernel->resolution_info->input_height
                                           : kernel->resolution_info->output_height;
        } else if (kernel->resolution_history) {
            fmtSettings->width = kernel->resolution_history->output_width;
            fmtSettings->height = kernel->resolution_history->output_height;
        }
    }
    int32_t bpp = useDest ? kernel->bpp_info.input_bpp : kernel->bpp_info.output_bpp;
    fmtSettings->fourcc = GraphUtils::getFourccFmt(node->resourceId, terminal, bpp);
    fmtSettings->format = CameraUtils::getV4L2Format(fmtSettings->fourcc);

    fmtSettings->bpl= CameraUtils::getBpl(fmtSettings->fourcc, fmtSettings->width);
    fmtSettings->bpp = CameraUtils::getBpp(fmtSettings->fourcc);
    LOG3("      format: term %x, %dx%d, %s, bpl %d, bpp %d", fmtSettings->terminalId,
         fmtSettings->width, fmtSettings->height,
         CameraUtils::fourcc2String(fmtSettings->fourcc).c_str(), fmtSettings->bpl,
         fmtSettings->bpl);

    return OK;
}

const StaticGraphRunKernel*
GraphConfig::findKernelForFrameTerminal(const OuterNode* node, int32_t terminalId) {
    int32_t kernelId = CBLayoutUtils::getKernelForDataTerminal(node->resourceId, terminalId);
    if (kernelId > 0) {
        for (uint32_t i = 0U; i < node->nodeKernels.kernelCount; i++) {
            if ((node->nodeKernels.kernelList[i].run_kernel.enable == 1U) &&
                (node->nodeKernels.kernelList[i].run_kernel.kernel_uuid == static_cast<uint32_t>(kernelId))) {
                return &(node->nodeKernels.kernelList[i].run_kernel);
            }
        }
    }
    LOGE("%s: No kernel %d for CB resource %d, term %d", __func__, kernelId, node->resourceId,
         terminalId);
    return nullptr;
}

void GraphConfig::fillConnectionConfig(const IpuGraphLink& ipuLink, int32_t terminalId,
                                       IGraphType::ConnectionConfig* conn) {
    const GraphLink* link = ipuLink.graphLink;
    int32_t stageId = 0;

    // Fill source
    if (link->srcNode && (link->srcNode->type != NodeTypes::Isys)) {
        stageId = GraphUtils::createStageId(link->srcNode->resourceId, link->srcNode->contextId);
        conn->mSourceStage = STAGE_UID(ipuLink.streamId, stageId);
        conn->mSourceTerminal = PORT_UID(ipuLink.streamId, stageId, link->srcTerminalId);
    }

    // Fill sink, use output stream in priority
    if (link->destNode) {
        stageId = GraphUtils::createStageId(link->destNode->resourceId, link->destNode->contextId);
        conn->mSinkStage = STAGE_UID(ipuLink.streamId, stageId);
        conn->mSinkTerminal = PORT_UID(ipuLink.streamId, stageId, link->destTerminalId);
    }

    if (static_cast<size_t>(terminalId) == conn->mSinkTerminal)
        conn->mConnectionType = IGraphType::connection_type_pull;
    else if (static_cast<size_t>(terminalId) == conn->mSourceTerminal)
        conn->mConnectionType = IGraphType::connection_type_push;

    LOG3("      conn: %x:%x -> %x:%x, type %d", conn->mSourceStage, conn->mSourceTerminal,
         conn->mSinkStage, conn->mSinkTerminal, conn->mConnectionType);
}

status_t GraphConfig::updateGraphSettingForPtz(const PtzInfo& cur, const PtzInfo& prev,
                                               bool* isKeyResChanged) {
    RegionOfInterest cRoi = {cur.zoomRatio, cur.x, cur.y, false};
    RegionOfInterest rRoi = {prev.zoomRatio, prev.x, prev.y, false};

    for (auto& item : mStaticGraphs) {
        StaticGraphInfo& info = item.second;
        if (!info.graphResolutionConfig) {
            continue;
        }

        bool changed = false;
        StaticGraphStatus ret = info.graphResolutionConfig->updateStaticGraphConfig(
                                cRoi, rRoi, cur.zoomCentered, prev.zoomCentered, changed);
        CheckAndLogError(ret != StaticGraphStatus::SG_OK, UNKNOWN_ERROR,
                         "Update resolution for PTZ fail for stream %d", item.first);
        LOG2("%s: update done for stream %d, isKeyResChanged %d", __func__, item.first,
             changed);
        if (isKeyResChanged != nullptr) {
            *isKeyResChanged = changed;
        }

        dumpNodes(info);
    }
    return OK;
}

void GraphConfig::dumpLink(const GraphLink* link) {
#define INVALID_SOURCE_ID (0xff)
    uint8_t srcId = link->srcNode ? link->srcNode->resourceId : INVALID_SOURCE_ID;
    uint8_t destId = link->destNode ? link->destNode->resourceId : INVALID_SOURCE_ID;
    LOG3("<link active=\"%d\" src=\"%d:%d\" dest=\"%d:%d\" delay=\"%d\" type=\"%d\"/>",
         link->isActive, srcId, link->srcTerminalId, destId, link->destTerminalId,
         link->frameDelay, link->type);
}

void GraphConfig::dumpLink(const IpuGraphLink& ipuLink) {
    const GraphLink* link = ipuLink.graphLink;
    uint8_t srcId = link->srcNode ? link->srcNode->resourceId : INVALID_SOURCE_ID;
    uint8_t destId = link->destNode ? link->destNode->resourceId : INVALID_SOURCE_ID;
    int32_t halStreamId = (ipuLink.stream) ? ipuLink.stream->streamId() : -1;
    LOG3("<link active=\"%d\" src=\"%d:%d\" dest=\"%d:%d\" delay=\"%d\" type=\"%d\" isEdge=\"%d\""
         " halstream=\"%d\"/>", link->isActive, srcId, link->srcTerminalId, destId,
         link->destTerminalId, link->frameDelay, link->type, ipuLink.isEdge, halStreamId);
}

void GraphConfig::dumpNodes(const StaticGraphInfo& graph) {
    if ((!Log::isLogTagEnabled(GET_FILE_SHIFT(GraphConfig))) ||
        (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_LEVEL3))) return;

    for (auto& stage : graph.stageInfos) {
        if (!stage.node) {
            continue;
        }
        LOG3("<node res=\"%d\" stream=\"%d\" >", stage.node->resourceId, stage.streamId);

        for (uint32_t i = 0U; i < stage.node->nodeKernels.kernelCount; i++) {
            const StaticGraphPacRunKernel& kernel =
                static_cast<const StaticGraphPacRunKernel>(stage.node->nodeKernels.kernelList[i]);
            const StaticGraphRunKernel& k =
                static_cast<const StaticGraphRunKernel&>(kernel.run_kernel);
            if (!k.enable) {
                continue;
            }

            LOG3("  <kernel id=\"%d\">", k.kernel_uuid);
            if (k.resolution_info) {
                LOG3("    <in w=\"%d\" h=\"%d\" crop=\"(%d, %d, %d, %d)\" />",
                     k.resolution_info->input_width, k.resolution_info->input_height,
                     k.resolution_info->input_crop.left, k.resolution_info->input_crop.top,
                     k.resolution_info->input_crop.right, k.resolution_info->input_crop.bottom);
                LOG3("    <out w=\"%d\" h=\"%d\" crop=\"(%d, %d, %d, %d)\" >",
                     k.resolution_info->output_width, k.resolution_info->output_height,
                     k.resolution_info->output_crop.left, k.resolution_info->output_crop.top,
                     k.resolution_info->output_crop.right, k.resolution_info->output_crop.bottom);
            }
            if (k.resolution_history) {
                LOG3("    <history_in w=\"%d\" h=\"%d\" crop=\"(%d, %d, %d, %d)\" />",
                     k.resolution_history->input_width, k.resolution_history->input_height,
                     k.resolution_history->input_crop.left, k.resolution_history->input_crop.top,
                     k.resolution_history->input_crop.right, k.resolution_history->input_crop.bottom);
                LOG3("    <history_out w=\"%d\" h=\"%d\" crop=\"(%d, %d, %d, %d)\" >",
                     k.resolution_history->output_width, k.resolution_history->output_height,
                     k.resolution_history->output_crop.left, k.resolution_history->output_crop.top,
                     k.resolution_history->output_crop.right, k.resolution_history->output_crop.bottom);
            }
        }
        LOG3("</node>");
    }
}

StageType GraphConfig::getPGType(int32_t pgId) {
    for (auto& info : mGPUStageInfos) {
        if (info.second.stageId == pgId) {
            return STAGE_GPU_TNR;
        }
    }
    for (auto& info : mPostStageInfos) {
        if (info.second.stageId == pgId) {
            return STAGE_SW_POST;
        }
    }
    return STAGE_IPU;
}

status_t GraphConfig::pipelineGetConnections(int32_t streamId,
    vector<IGraphType::PipelineConnection>* confVector,
    std::vector<IGraphType::PrivPortFormat>* tnrPortFormat) {
    CheckAndLogError(!confVector, UNKNOWN_ERROR, "%s, The confVector is nullptr", __func__);

    // Find PostStage for YUV reprocessing pipe
    if (streamId == YUV_REPROCESSING_STREAM_ID) {
        for (auto& post : mPostStageInfos) {
            PostStageInfo& info = post.second;
            if (info.streamId != YUV_REPROCESSING_STREAM_ID) {
                continue;
            }

            IGraphType::PipelineConnection sink;
            sink.stream = &info.inputStream;
            checkAndUpdatePostConnection(streamId, &sink, confVector, mPostStageInfos);
            confVector->push_back(sink);
            break;
        }
        return OK;
    }

    CHECK_STREAM_ID(streamId);
    for (auto& ipuLink : mStaticGraphs[streamId].links) {
        // Currently only return frame links (include sis)
        dumpLink(ipuLink);
        const OuterNode* node = findFrameTerminalOwner(ipuLink.graphLink);
        if (!node) {
            continue;
        }

        IGraphType::PipelineConnection conn;
        (void)fillConnectionFormat(ipuLink, node, &conn.portFormatSettings);
        fillConnectionConfig(ipuLink, conn.portFormatSettings.terminalId, &conn.connectionConfig);
        conn.stream = ipuLink.stream;
        conn.hasEdgePort = ipuLink.isEdge;
        confVector->push_back(conn);
    }

    vector<IGraphType::PipelineConnection> postVector;
    for (auto& conn : *confVector) {
        // TODO: clean sink/source if the link is between streams
        if (!conn.portFormatSettings.enabled) {
            continue;
        }
        if (conn.stream != nullptr) {
            checkAndUpdatePostConnection(streamId, &conn, &postVector, mGPUStageInfos);
        }
        IGraphType::PipelineConnection* connPtr =
            postVector.size() > 0 ? &(postVector[postVector.size() - 1]) : &conn;
        if (connPtr->stream)
            checkAndUpdatePostConnection(streamId, connPtr, &postVector, mPostStageInfos);
    }
    confVector->insert(confVector->end(), postVector.begin(), postVector.end());
    LOG3("%s dump for stream %d ++", __func__, streamId);
    GraphUtils::dumpConnections(*confVector);
    LOG3("%s dump for stream %d --", __func__, streamId);
    return OK;
}

void GraphConfig::checkAndUpdatePostConnection(int32_t streamId,
                                               IGraphType::PipelineConnection* conn,
                                               vector<IGraphType::PipelineConnection>* postVector,
                                               std::map<int32_t, PostStageInfo>& postStageInfos) {
    int32_t useStreamId = conn->stream->streamId();

    if (postStageInfos.find(useStreamId) == postStageInfos.end()) {
        return;
    }

    PostStageInfo& info = postStageInfos[useStreamId];
    uuid stageUuid = STAGE_UID(streamId, info.stageId);
    // No terminal but input stream, update conn for post stage sink terminal (YUV reprocessing)
    if (conn->portFormatSettings.terminalId == INVALID_PORT) {
        // post stage is both inEdge and outEdge, update sink conn (for YUV reprocessing case)
        LOG2("%s: add post %s for user input stream %d", __func__, info.stageName.c_str(),
             useStreamId);
        conn->portFormatSettings.terminalId = PORT_UID(streamId, info.stageId, POST_STAGE_INPUT);
        conn->portFormatSettings.width = info.inputStream.width();
        conn->portFormatSettings.height = info.inputStream.height();
        conn->portFormatSettings.format = info.inputStream.format();
        conn->portFormatSettings.fourcc = CameraUtils::getFourccFormat(info.inputStream.format());
        conn->portFormatSettings.bpl =
            CameraUtils::getBpl(conn->portFormatSettings.fourcc, conn->portFormatSettings.width);
        conn->portFormatSettings.bpp = CameraUtils::getBpp(conn->portFormatSettings.fourcc);
        conn->portFormatSettings.enabled = true;
        conn->connectionConfig.mSourceStage = INVALID_PORT;
        conn->connectionConfig.mSourceTerminal = INVALID_PORT;
        conn->connectionConfig.mSinkStage = stageUuid;
        conn->connectionConfig.mSinkTerminal = conn->portFormatSettings.terminalId;
        conn->stream = nullptr;
        conn->hasEdgePort = true;
    } else {
        // Add post stage as outEdge, update conn
        LOG2("%s: add post %s after stage: %d, size %zu", __func__, info.stageName.c_str(),
             conn->connectionConfig.mSourceStage, info.outputStreams.size());
        // Original: ipu output -> user stream
        // Changed: ipu output -> post stage sink
        conn->hasEdgePort = false;
        conn->stream = nullptr;
        conn->connectionConfig.mSinkStage = stageUuid;
        conn->connectionConfig.mSinkTerminal = PORT_UID(streamId, info.stageId, POST_STAGE_INPUT);
    }

    // Add source terminals conn for post stage
    for (size_t i = 0U; i < info.outputStreams.size(); i++) {
        const HalStream* stream = info.outputStreams[i];
        IGraphType::PipelineConnection source;
        source.portFormatSettings.terminalId =
            PORT_UID(streamId, info.stageId, POST_STAGE_OUTPUT_BASE + i);
        source.portFormatSettings.width = stream->width();
        source.portFormatSettings.height = stream->height();
        source.portFormatSettings.format = stream->format();
        source.portFormatSettings.fourcc = CameraUtils::getFourccFormat(stream->format());
        source.portFormatSettings.bpl =
            CameraUtils::getBpl(source.portFormatSettings.fourcc, stream->width());
        source.portFormatSettings.bpp = CameraUtils::getBpp(source.portFormatSettings.fourcc);
        source.portFormatSettings.enabled = true;
        source.connectionConfig = conn->connectionConfig;
        source.connectionConfig.mSourceStage = stageUuid;
        source.connectionConfig.mSourceTerminal = source.portFormatSettings.terminalId;
        source.connectionConfig.mSinkStage = INVALID_PORT;
        source.connectionConfig.mSinkTerminal = INVALID_PORT;
        source.stream = stream;
        source.hasEdgePort = true;
        postVector->push_back(source);
        LOG2("%s: add post out term %x", __func__, source.portFormatSettings.terminalId);
    }
}

status_t GraphConfig::graphGetEdgeConnections(
                         std::vector<IGraphType::PipelineConnection> *confVector) {
    vector<int32_t> streamIds;
    (void)graphGetStreamIds(streamIds);
    for (auto id : streamIds) {
        vector<IGraphType::PipelineConnection> confV;
        std::vector<IGraphType::PrivPortFormat> tnrPortFormat;
        status_t ret = pipelineGetConnections(id, &confV, &tnrPortFormat);
        CheckAndLogError(ret != OK, ret, "%s, Failed to pipelineGetConnections", __func__);

        for (auto& conn : confV) {
            if (conn.portFormatSettings.enabled &&
                ((conn.connectionConfig.mSinkStage == 0U) ||
                 (conn.connectionConfig.mSourceStage == 0U))) {
                confVector->push_back(conn);
            }
        }
    }

    return OK;
}

void GraphConfig::dumpPostStageInfo() {
    if ((!Log::isLogTagEnabled(GET_FILE_SHIFT(GraphConfig))) ||
        (!Log::isDebugLevelEnable(CAMERA_DEBUG_LOG_LEVEL3))) return;

    for (auto& item : mGPUStageInfos) {
        PostStageInfo& info= item.second;
        LOG3("dump %s: user input stream %d: %dx%d, fmt %x, usecase %d, pipe stream %d",
             info.stageName.c_str(), info.inputStream.streamId(), info.inputStream.width(),
             info.inputStream.height(), info.inputStream.format(), info.inputStream.useCase(),
             info.streamId);
        for (auto& s : info.outputStreams) {
            LOG3("           output stream %d: %dx%d, fmt %x, usecase %d", s->streamId(),
                 s->width(), s->height(), s->format(), s->useCase());
        }
    }
    for (auto& item : mPostStageInfos) {
        PostStageInfo& info= item.second;
        LOG3("dump %s: user input stream %d: %dx%d, fmt %x, usecase %d, pipe stream %d",
             info.stageName.c_str(), info.inputStream.streamId(), info.inputStream.width(),
             info.inputStream.height(), info.inputStream.format(), info.inputStream.useCase(),
             info.streamId);
        for (auto& s : info.outputStreams) {
            LOG3("           output stream %d: %dx%d, fmt %x, usecase %d", s->streamId(),
                 s->width(), s->height(), s->format(), s->useCase());
        }
    }
}

status_t GraphConfig::getIspRawCropInfo(IspRawCropInfo& info) {
    std::map<int32_t, OuterNode*> nodes;
    int32_t streamId = VIDEO_STREAM_ID;
    status_t ret;

    if (mStaticGraphs.empty()) {
        return NO_ENTRY;
    }
    if (mStaticGraphs.find(streamId) == mStaticGraphs.end()) {
        streamId = mStaticGraphs.begin()->first;
    }

    ret = getOuterNodes(streamId, nodes);
    if (ret != OK) {
        return ret;
    }

    uint32_t kernel_id = CBLayoutUtils::getIspIfdKernelId();
    for (auto& node : nodes) {
        StaticGraphNodeKernels& nks = node.second->nodeKernels;
        for (uint32_t i = 0U; i < nks.kernelCount; ++i) {
            if (nks.kernelList[i].run_kernel.kernel_uuid == kernel_id) {
                info.left = nks.kernelList[i].run_kernel.resolution_info->input_crop.left;
                info.top = nks.kernelList[i].run_kernel.resolution_info->input_crop.top;
                info.right = nks.kernelList[i].run_kernel.resolution_info->input_crop.right;
                info.bottom = nks.kernelList[i].run_kernel.resolution_info->input_crop.bottom;

                info.outputWidth = nks.kernelList[i].run_kernel.resolution_info->output_width;
                info.outputHeight = nks.kernelList[i].run_kernel.resolution_info->output_height;
                return OK;
            }
        }
    }

    return NAME_NOT_FOUND;
}

status_t GraphConfig::getIspTuningModeByStreamId(int32_t streamId, uint32_t& ispTuningMode) {
    for (auto& gc : mStaticGraphs) {
        if (streamId != gc.first) {
            continue;
        }
        if (gc.second.stageInfos.size() > 0) {
            auto& info = gc.second.stageInfos[0];
            ispTuningMode = info.node->nodeKernels.operationMode;

            return OK;
        }
    }

    return NAME_NOT_FOUND;
}

}  // namespace icamera
