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

#include "JsonParserBase.h"
#include "PlatformData.h"

namespace icamera {
#define NVM_OS "CrOS"
/**
 * Camera Module Information
 *
 * Camera Module Information is gotten from the EEPROM, which needs to be programmed with
 * an identification block located in the last 32 bytes of the EEPROM.
 */
struct CameraModuleInfo {
    char mOsInfo[4];
    uint16_t mCRC;
    uint8_t mVersion;
    uint8_t mLengthOfFields;
    uint16_t mDataFormat;
    uint16_t mModuleProduct;
    char mModuleVendor[2];
    char mSensorVendor[2];
    uint16_t mSensorModel;
    uint8_t mI2cAddress;
    uint8_t mReserved[13];
};
#define CAMERA_MODULE_INFO_OFFSET 32
#define CAMERA_MODULE_INFO_SIZE 32
#define NR_OF_CSI2_SRC_PADS 8

struct SensorInfo {
    std::string sinkEntityName;
    bool sensorResolved;
};

class CameraSensorsParser : public JsonParserBase {
 public:
    CameraSensorsParser(MediaControl* mc, PlatformData::StaticCfg* cfg, SensorInfo info);
    ~CameraSensorsParser();

    bool run(const std::string& filename) override final;

 private:
    struct NvmDeviceInfo {
        std::string nodeName;
        int dataSize;
        std::string directory;
    };

    MediaControl* mMediaCtl;
    PlatformData::StaticCfg* mStaticCfg;
    SensorInfo mSensorInfo;

    PlatformData::StaticCfg::CameraInfo* mCurCam = nullptr;
    std::string mI2CBus;
    std::string mCsiPort;
    std::vector<NvmDeviceInfo> mNVMDeviceInfo;

    std::string resolveI2CBusString(const std::string& name);
    void parseiSysRawFormat(const Json::Value& node);
    void parseAlgoRunningRate(const Json::Value& node);
    void parseLensHwType(const Json::Value& node);
    void parseGraphSettingsType(const Json::Value& node);
    void parseYUVColorRangeMode(const Json::Value& node);
    void parseTestPatternMap(const Json::Value& node);
    void parseDVSType(const Json::Value& node);
    void parseLardTags(const Json::Value& node);
    void parseMediaCtlConfigSection(const Json::Value& node);
    void parseMediaCtlSelectionObject(const Json::Value& node, MediaCtlConf* mc);
    void parseMediaCtlConfigFormatsObject(const Json::Value& node, MediaCtlConf* conf);
    void parseMediaCtlRouteObject(const Json::Value& node, MediaCtlConf* conf);
    void parseMediaCtlControlObject(const Json::Value& node, MediaCtlConf* conf);
    void parseMediaCtlLinkObject(const Json::Value& node, MediaCtlConf* conf);
    void parseMediaCtlVideoNodeObject(const Json::Value& node, MediaCtlConf* conf);
    void parseOutputMap(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedStreamConfig(const Json::Value& node);
    void parseStaticMetaDataSectionFpsRange(const Json::Value& node);
    void parseStaticMetaDataSectionEVRange(const Json::Value& node);
    void parseStaticMetaDataSectionEVSetp(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedFeatures(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedVideoStabilizationMode(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedAeMode(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedAWBModes(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedSceneMode(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedAfMode(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedAntibandingMode(const Json::Value& node);
    void parseStaticMetaDataSectionSupportedRotateMode(const Json::Value& node);
    void parseStaticMetaDataSection(const Json::Value& node);
    void parseGenericStaticMetaData(const Json::Value& node);
    void updateLensName();
    int getCameraModuleNameFromEEPROM(const std::string& nvmDir, std::string* cameraModule);
    void updateNVMDir();
    void parseNvmeDeviceInfo(const Json::Value& node);
    void parsesupportModuleNames(const Json::Value& node);
    void parseSensorSection(const Json::Value& node);
    void parseSupportedTuningConfig(const Json::Value& node);
    void resolveLensName(const Json::Value& node);
    void parseSupportedISysFormat(const Json::Value& node);
    void parseSupportedPSysFormat(const Json::Value& node);
    void resolveCsiPortAndI2CBus(const Json::Value& mediaCtlConfigs);
    void parseSupportedISysSizes(const Json::Value& node);
};

}  // namespace icamera
