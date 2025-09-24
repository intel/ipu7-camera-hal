/*
 * Copyright (C) 2015-2025 Intel Corporation.
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

#define LOG_TAG PlatformData

#include "PlatformData.h"

#include <memory>
#include <math.h>
#include <sys/sysinfo.h>

#include "iutils/CameraLog.h"
#include "CameraSchedulerPolicy.h"

#include "CameraContext.h"
#include "GraphConfig.h"

#include "gc/GraphConfig.h"

#include "src/platformdata/CameraParserInvoker.h"
#include "StageDescriptor.h"

using std::string;
using std::vector;

namespace icamera {
PlatformData* PlatformData::sInstance = nullptr;
Mutex PlatformData::sLock;

PlatformData* PlatformData::getInstance() {
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new PlatformData();
    }

    return sInstance;
}

void PlatformData::releaseInstance() {
    AutoMutex lock(sLock);
    LOG1("@%s", __func__);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

PlatformData::PlatformData() {
    LOG1("@%s", __func__);
    MediaControl* mc = MediaControl::getInstance();

    CameraParserInvoker(mc, &mStaticCfg).runParser();

    CameraSchedulerPolicy::getInstance();
}

PlatformData::~PlatformData() {
    LOG1("@%s", __func__);

    releaseGraphConfigNodes();

    MediaControl::releaseInstance();

    for (size_t i = 0U; i < mAiqInitData.size(); i++) {
        delete mAiqInitData[i];
    }

    CameraSchedulerPolicy::releaseInstance();
    mAiqInitData.clear();
}

int PlatformData::init() {
    LOG2("@%s", __func__);

    parseGraphFromXmlFile();

    StaticCfg* staticCfg = &(getInstance()->mStaticCfg);
    for (size_t i = 0U; i < staticCfg->mCameras.size(); i++) {
        std::string camModuleName;
        AiqInitData* aiqInitData = new AiqInitData(
            staticCfg->mCameras[i].sensorName, getCameraCfgPath(),
            staticCfg->mCameras[i].mSupportedTuningConfig, staticCfg->mCameras[i].mNvmDirectory,
            staticCfg->mCameras[i].mMaxNvmDataSize, staticCfg->mCameras[i].mCamModuleName);
        getInstance()->mAiqInitData.push_back(aiqInitData);
    }

    return OK;
}

/**
 * Read graph descriptor and settings from configuration files.
 *
 * The resulting graphs represent all possible graphs for given sensor, and
 * they are stored in capinfo structure.
 */
void PlatformData::parseGraphFromXmlFile() {
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();

    for (size_t i = 0U; i < getInstance()->mStaticCfg.mCameras.size(); ++i) {
        const string& fileName = getInstance()->mStaticCfg.mCameras[i].mGraphSettingsFile;
        if (fileName.empty()) {
            continue;
        }

        LOG2("Using graph setting file:%s for camera:%zu", fileName.c_str(), i);
        int ret = graphConfig->parse(i, fileName.c_str());
        CheckAndLogError(ret != OK, VOID_VALUE, "Could not read graph config file for camera %zu",
                         i);
    }
}

void PlatformData::releaseGraphConfigNodes() {
    std::shared_ptr<GraphConfig> graphConfig = std::make_shared<GraphConfig>();
    graphConfig->releaseGraphNodes();
}

const char* PlatformData::getSensorName(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorName.c_str();
}

bool PlatformData::isHALZslSupported(int cameraId) {
    const std::string str = "control.enableZsl";
    auto v = getByteStaticMetadata(cameraId, str);
    if (v.size() == 1) {
        return static_cast<bool>(v[0]);
    }

    return false;
}

float PlatformData::getSensorRatio(int cameraId) {
    const std::string str = "sensor.info.pixelArraySize";
    auto v = getFloatStaticMetadata(cameraId, str);
    if (v.size() == 2) {
        return static_cast<float>(v[0]) / v[1];
    }
    return 0.0f;
}

const char* PlatformData::getSensorDescription(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].sensorDescription.c_str();
}

const char* PlatformData::getLensName(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensName.c_str();
}

int PlatformData::getLensHwType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mLensHwType;
}

bool PlatformData::isPdafEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnablePdaf;
}

bool PlatformData::getSensorAwbEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorAwb;
}

bool PlatformData::getSensorAeEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorAe;
}

bool PlatformData::getRunIspAlways(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mRunIspAlways;
}

int PlatformData::getDVSType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDVSType;
}

bool PlatformData::getPSACompression(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPSACompression;
}

bool PlatformData::getOFSCompression(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mOFSCompression;
}

int PlatformData::getCITMaxMargin(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mCITMaxMargin;
}

bool PlatformData::isEnableAIQ(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAIQ;
}

int PlatformData::getAiqRunningInterval(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mAiqRunningInterval;
}

bool PlatformData::isEnableMkn(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableMkn;
}

float PlatformData::getAlgoRunningRate(int algo, int cameraId) {
    PlatformData::StaticCfg::CameraInfo* pCam = &getInstance()->mStaticCfg.mCameras[cameraId];

    if (pCam->mAlgoRunningRateMap.find(algo) != pCam->mAlgoRunningRateMap.end()) {
        return pCam->mAlgoRunningRateMap[algo];
    }

    return 0.0;
}

bool PlatformData::isStatsRunningRateSupport(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mStatsRunningRate;
}

bool PlatformData::isFaceDetectionSupported(int cameraId) {
    const std::string str = "statistics.info.availableFaceDetectModes";
    auto v = getByteStaticMetadata(cameraId, str);
    for (size_t i = 0U; i < v.size(); i++) {
        if (v[i] != CAMERA_STATISTICS_FACE_DETECT_MODE_OFF) {
            return true;
        }
    }

    return false;
}

