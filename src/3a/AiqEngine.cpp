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

#define LOG_TAG AiqEngine

#include "AiqEngine.h"

#include <memory>

#include "FaceDetection.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "CameraContext.h"

namespace icamera {

AiqEngine::AiqEngine(int cameraId, SensorHwCtrl* sensorHw, LensHw* lensHw)
        : mCameraId(cameraId),
          mRun3ACadence(1),
          mFirstAiqRunning(true) {
    LOG1("<id%d>%s", mCameraId, __func__);

    mAiqRunningForPerframe = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);
    mAiqCore = new AiqCore(mCameraId);
    mSensorManager = new SensorManager(mCameraId, sensorHw);
    mLensManager = new LensManager(mCameraId, lensHw);

    // Should consider better place to maintain the life cycle of AiqResultStorage
    auto cameraContext = CameraContext::getInstance(mCameraId);
    mAiqResultStorage = cameraContext->getAiqResultStorage();

    CLEAR(mAiqRunningHistory);
}

AiqEngine::~AiqEngine() {
    LOG1("<id%d>%s", mCameraId, __func__);

    delete mLensManager;
    delete mSensorManager;
    delete mAiqCore;
}

void AiqEngine::init() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);

    mAiqCore->init();
    mSensorManager->reset();
}

void AiqEngine::deinit() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    mSensorManager->reset();
    mAiqCore->deinit();
}

void AiqEngine::reset() {
    LOG1("<id%d>%s", mCameraId, __func__);

    AutoMutex l(mEngineLock);
    mFirstAiqRunning = true;
    mAiqResultStorage->resetAiqStatistics();
    mSensorManager->reset();
    mLensManager->reset();
}

int AiqEngine::run3A(int64_t ccaId, int64_t applyingSeq, int64_t frameNumber, int64_t* effectSeq) {
    LOG2("<id%d:cca%ld:fn%ld>%s: applying seq %ld", mCameraId, ccaId, frameNumber,
         __func__, applyingSeq);

    auto cameraContext = CameraContext::getInstance(mCameraId);
    auto dataContext = cameraContext->acquireDataContextByFn(frameNumber);

    // Run 3A in call thread
    AutoMutex l(mEngineLock);

    AiqStatistics* aiqStats =
        mFirstAiqRunning ? nullptr
                         : const_cast<AiqStatistics*>(mAiqResultStorage->getAndLockAiqStatistics());
    AiqState state = AIQ_STATE_IDLE;
    AiqResult* aiqResult = mAiqResultStorage->acquireAiqResult();

    if (!needRun3A(aiqStats, ccaId)) {
        LOG2("%s: needRun3A is false, return AIQ_STATE_WAIT", __func__);
        state = AIQ_STATE_WAIT;
    } else {
        state = prepareInputParam(aiqStats, aiqResult, dataContext->mAiqParams);
        aiqResult->mTuningMode = dataContext->mAiqParams.tuningMode;
    }

    bool aiqRun = false;
    if (state == AIQ_STATE_RUN) {
        state = runAiq(ccaId, applyingSeq, aiqResult, &aiqRun);
    }
    if (state == AIQ_STATE_RESULT_SET) {
        state = handleAiqResult(dataContext->mAiqParams, aiqResult);
    }

    if (state == AIQ_STATE_DONE) {
        (void)done(dataContext->mAiqParams, aiqResult);
    }

    mAiqResultStorage->unLockAiqStatistics();

    if (aiqRun) {
        mAiqRunningHistory.aiqResult = aiqResult;
        mAiqRunningHistory.ccaId = ccaId;
        mAiqRunningHistory.statsSequence = aiqStats != nullptr ? aiqStats->mSequence : -1;
    }

    if (effectSeq != nullptr) {
        *effectSeq = mAiqResultStorage->getAiqResult()->mSequence;
        LOG2("%s, effect sequence %ld, statsSequence %ld", __func__, *effectSeq,
             mAiqRunningHistory.statsSequence);
    }

    PlatformData::saveMakernoteData(mCameraId, dataContext->mAiqParams.makernoteMode,
                                    mAiqResultStorage->getAiqResult()->mSequence,
                                    aiqResult->mTuningMode);

    return ((state == AIQ_STATE_DONE) || (state == AIQ_STATE_WAIT)) ? 0 : UNKNOWN_ERROR;
}

EventListener* AiqEngine::getSofEventListener() {
    AutoMutex l(mEngineLock);
    return this;
}

void AiqEngine::handleEvent(EventData eventData) {
    AutoMutex l(mEngineLock);
    mSensorManager->handleSofEvent(eventData);
    mLensManager->handleSofEvent(eventData);
}

