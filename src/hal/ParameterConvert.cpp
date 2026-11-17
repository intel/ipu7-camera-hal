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

#define LOG_TAG ParameterConvert

#include "ParameterConvert.h"
#include "PlatformData.h"
#include "ParameterHelper.h"
#include "AiqResultStorage.h"
#include "iutils/CameraLog.h"

namespace icamera {

void ParameterConvert::getConfigInfo(const stream_config_t* streamList, ConfigInfo& info) {
    info.frameUsage = CameraUtils::getFrameUsage(streamList);
    info.resolution = {streamList->streams[0].width, streamList->streams[0].height};
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            info.resolution = {streamList->streams[i].width, streamList->streams[i].height};
            break;
        }
    }
}

int ParameterConvert::setParameters(const Parameters& param, DataContext* dataContext) {
    CheckAndLogError(dataContext == nullptr, UNKNOWN_ERROR, "dataContext is nullptr");

    param.getCropRegion(dataContext->cropRegion);
    param.getDeinterlaceMode(dataContext->deinterlaceMode);
    param.getMonoDsMode(dataContext->monoDsMode);

    setIspSettings(param, dataContext);
    setAiqSettings(param, dataContext);
    return OK;
}

void ParameterConvert::setIspSettings(const Parameters& param, DataContext* dataContext) {
    param.getImageEnhancement(dataContext->mIspParams.enhancement);
    param.getEdgeMode(dataContext->mIspParams.edgeMode);
    param.getNrMode(dataContext->mIspParams.nrMode);
    if (param.getNrLevel(dataContext->mIspParams.nrLevel.nrLevel) == OK) {
        dataContext->mIspParams.nrLevel.set = true;
    }
    param.getVideoStabilizationMode(dataContext->mAiqParams.videoStabilizationMode);
    param.getDigitalZoomRatio(dataContext->mIspParams.digitalZoomRatio);
}

void ParameterConvert::setAiqSettings(const Parameters& param, DataContext* dataContext) {
    // Update AE related parameters
    param.getAeMode(dataContext->mAiqParams.aeMode);
    param.getExposureTime(dataContext->mAiqParams.manualExpTimeUs);
    param.getSensitivityGain(dataContext->mAiqParams.manualGain);
    param.getSensitivityIso(dataContext->mAiqParams.manualIso);
    param.getBlcAreaMode(dataContext->mAiqParams.blcAreaMode);
    param.getAeRegions(dataContext->mAiqParams.aeRegions);
    param.getAeConvergeSpeedMode(dataContext->mAiqParams.aeConvergeSpeedMode);
    param.getAeConvergeSpeed(dataContext->mAiqParams.aeConvergeSpeed);
    param.getRun3ACadence(dataContext->mAiqParams.run3ACadence);
    if (dataContext->mAiqParams.run3ACadence < 1) {
        LOGW("Invalid 3A cadence %d, use default 1.", dataContext->mAiqParams.run3ACadence);
        dataContext->mAiqParams.run3ACadence = 1;
    }

    int ev = 0;
    param.getAeCompensation(ev);
    if (dataContext->mAiqParams.evStep.denominator == 0) {
        dataContext->mAiqParams.evShift = 0.0;
    } else {
        ev = CLIP(ev, dataContext->mAiqParams.evRange.max, dataContext->mAiqParams.evRange.min);
        dataContext->mAiqParams.evShift = static_cast<float>(ev) *
            dataContext->mAiqParams.evStep.numerator / dataContext->mAiqParams.evStep.denominator;
    }

    if (OK == param.getFrameRate(dataContext->mAiqParams.fps)) {
        dataContext->mAiqParams.aeFpsRange.min = dataContext->mAiqParams.fps;
        dataContext->mAiqParams.aeFpsRange.max = dataContext->mAiqParams.fps;
    }
    param.getFpsRange(dataContext->mAiqParams.aeFpsRange);
    param.getAntiBandingMode(dataContext->mAiqParams.antibandingMode);
    // Update AWB related parameters
    param.getAwbMode(dataContext->mAiqParams.awbMode);
    param.getAwbCctRange(dataContext->mAiqParams.cctRange);
    param.getAwbGains(dataContext->mAiqParams.awbManualGain);
    param.getAwbWhitePoint(dataContext->mAiqParams.whitePoint);
    param.getAwbGainShift(dataContext->mAiqParams.awbGainShift);
    param.getColorTransform(dataContext->mAiqParams.manualColorMatrix);
    param.getColorGains(dataContext->mAiqParams.manualColorGains);
    param.getAwbConvergeSpeedMode(dataContext->mAiqParams.awbConvergeSpeedMode);
    param.getAwbConvergeSpeed(dataContext->mAiqParams.awbConvergeSpeed);

    // Update AF related parameters
    param.getAfMode(dataContext->mAiqParams.afMode);
    param.getAfTrigger(dataContext->mAiqParams.afTrigger);
    param.getFocusDistance(dataContext->mAiqParams.focusDistance);

    param.getSceneMode(dataContext->mAiqParams.sceneMode);

    param.getAeDistributionPriority(dataContext->mAiqParams.aeDistributionPriority);

    unsigned int length = sizeof(dataContext->mAiqParams.customAicParam.data);
    if (param.getCustomAicParam(dataContext->mAiqParams.customAicParam.data, &length) == OK) {
        dataContext->mAiqParams.customAicParam.length = length;
    }

    param.getYuvColorRangeMode(dataContext->mAiqParams.yuvColorRangeMode);

    param.getExposureTimeRange(dataContext->mAiqParams.exposureTimeRange);
    param.getSensitivityGainRange(dataContext->mAiqParams.sensitivityGainRange);

    param.getVideoStabilizationMode(dataContext->mAiqParams.videoStabilizationMode);
    param.getLdcMode(dataContext->mAiqParams.ldcMode);
    param.getRscMode(dataContext->mAiqParams.rscMode);
    dataContext->hasFlipMode = param.getFlipMode(dataContext->mAiqParams.flipMode) == OK;
    param.getDigitalZoomRatio(dataContext->mAiqParams.digitalZoomRatio);

    param.getShadingMode(dataContext->mAiqParams.shadingMode);

    dataContext->mAiqParams.dump();
}