bool PlatformData::isFaceAeEnabled(int cameraId) {
    return (isFaceDetectionSupported(cameraId) &&
            getInstance()->mStaticCfg.mCameras[cameraId].mFaceAeEnabled);
}

int PlatformData::faceEngineVendor(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineVendor;
}

int PlatformData::faceEngineRunningInterval(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningInterval;
}

int PlatformData::faceEngineRunningIntervalNoFace(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mFaceEngineRunningIntervalNoFace;
}

bool PlatformData::runFaceWithSyncMode(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mRunFaceWithSyncMode;
}

unsigned int PlatformData::getMaxFaceDetectionNumber(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxFaceDetectionNumber;
}

bool PlatformData::isDvsSupported(int cameraId) {
    auto metadata = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;
    bool supported = false;
    for (auto it : metadata->mVideoStabilizationModes) {
        if (it == VIDEO_STABILIZATION_MODE_ON) {
            supported = true;
        }
    }

    LOG2("@%s, dvs supported:%d", __func__, supported);
    return supported;
}

bool PlatformData::psysBundleWithAic(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysBundleWithAic;
}

bool PlatformData::swProcessingAlignWithIsp(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSwProcessingAlignWithIsp;
}

bool PlatformData::isUsingSensorDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseSensorDigitalGain;
}

bool PlatformData::isUsingIspDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseIspDigitalGain;
}

bool PlatformData::isNeedToPreRegisterBuffer(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mNeedPreRegisterBuffers;
}

int PlatformData::getExposureNum(int cameraId, bool multiExposure) {
    if (multiExposure) {
        return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureNum;
    }

    return 1;
}

// HDR_FEATURE_S
bool PlatformData::isEnableHDR(int cameraId) {
    return (getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureType !=
            SENSOR_EXPOSURE_SINGLE);
}

int PlatformData::getHDRStatsInputBitDepth(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mHdrStatsInputBitDepth;
}

int PlatformData::getHDRStatsOutputBitDepth(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mHdrStatsOutputBitDepth;
}

int PlatformData::isUseFixedHDRExposureInfo(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseFixedHdrExposureInfo;
}
// HDR_FEATURE_E

bool PlatformData::isMultiExposureCase(int cameraId, TuningMode tuningMode) {
    // HDR_FEATURE_S
    if ((tuningMode == TUNING_MODE_VIDEO_HDR) || (tuningMode == TUNING_MODE_VIDEO_HDR2) ||
        (tuningMode == TUNING_MODE_VIDEO_HLC)) {
        return true;
    } else if (getSensorAeEnable(cameraId)) {
        return true;
    }
    // HDR_FEATURE_E

    return false;
}

int PlatformData::getSensorExposureType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorExposureType;
}

int PlatformData::getSensorGainType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorGainType;
}

bool PlatformData::isSkipFrameOnSTR2MMIOErr(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSkipFrameV4L2Error;
}

unsigned int PlatformData::getInitialSkipFrame(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mInitialSkipFrame;
}

unsigned int PlatformData::getMaxRawDataNum(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxRawDataNum;
}

bool PlatformData::getTopBottomReverse(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mTopBottomReverse;
}

bool PlatformData::isPsysContinueStats(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPsysContinueStats;
}

bool PlatformData::unregisterExtDmaBuf(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUnregisterExtDmaBuf;
}

unsigned int PlatformData::getPreferredBufQSize(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPreferredBufQSize;
}

int PlatformData::getMaxSensorDigitalGain(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMaxSensorDigitalGain;
}

SensorDgType PlatformData::sensorDigitalGainType(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorDgType;
}

int PlatformData::getDigitalGainLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDigitalGainLag;
}

int PlatformData::getExposureLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mExposureLag;
}

int PlatformData::getAnalogGainLag(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mAnalogGainLag;
}

int PlatformData::numberOfCameras() {
    return getInstance()->mStaticCfg.mCameras.size();
}

int PlatformData::getXmlCameraNumber() {
    return getInstance()->mStaticCfg.mCommonConfig.cameraNumber;
}

MediaCtlConf* PlatformData::getMediaCtlConf(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf;
}

void PlatformData::getDeviceInfo(int cameraId, device_info_t& info) {
    info.device_version = 1;
    info.facing = getInstance()->mStaticCfg.mCameras[cameraId].mFacing;
    info.orientation = getInstance()->mStaticCfg.mCameras[cameraId].mOrientation;
    info.name = getSensorName(cameraId);
    info.description = getSensorDescription(cameraId);
}

// VIRTUAL_CHANNEL_S
int PlatformData::getVCInfo(int cameraId, vc_info_t& vc) {
    vc.total_num = 0;
    if (getInstance()->mStaticCfg.mCameras[cameraId].mVCCount != 0) {
        vc.total_num = getInstance()->mStaticCfg.mCameras[cameraId].mVCCount;
        vc.id = getInstance()->mStaticCfg.mCameras[cameraId].mVCId;
        vc.group = getInstance()->mStaticCfg.mCameras[cameraId].mVCGroupId;
    }
    return OK;
}
// VIRTUAL_CHANNEL_E

const StaticMetadata *PlatformData::getStaticMetadata(int cameraId) {
    return &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;
}

const vector<uint8_t> PlatformData::getByteStaticMetadata(int cameraId, const std::string str) {
    const StaticMetadata *staticMetadata
        = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    std::vector<uint8_t> v;
    if (staticMetadata->mByteMetadata.find(str) != staticMetadata->mByteMetadata.end()) {
        v = staticMetadata->mByteMetadata.at(str);
    }

    return v;
}

