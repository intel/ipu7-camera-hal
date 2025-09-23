/*
 * Copyright (C) 2022-2025 Intel Corporation.
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

#define LOG_TAG CameraSensorsParser

#include "CameraSensorsParser.h"

#include <dirent.h>

#include <string>
#include <vector>

// CRL_MODULE_S
#include <linux/crlmodule.h>

// CRL_MODULE_E
#include "iutils/CameraLog.h"
namespace icamera {

CameraSensorsParser::CameraSensorsParser(MediaControl* mc, PlatformData::StaticCfg* cfg,
                                         SensorInfo info)
        : mMediaCtl(mc),
          mStaticCfg(cfg),
          mSensorInfo(info) {}

CameraSensorsParser::~CameraSensorsParser() {}

void CameraSensorsParser::parseMediaCtlConfigSection(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        MediaCtlConf mc;
        const Json::Value& cfg = node[i];

        if (cfg.isMember("id")) {
            mc.mcId = cfg["id"].asInt();
        }
        if (cfg.isMember("configMode")) {
            mc.configMode.push_back(
                CameraUtils::getConfigModeByName(cfg["configMode"].asString().c_str()));
        }

        if (cfg.isMember("output")) {
            if (cfg["output"].size() != 2) {
                LOGW("%s, Bad output dim in MediaCtl section", __func__);
                return;
            }
            mc.outputWidth = cfg["output"][0].asInt();
            mc.outputHeight = cfg["output"][1].asInt();
        }
        if (cfg.isMember("format")) {
            mc.format = CameraUtils::string2PixelCode(cfg["format"].asString().c_str());
        }

        if (cfg.isMember("formats")) {
            parseMediaCtlConfigFormatsObject(cfg["formats"], &mc);
        }
        if (cfg.isMember("selection")) {
            parseMediaCtlSelectionObject(cfg["selection"], &mc);
        }
        if (cfg.isMember("link")) {
            parseMediaCtlLinkObject(cfg["link"], &mc);
        }
        if (cfg.isMember("route")) {
            parseMediaCtlRouteObject(cfg["route"], &mc);
        }
        if (cfg.isMember("control")) {
            parseMediaCtlControlObject(cfg["control"], &mc);
        }
        if (cfg.isMember("videonode")) {
            parseMediaCtlVideoNodeObject(cfg["videonode"], &mc);
        }

        mCurCam->mMediaCtlConfs.push_back(mc);
    }
}

void CameraSensorsParser::parseMediaCtlRouteObject(const Json::Value& node, MediaCtlConf* conf) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        const auto ele = node[i];
        McRoute route;

        if (ele.isMember("name")) {
            route.entityName = resolveI2CBusString(ele["name"].asString());
            if (mMediaCtl != nullptr) {
                route.entity = mMediaCtl->getEntityIdByName(route.entityName);
            }
        }
        if (ele.isMember("srcPad")) {
            route.srcPad = ele["srcPad"].asInt();
        }
        if (ele.isMember("srcStream")) {
            route.srcStream = ele["srcStream"].asInt();
        }
        if (ele.isMember("sinkPad")) {
            route.sinkPad = ele["sinkPad"].asInt();
        }
        if (ele.isMember("sinkStream")) {
            route.sinkStream = ele["sinkStream"].asInt();
        }
        if (ele.isMember("flag")) {
            route.flag = ele["flag"].asInt();
        }

        auto it = conf->routings.find(route.entityName);
        if (it != conf->routings.end()) {
            it->second.push_back(route);
        } else {
            std::vector<McRoute> routes;
            routes.push_back(route);
            conf->routings.insert(std::pair<std::string, std::vector<McRoute>>
                                  (route.entityName, routes));
        }
    }
}

std::map<std::string, int> ctlCmdMapTable = {
    {"V4L2_CID_LINK_FREQ", V4L2_CID_LINK_FREQ},
    {"V4L2_CID_VBLANK", V4L2_CID_VBLANK},
    {"V4L2_CID_HBLANK", V4L2_CID_HBLANK},
    {"V4L2_CID_EXPOSURE", V4L2_CID_EXPOSURE},
    {"V4L2_CID_ANALOGUE_GAIN", V4L2_CID_ANALOGUE_GAIN},
    {"V4L2_CID_HFLIP", V4L2_CID_HFLIP},
    {"V4L2_CID_VFLIP", V4L2_CID_VFLIP},
    {"V4L2_CID_TEST_PATTERN", V4L2_CID_TEST_PATTERN},
    {"V4L2_CID_MIPI_LANES", V4L2_CID_MIPI_LANES},
// HDR_FEATURE_S
    {"V4L2_CID_WDR_MODE", V4L2_CID_WDR_MODE},
// HDR_FEATURE_E
// CRL_MODULE_S
    {"V4L2_CID_LINE_LENGTH_PIXELS", V4L2_CID_LINE_LENGTH_PIXELS},
    {"V4L2_CID_FRAME_LENGTH_LINES", V4L2_CID_FRAME_LENGTH_LINES},
    {"CRL_CID_SENSOR_MODE", CRL_CID_SENSOR_MODE},
    {"CRL_CID_EXPOSURE_MODE", CRL_CID_EXPOSURE_MODE},
    {"CRL_CID_EXPOSURE_HDR_RATIO", CRL_CID_EXPOSURE_HDR_RATIO},
// CRL_MODULE_E
};
void CameraSensorsParser::parseMediaCtlControlObject(const Json::Value& node, MediaCtlConf* conf) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        const auto ele = node[i];
        McCtl ctl;

        if (ele.isMember("name")) {
            ctl.entityName = resolveI2CBusString(ele["name"].asString());
            if (mMediaCtl)
                ctl.entity = mMediaCtl->getEntityIdByName(ctl.entityName);
        }
        if (ele.isMember("ctrlId")) {
            const auto target = ele["ctrlId"].asString();
            if (ctlCmdMapTable.find(target) != ctlCmdMapTable.end())
                ctl.ctlCmd = ctlCmdMapTable[target];
            else
                LOGW("%s, Unknown control id %s", __func__, target.c_str());
        }
        if (ele.isMember("value")) {
            ctl.ctlValue = ele["value"].asInt();
        }
        if (ele.isMember("ctrlName")) {
            ctl.ctlName = ele["ctrlName"].asString();
        }

        conf->ctls.push_back(ctl);
    }
}

void CameraSensorsParser::parseMediaCtlLinkObject(const Json::Value& node, MediaCtlConf* conf) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        McLink link;

        const auto ele = node[i];
        if (ele.isMember("srcName")) {
            link.srcEntityName = resolveI2CBusString(ele["srcName"].asString());
            if (mMediaCtl)
                link.srcEntity = mMediaCtl->getEntityIdByName(link.srcEntityName);
        }
        if (ele.isMember("srcPad")) {
            link.srcPad = ele["srcPad"].asInt();
        }
        if (ele.isMember("sinkName")) {
            link.sinkEntityName = resolveI2CBusString(ele["sinkName"].asString());
            if (mMediaCtl)
                link.sinkEntity = mMediaCtl->getEntityIdByName(link.sinkEntityName);
        }
        if (ele.isMember("sinkPad")) {
            link.sinkPad = ele["sinkPad"].asInt();
        }
        if (ele.isMember("enable")) {
            link.enable = ele["enable"].asBool();
        }
        conf->links.push_back(link);
    }
}

void CameraSensorsParser::parseMediaCtlVideoNodeObject(const Json::Value& node,
                                                       MediaCtlConf* conf) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        const auto ele = node[i];
        McVideoNode videoNode;

        videoNode.name = resolveI2CBusString(ele["name"].asString());
        videoNode.videoNodeType = GetNodeType(ele["videoNodeType"].asString().c_str());
        conf->videoNodes.push_back(videoNode);
    }
}

void CameraSensorsParser::parseMediaCtlConfigFormatsObject(const Json::Value& node,
                                                           MediaCtlConf* conf) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto ele = node[i];
        McFormat fmt;
        fmt.type = RESOLUTION_TARGET;

        if (ele.isMember("name")) {
            fmt.entityName = resolveI2CBusString(ele["name"].asString());
            if (mMediaCtl != nullptr) {
                fmt.entity = mMediaCtl->getEntityIdByName(fmt.entityName);
            }
        }
        if (ele.isMember("pad")) {
            fmt.pad = ele["pad"].asUInt();
        }
        if (ele.isMember("stream")) {
            fmt.stream = ele["stream"].asInt();
        }
        if (ele.isMember("type")) {
            const auto typeStr = ele["type"].asString();
            if (typeStr == "RESOLUTION_MAX") {
                fmt.type = RESOLUTION_MAX;
            }
            else if (typeStr == "RESOLUTION_COMPOSE") {
                fmt.type = RESOLUTION_COMPOSE;
            }
            else if (typeStr == "RESOLUTION_CROP") {
                fmt.type = RESOLUTION_CROP;
            }
            else if (typeStr == "RESOLUTION_TARGET") {
                fmt.type = RESOLUTION_TARGET;
            }
            else {
                LOGW("%s, Unknown format type %s", __func__, typeStr.c_str());
            }
        }
        if (ele.isMember("width")) {
            fmt.width = ele["width"].asUInt();
        }
        if (ele.isMember("height")) {
            fmt.height = ele["height"].asUInt();
        }
        if (ele.isMember("format")) {
            fmt.pixelCode = CameraUtils::string2PixelCode(ele["format"].asString().c_str());
        }

        fmt.formatType = FC_FORMAT;
        conf->formats.push_back(fmt);
    }
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedStreamConfig(const Json::Value& node) {
    stream_array_t configsArray;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        int mcId = -1;
        stream_t cfg;
        CLEAR(cfg);

        auto ele = node[i];
        if (ele.isMember("format")) {
            cfg.format = CameraUtils::string2PixelCode(ele["format"].asString().c_str());
        }
        if (ele.isMember("size") && (ele["size"].size() == 2)) {
            cfg.width = ele["size"][0].asUInt();
            cfg.height = ele["size"][1].asUInt();
        }
        if (ele.isMember("field")) {
            cfg.field = ele["field"].asInt();
        }
        if (ele.isMember("mcId")) {
            mcId = ele["mcId"].asInt();
            if (mCurCam->mStreamToMcMap.find(mcId) == mCurCam->mStreamToMcMap.end())
                mCurCam->mStreamToMcMap.insert({mcId, stream_array_t()});
        }
        configsArray.push_back(cfg);
        mCurCam->mStreamToMcMap[mcId].push_back(cfg);
    }

    mCurCam->mStaticMetadata.mConfigsArray = configsArray;
}

void CameraSensorsParser::parseMediaCtlSelectionObject(const Json::Value& node, MediaCtlConf* mc) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        McFormat sel;

        sel.top = -1;
        sel.left = -1;
        sel.width = 0;
        sel.height = 0;
        sel.formatType = FC_SELECTION;

        const auto ele = node[i];
        if (ele.isMember("name")) {
            const auto name = ele["name"].asString();
            sel.entityName = resolveI2CBusString(name);
            if (mMediaCtl != nullptr) {
                sel.entity = mMediaCtl->getEntityIdByName(sel.entityName);
            }
        }
        if (ele.isMember("pad")) {
            sel.pad = ele["pad"].asInt();
        }
        if (ele.isMember("target")) {
            const auto target = ele["target"].asString();
            if (target == "V4L2_SEL_TGT_COMPOSE") {
                sel.selCmd = V4L2_SEL_TGT_COMPOSE;
            }
            if (target == "V4L2_SEL_TGT_CROP") {
                sel.selCmd = V4L2_SEL_TGT_CROP;
            }
        }
        if (ele.isMember("top")) {
            sel.top = ele["top"].asInt();
        }
        if (ele.isMember("left")) {
            sel.left = ele["left"].asInt();
        }
        if (ele.isMember("width")) {
            sel.width = ele["width"].asInt();
        }
        if (ele.isMember("height")) {
            sel.height = ele["height"].asInt();
        }
        mc->formats.push_back(sel);
    }
}

void CameraSensorsParser::parseLardTags(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        if (node[i].size() != 5) {
            LOGW("%s, Bad lard tags node", __func__);
            continue;
        }

        for (Json::Value::ArrayIndex j = 0; j < node[i].size(); ++j) {
            auto tuningMode = node[i][0].asString();
            auto cmcTag = node[i][1].asString();
            auto aiqTag = node[i][2].asString();
            auto ispTag = node[i][3].asString();
            auto othersTag = node[i][4].asString();

            LardTagConfig cfg;
            cfg.tuningMode = CameraUtils::string2TuningMode(tuningMode.c_str());
            cfg.cmcTag = CameraUtils::fourcc2UL(cmcTag.c_str());
            cfg.aiqTag = CameraUtils::fourcc2UL(aiqTag.c_str());
            cfg.ispTag = CameraUtils::fourcc2UL(ispTag.c_str());
            cfg.othersTag = CameraUtils::fourcc2UL(othersTag.c_str());

            mCurCam->mLardTagsConfig.push_back(cfg);
        }
    }
}

void CameraSensorsParser::parseSupportedISysSizes(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto ele = node[i];
        if (ele.size() != 2) {
            LOGW("%s, Wrong ISys size object", __func__);
            continue;
        }
        mCurCam->mSupportedISysSizes.push_back({ele[0].asInt(), ele[1].asInt()});
    }
}

void CameraSensorsParser::parseSupportedISysFormat(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        std::string fmt = node[i].asString();
        int fmtDsc = CameraUtils::string2PixelCode(fmt.c_str());
        if (fmtDsc == -1) {
            LOGW("%s, Unknown ISYS format: %s", __func__, fmt.c_str());
            continue;
        }
        mCurCam->mSupportedISysFormat.push_back(fmtDsc);
    }
}

void CameraSensorsParser::parseiSysRawFormat(const Json::Value& node) {
    mCurCam->mISysRawFormat = CameraUtils::string2PixelCode(node.asString().c_str());
}

void CameraSensorsParser::parseYUVColorRangeMode(const Json::Value& node) {
    auto typeStr = node.asString();
    if (typeStr == "full")
        mCurCam->mYuvColorRangeMode = CAMERA_FULL_MODE_YUV_COLOR_RANGE;
    else if (typeStr == "reduced")
        mCurCam->mYuvColorRangeMode = CAMERA_REDUCED_MODE_YUV_COLOR_RANGE;
}

void CameraSensorsParser::parseNvmeDeviceInfo(const Json::Value& node) {
    if (node.size() != 2) {
        LOGW("%s, Bad NVME info object", __func__);
        return;
    }
    NvmDeviceInfo info{node[0].asString(), node[1].asInt()};
    mNVMDeviceInfo.push_back(info);
}

void CameraSensorsParser::parsesupportModuleNames(const Json::Value& node) {
    std::vector<std::string> supportModuleNames;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto name = node[i].asString();
        supportModuleNames.push_back(name);
    }

    mCurCam->mSupportModuleNames = supportModuleNames;
}

void CameraSensorsParser::parseDVSType(const Json::Value& node) {
    auto typeStr = node.asString();
    if (typeStr == "MORPH_TABLE")
        mCurCam->mDVSType = MORPH_TABLE;
    else if (typeStr == "IMG_TRANS")
        mCurCam->mDVSType = IMG_TRANS;
}

void CameraSensorsParser::resolveLensName(const Json::Value& node) {
    std::string vcmName = node.asString();
    if (!mI2CBus.empty()) {
        int i2cBusID = std::atoi(mI2CBus.c_str());
        vcmName.append(" ");
        vcmName.append(std::to_string(i2cBusID));
    } else {
        LOGW("%s, i2c bus is unknown", __func__);
    }

    if (mMediaCtl != nullptr) {
        mMediaCtl->getVCMI2CAddr(vcmName.c_str(), &mCurCam->mLensName);
    }
}

void CameraSensorsParser::parseLensHwType(const Json::Value& node) {
    auto typeStr = node.asString();
    if (typeStr == "LENS_VCM_HW")
        mCurCam->mLensHwType = LENS_VCM_HW;
    else {
        LOGW("%s, Unknown Lens HW type %s, set to LENS_NONE_HW", __func__, typeStr.c_str());
        mCurCam->mLensHwType = LENS_NONE_HW;
    }
}

void CameraSensorsParser::parseTestPatternMap(const Json::Value& node) {
    if (node.isMember("Off")) {
        mCurCam->mTestPatternMap[TEST_PATTERN_OFF] = node["Off"].asInt();
    }
    if (node.isMember("ColorBars")) {
        mCurCam->mTestPatternMap[COLOR_BARS] = node["ColorBars"].asInt();
    }
    if (node.isMember("SolidColor")) {
        mCurCam->mTestPatternMap[SOLID_COLOR] = node["SolidColor"].asInt();
    }
    if (node.isMember("ColorBarsFadeToGray")) {
        mCurCam->mTestPatternMap[COLOR_BARS_FADE_TO_GRAY] = node["ColorBarsFadeToGray"].asInt();
    }
    if (node.isMember("PN9")) {
        mCurCam->mTestPatternMap[PN9] = node["PN9"].asInt();
    }
}

void CameraSensorsParser::parseOutputMap(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto resNode = node[i];
        if ((resNode.size() != 2) || (resNode[0].size() != 2) || (resNode[1].size() != 2)) {
            LOGW("%s, Bad resolution object.", __func__);
            continue;
        }
        UserToPslOutputMap map;
        map.User.width = resNode[0][0].asInt();
        map.User.height = resNode[0][1].asInt();
        map.Psl.width = resNode[1][0].asInt();
        map.Psl.height = resNode[1][1].asInt();

        mCurCam->mOutputMap.push_back(map);
    }
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedFeatures(const Json::Value& node) {
    std::vector<camera_features> features;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        camera_features feature = INVALID_FEATURE;
        const auto featureStr = node[i].asString();

        if (featureStr == "MANUAL_EXPOSURE")
            feature = MANUAL_EXPOSURE;
        else if (featureStr == "MANUAL_WHITE_BALANCE")
            feature = MANUAL_WHITE_BALANCE;
        else if (featureStr == "IMAGE_ENHANCEMENT")
            feature = IMAGE_ENHANCEMENT;
        else if (featureStr == "NOISE_REDUCTION")
            feature = NOISE_REDUCTION;
        else if (featureStr == "SCENE_MODE")
            feature = SCENE_MODE;
        else if (featureStr == "PER_FRAME_CONTROL")
            feature = PER_FRAME_CONTROL;

        if (feature != INVALID_FEATURE) {
            features.push_back(feature);
        }
    }

    mCurCam->mStaticMetadata.mSupportedFeatures = features;
}

void CameraSensorsParser::parseStaticMetaDataSectionFpsRange(const Json::Value& node) {
    std::vector<double> range;
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) range.push_back(node[i].asDouble());

    mCurCam->mStaticMetadata.mFpsRange = range;
}

void CameraSensorsParser::parseStaticMetaDataSectionEVRange(const Json::Value& node) {
    std::vector<int> range;
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) range.push_back(node[i].asInt());

    mCurCam->mStaticMetadata.mEvRange = range;
}

void CameraSensorsParser::parseStaticMetaDataSectionEVSetp(const Json::Value& node) {
    if (node.size() != 2) {
        return;
    }

    std::vector<int> evStep = {node[0].asInt(), node[1].asInt()};
    mCurCam->mStaticMetadata.mEvStep = evStep;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedAeMode(const Json::Value& node) {
    std::vector<camera_ae_mode_t> supportedModes;
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        camera_ae_mode_t aeMode = AE_MODE_AUTO;
        auto name = node[i].asString();

        if (name == "AUTO")
            aeMode = AE_MODE_AUTO;
        else if (name == "MANUAL")
            aeMode = AE_MODE_MANUAL;

        supportedModes.push_back(aeMode);
    }

    mCurCam->mStaticMetadata.mSupportedAeMode = supportedModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedAWBModes(const Json::Value& node) {
    std::vector<camera_awb_mode_t> awbModes;
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i)
        awbModes.push_back(CameraUtils::getAwbModeByName(node[i].asString().c_str()));

    mCurCam->mStaticMetadata.mSupportedAwbMode = awbModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedSceneMode(const Json::Value& node) {
    std::vector<camera_scene_mode_t> sceneModes;
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i)
        sceneModes.push_back(CameraUtils::getSceneModeByName(node[i].asString().c_str()));

    mCurCam->mStaticMetadata.mSupportedSceneMode = sceneModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedAfMode(const Json::Value& node) {
    std::vector<camera_af_mode_t> supportedModes;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto name = node[i].asString();
        camera_af_mode_t afMode = AF_MODE_OFF;
        if (name == "AUTO")
            afMode = AF_MODE_AUTO;
        else if (name == "MACRO")
            afMode = AF_MODE_MACRO;
        else if (name == "CONTINUOUS_VIDEO")
            afMode = AF_MODE_CONTINUOUS_VIDEO;
        else if (name == "CONTINUOUS_PICTURE")
            afMode = AF_MODE_CONTINUOUS_PICTURE;
        else if (name == "OFF")
            afMode = AF_MODE_OFF;
        supportedModes.push_back(afMode);
    }

    mCurCam->mStaticMetadata.mSupportedAfMode = supportedModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedAntibandingMode(
    const Json::Value& node) {
    std::vector<camera_antibanding_mode_t> supportedModes;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto name = node[i].asString();
        camera_antibanding_mode_t antibandingMode = ANTIBANDING_MODE_OFF;
        if (name == "AUTO")
            antibandingMode = ANTIBANDING_MODE_AUTO;
        else if (name == "50Hz")
            antibandingMode = ANTIBANDING_MODE_50HZ;
        else if (name == "60Hz")
            antibandingMode = ANTIBANDING_MODE_60HZ;
        else if (name == "OFF")
            antibandingMode = ANTIBANDING_MODE_OFF;

        supportedModes.push_back(antibandingMode);
    }

    mCurCam->mStaticMetadata.mSupportedAntibandingMode = supportedModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedVideoStabilizationMode(
    const Json::Value& node) {
    std::vector<camera_video_stabilization_mode_t> supportedModes;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        camera_video_stabilization_mode_t mode = VIDEO_STABILIZATION_MODE_OFF;
        auto name = node[i].asString();

        if (name == "ON")
            mode = VIDEO_STABILIZATION_MODE_ON;
        else if (name == "OFF")
            mode = VIDEO_STABILIZATION_MODE_OFF;

        supportedModes.push_back(mode);
    }

    mCurCam->mStaticMetadata.mVideoStabilizationModes = supportedModes;
}

void CameraSensorsParser::parseStaticMetaDataSectionSupportedRotateMode(const Json::Value& node) {
    std::vector<camera_rotate_mode_t> supportedModes;

    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        camera_rotate_mode_t mode = ROTATE_NONE;
        auto name = node[i].asString();

        if (name == "NONE")
            mode = ROTATE_NONE;
        else if (name == "90")
            mode = ROTATE_90;
        else if (name == "180")
            mode = ROTATE_180;
        else if (name == "270")
            mode = ROTATE_270;
        else if (name == "AUTO")
            mode = ROTATE_AUTO;
        supportedModes.push_back(mode);
    }

    mCurCam->mStaticMetadata.mSupportedRotateMode = supportedModes;
}

void CameraSensorsParser::parseStaticMetaDataSection(const Json::Value& node) {
    if (node.isMember("supportedStreamConfig")) {
        parseStaticMetaDataSectionSupportedStreamConfig(node["supportedStreamConfig"]);
    }
    if (node.isMember("supportedFeatures")) {
        parseStaticMetaDataSectionSupportedFeatures(node["supportedFeatures"]);
    }
    if (node.isMember("fpsRange")) {
        parseStaticMetaDataSectionFpsRange(node["fpsRange"]);
    }
    if (node.isMember("evRange")) {
        parseStaticMetaDataSectionEVRange(node["evRange"]);
    }
    if (node.isMember("evStep")) {
        parseStaticMetaDataSectionEVSetp(node["evStep"]);
    }
    if (node.isMember("supportedAeMode")) {
        parseStaticMetaDataSectionSupportedAeMode(node["supportedAeMode"]);
    }
    if (node.isMember("supportedAwbMode")) {
        parseStaticMetaDataSectionSupportedAWBModes(node["supportedAwbMode"]);
    }
    if (node.isMember("supportedSceneMode")) {
        parseStaticMetaDataSectionSupportedSceneMode(node["supportedSceneMode"]);
    }
    if (node.isMember("supportedAfMode")) {
        parseStaticMetaDataSectionSupportedAfMode(node["supportedAfMode"]);
    }
    if (node.isMember("supportedAntibandingMode")) {
        parseStaticMetaDataSectionSupportedAntibandingMode(node["supportedAntibandingMode"]);
    }
    if (node.isMember("supportedVideoStabilizationModes")) {
        parseStaticMetaDataSectionSupportedVideoStabilizationMode(
            node["supportedVideoStabilizationModes"]);
    }
    if (node.isMember("supportedRotateMode")) {
        parseStaticMetaDataSectionSupportedRotateMode(node["supportedRotateMode"]);
    }
    if (node.isMember("metadata")) {
        parseGenericStaticMetaData(node["metadata"]);
    }
}

void CameraSensorsParser::parseSupportedTuningConfig(const Json::Value& node) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); ++i) {
        auto cnode = node[i];
        TuningConfig cfg;
        if (cnode.size() < 3) {
            LOGW("%s, Bad tuning config Node", __func__);
            continue;
        }
        cfg.configMode = CameraUtils::getConfigModeByName(cnode[0].asString().c_str());
        cfg.tuningMode = CameraUtils::string2TuningMode(cnode[1].asString().c_str());
        cfg.aiqbName = cnode[2].asString();
        mCurCam->mSupportedTuningConfig.push_back(cfg);
    }
}

std::string CameraSensorsParser::resolveI2CBusString(const std::string& name) {
    std::string res = name;

    auto pos = res.find("$I2CBUS");
    if (pos != std::string::npos) {
        res.replace(pos, sizeof("$I2CBUS"), mI2CBus);
    }
    pos = res.find("$CSI_PORT");
    if (pos != std::string::npos) {
        res.replace(pos, sizeof("$CSI_PORT"), mCsiPort);
    }
    pos = res.find("$CAP_N");
    if (pos != std::string::npos) {
        res.replace(pos, sizeof("$CAP_N"),
                    std::to_string(std::atoi(mCsiPort.c_str()) * NR_OF_CSI2_SRC_PADS));
    }

    return res;
}

void CameraSensorsParser::parseSensorSection(const Json::Value& node) {
    if (node.isMember("name")) {
        mCurCam->sensorName = node["name"].asString();
    }
    if (node.isMember("description")) {
        mCurCam->sensorDescription = node["description"].asString();
    }
    // VIRTUAL_CHANNEL_S
    if (node.isMember("vcCount")) {
        mCurCam->mVCCount = node["vcCount"].asInt();
    }
    if (node.isMember("vcId")) {
        mCurCam->mVCId = node["vcId"].asInt();
    }
    if (node.isMember("vcGoupId")) {
        mCurCam->mVCGroupId = node["vcGoupId"].asInt();
    }
    // VIRTUAL_CHANNEL_E
    resolveCsiPortAndI2CBus();
    if (node.isMember("supportedTuningConfig")) {
        parseSupportedTuningConfig(node["supportedTuningConfig"]);
    }
    if (node.isMember("lardTags")) {
        parseLardTags(node["lardTags"]);
    }
    if (node.isMember("supportedISysSizes")) {
        parseSupportedISysSizes(node["supportedISysSizes"]);
    }
    if (node.isMember("supportedISysFormat")) {
        parseSupportedISysFormat(node["supportedISysFormat"]);
    }
    if (node.isMember("enableAIQ")) {
        mCurCam->mEnableAIQ = node["enableAIQ"].asBool();
    }
    if (node.isMember("ispTuningUpdate")) {
        mCurCam->mIspTuningUpdate = node["ispTuningUpdate"].asBool();
    }
    if (node.isMember("iSysRawFormat")) {
        parseiSysRawFormat(node["iSysRawFormat"]);
    }
    if (node.isMember("maxRawDataNum")) {
        mCurCam->mMaxRawDataNum = node["maxRawDataNum"].asInt();
    }
    if (node.isMember("initialSkipFrame")) {
        mCurCam->mInitialSkipFrame = node[";initialSkipFrame"].asInt();
    }
    if (node.isMember("exposureLag")) {
        mCurCam->mExposureLag = node["exposureLag"].asInt();
    }
    if (node.isMember("gainLag")) {
        mCurCam->mAnalogGainLag = node["gainLag"].asInt();
    }
    if (node.isMember("digitalGainLag")) {
        mCurCam->mDigitalGainLag = node["digitalGainLag"].asInt();
    }
    if (node.isMember("yuvColorRangeMode")) {
        parseYUVColorRangeMode(node["yuvColorRangeMode"]);
    }

    if (node.isMember("graphSettingsFile")) {
        mCurCam->mGraphSettingsFile = node["graphSettingsFile"].asString();
    }
    if (node.isMember("dvsType")) {
        parseDVSType(node["dvsType"]);
    }
    if (node.isMember("nvmDeviceInfo")) {
        parseNvmeDeviceInfo(node["nvmDeviceInfo"]);
    }
    if (node.isMember("supportModuleNames")) {
        parsesupportModuleNames(node["supportModuleNames"]);
    }
    if (node.isMember("lensName")) {
        resolveLensName(node["lensName"]);
    }
    if (node.isMember("lensHwType")) {
        parseLensHwType(node["lensHwType"]);
    }
    if (node.isMember("testPatternMap")) {
        parseTestPatternMap(node["testPatternMap"]);
    }
    if (node.isMember("enableAiqd")) {
        mCurCam->mEnableAiqd = node["enableAiqd"].asBool();
    }
    if (node.isMember("useCrlModule")) {
        mCurCam->mUseCrlModule = node["useCrlModule"].asBool();
    }
    if (node.isMember("pslOutputMapForRotation")) {
        parseOutputMap(node["pslOutputMapForRotation"]);
    }

    if (node.isMember("unregisterExtDmaBuf")) {
        mCurCam->mUnregisterExtDmaBuf = node["unregisterExtDmaBuf"].asBool();
    }
    if (node.isMember("maxRequestsInflight")) {
        mCurCam->mMaxRequestsInflight = node["maxRequestsInflight"].asInt();
    }
    if (node.isMember("faceEngineRunningInterval")) {
        mCurCam->mFaceEngineRunningInterval = node["faceEngineRunningInterval"].asInt();
    }
    if (node.isMember("faceEngineRunningIntervalNoFace")) {
        mCurCam->mFaceEngineRunningIntervalNoFace = node["faceEngineRunningIntervalNoFace"].asInt();
    }
    if (node.isMember("faceAeEnabled")) {
        mCurCam->mFaceAeEnabled = node["faceAeEnabled"].asBool();
    }
    if (node.isMember("faceEngineVendor")) {
        mCurCam->mFaceEngineVendor = node["faceEngineVendor"].asInt();
    }
    if (node.isMember("runFaceWithSyncMode")) {
        mCurCam->mRunFaceWithSyncMode = node["runFaceWithSyncMode"].asBool();
    }
    if (node.isMember("psysBundleWithAic")) {
        mCurCam->mPsysBundleWithAic = node["psysBundleWithAic"].asBool();
    }
    if (node.isMember("skipFrameV4L2Error")) {
        mCurCam->mSkipFrameV4L2Error = node["skipFrameV4L2Error"].asBool();
    }
    if (node.isMember("isPSACompression")) {
        mCurCam->mPSACompression = node["isPSACompression"].asBool();
    }
    if (node.isMember("tnrExtraFrameNum")) {
        mCurCam->mTnrExtraFrameNum = node["tnrExtraFrameNum"].asInt();
    }
    if (node.isMember("isPLCEnable")) {
        mCurCam->mPLCEnable = node["isPLCEnable"].asBool();
    }
    if (node.isMember("enableAIQ")) {
        mCurCam->mEnableAIQ = node["enableAIQ"].asBool();
    }
    if (node.isMember("dummyStillSink")) {
        mCurCam->mDummyStillSink = node["dummyStillSink"].asBool();
    }
    if (node.isMember("useGpuTnr")) {
        mCurCam->mGpuTnrEnabled = node["useGpuTnr"].asBool();
    }
    if (node.isMember("useGpuIpa")) {
        mCurCam->mGpuIpaEnabled = node["useGpuIpa"].asBool();
    }
    if (node.isMember("psysAlignWithSystem")) {
        mCurCam->mMsPsysAlignWithSystem = node["psysAlignWithSystem"].asInt();
    }
    if (node.isMember("removeCacheFlushOutputBuffer")) {
        mCurCam->mRemoveCacheFlushOutputBuffer = node["removeCacheFlushOutputBuffer"].asBool();
    }
    if (node.isMember("sensorExposureNum")) {
        mCurCam->mSensorExposureNum = node["sensorExposureNum"].asInt();
    }

    if (node.isMember("MediaCtlConfig")) {
        parseMediaCtlConfigSection(node["MediaCtlConfig"]);
    }
    if (node.isMember("StaticMetadata")) {
        parseStaticMetaDataSection(node["StaticMetadata"]);
    }

    if (node.isMember("usePSysProcessor")) {
        mCurCam->mUsePSysProcessor = node["usePSysProcessor"].asBool();
    }
}

void CameraSensorsParser::resolveCsiPortAndI2CBus() {
    std::string fullSensorName = mCurCam->sensorName;
    if (fullSensorName.empty()) {
        LOGW("%s: Cannot find any sensors in this box.", __func__);
        return;
    }

    if (mSensorInfo.sensorResolved != true) {
        auto sinkEntityName = mSensorInfo.sinkEntityName;
        mSensorInfo.sensorResolved = true;
        mCsiPort = sinkEntityName.substr(sinkEntityName.find_last_of(' ') + 1);

        auto sensorName = fullSensorName;
        if (sensorName.find_first_of('-') != std::string::npos)
            sensorName = fullSensorName.substr(0, (sensorName.find_first_of('-')));

        if (mMediaCtl != nullptr) {
            mMediaCtl->getI2CBusAddress(sensorName, sinkEntityName, &mI2CBus);
        }

        LOGI("%s: I2CBus:%s <=> CSI Port:%s", __func__, mI2CBus.c_str(), mCsiPort.c_str());
    }
}

int CameraSensorsParser::getCameraModuleNameFromEEPROM(const std::string& nvmDir,
                                                       std::string* cameraModule) {
    const int moduleInfoOffset = CAMERA_MODULE_INFO_OFFSET;
    FILE* eepromFile = fopen(nvmDir.c_str(), "rb");
    CheckAndLogError(!eepromFile, UNKNOWN_ERROR, "Failed to open EEPROM file in %s",
                     nvmDir.c_str());

    // file size should be larger than CAMERA_MODULE_INFO_OFFSET
    (void)fseek(eepromFile, 0, SEEK_END);
    int nvmDataSize = static_cast<int>(ftell(eepromFile));
    if (nvmDataSize < moduleInfoOffset) {
        LOGE("EEPROM data is too small");
        fclose(eepromFile);
        return NOT_ENOUGH_DATA;
    }

    int ret = fseek(eepromFile, -1 * moduleInfoOffset, SEEK_END);
    if (ret != 0) {
        LOGE("Failed to fseek file");
        fclose(eepromFile);
        return NOT_ENOUGH_DATA;
    }

    const int moduleInfoSize = CAMERA_MODULE_INFO_SIZE;
    struct CameraModuleInfo cameraModuleInfo;
    CLEAR(cameraModuleInfo);
    ret = fread(&cameraModuleInfo, moduleInfoSize, 1, eepromFile);
    fclose(eepromFile);
    CheckAndLogError(!ret, UNKNOWN_ERROR, "Failed to read module info %d", ret);

    if (strncmp(cameraModuleInfo.mOsInfo, NVM_OS, strlen(NVM_OS)) != 0) {
        LOG1("NVM OS string doesn't match with module info");
        return NO_ENTRY;
    }

    char tmpName[CAMERA_MODULE_INFO_SIZE];
    snprintf(tmpName, CAMERA_MODULE_INFO_SIZE, "%c%c_%04x", cameraModuleInfo.mModuleVendor[0],
             cameraModuleInfo.mModuleVendor[1], cameraModuleInfo.mModuleProduct);

    cameraModule->assign(tmpName);
    LOG1("%s, aiqb name %s", __func__, cameraModule->c_str());

    return OK;
}

void CameraSensorsParser::updateNVMDir() {
    // OLD Code. Do not change unless you know what you are doing.
    // I2CBus is adaptor-bus, like 18-0010, and use adaptor id to select NVM path.
    if ((mI2CBus.size() < 2) && mNVMDeviceInfo.empty()) {
        return;
    }

    // attach i2c adaptor id, like 18-0010
    std::size_t found = mI2CBus.find("-");
    CheckAndLogError(found == std::string::npos, VOID_VALUE, "Failed to get adaptor id");

    /* the path of NVM device is in /sys/bus/i2c/devices/i2c-'adaptorId'/firmware_node/XXXX/path. */
    std::string nvmPath("/sys/bus/i2c/devices/i2c-");
    // attach i2c adaptor id, like 18-0010
    found = mI2CBus.find("-");
    CheckAndLogError(found == std::string::npos, VOID_VALUE, "Failed to get adaptor id");
    nvmPath += mI2CBus.substr(0, found);
    nvmPath += "/firmware_node/";
    DIR* dir = opendir(nvmPath.c_str());
    if (dir) {
        struct dirent* direntPtr = nullptr;
        while ((direntPtr = readdir(dir)) != nullptr) {
            if (direntPtr->d_type != DT_DIR) {
                continue;
            }

            std::string fwNodePath(nvmPath.c_str());
            fwNodePath += direntPtr->d_name;
            fwNodePath += "/path";

            bool found = false;
            FILE* fp = fopen(fwNodePath.c_str(), "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                int size = static_cast<int>(ftell(fp));
                if (size <= 0) {
                    fclose(fp);
                    continue;
                }
                fseek(fp, 0, SEEK_SET);
                std::unique_ptr<char[]> ptr(new char[size + 1]);
                size_t readSize = fread(ptr.get(), sizeof(char), size, fp);
                fclose(fp);

                if (readSize < size) {
                    ptr[readSize] = '\0';
                } else {
                    ptr[size] = '\0';
                }
                if (readSize > 0) {
                    for (auto& nvm : mNVMDeviceInfo) {
                        if (strstr(ptr.get(), nvm.nodeName.c_str()) != nullptr) {
                            std::string nvmPath(NVM_DATA_PATH);
                            nvmPath.append("i2c-");
                            nvmPath.append(direntPtr->d_name);
                            nvmPath.append("/eeprom");
                            // Check if eeprom file exists
                            struct stat buf;
                            int ret = stat(nvmPath.c_str(), &buf);
                            LOG1("%s, nvmPath %s, ret %d", __func__, nvmPath.c_str(), ret);
                            if (ret == 0) {
                                nvm.directory = "i2c-";
                                nvm.directory += direntPtr->d_name;
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (found) {
                break;
            }
        }
        closedir(dir);
    } else {
        LOGE("Failed to open dir %s", nvmPath.c_str());
    }

    for (auto nvm : mNVMDeviceInfo) {
        if (!nvm.directory.empty()) {
            // The first one in list is prioritized and should be selected.
            std::string nvmPath;
            nvmPath.append(NVM_DATA_PATH);
            nvmPath.append(nvm.directory);
            if (nvmPath.back() != '/') {
                nvmPath.append("/");
            }

            nvmPath.append("eeprom");
            LOG2("NVM data is located in %s", nvmPath.c_str());
            mCurCam->mNvmDirectory = nvmPath;
            mCurCam->mMaxNvmDataSize = nvm.dataSize;
            int ret =
                getCameraModuleNameFromEEPROM(mCurCam->mNvmDirectory, &mCurCam->mCamModuleName);
            LOG2("NVM dir %s, ret %d", mCurCam->mNvmDirectory.c_str(), ret);
            break;
        } else {
            LOGE("Failed to find NVM directory");
        }
    }
}

void CameraSensorsParser::parseGenericStaticMetaData(const Json::Value& node) {
    auto keyNames = node.getMemberNames();
    for (const auto& key : keyNames) {
        int tagType;
        auto name = key;

        if (mCurCam->mStaticMetadata.mStaticMetadataToType.count(name))
            tagType = mCurCam->mStaticMetadata.mStaticMetadataToType[name];
        else {
            LOGW("%s, No %s in HAL metadata database!", __func__, name.c_str());
            continue;
        }

        auto itemNode = node[name];
        for (Json::Value::ArrayIndex i = 0; i < itemNode.size(); ++i) {
            switch (tagType) {
                case TYPE_BYTE:
                    mCurCam->mStaticMetadata.mByteMetadata[name].push_back(
                        static_cast<uint8_t>(itemNode[i].asInt()));
                    break;
                case TYPE_INT32:
                case TYPE_RATIONAL:
                    mCurCam->mStaticMetadata.mInt32Metadata[name].push_back(itemNode[i].asInt());
                    break;
                case TYPE_INT64:
                    mCurCam->mStaticMetadata.mInt64Metadata[name].push_back(itemNode[i].asInt64());
                    break;
                case TYPE_FLOAT:
                    mCurCam->mStaticMetadata.mFloatMetadata[name].push_back(itemNode[i].asFloat());
                    break;
                case TYPE_DOUBLE:
                    mCurCam->mStaticMetadata.mDoubleMetadata[name].push_back(
                        itemNode[i].asDouble());
                    break;
            }
        }
    }
}

void CameraSensorsParser::updateLensName() {
    if ((!mCurCam->mLensName.empty()) || (mCurCam->sensorName.find("-wf-") == std::string::npos))
        return;

    if (mMediaCtl) {
        int ret = mMediaCtl->getLensName(&mCurCam->mLensName);
        if (ret != OK) {
            LOG2("%s, Cannot get lens name", __func__);
        }
    }
}

bool CameraSensorsParser::run(const std::string& filename) {
    Json::Value root = openJsonFile(filename);

    if (root.isMember("CameraSettings") && root["CameraSettings"].isMember("Sensor")) {
        for (Json::Value::ArrayIndex i = 0; i < root["CameraSettings"]["Sensor"].size(); i++) {
            mCurCam = new PlatformData::StaticCfg::CameraInfo;
            parseSensorSection(root["CameraSettings"]["Sensor"][i]);
            updateNVMDir();
            updateLensName();

            bool isCameraAvailable = true;
            // Check if the camera is available
            if (!mCurCam->mSupportModuleNames.empty()) {
                isCameraAvailable = false;
                for (Json::Value::ArrayIndex i = 0; i < mCurCam->mSupportModuleNames.size(); i++) {
                    if (strcmp(mCurCam->mSupportModuleNames[i].c_str(),
                               mCurCam->mCamModuleName.c_str()) == 0) {
                        isCameraAvailable = true;
                        break;
                    }
                }
            }

            if (isCameraAvailable) {
                mStaticCfg->mCameras.push_back(*mCurCam);
            }

            delete mCurCam;
        }

        return true;
    } else if (root.empty()) {
        return true;
    } else {
        return false;
    }
}

}  // namespace icamera
