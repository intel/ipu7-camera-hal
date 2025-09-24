/*
 * Copyright (C) 2022-2024 Intel Corporation.
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

#define LOG_TAG CameraParserInvoker

#include "CameraParserInvoker.h"

#include <string>
#include <vector>
#include <utility>
#include "iutils/CameraLog.h"

namespace icamera {
CameraParserInvoker::CameraParserInvoker(MediaControl* mc, PlatformData::StaticCfg* cfg)
        : mMediaCtl(mc),
          mStaticCfg(cfg),
          mNumSensors(0) {}

CameraParserInvoker::~CameraParserInvoker() {}

void CameraParserInvoker::parseCommon() {
    constexpr const char* LIBCAMHAL_PROFILE_NAME = "libcamhal_configs.json";

    CameraCommonParser commonParser{mStaticCfg};
    (void)commonParser.run(getJsonFileFullName(LIBCAMHAL_PROFILE_NAME));
}

void CameraParserInvoker::parseSensors() {
    auto allSensors = getAvailableSensors(mStaticCfg->mCommonConfig.ipuName,
                                          mStaticCfg->mCommonConfig.availableSensors);

    if (allSensors.empty()) {
        LOGW("%s: No sensors available", __func__);
        return;
    }

    for (const auto& sensor : allSensors) {
        ++mNumSensors;

        std::string sensorFileName = "sensors/" + sensor.first + ".json";
        LOGI("%s: I will Load config file: %s", __func__, sensorFileName.c_str());

        CameraSensorsParser cameraSensorsParser(mMediaCtl, mStaticCfg, sensor.second);
        const bool ret = cameraSensorsParser.run(getJsonFileFullName(sensorFileName));
        if (!ret)
            LOGE("%s, %s loaded failed!", __func__, sensorFileName.c_str());
        else
            LOGI("%s, %s loaded!", __func__, sensorFileName.c_str());
    }
}

void CameraParserInvoker::runParser() {
    parseCommon();
    parseSensors();
    dumpSensorInfo();
}

void CameraParserInvoker::chooseAvailableJsonFile(
    const std::vector<const char*>& availableJsonFiles, std::string* jsonFile) const {
    struct stat st;
    for (const auto json : availableJsonFiles) {
        const int ret = stat(json, &st);
        if (ret == 0) {
            *jsonFile = json;
            return;
        }
    }
}

std::string CameraParserInvoker::getJsonFileFullName(std::string fileName) {
    std::string curFolderFileName = std::string("./") + fileName;
    std::string sysFolderFileName = PlatformData::getCameraCfgPath() + fileName;
    const std::vector<const char*> profiles = {curFolderFileName.c_str(),
                                               sysFolderFileName.c_str()};

    std::string chosenJsonFile = fileName;
    chooseAvailableJsonFile(profiles, &chosenJsonFile);

    return chosenJsonFile;
}

std::vector<std::pair<std::string, SensorInfo>> CameraParserInvoker::getAvailableSensors(
    const std::string& ipuName, const std::vector<std::string>& sensorsList) {
    LOGI("%s, Found IPU: %s", __func__, ipuName.c_str());

    std::string sensorSinkName = "Intel ";
    sensorSinkName.append(ipuName);
    sensorSinkName.append(" ");
    sensorSinkName.append(CSI_PORT_NAME);
    sensorSinkName.append(" ");

    std::vector<std::pair<std::string, SensorInfo>> availableSensors;
    for (const auto& sensor : sensorsList) {
        if (sensor.find("-") == std::string::npos) {
            // sensors without suffix port number
            if (mMediaCtl && mMediaCtl->checkAvailableSensor(sensor)) {
                SensorInfo sensorInfo = {sensor, true};
                availableSensors.push_back({sensor, sensorInfo});
                LOG1("@%s, found %s", __func__, sensor.c_str());
            }
        } else {
            // sensors with suffix port number
            std::string portNum = sensor.substr(sensor.find_last_of('-') + 1);
            std::string sensorSinkNameWithPort = sensorSinkName + portNum;
            std::string sensorName = sensor.substr(0, sensor.find_first_of('-'));
            std::string sensorOutName = sensor.substr(0, sensor.find_last_of('-'));

            if (mMediaCtl && mMediaCtl->checkAvailableSensor(sensorName, sensorSinkNameWithPort)) {
                SensorInfo sensorInfo = {sensorSinkNameWithPort, false};
                availableSensors.push_back({sensorOutName, sensorInfo});
                LOG1("@%s, found %s, Sinkname with port: %s", __func__,
                     sensor.c_str(), sensorSinkNameWithPort.c_str());
            }
        }
    }

    return availableSensors;
}

void CameraParserInvoker::dumpSensorInfo(void) {
    // OLD Code. Do not change unless you know what you are doing.
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(CameraParserInvoker))) {
        return;
    }

    LOG3("@%s, sensor number: %d ==================", __func__, mNumSensors);
    for (int i = 0; i < mNumSensors; i++) {
        LOG3("Dump for mCameras[%d].sensorName:%s, mISysFourcc:%d", i,
             mStaticCfg->mCameras[i].sensorName.c_str(), mStaticCfg->mCameras[i].mISysFourcc);

        const stream_array_t& configs= mStaticCfg->mCameras[i].mStaticMetadata.mConfigsArray;
        for (size_t j = 0U; j < configs.size(); j++) {
            LOG3("    format:%d size(%dx%d) field:%d", configs[j].format,
                 configs[j].width, configs[j].height, configs[j].field);
        }

        for (unsigned j = 0U; j < mStaticCfg->mCameras[i].mSupportedISysFormat.size(); j++) {
            LOG3("    mSupportedISysFormat:%d", mStaticCfg->mCameras[i].mSupportedISysFormat[j]);
        }

        // dump the media controller mapping table for supportedStreamConfig
        LOG3("    The media controller mapping table size: %zu",
             mStaticCfg->mCameras[i].mStreamToMcMap.size());
        for (auto& pool : mStaticCfg->mCameras[i].mStreamToMcMap) {
            int mcId = pool.first;
            stream_array_t& mcMapVector = pool.second;
            LOG3("    mcId: %d, the supportedStreamConfig size: %zu", mcId, mcMapVector.size());
        }

        // dump the media controller information
        LOG3("    Format Configuration:");
        for (unsigned j = 0U; j < mStaticCfg->mCameras[i].mMediaCtlConfs.size(); j++) {
            const MediaCtlConf* mc = &mStaticCfg->mCameras[i].mMediaCtlConfs[j];
            for (unsigned k = 0U; k < mc->links.size(); k++) {
                const McLink* link = &mc->links[k];
                LOG3("        link src %s [%d:%d] ==> %s [%d:%d] enable %d",
                     link->srcEntityName.c_str(), link->srcEntity, link->srcPad,
                     link->sinkEntityName.c_str(), link->sinkEntity, link->sinkPad, link->enable);
            }
            for (unsigned k = 0U; k < mc->ctls.size(); k++) {
                const McCtl* ctl = &mc->ctls[k];
                LOG3("        Ctl %s [%d] cmd %s [0x%08x] value %d", ctl->entityName.c_str(),
                     ctl->entity, ctl->ctlName.c_str(), ctl->ctlCmd, ctl->ctlValue);
            }
            for (unsigned k = 0U; k < mc->formats.size(); k++) {
                const McFormat* format = &mc->formats[k];
                if (format->formatType == FC_FORMAT)
                    LOG3("        format %s [%d:%d] [%dx%d] %s", format->entityName.c_str(),
                         format->entity, format->pad, format->width, format->height,
                         CameraUtils::pixelCode2String(format->pixelCode));
                else if (format->formatType == FC_SELECTION)
                    LOG3("        select %s [%d:%d] selCmd: %d [%d, %d] [%dx%d]",
                         format->entityName.c_str(), format->entity, format->pad, format->selCmd,
                         format->top, format->left, format->width, format->height);
            }
        }
    }

    LOG3("@%s, done ==================", __func__);
}

}  // namespace icamera