const vector<int32_t> PlatformData::getInt32StaticMetadata(int cameraId, const std::string str) {
    const StaticMetadata *staticMetadata
        = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    std::vector<int32_t> v;
    if (staticMetadata->mInt32Metadata.find(str) != staticMetadata->mInt32Metadata.end()) {
        v = staticMetadata->mInt32Metadata.at(str);
    }

    return v;
}

const vector<int64_t> PlatformData::getInt64StaticMetadata(int cameraId, const std::string str) {
    const StaticMetadata *staticMetadata
        = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    std::vector<int64_t> v;
    if (staticMetadata->mInt64Metadata.find(str) != staticMetadata->mInt64Metadata.end()) {
        v = staticMetadata->mInt64Metadata.at(str);
    }

    return v;
}

const vector<float> PlatformData::getFloatStaticMetadata(int cameraId, const std::string str) {
    const StaticMetadata *staticMetadata
        = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    std::vector<float> v;
    if (staticMetadata->mFloatMetadata.find(str) != staticMetadata->mFloatMetadata.end()) {
        v = staticMetadata->mFloatMetadata.at(str);
    }

    return v;
}

const vector<double> PlatformData::getDoubleStaticMetadata(int cameraId, const std::string str) {
    const StaticMetadata *staticMetadata
        = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    std::vector<double> v;
    if (staticMetadata->mDoubleMetadata.find(str) != staticMetadata->mDoubleMetadata.end()) {
        v = staticMetadata->mDoubleMetadata.at(str);
    }

    return v;
}

bool PlatformData::isFeatureSupported(int cameraId, camera_features feature) {
    auto metadata = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;
    for (auto& item : metadata->mSupportedFeatures) {
        if (item == feature) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isSupportedStream(int cameraId, const stream_t& conf) {
    int width = conf.width;
    int height = conf.height;
    int format = conf.format;
    int field = conf.field;

    auto metadata = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;
    bool sameConfigFound = false;
    for (auto const& config : metadata->mConfigsArray) {
        if ((config.format == format) && (config.field == field) && (config.width == width) &&
            (config.height == height)) {
            sameConfigFound = true;
            break;
        }
    }

    return sameConfigFound;
}

void PlatformData::getSupportedISysSizes(int cameraId, vector<camera_resolution_t>& resolutions) {
    resolutions = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysSizes;
}

bool PlatformData::getSupportedISysFormats(int cameraId, vector<int>& formats) {
    formats = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;

    return true;
}

int PlatformData::getISysFormat(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc;
}

/**
 * The ISYS format is determined by the steps below:
 * 1. Try to use the specified format in media control config if it exists.
 * 2. If the given format is supported by ISYS, then use it.
 * 3. Use the first supported format if still could not find an appropriate one.
 */
void PlatformData::selectISysFormat(int cameraId, int format) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    if ((mc != nullptr) && (mc->format != -1)) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = mc->format;
    } else if (isISysSupportedFormat(cameraId, format)) {
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = format;
    } else {
        // Set the first one in support list to default Isys output.
        vector<int> supportedFormat =
            getInstance()->mStaticCfg.mCameras[cameraId].mSupportedISysFormat;
        getInstance()->mStaticCfg.mCameras[cameraId].mISysFourcc = supportedFormat[0];
    }
}

/**
 * The media control config is determined by the steps below:
 * 1. Check if can get one from the given MC ID.
 * 2. And then, try to use ConfigMode to find matched one.
 * 3. Use stream config to get a corresponding mc id, and then get the config by id.
 * 4. Return nullptr if still could not find an appropriate one.
 */
void PlatformData::selectMcConf(int cameraId, stream_t stream, ConfigMode mode, int mcId) {
    if (!isIsysEnabled(cameraId)) {
        return;
    }

    const StaticCfg::CameraInfo& pCam = getInstance()->mStaticCfg.mCameras[cameraId];

    MediaCtlConf* mcConfig = getMcConfByMcId(pCam, mcId);
    if (!mcConfig) {
        mcConfig = getMcConfByConfigMode(pCam, stream, mode);
    }

    if (!mcConfig) {
        mcConfig = getMcConfByStream(pCam, stream);
    }

    getInstance()->mStaticCfg.mCameras[cameraId].mCurrentMcConf = mcConfig;

    if (!mcConfig) {
        LOGE("No matching McConf: cameraId %d, configMode %d, mcId %d", cameraId, mode, mcId);
    }
}

/*
 * Find the MediaCtlConf based on the given MC id.
 */
MediaCtlConf* PlatformData::getMcConfByMcId(const StaticCfg::CameraInfo& cameraInfo, int mcId) {
    if (mcId == -1) {
        return nullptr;
    }

    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        if (mcId == mc.mcId) {
            return (MediaCtlConf*)&mc;
        }
    }

    return nullptr;
}

/*
 * Find the MediaCtlConf based on MC id in mStreamToMcMap.
 */
MediaCtlConf* PlatformData::getMcConfByStream(const StaticCfg::CameraInfo& cameraInfo,
                                              const stream_t& stream) {
    int mcId = -1;
    for (auto& table : cameraInfo.mStreamToMcMap) {
        for (auto& config : table.second) {
            if ((config.format == stream.format) && (config.field == stream.field) &&
                (config.width == stream.width) && (config.height == stream.height)) {
                mcId = table.first;
                break;
            }
        }
        if (mcId != -1) {
            break;
        }
    }

    return getMcConfByMcId(cameraInfo, mcId);
}

/*
 * Find the MediaCtlConf based on operation mode and stream info.
 */
