/*
 * Copyright (C) 2022 Intel Corporation.
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

#include <unordered_map>

#include "PlatformData.h"
#include "CameraSensorsParser.h"
#include "JsonCommonParser.h"

namespace icamera {
class CameraParserInvoker {
 public:
    CameraParserInvoker(MediaControl* mc, PlatformData::StaticCfg* cfg);
    ~CameraParserInvoker();

    void runParser();

 private:
    MediaControl* mMediaCtl;
    PlatformData::StaticCfg* mStaticCfg;
    int mNumSensors;

    std::vector<std::pair<std::string, SensorInfo>> getAvailableSensors(
        const std::string& ipuName, const std::vector<std::string>& sensorsList);
    void parseCommon();
    void parseSensors();
    void dumpSensorInfo(void);
    void chooseAvailableJsonFile(const std::vector<const char*>& availableJsonFiles,
                                 std::string* jsonFile) const;
    std::string getJsonFileFullName(std::string fileName);

 private:
    DISALLOW_COPY_AND_ASSIGN(CameraParserInvoker);
};

}  // namespace icamera