int AiqEngine::prepareStatsParams(const aiq_parameter_t& aiqParams,
                                  cca::cca_stats_params* statsParams,
                                  const AiqStatistics* aiqStatistics, AiqResult* aiqResult) {
    LOG2("%s, sequence %ld", __func__, aiqStatistics->mSequence);

    // update face detection related parameters
    if (PlatformData::isFaceAeEnabled(mCameraId)) {
        FaceDetectionResult* faceResult = mAiqResultStorage->getFaceResult();
        if ((faceResult != nullptr) && (faceResult->ccaFaceState.num_faces > 0U)) {
            statsParams->faces = faceResult->ccaFaceState;
            ia_rectangle& rect = statsParams->faces.faces[0].face_area;
            LOG2("<seq:%ld>%s, face number:%d, left:%d, top:%d, right:%d, bottom:%d",
                 faceResult->sequence, __func__, faceResult->ccaFaceState.num_faces,
                 rect.left, rect.top, rect.right, rect.bottom);
            if(faceResult->ccaFaceState.updated) {
                faceResult->ccaFaceState.updated = false;
            }
        }
    }

    int ret = OK;
    do {
        // HDR_FEATURE_S
        // Run 3A without statistics when switching pipe.
        if ((aiqParams.tuningMode == TUNING_MODE_VIDEO_ULL) &&
            (aiqStatistics->mTuningMode == TUNING_MODE_VIDEO_HDR)) {
            LOG2("Switching from HDR to ULL pipe");
            ret = INVALID_OPERATION;
            break;
        } else if ((aiqParams.tuningMode == TUNING_MODE_VIDEO_HDR) &&
                   (aiqStatistics->mTuningMode == TUNING_MODE_VIDEO_ULL)) {
            LOG2("Switching from ULL to HDR pipe");
            ret = INVALID_OPERATION;
            break;
        }
        // HDR_FEATURE_E

        // The statistics timestamp is incorrect. If possible, use SOF timestamp instead.
        unsigned long long timestamp = mSensorManager->getSofTimestamp(aiqStatistics->mSequence);
        if (timestamp == 0U) {
            LOG2("<seq%ld>The sof sequence was not found", aiqStatistics->mSequence);
            timestamp = aiqStatistics->mTimestamp;
        }

        const AiqResult* aiqResult = mAiqResultStorage->getAiqResult(aiqStatistics->mSequence);

        std::shared_ptr<GraphConfig> gc = nullptr;
        CameraContext* context = CameraContext::getInstance(mCameraId);
        gc = context->getGraphConfig(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL);
        if (PlatformData::isDvsSupported(mCameraId) && (gc != nullptr)) {
            ia_isp_bxt_resolution_info_t resolution{};
            uint32_t gdcKernelId;
            const int status = gc->getGdcKernelSetting(&gdcKernelId, &resolution);
            CheckWarning(status != OK, UNKNOWN_ERROR, "Failed to get GDC kernel setting");

            statsParams->dvs_stats_height = resolution.output_height;
            statsParams->dvs_stats_width = resolution.output_width;
        }

        statsParams->frame_id = aiqResult != nullptr ? aiqResult->mFrameId : 0;
        statsParams->frame_timestamp = static_cast<uint32_t>(timestamp);
        statsParams->camera_orientation = ia_aiq_camera_orientation_unknown;
    } while (0);

    return ret;
}

void AiqEngine::setAiqResult(const aiq_parameter_t& aiqParams, AiqResult* aiqResult, bool skip) {
    aiqResult->mSkip = skip;
    if (skip) {
        LOG2("<seq%ld>%s, skipping the frame", aiqResult->mSequence, __func__);
    }

    // HDR_FEATURE_S
    const int64_t sequence = aiqResult->mSequence - PlatformData::getExposureLag(mCameraId);
    mSensorManager->setWdrMode(aiqResult->mTuningMode, sequence);

    if (PlatformData::getSensorAwbEnable(mCameraId)) {
        LOG2("%s, Set sensor awb %f %f", __func__, aiqResult->mAwbResults.accurate_r_per_g,
             aiqResult->mAwbResults.accurate_b_per_g);

        mSensorManager->setAWB(aiqResult->mAwbResults.accurate_r_per_g,
                               aiqResult->mAwbResults.accurate_b_per_g);
    }
    // HDR_FEATURE_E

    mLensManager->setLensResult(aiqResult->mAfResults, aiqResult->mSequence, aiqParams);
}