MediaCtlConf* PlatformData::getMcConfByConfigMode(const StaticCfg::CameraInfo& cameraInfo,
                                                  const stream_t& stream, ConfigMode mode) {
    for (auto& mc : cameraInfo.mMediaCtlConfs) {
        for (auto& cfgMode : mc.configMode) {
            if (mode != cfgMode) {
                continue;
            }

            int outputWidth = mc.outputWidth;
            int outputHeight = mc.outputHeight;
            int stride = CameraUtils::getStride(mc.format, mc.outputWidth);
            bool sameStride = (stride == CameraUtils::getStride(mc.format, stream.width));
            /*
             * outputWidth and outputHeight is 0 means the ISYS output size
             * is dynamic, we don't need to check if it matches with stream config.
             */
            if (((outputWidth == 0) && (outputHeight == 0)) ||
                (((stream.width == outputWidth) || sameStride) &&
                 (stream.height == outputHeight))) {
                return (MediaCtlConf*)&mc;
            }
        }
    }

    return nullptr;
}

/*
 * Check if video node is enabled via camera Id and video node type.
 */
bool PlatformData::isVideoNodeEnabled(int cameraId, VideoNodeType type) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    if (mc != nullptr) {
        return false;
    }

    for (auto const& nd : mc->videoNodes) {
        if (type == nd.videoNodeType) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isISysSupportedFormat(int cameraId, int format) {
    vector<int> supportedFormat;
    (void)getSupportedISysFormats(cameraId, supportedFormat);

    for (auto const fmt : supportedFormat) {
        if (format == fmt) {
            return true;
        }
    }
    return false;
}

bool PlatformData::isISysSupportedResolution(int cameraId, camera_resolution_t resolution) {
    vector<camera_resolution_t> res;
    getSupportedISysSizes(cameraId, res);

    for (auto const& size : res) {
        if (resolution.width == size.width && resolution.height == size.height) {
            return true;
        }
    }

    return false;
}

int PlatformData::getISysRawFormat(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mISysRawFormat;
}

// CSI_META_S
bool PlatformData::isCsiMetaEnabled(int cameraId) {
    // FILE_SOURCE_S
    if (isFileSourceEnabled()) {
        return false;
    }
    // FILE_SOURCE_E
    return isVideoNodeEnabled(cameraId, VIDEO_CSI_META);
}
// CSI_META_E

bool PlatformData::isAiqdEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mEnableAiqd;
}

int PlatformData::getFormatByDevName(int cameraId, const string& devName, McFormat& format) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (auto& fmt : mc->formats) {
        if ((fmt.formatType == FC_FORMAT) && (devName == fmt.entityName)) {
            format = fmt;
            return OK;
        }
    }

    LOGE("Failed to find DevName for cameraId: %d, devname: %s", cameraId, devName.c_str());
    return BAD_VALUE;
}

int PlatformData::getVideoNodeNameByType(int cameraId, VideoNodeType videoNodeType,
                                         string& videoNodeName) {
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, BAD_VALUE, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (auto const& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            videoNodeName = nd.name;
            return OK;
        }
    }

    LOGE("failed to find video note name for cameraId: %d", cameraId);
    return BAD_VALUE;
}

int PlatformData::getDevNameByType(int cameraId, VideoNodeType videoNodeType, string& devName) {
    if (!isIsysEnabled(cameraId)) {
        return OK;
    }

    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    bool isSubDev = false;

    switch (videoNodeType) {
        case VIDEO_PIXEL_ARRAY:
        case VIDEO_PIXEL_BINNER:
        case VIDEO_PIXEL_SCALER: {
            isSubDev = true;
            // For sensor subdevices are fixed and sensor HW may be initialized before configure,
            // the first MediaCtlConf is used to find sensor subdevice name.
            PlatformData::StaticCfg::CameraInfo* pCam =
                &getInstance()->mStaticCfg.mCameras[cameraId];
            mc = &pCam->mMediaCtlConfs[0];
            break;
        }
        case VIDEO_ISYS_RECEIVER_BACKEND:
        case VIDEO_ISYS_RECEIVER: {
            isSubDev = true;
            break;
        }
        default:
            break;
    }

    CheckAndLogError(!mc, NAME_NOT_FOUND, "failed to get MediaCtlConf, videoNodeType %d",
                     videoNodeType);

    for (auto& nd : mc->videoNodes) {
        if (videoNodeType == nd.videoNodeType) {
            string tmpDevName;
            CameraUtils::getDeviceName(nd.name.c_str(), tmpDevName, isSubDev);
            if (!tmpDevName.empty()) {
                devName = tmpDevName;
                LOG2("@%s, Found DevName. cameraId: %d, get video node: %s, devname: %s", __func__,
                     cameraId, nd.name.c_str(), devName.c_str());
                return OK;
            } else {
                // Use default device name if cannot find it
                if (isSubDev)
                    devName = "/dev/v4l-subdev1";
                else
                    devName = "/dev/video5";
                LOGE("Failed to find DevName for cameraId: %d, get video node: %s, devname: %s",
                     cameraId, nd.name.c_str(), devName.c_str());
                return NAME_NOT_FOUND;
            }
        }
    }

    LOG1("Failed to find devname for cameraId: %d, use default setting instead", cameraId);
    return NAME_NOT_FOUND;
}

/**
 * The ISYS best resolution is determined by the steps below:
 * 1. If the resolution is specified in MediaCtlConf, then use it.
 * 2. Try to find the exact matched one in ISYS supported resolutions.
 * 3. Try to find the same ratio resolution.
 * 4. If still couldn't get one, then use the biggest one.
 */