int ParameterConvert::getParameters(CameraContext* cameraContext, Parameters& param) {
    CheckAndLogError(cameraContext == nullptr, UNKNOWN_ERROR, "cameraContext is nullptr");

    auto resultStorage = cameraContext->getAiqResultStorage();
    auto aiqResult = resultStorage->getAiqResult();

    param.setExposureTime(aiqResult->mAeResults.exposures[0].exposure[0].exposure_time_us);

    param.setSensitivityIso(aiqResult->mAeResults.exposures[0].exposure[0].iso);
    const float fps = 1000000.0 / aiqResult->mFrameDuration;
    param.setFrameRate(fps);
    LOG2("@%s, ae result: iso %d, exposure time %u, fps %f", __func__,
         aiqResult->mAeResults.exposures[0].exposure[0].iso,
         aiqResult->mAeResults.exposures[0].exposure[0].exposure_time_us, fps);

    // Update AWB related parameters
    const cca::cca_awb_results& result = aiqResult->mAwbResults;
    camera_awb_gains_t awbGains;
    CLEAR(awbGains);
    float normalizedR, normalizedG, normalizedB;

    if (param.getAwbGains(awbGains) == OK) {
        // User manual AWB gains
        awbGains.g_gain = CLIP(awbGains.g_gain, AWB_GAIN_MAX, AWB_GAIN_MIN);
        normalizedG = AiqUtils::normalizeAwbGain(awbGains.g_gain);
    } else {
        // non-manual AWB gains, try to find a proper G that makes R/G/B all in the gain range.
        normalizedG = sqrt((AWB_GAIN_NORMALIZED_START * AWB_GAIN_NORMALIZED_END) /
                           (result.accurate_r_per_g * result.accurate_b_per_g));
        awbGains.g_gain = AiqUtils::convertToUserAwbGain(normalizedG);
    }

    normalizedR = result.accurate_r_per_g * normalizedG;
    normalizedB = result.accurate_b_per_g * normalizedG;

    awbGains.r_gain = AiqUtils::convertToUserAwbGain(normalizedR);
    awbGains.b_gain = AiqUtils::convertToUserAwbGain(normalizedB);

    LOG2("awbGains [r, g, b] = [%d, %d, %d]", awbGains.r_gain, awbGains.g_gain, awbGains.b_gain);
    param.setAwbGains(awbGains);

    // Update the AWB result
    camera_awb_result_t awbResult;
    awbResult.r_per_g = result.accurate_r_per_g;
    awbResult.b_per_g = result.accurate_b_per_g;
    LOG2("awb result: %f, %f", awbResult.r_per_g, awbResult.b_per_g);
    param.setAwbResult(&awbResult);

    camera_color_transform_t ccm;
    MEMCPY_S(ccm.color_transform, sizeof(ccm.color_transform),
             aiqResult->mPaResults.color_conversion_matrix,
             sizeof(aiqResult->mPaResults.color_conversion_matrix));
    param.setColorTransform(ccm);

    camera_color_gains_t colorGains;
    colorGains.color_gains_rggb[0] = aiqResult->mPaResults.color_gains.r;
    colorGains.color_gains_rggb[1] = aiqResult->mPaResults.color_gains.gr;
    colorGains.color_gains_rggb[2] = aiqResult->mPaResults.color_gains.gb;
    colorGains.color_gains_rggb[3] = aiqResult->mPaResults.color_gains.b;
    param.setColorGains(colorGains);

    // Update AF result
    const cca::cca_af_results& afResult = aiqResult->mAfResults;

    LOG2("af status: %u, focus distance: %u mm", afResult.status, afResult.current_focus_distance);
    if (afResult.status <= ia_aiq_af_status_fail) {
        param.setAfState(static_cast<camera_af_state_t>(afResult.status));
    }
    param.setFocusDistance(static_cast<float>(afResult.current_focus_distance));

    // Update scene mode
    param.setSceneMode(aiqResult->mSceneMode);

    return OK;
}

