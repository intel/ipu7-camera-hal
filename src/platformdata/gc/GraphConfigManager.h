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

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "GraphConfig.h"
#include "iutils/Errors.h"

namespace icamera {

/**
 * \class GraphConfigManager
 *
 * Class to wrap over parsing and executing queries on graph settings.
 * GraphConfigManager owns the interface towards GCSS and provides convenience
 * for HAL to execute queries and it generates GraphConfig objects as results.
 *
 * GraphConfigManager also provides static method for parsing graph descriptor
 * and graph settings from XML files and filtering that data based on sensor.
 * The \class GraphConfigmanager::Nodes object is stored in CameraCapInfo and
 * is used when instantiating GCM.
 *
 * At camera open, GraphConfigManager object is created.
 * At stream config time the state of GraphConfig manager changes with the
 * result of the first query. This is the possible subset of graph settings that
 * can fulfill the requirements of requested streams.
 * At this point, there may be more than one options, but
 * GCM can always return some default settings.
 *
 * Per each request, GraphConfigManager creates GraphConfig objects based
 * on request content. These objects are owned by GCM in a pool, and passed
 * around HAL via shared pointers.
 */
class GraphConfigManager {
 public:
    explicit GraphConfigManager(int32_t cameraId);
    ~GraphConfigManager();

    // Public APIs in GraphConfigManager
    status_t configStreams(const stream_config_t* streamList,
                           std::map<ConfigMode, std::shared_ptr<GraphConfig> >& gcs,
                           const stream_t* extraStream = nullptr);
    int getSelectedMcId() {
        return mMcId;
    }

 private:
    // Disable copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(GraphConfigManager);

    int createHalStreamVector(ConfigMode configMode, const stream_config_t* streamList,
                              std::vector<HalStream*>* halStreamVec);

    PipeUseCase getUseCaseFromStream(ConfigMode configMode, const stream_t& stream);
    void releaseHalStream(std::vector<HalStream*>* halStreamVec);

    // Debugging helpers
    void dumpStreamConfig();

 private:
    int32_t mCameraId;
    std::vector<HalStream*> mHalStreams; // Contain input and output Stream
    int mMcId;
};

}  // namespace icamera