camera_resolution_t PlatformData::getISysBestResolution(int cameraId, int width, int height,
                                                        int field) {
    LOG1("@%s, width:%d, height:%d", __func__, width, height);

    // Skip for interlace, we only support by-pass in interlaced mode
    if (field == V4L2_FIELD_ALTERNATE) {
        return {width, height};
    }

    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    // The isys output size is fixed if outputWidth/outputHeight != 0
    // So we use it to as the ISYS resolution.
    if ((mc != nullptr) && (mc->outputWidth != 0) && (mc->outputHeight != 0)) {
        return {mc->outputWidth, mc->outputHeight};
    }

    const float RATIO_TOLERANCE = 0.05f;  // Supported aspect ratios that are within RATIO_TOLERANCE
    const float kTargetRatio = static_cast<float>(width) / height;

    vector<camera_resolution_t> res;
    // The supported resolutions are saved in res with ascending order(small -> bigger)
    getSupportedISysSizes(cameraId, res);

    // Try to find out the same resolution in the supported isys resolution list
    // if it couldn't find out the same one, then use the bigger one which is the same ratio
    for (auto const& size : res) {
        if ((width <= size.width) && (height <= size.height) &&
            (fabs(static_cast<float>(size.width) / size.height - kTargetRatio) < RATIO_TOLERANCE)) {
            LOG1("@%s: Found the best ISYS resoltoution (%d)x(%d)", __func__, size.width,
                 size.height);
            return {size.width, size.height};
        }
    }

    // If it still couldn't find one, then use the biggest one in the supported list.
    LOG1("@%s: ISYS resolution not found, used the biggest one: (%d)x(%d)", __func__,
         res.back().width, res.back().height);
    return {res.back().width, res.back().height};
}

bool PlatformData::isIsysEnabled(int cameraId) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mMediaCtlConfs.empty()) {
        return false;
    }
    return true;
}

int PlatformData::calculateFrameParams(int cameraId, SensorFrameParams& sensorFrameParams) {
    if (!isIsysEnabled(cameraId)) {
        LOG2("%s, no mc, just use default from xml", __func__);
        vector<camera_resolution_t> res;
        getSupportedISysSizes(cameraId, res);

        CheckAndLogError(res.empty(), BAD_VALUE, "Supported ISYS resolutions are not configured.");
        sensorFrameParams = {
            0, 0, static_cast<uint32_t>(res[0].width), static_cast<uint32_t>(res[0].height), 1, 1,
            1, 1};

        return OK;
    }

    CLEAR(sensorFrameParams);

    uint32_t width = 0U;
    uint32_t horizontalOffset = 0U;
    uint32_t horizontalBinNum = 1U;
    uint32_t horizontalBinDenom = 1U;
    uint32_t horizontalBin = 1U;

    uint32_t height = 0U;
    uint32_t verticalOffset = 0U;
    uint32_t verticalBinNum = 1U;
    uint32_t verticalBinDenom = 1U;
    uint32_t verticalBin = 1U;

    /**
     * For this function, it may be called without configuring stream
     * in some UT cases, the mc is nullptr at this moment. So we need to
     * get one default mc to calculate frame params.
     */
    MediaCtlConf* mc = PlatformData::getMediaCtlConf(cameraId);
    if (mc == nullptr) {
        PlatformData::StaticCfg::CameraInfo* pCam = &getInstance()->mStaticCfg.mCameras[cameraId];
        mc = &pCam->mMediaCtlConfs[0];
    }

    bool pixArraySizeFound = false;
    for (auto const& current : mc->formats) {
        if ((!pixArraySizeFound) && (current.width > 0) && (current.height > 0)) {
            width = current.width;
            height = current.height;
            pixArraySizeFound = true;
            LOG2("%s: active pixel array H=%d, W=%d", __func__, height, width);
            // Setup initial sensor frame params.
            sensorFrameParams.horizontal_crop_offset += horizontalOffset;
            sensorFrameParams.vertical_crop_offset += verticalOffset;
            sensorFrameParams.cropped_image_width = width;
            sensorFrameParams.cropped_image_height = height;
            sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
            sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
            sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
            sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
        }

        if (current.formatType != FC_SELECTION) {
            continue;
        }

        if (current.selCmd == V4L2_SEL_TGT_CROP) {
            width = current.width * horizontalBin;
            horizontalOffset = current.left * horizontalBin;
            height = current.height * verticalBin;
            verticalOffset = current.top * verticalBin;

            LOG2("%s: crop (binning factor: hor/vert:%d,%d)", __func__, horizontalBin, verticalBin);

            LOG2("%s: crop left = %d, top = %d, width = %d height = %d", __func__, horizontalOffset,
                 verticalOffset, width, height);

        } else if (current.selCmd == V4L2_SEL_TGT_COMPOSE) {
            if ((width == 0U) || (height == 0U)) {
                LOGE(
                    "Invalid XML configuration, no pixel array width/height when handling compose, "
                    "skip.");
                return BAD_VALUE;
            }
            if ((current.width == 0) || (current.height == 0)) {
                LOGW(
                    "%s: Invalid XML configuration for TGT_COMPOSE,"
                    "0 value detected in width or height",
                    __func__);
                return BAD_VALUE;
            } else {
                LOG2("%s: Compose width %d/%d, height %d/%d", __func__, width, current.width,
                     height, current.height);
                // the scale factor should be float, so multiple numerator and denominator
                // with coefficient to indicate float factor
                const int SCALE_FACTOR_COEF = 10;
                horizontalBin = width / current.width;
                horizontalBinNum = width * SCALE_FACTOR_COEF / current.width;
                horizontalBinDenom = SCALE_FACTOR_COEF;
                verticalBin = height / current.height;
                verticalBinNum = height * SCALE_FACTOR_COEF / current.height;
                verticalBinDenom = SCALE_FACTOR_COEF;
            }

            LOG2("%s: COMPOSE horizontal bin factor=%d, (%d/%d)", __func__, horizontalBin,
                 horizontalBinNum, horizontalBinDenom);
            LOG2("%s: COMPOSE vertical bin factor=%d, (%d/%d)", __func__, verticalBin,
                 verticalBinNum, verticalBinDenom);
        } else {
            LOGW("%s: Target for selection is not CROP neither COMPOSE!", __func__);
            continue;
        }

        sensorFrameParams.horizontal_crop_offset += horizontalOffset;
        sensorFrameParams.vertical_crop_offset += verticalOffset;
        sensorFrameParams.cropped_image_width = width;
        sensorFrameParams.cropped_image_height = height;
        sensorFrameParams.horizontal_scaling_numerator = horizontalBinNum;
        sensorFrameParams.horizontal_scaling_denominator = horizontalBinDenom;
        sensorFrameParams.vertical_scaling_numerator = verticalBinNum;
        sensorFrameParams.vertical_scaling_denominator = verticalBinDenom;
    }

    vector<ConfigMode> cms;

    int ret = PlatformData::getConfigModesByOperationMode(
        cameraId, CAMERA_STREAM_CONFIGURATION_MODE_AUTO, cms);
    CheckWarning((ret != 0) || cms.empty(), ret,
                 "@%s, getConfigModesByOperationMode: %d, cms size %llu", __func__, ret,
                 cms.size());

    auto gc = CameraContext::getInstance(cameraId)->getGraphConfig(cms[0]);
    CheckWarning(gc == nullptr, BAD_VALUE, "@%s, gc is nullptr", __func__);

    IspRawCropInfo info;
    ret = gc->getIspRawCropInfo(info);
    CheckWarning(ret != OK, BAD_VALUE, "failed to get raw crop info (%d)", ret);

    LOG1("Isp raw crop [%d, %d, %d, %d], wxh [%d x %d]", info.left, info.top, info.right,
         info.bottom, info.outputWidth, info.outputHeight);

    if (static_cast<int32_t>(sensorFrameParams.horizontal_crop_offset) + info.left < 0) {
        sensorFrameParams.horizontal_crop_offset = 0U;
    } else {
        sensorFrameParams.horizontal_crop_offset += info.left;
    }
    if (static_cast<int32_t>(sensorFrameParams.vertical_crop_offset) + info.top < 0) {
        sensorFrameParams.vertical_crop_offset = 0U;
    } else {
        sensorFrameParams.vertical_crop_offset += info.top;
    }
    sensorFrameParams.cropped_image_width = info.outputWidth;
    sensorFrameParams.cropped_image_height = info.outputHeight;

    return OK;
}