int AiqEngine::getSkippingNum(const AiqResult* aiqResult) {
    int skipNum = 0;

    if (!mFirstAiqRunning) {
        const AiqResult* lastResult = mAiqResultStorage->getAiqResult();
        if (lastResult->mTuningMode != aiqResult->mTuningMode) {
            // Skip 3 frames when pipe switching
            skipNum = 3;
        }
    } else if (mAiqRunningForPerframe) {
        // The 1st result takes effect @ frame (initialSkip) (applied before stream on)
        skipNum = PlatformData::getInitialSkipFrame(mCameraId);
    }

    return skipNum;
}

bool AiqEngine::needRun3A(const AiqStatistics* aiqStatistics, int64_t ccaId) {
    // Force to run 3a for per-frame control case
    if (mAiqRunningForPerframe) {
        return true;
    }

    // Force to run 3a for the first time running
    if (mFirstAiqRunning) {
        return true;
    }

    if (ccaId % mRun3ACadence != 0) {
        // Skip 3A per cadence
        return false;
    }

    if (aiqStatistics == nullptr) {
        LOG2("no stats and not need to re-run 3A");
        return false;
    }

    if (mAiqRunningHistory.statsSequence == aiqStatistics->mSequence) {
        LOG2("no new stats skip, statsSequence = %ld", aiqStatistics->mSequence);
        return false;
    } else if (mSensorManager->getCurrentExposureAppliedDelay() > kMaxExposureAppliedDelay) {
        LOG2("exposure setting applied delay is too larger, skip it");
        return false;
    }

    return true;
}

AiqEngine::AiqState AiqEngine::prepareInputParam(AiqStatistics* aiqStats, AiqResult* aiqResult,
                                                 aiq_parameter_t& aiqParams) {
    int ret = OK;
    mRun3ACadence = aiqParams.run3ACadence;

    // Update sensor info for the first-run of AIQ
    if (mFirstAiqRunning) {
        // CRL_MODULE_S
        mSensorManager->setFrameRate(aiqParams.fps);
        // CRL_MODULE_E
        // set sensor info if needed
        ia_aiq_exposure_sensor_descriptor sensorDescriptor = {};
        ia_aiq_frame_params frameParams = {};
        ret = mSensorManager->getSensorInfo(frameParams, sensorDescriptor);
        CheckAndLogError((ret != OK), AIQ_STATE_ERROR, "Get sensor info failed:%d", ret);
        mAiqCore->setSensorInfo(frameParams, sensorDescriptor);
    }

    // update lens related parameters
    mLensManager->getLensInfo(aiqParams);

    mAiqCore->updateParameter(aiqParams);

    if (aiqStats == nullptr) {
        LOG2("%s: run aiq without stats data", __func__);
        return AIQ_STATE_RUN;
    }

    // set Stats
    cca::cca_stats_params statsParams = {};
    ret = prepareStatsParams(aiqParams, &statsParams, aiqStats, aiqResult);
    if (ret != OK) {
        LOG2("%s: no useful stats", __func__);
        return AIQ_STATE_RUN;
    }

    if (PlatformData::getSensorAeEnable(mCameraId)) {
        LOG2("@%s, sensor ae is enabled", __func__);
        statsParams.using_rgbs_for_aec = true;
    }

    mAiqCore->setStatsParams(statsParams, aiqStats);

    return AIQ_STATE_RUN;
}

AiqEngine::AiqState AiqEngine::runAiq(int64_t ccaId, int64_t applyingSeq, AiqResult* aiqResult,
                                      bool* aiqRun) {
    if ((ccaId % PlatformData::getAiqRunningInterval(mCameraId) == 0) || mFirstAiqRunning) {
        int ret = mAiqCore->runAe(ccaId, aiqResult);
        if (ret != OK) {
            return AIQ_STATE_ERROR;
        }

        setSensorExposure(aiqResult, applyingSeq);

        ret = mAiqCore->runAiq(ccaId, aiqResult);
        if (ret != OK) {
            return AIQ_STATE_ERROR;
        }
        *aiqRun = true;
        aiqResult->mFrameId = ccaId;
    } else {
        *aiqResult = *(mAiqRunningHistory.aiqResult);
        setSensorExposure(aiqResult, applyingSeq);
    }

    return AIQ_STATE_RESULT_SET;
}

void AiqEngine::setSensorExposure(AiqResult* aiqResult, int64_t applyingSeq) {
    SensorExpGroup sensorExposures;
    for (unsigned int i = 0U; i < aiqResult->mAeResults.num_exposures; i++) {
        SensorExposure exposure;
        exposure.sensorParam = *aiqResult->mAeResults.exposures[i].sensor_exposure;
        exposure.realDigitalGain = aiqResult->mAeResults.exposures[i].exposure[0].digital_gain;
        sensorExposures.push_back(exposure);
    }
    aiqResult->mSequence = mSensorManager->updateSensorExposure(sensorExposures, applyingSeq);
}

