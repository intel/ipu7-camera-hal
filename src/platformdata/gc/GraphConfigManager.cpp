/*
 * Copyright (C) 2015-2023 Intel Corporation
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

#define LOG_TAG GraphConfigManager

#include "src/platformdata/gc/GraphConfigManager.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

using std::map;
using std::vector;

namespace icamera {
GraphConfigManager::GraphConfigManager(int32_t cameraId)
        : mCameraId(cameraId),
          mMcId(-1) {}

GraphConfigManager::~GraphConfigManager() {
    releaseHalStream(&mHalStreams);
}

void GraphConfigManager::releaseHalStream(std::vector<HalStream*>* halStreamVec) {
    for (auto& halStream : *halStreamVec) {
        delete halStream;
    }
    (*halStreamVec).clear();
}

/*
 * Get the useCase from the stream and operationMode.
 */
PipeUseCase GraphConfigManager::getUseCaseFromStream(ConfigMode configMode,
                                                       const stream_t& stream) {
    if ((configMode == CAMERA_STREAM_CONFIGURATION_MODE_STILL_CAPTURE) ||
        (stream.usage == CAMERA_STREAM_STILL_CAPTURE))
        return USE_CASE_STILL;

    return USE_CASE_VIDEO;
}

/*
 * Create hal stream vector.
 */
int GraphConfigManager::createHalStreamVector(ConfigMode configMode,
                                              const stream_config_t* streamList,
                                              std::vector<HalStream*>* halStreamVec) {
    CheckAndLogError(!streamList, BAD_VALUE, "%s: Null streamList configured", __func__);
    LOG2("%s", __func__);

    // Convert the stream_t to HalStream
    // Use the stream list with descending order to find graph settings.
    for (int i = 0; i < streamList->num_streams; i++) {
        // Don't handle opaque RAW stream when configure graph configuration.
        if (streamList->streams[i].usage == CAMERA_STREAM_OPAQUE_RAW)
            continue;

        bool stored = false;
        PipeUseCase useCase = getUseCaseFromStream(configMode, streamList->streams[i]);
        streamProps props = {
            static_cast<uint32_t>(streamList->streams[i].width),
            static_cast<uint32_t>(streamList->streams[i].height),
            streamList->streams[i].format,
            streamList->streams[i].id,
            useCase,
        };
        HalStream* halStream = new HalStream(props, static_cast<void*>(&streamList->streams[i]));
        if (!halStream) {
            LOGE("Failed to create hal stream");
            releaseHalStream(halStreamVec);
            return NO_MEMORY;
        }

        for (size_t j = 0U; j < (*halStreamVec).size(); j++) {
            if (halStream->width() * halStream->height() >
                (*halStreamVec)[j]->width() * (*halStreamVec)[j]->height()) {
                stored = true;
                (*halStreamVec).insert(((*halStreamVec).begin() + j), halStream);
                break;
            }
        }
        if (!stored) {
            (*halStreamVec).push_back(halStream);
        }
    }

    return OK;
}

/**
 * Initialize the state of the GraphConfigManager after parsing the stream
 * configuration.
 * Perform the first level query to find a subset of settings that fulfill the
 * constrains from the stream configuration.
 *
 * \param[in] outStreamList: all the usr output streams info.
 * \param[in] extraOutStream: extra outputstream to support.

 * Return configured stream number: could be:
 *     outStreamList.num_streams: don't support extraOutStream
 *     outStreamList.num_streams + 1: support extraOutStream and configured
 *     < 0: configure failed.
 */
int32_t GraphConfigManager::configStreams(const stream_config_t* streamList,
                                          std::map<ConfigMode, std::shared_ptr<GraphConfig> >& gcs,
                                          const stream_t* extraStream) {
    HAL_TRACE_CALL(CAMERA_DEBUG_LOG_LEVEL1);
    CheckAndLogError(!streamList, BAD_VALUE, "%s: Null streamList configured", __func__);

    std::vector<ConfigMode> configModes;
    int ret = PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode,
                                                          configModes);
    CheckAndLogError(ret != OK, ret, "%s, get ConfigMode failed %d", __func__, ret);

    // Convert the stream_t to HalStream (ignore input stream)
    // Use the stream list with descending order to find graph settings.
    releaseHalStream(&mHalStreams);
    ret = createHalStreamVector(configModes[0], streamList, &mHalStreams);
    CheckAndLogError(ret != OK, ret, "%s, create hal stream failed %d", __func__, ret);

    std::vector<HalStream*> extraOutHalStreams;
    if (extraStream) {
        stream_t extraS = *extraStream;
        stream_config_t extraStreamList = {1, &extraS, streamList->operation_mode};
        ret = createHalStreamVector(configModes[0], &extraStreamList, &extraOutHalStreams);
        CheckAndLogError(ret != OK, ret, "%s, create extra hal stream failed %d", __func__, ret);
    }

    // debug
    dumpStreamConfig();
    mMcId = -1;

    int configuredNum = 0;
    for (auto mode : configModes) {
        LOG1("Mapping the operationMode %d to ConfigMode %d", streamList->operation_mode, mode);

        std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>(mCameraId, mode);
        CheckAndLogError(!graphConfig, UNKNOWN_ERROR, "%s, Failed to create graphConfig", __func__);
        configuredNum = graphConfig->configStreams(mHalStreams, extraOutHalStreams);
        CheckWarning(configuredNum <= 0, ret, "%s, Failed to configure graph: real ConfigMode %x",
                     __func__, mode);

        int id = graphConfig->getSelectedMcId();
        CheckAndLogError((id != -1) && (mMcId != -1) && (mMcId != id), UNKNOWN_ERROR,
                         "Not support two different MC ID at same time:(%d/%d)", mMcId, id);
        mMcId = id;
        LOG2("%s: Add graph setting for op_mode %d", __func__, mode);
        gcs[mode] = std::move(graphConfig);
    }

    if (static_cast<size_t>(configuredNum) > mHalStreams.size()) {
        mHalStreams.insert(mHalStreams.end(), extraOutHalStreams.begin(), extraOutHalStreams.end());
        // Configurate streamList successfully and support extra streams.
        configuredNum = streamList->num_streams + extraOutHalStreams.size();
    }

    return configuredNum;
}

void GraphConfigManager::dumpStreamConfig() {
    for (size_t i = 0U; i < mHalStreams.size(); i++) {
        if (static_cast<stream_t*>(mHalStreams[i]->mPrivate)->streamType == CAMERA_STREAM_OUTPUT) {
            LOG1("out stream[%zu] %dx%d, fmt %s", i, mHalStreams[i]->width(),
                 mHalStreams[i]->height(), CameraUtils::pixelCode2String(mHalStreams[i]->format()));
        } else {
            LOG1("in stream[%zu] %dx%d, fmt %s", i, mHalStreams[i]->width(),
                 mHalStreams[i]->height(), CameraUtils::pixelCode2String(mHalStreams[i]->format()));
        }
    }
}

}  // namespace icamera