void PlatformData::getSupportedTuningConfig(int cameraId, vector<TuningConfig>& configs) {
    configs = getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig;
}

int PlatformData::getConfigModesByOperationMode(int cameraId, uint32_t operationMode,
                                                vector<ConfigMode>& configModes) {
    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_END) {
        LOG2("%s: operationMode was invalid operation mode", __func__);
        return INVALID_OPERATION;
    }

    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist", __func__);

    if (operationMode == CAMERA_STREAM_CONFIGURATION_MODE_AUTO) {
        if (getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto.empty()) {
            // Use the first config mode as default for auto
            configModes.push_back(
                getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig[0].configMode);
            LOG2("%s: add config mode %d for operation mode %d", __func__, configModes[0],
                 operationMode);
        } else {
            configModes = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModesForAuto;
        }
    } else {
        for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
            if (operationMode == (uint32_t)cfg.configMode) {
                configModes.push_back(cfg.configMode);
                LOG2("%s: add config mode %d for operation mode %d", __func__, cfg.configMode,
                     operationMode);
            }
        }
    }

    if (configModes.size() > 0) {
        return OK;
    }
    LOGW("%s, configure number %zu, operationMode %x, cameraId %d", __func__, configModes.size(),
         operationMode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getTuningModeByConfigMode(int cameraId, ConfigMode configMode,
                                            TuningMode& tuningMode) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "the tuning config in xml does not exist");

    for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        LOG2("%s, tuningMode %d, configMode %x", __func__, cfg.tuningMode, cfg.configMode);
        if (cfg.configMode == configMode) {
            tuningMode = cfg.tuningMode;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no tuningModes", __func__, configMode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getTuningConfigByConfigMode(int cameraId, ConfigMode mode, TuningConfig& config) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist.", __func__);

    for (auto& cfg : getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig) {
        if (cfg.configMode == mode) {
            config = cfg;
            return OK;
        }
    }

    LOGW("%s, configMode %x, cameraId %d, no TuningConfig", __func__, mode, cameraId);
    return INVALID_OPERATION;
}

int PlatformData::getStreamIdByConfigMode(int cameraId, ConfigMode configMode) {
    std::map<int, int> modeMap = getInstance()->mStaticCfg.mCameras[cameraId].mConfigModeToStreamId;
    return modeMap.find(configMode) == modeMap.end() ? -1 : modeMap[configMode];
}

int32_t PlatformData::getMaxPipelineDepth(int cameraId) {
    const std::string str = "request.pipelineMaxDepth";
    auto v = getByteStaticMetadata(cameraId, str);
    if (v.size() == 1) {
        return v[0];
    }

    return MAX_BUFFER_COUNT;
}

int PlatformData::getMaxRequestsInflight(int cameraId) {
    int inflight = getInstance()->mStaticCfg.mCameras[cameraId].mMaxRequestsInflight;
    if (inflight <= 0) {
        inflight = isEnableAIQ(cameraId) ? 4 : MAX_BUFFER_COUNT;
    }

    return inflight;
}

camera_yuv_color_range_mode_t PlatformData::getYuvColorRangeMode(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mYuvColorRangeMode;
}

ia_binary_data* PlatformData::getAiqd(int cameraId, TuningMode mode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getAiqd(mode);
}