AiqEngine::AiqState AiqEngine::handleAiqResult(const aiq_parameter_t& aiqParams,
                                               AiqResult* aiqResult) {
    LOG2("%s: aiqResult->mTuningMode = %d", __func__, aiqResult->mTuningMode);

    aiqResult->mSceneMode = SCENE_MODE_AUTO;
    /* Use direct AE result to update sceneMode to reflect the actual mode AE want to have,
     * Besides needed by full pipe auto-switch, this is also necessary when user want to
     * switch pipe in user app according to AE result.
     */
    if (aiqParams.sceneMode == SCENE_MODE_AUTO) {
        if (aiqResult->mAeResults.multiframe == ia_aiq_bracket_mode_hdr) {
            aiqResult->mSceneMode = SCENE_MODE_HDR;
        } else if (aiqResult->mAeResults.multiframe == ia_aiq_bracket_mode_ull) {
            aiqResult->mSceneMode = SCENE_MODE_ULL;
        }
    }
    LOG2("%s, sceneMode:%d", __func__, aiqResult->mSceneMode);

    (void)applyManualTonemaps(aiqParams, aiqResult);

    return AIQ_STATE_DONE;
}

int AiqEngine::applyManualTonemaps(const aiq_parameter_t& aiqParams, AiqResult* aiqResult) {
    aiqResult->mGbceResults.have_manual_settings = true;

    // Due to the tone map curve effect on image IQ, so need to apply
    // manual/fixed tone map table in manual tonemap or manual ISO/ET mode
    if ((aiqParams.tonemapMode == TONEMAP_MODE_FAST) ||
        (aiqParams.tonemapMode == TONEMAP_MODE_HIGH_QUALITY)) {
        aiqResult->mGbceResults.have_manual_settings = false;

        if ((aiqParams.aeMode != AE_MODE_AUTO) && (aiqParams.manualIso != 0)
            && (aiqParams.manualExpTimeUs != 0)) {
            aiqResult->mGbceResults.have_manual_settings = true;
        }
    }
    LOG2("%s, has manual setting: %d, aeMode: %d, tonemapMode: %d", __func__,
         aiqResult->mGbceResults.have_manual_settings, aiqParams.aeMode,
         aiqParams.tonemapMode);

    if (!aiqResult->mGbceResults.have_manual_settings) {
        return OK;
    }

    // Apply user value or gamma curve for gamma table
    if (aiqParams.tonemapMode == TONEMAP_MODE_GAMMA_VALUE) {
        AiqUtils::applyTonemapGamma(aiqParams.tonemapGamma, &aiqResult->mGbceResults);
    } else if (aiqParams.tonemapMode == TONEMAP_MODE_PRESET_CURVE) {
        if (aiqParams.tonemapPresetCurve == TONEMAP_PRESET_CURVE_SRGB) {
            AiqUtils::applyTonemapSRGB(&aiqResult->mGbceResults);
        } else if (aiqParams.tonemapPresetCurve == TONEMAP_PRESET_CURVE_REC709) {
            AiqUtils::applyTonemapREC709(&aiqResult->mGbceResults);
        }
    } else if (aiqParams.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        AiqUtils::applyTonemapCurve(aiqParams.tonemapCurves, &aiqResult->mGbceResults);
        AiqUtils::applyAwbGainForTonemapCurve(aiqParams.tonemapCurves,
                                              &aiqResult->mAwbResults);
    }

    // Apply the fixed unity value for tone map table
    if (aiqResult->mGbceResults.tone_map_lut_size > 0U) {
        for (unsigned int i = 0U; i < aiqResult->mGbceResults.tone_map_lut_size; i++) {
            aiqResult->mGbceResults.tone_map_lut[i] = 1.0;
        }
    }

    return OK;
}

AiqEngine::AiqState AiqEngine::done(const aiq_parameter_t& aiqParams, AiqResult* aiqResult) {
    const int skipNum = getSkippingNum(aiqResult);
    AiqResult* tmp = aiqResult;

    for (int i = 0; i < skipNum; i++) {
        // Increase the sensor settings sequence id, so for any frame that
        // its sequence id is bigger than the user expected id will be discarded.
        setAiqResult(aiqParams, tmp, true);
        mAiqResultStorage->updateAiqResult(tmp->mSequence);
        tmp = mAiqResultStorage->acquireAiqResult();
        *tmp = *aiqResult;
        setSensorExposure(tmp);
    }

    setAiqResult(aiqParams, tmp, false);
    mAiqResultStorage->updateAiqResult(tmp->mSequence);

    mFirstAiqRunning = false;
    return AIQ_STATE_WAIT;
}

} /* namespace icamera */