void ParameterConvert::getCapabilityInfo(int cameraId, Parameters& param) {
    auto staticMetadata = PlatformData::getStaticMetadata(cameraId);
    CameraMetadata cameraMetadata;

    if (staticMetadata->mConfigsArray.size() > 0) {
        const stream_array_t& configsArray = staticMetadata->mConfigsArray;
        const int STREAM_MEMBER_NUM = sizeof(stream_t) / sizeof(int);
        const int dataSize = configsArray.size() * STREAM_MEMBER_NUM;
        int configs[dataSize];
        CLEAR(configs);
        for (size_t i = 0; i < configsArray.size(); i++) {
            LOG2("@%s, stream config info: format=%s (%dx%d) field=%d type=%d", __func__,
                 CameraUtils::format2string(configsArray[i].format).c_str(), configsArray[i].width,
                 configsArray[i].height, configsArray[i].field, configsArray[i].streamType);
            MEMCPY_S(&configs[i * STREAM_MEMBER_NUM], sizeof(stream_t), &configsArray[i],
                     sizeof(stream_t));
        }
        cameraMetadata.update(INTEL_INFO_AVAILABLE_CONFIGURATIONS, configs, dataSize);
    }
    if (staticMetadata->mFpsRange.size() > 0) {
        const std::vector<double>& rangeArray = staticMetadata->mFpsRange;
        float fpsRange[rangeArray.size()];
        CLEAR(fpsRange);
        for (size_t i = 0; i < rangeArray.size(); i++) {
            fpsRange[i] = static_cast<float>(rangeArray[i]);
        }
        LOG2("@%s, supported fps range size: %zu", __func__, rangeArray.size());
        cameraMetadata.update(CAMERA_AE_AVAILABLE_TARGET_FPS_RANGES, fpsRange, ARRAY_SIZE(fpsRange));
    }
    if (staticMetadata->mEvRange.size() > 0) {
        const std::vector<int>& rangeArray = staticMetadata->mEvRange;
        int evRange[rangeArray.size()];
        CLEAR(evRange);
        for (size_t i = 0; i < rangeArray.size(); i++) {
            evRange[i] = rangeArray[i];
        }
        LOG2("@%s, supported ev range size: %zu", __func__, rangeArray.size());
        cameraMetadata.update(CAMERA_AE_COMPENSATION_RANGE, evRange, ARRAY_SIZE(evRange));
    }
    if (staticMetadata->mEvStep.size() > 0) {
        const std::vector<int>& rationalType = staticMetadata->mEvStep;
        icamera_metadata_rational_t evStep = {rationalType[0], rationalType[1]};
        LOG2("@%s, the numerator: %d, denominator: %d", __func__, evStep.numerator,
             evStep.denominator);
        cameraMetadata.update(CAMERA_AE_COMPENSATION_STEP, &evStep, 1);
    }
    if (staticMetadata->mSupportedFeatures.size() > 0) {
        const camera_features_list_t& supportedFeatures = staticMetadata->mSupportedFeatures;
        const int numberOfFeatures = supportedFeatures.size();
        uint8_t features[numberOfFeatures];
        CLEAR(features);
        for (int i = 0; i < numberOfFeatures; i++) {
            features[i] = supportedFeatures[i];
        }
        cameraMetadata.update(INTEL_INFO_AVAILABLE_FEATURES, features, numberOfFeatures);
    }
    if (staticMetadata->mAeExposureTimeRange.size() > 0) {
        const std::vector<struct AeRange>& aeRange = staticMetadata->mAeExposureTimeRange;
        const int MEMBER_COUNT = 3;
        const int dataSize = aeRange.size() * MEMBER_COUNT;
        int rangeData[dataSize];
        CLEAR(rangeData);

        for (size_t i = 0; i < aeRange.size(); i++) {
            LOG2("@%s, scene mode:%d supported exposure time range (%f-%f)", __func__,
                 aeRange[i].scene, aeRange[i].minValue, aeRange[i].maxValue);
            rangeData[i * MEMBER_COUNT] = aeRange[i].scene;
            rangeData[i * MEMBER_COUNT + 1] = static_cast<int>(aeRange[i].minValue);
            rangeData[i * MEMBER_COUNT + 2] = static_cast<int>(aeRange[i].maxValue);
        }
        cameraMetadata.update(INTEL_INFO_AE_EXPOSURE_TIME_RANGE, rangeData, dataSize);
    }
    if (staticMetadata->mAeGainRange.size() > 0) {
        const std::vector<struct AeRange>& aeRange = staticMetadata->mAeGainRange;
        const int MEMBER_COUNT = 3;
        const int dataSize = aeRange.size() * MEMBER_COUNT;
        int rangeData[dataSize];
        CLEAR(rangeData);

        for (size_t i = 0; i < aeRange.size(); i++) {
            LOG2("@%s, scene mode:%d supported gain range (%f-%f)", __func__,
                 aeRange[i].scene, aeRange[i].minValue, aeRange[i].maxValue);
            rangeData[i * MEMBER_COUNT] = aeRange[i].scene;
            // Since we use int to store float, before storing it we multiply min and max by 100.
            rangeData[i * MEMBER_COUNT + 1] = static_cast<int>(aeRange[i].minValue * 100);
            rangeData[i * MEMBER_COUNT + 2] = static_cast<int>(aeRange[i].maxValue * 100);
        }
        cameraMetadata.update(INTEL_INFO_AE_GAIN_RANGE, rangeData, dataSize);
    }
    if (staticMetadata->mVideoStabilizationModes.size() > 0) {
        const camera_video_stabilization_list_t& supportedMode
            = staticMetadata->mVideoStabilizationModes;
        uint8_t modes[supportedMode.size()];
        CLEAR(modes);
        for (size_t i = 0; i < supportedMode.size(); i++) {
            modes[i] = supportedMode[i];
        }
        cameraMetadata.update(CAMERA_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES, modes,
                              supportedMode.size());
    }
    if (staticMetadata->mSupportedAeMode.size() > 0) {
        const std::vector<camera_ae_mode_t>& supportedAeMode = staticMetadata->mSupportedAeMode;
        uint8_t aeModes[supportedAeMode.size()];
        CLEAR(aeModes);
        for (size_t i = 0; i < supportedAeMode.size(); i++) {
            aeModes[i] = supportedAeMode[i];
        }
        cameraMetadata.update(CAMERA_AE_AVAILABLE_MODES, aeModes, supportedAeMode.size());
    }
    if (staticMetadata->mSupportedAwbMode.size() > 0 ) {
        const std::vector<camera_awb_mode_t>& supportedAwbMode = staticMetadata->mSupportedAwbMode;
        uint8_t awbModes[supportedAwbMode.size()];
        CLEAR(awbModes);
        for (size_t i = 0; i < supportedAwbMode.size(); i++) {
            awbModes[i] = supportedAwbMode[i];
        }
        cameraMetadata.update(CAMERA_AWB_AVAILABLE_MODES, awbModes, supportedAwbMode.size());
    }
    if (staticMetadata->mSupportedSceneMode.size() > 0) {
        const std::vector<camera_scene_mode_t>& supportedSceneMode
            = staticMetadata->mSupportedSceneMode;
        uint8_t sceneModes[supportedSceneMode.size()];
        CLEAR(sceneModes);
        for (size_t i = 0; i < supportedSceneMode.size(); i++) {
            sceneModes[i] = supportedSceneMode[i];
        }
        cameraMetadata.update(CAMERA_CONTROL_AVAILABLE_SCENE_MODES, sceneModes,
                         supportedSceneMode.size());
    }
    if (staticMetadata->mSupportedAfMode.size() > 0) {
        const std::vector<camera_af_mode_t>& supportedAfMode = staticMetadata->mSupportedAfMode;
        uint8_t afModes[supportedAfMode.size()];
        CLEAR(afModes);
        for (size_t i = 0; i < supportedAfMode.size(); i++) {
            afModes[i] = supportedAfMode[i];
        }
        cameraMetadata.update(CAMERA_AF_AVAILABLE_MODES, afModes, supportedAfMode.size());
    }
    if (staticMetadata->mSupportedAntibandingMode.size() > 0) {
        const std::vector<camera_antibanding_mode_t>& supportedAntibandingMode
            = staticMetadata->mSupportedAntibandingMode;
        uint8_t antibandingModes[supportedAntibandingMode.size()];
        CLEAR(antibandingModes);
        for (size_t i = 0; i < supportedAntibandingMode.size(); i++) {
            antibandingModes[i] = supportedAntibandingMode[i];
        }
        cameraMetadata.update(CAMERA_AE_AVAILABLE_ANTIBANDING_MODES, antibandingModes,
                         supportedAntibandingMode.size());
    }

    cameraMetadata.update(INTEL_INFO_SENSOR_MOUNT_TYPE, &staticMetadata->mMountType, 1);

    // Merge the content of cameraMetadata into param
    ParameterHelper::merge(cameraMetadata, &param);
}

}  /* icamera */