void PlatformData::saveAiqd(int cameraId, TuningMode tuningMode, const ia_binary_data& data) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    aiqInitData->saveAiqd(tuningMode, data);
}

int PlatformData::getCpf(int cameraId, TuningMode mode, ia_binary_data* aiqbData) {
    CheckAndLogError(cameraId >= MAX_CAMERA_NUMBER, BAD_VALUE, "@%s, bad cameraId:%d", __func__,
                     cameraId);
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mSupportedTuningConfig.empty(),
                     INVALID_OPERATION, "@%s, the tuning config in xml does not exist", __func__);

    AiqInitData* aiqInitData = getInstance()->mAiqInitData[cameraId];
    return aiqInitData->getCpf(mode, aiqbData);
}

bool PlatformData::isCSIBackEndCapture(int cameraId) {
    bool isCsiBECapture = false;
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (const auto& node : mc->videoNodes) {
        if ((node.videoNodeType == VIDEO_GENERIC) &&
            ((node.name.find("BE capture") != string::npos) ||
             (node.name.find("BE SOC capture") != string::npos))) {
            isCsiBECapture = true;
            break;
        }
    }

    return isCsiBECapture;
}

bool PlatformData::isTPGReceiver(int cameraId) {
    bool isTPGCapture = false;
    MediaCtlConf* mc = getMediaCtlConf(cameraId);
    CheckAndLogError(!mc, false, "getMediaCtlConf returns nullptr, cameraId:%d", cameraId);

    for (const auto& node : mc->videoNodes) {
        if ((node.videoNodeType == VIDEO_ISYS_RECEIVER) &&
            ((node.name.find("TPG") != string::npos))) {
            isTPGCapture = true;
            break;
        }
    }
    return isTPGCapture;
}

int PlatformData::getSupportAeExposureTimeRange(int cameraId, camera_scene_mode_t sceneMode,
                                                camera_range_t& etRange) {
    const std::string str = "sensor.info.exposureTimeRange";
    auto v = PlatformData::getInt64StaticMetadata(cameraId, str);
    if (v.size() == 2) {
        etRange = {static_cast<float>(v[0]), static_cast<float>(v[1])};
        return OK;
    }

    auto metadata = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    for (auto& item : metadata->mAeExposureTimeRange) {
        if (item.scene == sceneMode) {
            etRange = {item.minValue, item.maxValue};
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

int PlatformData::getSupportAeGainRange(int cameraId, camera_scene_mode_t sceneMode,
                                        camera_range_t& gainRange) {
    auto metadata = &getInstance()->mStaticCfg.mCameras[cameraId].mStaticMetadata;

    for (auto& item : metadata->mAeGainRange) {
        if (item.scene == sceneMode) {
            gainRange = {item.minValue, item.maxValue};
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

bool PlatformData::isUsingCrlModule(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUseCrlModule;
}

vector<MultiExpRange> PlatformData::getMultiExpRanges(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMultiExpRanges;
}

// FILE_SOURCE_S
const char* PlatformData::getInjectedFile() {
    const char* PROP_CAMERA_FILE_INJECTION = "cameraInjectFile";
    return getenv(PROP_CAMERA_FILE_INJECTION);
}

bool PlatformData::isFileSourceEnabled() {
    return getInjectedFile() != nullptr;
}
// FILE_SOURCE_E

// VIRTUAL_CHANNEL_S
int PlatformData::getVirtualChannelId(int cameraId) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mVCCount != 0) {
        return getInstance()->mStaticCfg.mCameras[cameraId].mVCId;
    }

    /* the default value of virtual channel is 0  */
    return 0;
}
// VIRTUAL_CHANNEL_E

bool PlatformData::isTestPatternSupported(int cameraId) {
    return !getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty();
}

int32_t PlatformData::getSensorTestPattern(int cameraId, int32_t mode) {
    CheckAndLogError(getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap.empty(), -1,
                     "<id%d>@%s, mTestPatternMap is empty!", cameraId, __func__);
    auto testPatternMap = getInstance()->mStaticCfg.mCameras[cameraId].mTestPatternMap;

    if (testPatternMap.find(mode) == testPatternMap.end()) {
        LOGW("Test pattern %d wasn't found in configuration file, return -1", mode);
        return -1;
    }
    return testPatternMap[mode];
}

ia_binary_data* PlatformData::getNvm(int cameraId) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), nullptr,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    // Allow overwritten nvm file if needed
    int size = getInstance()->mStaticCfg.mCameras[cameraId].mNvmOverwrittenFileSize;
    const char* nvmFile = getInstance()->mStaticCfg.mCameras[cameraId].mNvmOverwrittenFile.c_str();
    return getInstance()->mAiqInitData[cameraId]->getNvm(cameraId, nvmFile, size);
}

camera_coordinate_system_t PlatformData::getActivePixelArray(int cameraId) {
    const std::string str = "sensor.info.activeArraySize";
    auto v = getInt32StaticMetadata(cameraId, str);
    if (v.size() != 4) {
        return {0, 0, 0, 0};
    }

    return {v[0], v[1], v[2], v[3]};
}

string PlatformData::getCameraCfgPath() {
    char* p = getenv("CAMERA_CFG_PATH");

    return p ? string(p) : string(CAMERA_DEFAULT_CFG_PATH);
}

string PlatformData::getGraphSettingFilePath() {
    return PlatformData::getCameraCfgPath() + string(CAMERA_GRAPH_SETTINGS_DIR);
}

int PlatformData::getSensorDigitalGain(int cameraId, float realDigitalGain) {
    int sensorDg = 0;
    int maxSensorDg = PlatformData::getMaxSensorDigitalGain(cameraId);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        int index = 0;
        while (pow(2, index) <= realDigitalGain) {
            sensorDg = index;
            index++;
        }
        sensorDg = CLIP(sensorDg, maxSensorDg, 0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d", __func__,
             PlatformData::sensorDigitalGainType(cameraId));
    }

    return sensorDg;
}

float PlatformData::getIspDigitalGain(int cameraId, float realDigitalGain) {
    float ispDg = 1.0f;
    int sensorDg = getSensorDigitalGain(cameraId, realDigitalGain);

    if (PlatformData::sensorDigitalGainType(cameraId) == SENSOR_DG_TYPE_2_X) {
        ispDg = realDigitalGain / pow(2, sensorDg);
        ispDg = CLIP(ispDg, ispDg, 1.0);
    } else {
        LOGE("%s, don't support the sensor digital gain type: %d", __func__,
             PlatformData::sensorDigitalGainType(cameraId));
    }

    return ispDg;
}

int PlatformData::initMakernote(int cameraId, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);
    return getInstance()->mAiqInitData[cameraId]->initMakernote(cameraId, tuningMode);
}

int PlatformData::deinitMakernote(int cameraId, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);
    return getInstance()->mAiqInitData[cameraId]->deinitMakernote(cameraId, tuningMode);
}

int PlatformData::saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                    int64_t sequence, TuningMode tuningMode) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), BAD_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    return getInstance()->mAiqInitData[cameraId]->saveMakernoteData(cameraId, makernoteMode,
                                                                    sequence, tuningMode);
}

void PlatformData::updateMakernoteTimeStamp(int cameraId, int64_t sequence, uint64_t timestamp) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    getInstance()->mAiqInitData[cameraId]->updateMakernoteTimeStamp(sequence, timestamp);
}

void PlatformData::acquireMakernoteData(int cameraId, uint64_t timestamp, uint8_t* buf,
                                        uint32_t& size) {
    CheckAndLogError(cameraId >= static_cast<int>(getInstance()->mAiqInitData.size()), VOID_VALUE,
                     "@%s, bad cameraId:%d", __func__, cameraId);

    getInstance()->mAiqInitData[cameraId]->acquireMakernoteData(timestamp, buf, size);
}

int PlatformData::getScalerInfo(int cameraId, int32_t streamId, float* scalerWidth,
                                float* scalerHeight) {
    if (getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.empty()) {
        *scalerWidth = 1.0;
        *scalerHeight = 1.0;
        return OK;
    }

    for (auto& scalerInfo : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
        LOG2("%s, streamId %d, scalerWidth %f, scalerHeight %f", __func__, scalerInfo.streamId,
             scalerInfo.scalerWidth, scalerInfo.scalerHeight);
        if (scalerInfo.streamId == streamId) {
            *scalerWidth = scalerInfo.scalerWidth;
            *scalerHeight = scalerInfo.scalerHeight;
            break;
        }
    }

    return OK;
}

void PlatformData::setScalerInfo(int cameraId, std::vector<IGraphType::ScalerInfo> scalerInfo) {
    for (auto& scalerInfoInput : scalerInfo) {
        bool flag = false;
        for (auto& scalerInfoTmp : getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo) {
            if (scalerInfoInput.streamId == scalerInfoTmp.streamId) {
                scalerInfoTmp.scalerWidth = scalerInfoInput.scalerWidth;
                scalerInfoTmp.scalerHeight = scalerInfoInput.scalerHeight;
                flag = true;
                break;
            }
        }
        if (!flag) {
            getInstance()->mStaticCfg.mCameras[cameraId].mScalerInfo.push_back(scalerInfoInput);
        }
    }
}

bool PlatformData::isGpuTnrEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mGpuTnrEnabled;
}

bool PlatformData::isUsingGpuIpa() {
    bool enabled = false;
    for (int cameraId =0; cameraId < static_cast<int>(getInstance()->mStaticCfg.mCameras.size());
         cameraId++)
        enabled |= getInstance()->mStaticCfg.mCameras[cameraId].mGpuIpaEnabled;
    return enabled;
}

int PlatformData::getVideoStreamNum() {
    return getInstance()->mStaticCfg.mCommonConfig.videoStreamNum;
}

bool PlatformData::supportUpdateTuning(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mIspTuningUpdate;
}

bool PlatformData::isUsingGpuAlgo() {
    bool enabled = false;
    // currently we have gpu tnr only, we may have other gpu algos
    for (int cameraId =0; cameraId < static_cast<int>(getInstance()->mStaticCfg.mCameras.size());
         cameraId++)
        enabled |= isGpuTnrEnabled(cameraId);
    return enabled;
}

int PlatformData::getTnrExtraFrameCount(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mTnrExtraFrameNum;
}

int PlatformData::getMsOfPsysAlignWithSystem(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mMsPsysAlignWithSystem;
}

void PlatformData::setSensorOrientation(int cameraId, int orientation) {
    getInstance()->mStaticCfg.mCameras[cameraId].mSensorOrientation = orientation;
}

int PlatformData::getSensorOrientation(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mSensorOrientation;
}

bool PlatformData::isDummyStillSink(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mDummyStillSink;
}

bool PlatformData::removeCacheFlushOutputBuffer(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mRemoveCacheFlushOutputBuffer;
}

bool PlatformData::getPLCEnable(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mPLCEnable;
}

bool PlatformData::useGPUProcessor() {
    return getInstance()->mStaticCfg.mCommonConfig.useGpuProcessor;
}

bool PlatformData::isStillOnlyPipeEnabled(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mStillOnlyPipe;
}

bool PlatformData::isUsePSysProcessor(int cameraId) {
    return getInstance()->mStaticCfg.mCameras[cameraId].mUsePSysProcessor;
}

}  // namespace icamera
