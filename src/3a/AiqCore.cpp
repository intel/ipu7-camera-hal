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

#define LOG_TAG AiqCore

#include "AiqCore.h"

#include <math.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>

#include "AiqUtils.h"
#include "ParamDataType.h"
#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {
AiqCore::AiqCore(int cameraId)
        : mCameraId(cameraId),
          mTimestamp(0),
          mSensorPixelClock(0.0),
          mAeForceLock(false),
          mAwbForceLock(false),
          mAfForceLock(false),
          mAeRunTime(0),
          mAwbRunTime(0),
          mAiqRunTime(0),
          mHyperFocalDistance(0.0f),
          mTuningMode(TUNING_MODE_MAX),
          mShadingMode(SHADING_MODE_FAST),
          mLensShadingMapMode(LENS_SHADING_MAP_MODE_OFF),
          mLscGridRGGBLen(0),
          mLastEvShift(0.0f),
          mAeAndAwbConverged(false),
          mAeBypassed(false),
          mAfBypassed(false),
          mAwbBypassed(false),
          mLockedExposureTimeUs(0),
          mLockedIso(0) {
    mIntel3AParameter = std::unique_ptr<Intel3AParameter>(new Intel3AParameter(cameraId));

    CLEAR(mFrameParams);
    CLEAR(mLastAeResult);

    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mSaParams);
    CLEAR(mPaColorGains);

    CLEAR(mResizeLscGridR);
    CLEAR(mResizeLscGridGr);
    CLEAR(mResizeLscGridGb);
    CLEAR(mResizeLscGridB);

    CLEAR(mLensShadingMapSize);
    CLEAR(mLscGridRGGB);

    // init LscOffGrid to 1.0f
    std::fill(std::begin(mLscOffGrid), std::end(mLscOffGrid), 1.0F);

    mAiqParams = std::unique_ptr<cca::cca_aiq_params>(new cca::cca_aiq_params);
    mAiqResults = std::unique_ptr<cca::cca_aiq_results>(new cca::cca_aiq_results);

    memset(mAiqParams.get(), 0, sizeof(cca::cca_aiq_params));
    memset(mAiqResults.get(), 0, sizeof(cca::cca_aiq_results));
}

AiqCore::~AiqCore() {}

int AiqCore::initAiqPlusParams() {
    CLEAR(mGbceParams);
    CLEAR(mPaParams);
    CLEAR(mPaColorGains);
    CLEAR(mSaParams);

    /* use convergence time from tunings */
    mSaParams.manual_convergence_time = -1.0;

    int32_t tonemapMaxCurvePoints = 0;

    const std::string str = "tonemap.maxCurvePoints";
    const auto v = PlatformData::getInt32StaticMetadata(mCameraId, str);
    if (v.size() == 1) {
        tonemapMaxCurvePoints = v[0];
    }

    if ((tonemapMaxCurvePoints > 0) && (tonemapMaxCurvePoints < MIN_TONEMAP_POINTS)) {
        LOGW("%s: wrong tonemap points", __func__);
        tonemapMaxCurvePoints = 0;
    }
    mGbceParams.gbce_on = (tonemapMaxCurvePoints > 0) ? true : false;
    mGbceParams.athena_mode = PlatformData::getPLCEnable(mCameraId);
    LOG1("%s, gbce_on: %d, plc enable: %d", __func__, mGbceParams.gbce_on,
         mGbceParams.athena_mode);

    // HDR_FEATURE_S
    if (PlatformData::getSensorAeEnable(mCameraId)) {
        LOG2("@%s, enable_gtm_desaturation for HDR sensor", __func__);
        mPaParams.enable_gtm_desaturation = true;
    }
    // HDR_FEATURE_E

    return OK;
}

void AiqCore::init() {
    (void)initAiqPlusParams();

#ifndef IPA_SANDBOXING
    ia_env env = {&Log::ccaPrintInfo, &Log::ccaPrintError, &Log::ccaPrintInfo, nullptr};
    (void)ia_log_init(&env);
#endif

    mIntel3AParameter->init();

    CLEAR(mLastAeResult);
    mAeRunTime = 0U;
    mAwbRunTime = 0U;
    mAiqRunTime = 0U;
}

void AiqCore::deinit() {
#ifndef IPA_SANDBOXING
    ia_log_deinit();
#endif
}

int AiqCore::setSensorInfo(const ia_aiq_frame_params& frameParams,
                           const ia_aiq_exposure_sensor_descriptor& descriptor) {
    LOG2("@%s", __func__);

    mFrameParams = frameParams;
    mSensorPixelClock = descriptor.pixel_clock_freq_mhz;
    mIntel3AParameter->setSensorInfo(descriptor);

    return OK;
}

/**
 *
 * Calculate the Depth of field (DOF) for a given AF Result.
 *
 * The Formulas to calculate the near and afar DOF are:
 *          H * s
 * Dn = ---------------
 *         H + (s-f)
 *
 *          H * s
 * Df =  ------------
 *         H - (s-f)
 *
 * Where:
 * H is the hyperfocal distance (that we get from CPF) (it cannot be 0)
 * s is the distance to focused object (current focus distance)
 * f is the focal length
 *
 * \param[in] afResults with current focus distance in mm
 * \param[out] dof info: DOF for near and far limit in diopters
 */
int AiqCore::calculateDepthOfField(const cca::cca_af_results& afResults,
                                   camera_range_t* focusRange) {
    CheckAndLogError(focusRange == nullptr, BAD_VALUE, "@%s, Bad input values", __func__);

    const float DEFAULT_DOF = 5000.0F;
    focusRange->min = 1000.0F / DEFAULT_DOF;
    focusRange->max = 1000.0F / DEFAULT_DOF;

    const float focusDistance = 1.0F * afResults.current_focus_distance;
    if (focusDistance < std::numeric_limits<float>::epsilon()) {
        // Not reporting error since this may be normal in fixed focus sensors
        return OK;
    }

    // focal length is stored in CMC in hundreds of millimeters
    const float focalLengthMillis =
        static_cast<float>(mIntel3AParameter->mCMC.optics.effect_focal_length) / 100;

    const float num = mHyperFocalDistance * focusDistance;
    float denom = (mHyperFocalDistance + focusDistance - focalLengthMillis);
    if (std::fabs(denom) > std::numeric_limits<float>::epsilon()) {
        focusRange->min = num / denom;
    }

    denom = (mHyperFocalDistance - focusDistance + focalLengthMillis);
    if (std::fabs(denom) > std::numeric_limits<float>::epsilon()) {
        focusRange->max = num / denom;
    }

    focusRange->min = 1000.0F / focusRange->min;
    focusRange->max = 1000.0F / focusRange->max;

    return OK;
}

int AiqCore::updateParameter(const aiq_parameter_t& param) {
    if (mTuningMode != param.tuningMode) {
        int ret = mIntel3AParameter->getCMCInfo(param.tuningMode);
        CheckAndLogError(ret != OK, ret, "failed to get CMC info");

        mHyperFocalDistance = AiqUtils::calculateHyperfocalDistance(mIntel3AParameter->mCMC);
        mTuningMode = param.tuningMode;
    }
    mShadingMode = param.shadingMode;
    mLensShadingMapMode = param.lensShadingMapMode;
    mLensShadingMapSize = param.lensShadingMapSize;

    mGbceParams.ev_shift = param.evShift;

    // In still frame use force update by setting convergence time to 0.
    // in other cases use tunings.
    mSaParams.manual_convergence_time = (param.frameUsage == FRAME_USAGE_STILL) ? 0.0F : -1.0F;

    mIntel3AParameter->updateParameter(param);
    mAeForceLock = param.aeForceLock;
    mAwbForceLock = param.awbForceLock;
    mAfForceLock = mIntel3AParameter->mAfForceLock;

    // Enable AE/AWB/AF running rate if working in AUTO modes
    mAeBypassed = bypassAe(param);
    mAfBypassed = bypassAf(param);
    mAwbBypassed = bypassAwb(param);
    LOG2("Ae Bypass: %d, Af Bypass: %d, Awb Bypass: %d", mAeBypassed, mAfBypassed, mAwbBypassed);

    return OK;
}

int AiqCore::setStatsParams(const cca::cca_stats_params& statsParams, AiqStatistics* aiqStats) {
    LOG2("<aiq%lu>@%s, frame_timestamp:%lu, mTuningMode:%d", statsParams.frame_id, __func__,
         statsParams.frame_timestamp, mTuningMode);

    int ret = OK;
    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(intelCca == nullptr, UNKNOWN_ERROR, "%s, intelCca is nullptr, mode:%d",
                     __func__, mTuningMode);

    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->setStatsParams", 1U);
        const ia_err iaErr = intelCca->setStatsParams(statsParams);
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "setStatsParams fails, ret: %d", ret);
    }

    mTimestamp = statsParams.frame_timestamp;

    return ret;
}

int AiqCore::runAe(int64_t ccaId, AiqResult* aiqResult) {
    CheckAndLogError(aiqResult == nullptr, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);
    LOG2("<cca%ld>@%s, aiqResult %p", ccaId, __func__, aiqResult);

    // run AE
    return runAEC(ccaId, &aiqResult->mAeResults);
}

int AiqCore::runAiq(int64_t ccaId, AiqResult* aiqResult) {
    CheckAndLogError(aiqResult == nullptr, BAD_VALUE, "@%s, aiqResult is nullptr", __func__);

    uint32_t aaaRunType = ((static_cast<uint32_t>(IMAGING_ALGO_AWB) |
                           static_cast<uint32_t>(IMAGING_ALGO_GBCE)) |
                           static_cast<uint32_t>(IMAGING_ALGO_PA));
    if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
        aaaRunType |= static_cast<uint32_t>(IMAGING_ALGO_AF);
    }
    if (mShadingMode != SHADING_MODE_OFF) {
        aaaRunType |= static_cast<uint32_t>(IMAGING_ALGO_SA);
    }
    LOG2("<cca%ld>@%s, aiqResult %p, aaaRunType %x", ccaId, __func__, aiqResult, aaaRunType);

    // get the IntelCca instance
    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(intelCca == nullptr, UNKNOWN_ERROR, "%s, intelCca is null, mode:%d", __func__,
                     mTuningMode);

    mAiqParams->bitmap = 0U;

    // fill the parameter
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_AWB)) != 0U) {
        mIntel3AParameter->mAwbParams.is_bypass = mAwbBypassed;
        mAiqParams->awb_input = mIntel3AParameter->mAwbParams;
        LOG2("AWB bypass %d", mAiqParams->awb_input.is_bypass);
        mAiqParams->bitmap |= cca::CCA_MODULE_AWB;
    }

    if (((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_AF)) != 0U) && (!mAfBypassed)) {
        mAiqParams->bitmap |= cca::CCA_MODULE_AF;
        mAiqParams->af_input = mIntel3AParameter->mAfParams;
    }

    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_GBCE)) != 0U) {
        // run gbce with bypass level if AE lock
        if (mAeForceLock || (mIntel3AParameter->mTestPatternMode != TEST_PATTERN_OFF)) {
            mGbceParams.is_bypass = true;
        } else {
            mGbceParams.is_bypass = false;
        }
        mAiqParams->bitmap |= cca::CCA_MODULE_GBCE;
        mAiqParams->gbce_input = mGbceParams;
    }

    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_PA)) != 0U) {
        mPaParams.color_gains = {};
        mAiqParams->bitmap |= cca::CCA_MODULE_PA;
        mAiqParams->pa_input = mPaParams;
    }

    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_SA)) != 0U) {
        mAiqParams->bitmap |= cca::CCA_MODULE_SA;
        mSaParams.lsc_on = mLensShadingMapMode == LENS_SHADING_MAP_MODE_ON ? true : false;
        mAiqParams->sa_input = mSaParams;
    }
    LOG2("bitmap:%d, mAiqRunTime:%lu", mAiqParams->bitmap, mAiqRunTime);

    // runAIQ for awb/af/gbce/pa/sa
    int ret = OK;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelAiq->runAIQ", 1U);

        ia_err iaErr = intelCca->runAIQ(ccaId, *mAiqParams.get(), mAiqResults.get());
        mAiqRunTime++;
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "@%s, runAIQ, ret: %d", __func__, ret);
    }

    // handle awb result
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_AWB)) != 0U) {
        cca::cca_awb_results* newAwbResults = &mAiqResults->awb_output;

        if (!PlatformData::isIsysEnabled(mCameraId)) {
            // Fix AWB gain to 1 for none-ISYS cases
            newAwbResults->accurate_r_per_g = 1.0;
            newAwbResults->accurate_b_per_g = 1.0;
        }

        mIntel3AParameter->updateAwbResult(newAwbResults);
        aiqResult->mAwbResults = *newAwbResults;
        AiqUtils::dumpAwbResults(aiqResult->mAwbResults);
        ++mAwbRunTime;
    }

    // handle af result
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_AF)) != 0U) {
        focusDistanceResult(&mAiqResults->af_output, &aiqResult->mAfDistanceDiopters,
                            &aiqResult->mFocusRange);
        aiqResult->mAfResults = mAiqResults->af_output;
        AiqUtils::dumpAfResults(aiqResult->mAfResults);

        aiqResult->mLensPosition = mIntel3AParameter->mAfParams.lens_position;
        mIntel3AParameter->fillAfTriggerResult(&aiqResult->mAfResults);
    }

    // handle gbce result
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_GBCE)) != 0U) {
        aiqResult->mGbceResults = mAiqResults->gbce_output;
        AiqUtils::dumpGbceResults(aiqResult->mGbceResults);
    }

    // handle pa result
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_PA)) != 0U) {
        mIntel3AParameter->updatePaResult(&mAiqResults->pa_output);
        aiqResult->mPaResults = mAiqResults->pa_output;
        AiqUtils::dumpPaResults(aiqResult->mPaResults);
    }

    // handle sa result
    if ((aaaRunType & static_cast<uint32_t>(IMAGING_ALGO_SA)) != 0U) {
        AiqUtils::dumpSaResults(mAiqResults->sa_output);
        ret = processSAResults(&mAiqResults->sa_output, aiqResult->mLensShadingMap);
    }
    CheckAndLogError(ret != OK, ret, "run3A failed, ret: %d", ret);

    const uint16_t pixelInLine =
            aiqResult->mAeResults.exposures[0].sensor_exposure->line_length_pixels;
    const uint16_t lineInFrame =
            aiqResult->mAeResults.exposures[0].sensor_exposure->frame_length_lines;
    aiqResult->mFrameDuration = pixelInLine * lineInFrame / mSensorPixelClock;
    aiqResult->mRollingShutter =
        pixelInLine * (mFrameParams.cropped_image_height - 1) / mSensorPixelClock;

    mLastEvShift = mIntel3AParameter->mAeParams.ev_shift;
    aiqResult->mTimestamp = mTimestamp;

    if (PlatformData::isStatsRunningRateSupport(mCameraId)) {
        const bool bothConverged = ((mLastAeResult.exposures[0].converged) &&
                              (mAiqResults->awb_output.distance_from_convergence < EPSILON));
        if (!mAeAndAwbConverged && bothConverged) {
            mAeRunRateInfo.reset();
            mAwbRunRateInfo.reset();
        }
        mAeAndAwbConverged = bothConverged;
    }

    return OK;
}

int AiqCore::runAEC(int64_t ccaId, cca::cca_ae_results* aeResults) {
    PERF_CAMERA_ATRACE();

    int ret = OK;
    cca::cca_ae_results* newAeResults = &mLastAeResult;

    // Run AEC with setting bypass mode to false
    mIntel3AParameter->mAeParams.is_bypass = mAeBypassed;

    if (mAeForceLock && (mIntel3AParameter->mAeMode != AE_MODE_MANUAL) && (mAeRunTime != 0U) &&
        (!mIntel3AParameter->mAeParams.is_bypass)) {
        // Use manual settings if AE had been locked
        mIntel3AParameter->mAeParams.manual_exposure_time_us[0] = mLockedExposureTimeUs;
        mIntel3AParameter->mAeParams.manual_iso[0] = mLockedIso;
    }

    IntelCca* intelCca = getIntelCca(mTuningMode);
    CheckAndLogError(intelCca == nullptr, UNKNOWN_ERROR, "%s, intelCca is null, m:%d",
                     __func__, mTuningMode);
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->runAEC", 1U);
        const ia_err iaErr = intelCca->runAEC(ccaId, mIntel3AParameter->mAeParams, newAeResults);
        ret = AiqUtils::convertError(iaErr);
        CheckAndLogError(ret != OK, ret, "Error running AE, ret: %d", ret);
    }

    if (!mAeForceLock) {
        // Save exposure results if unlocked
        mLockedExposureTimeUs = newAeResults->exposures[0].exposure[0].exposure_time_us;
        mLockedIso = newAeResults->exposures[0].exposure[0].iso;
    }

    mIntel3AParameter->updateAeResult(newAeResults);
    *aeResults = *newAeResults;
    AiqUtils::dumpAeResults(*aeResults);
    ++mAeRunTime;

    return ret;
}

void AiqCore::focusDistanceResult(const cca::cca_af_results* afResults, float* afDistanceDiopters,
                                  camera_range_t* focusRange) {
    LOG2("@%s, afResults:%p, afDistanceDiopters:%p, focusRange:%p", __func__, afResults,
         afDistanceDiopters, focusRange);

    *afDistanceDiopters = 1.2F;
    if (mIntel3AParameter->mAfParams.focus_mode == ia_aiq_af_operation_mode_infinity) {
        // infinity mode is special: we need to report 0.0f (1/inf = 0)
        *afDistanceDiopters = 0.0F;
    } else if ((mIntel3AParameter->mAfParams.focus_mode == ia_aiq_af_operation_mode_manual) &&
               (mIntel3AParameter->mAfParams.manual_focus_parameters.manual_focus_action ==
                ia_aiq_manual_focus_action_set_distance) &&
               (mIntel3AParameter->mAfParams.manual_focus_parameters.manual_focus_distance ==
                afResults->current_focus_distance)) {
        // Don't need to calculate diopter in manual mode
        *afDistanceDiopters = mIntel3AParameter->mManualFocusDistance;
    } else if (afResults->next_focus_distance != 0) {
        // In AIQ, 'next_focus_distance' is in millimeters
        // For rounding multiply by extra 100.
        // This allows the diopters to have 2 decimal values
        *afDistanceDiopters = 100.0 * 1000.0 *
            (1.0 / static_cast<float>(afResults->next_focus_distance));
        *afDistanceDiopters = static_cast<float>(ceil(*afDistanceDiopters));
        // Divide by 100 for final result.
        *afDistanceDiopters = *afDistanceDiopters / 100;
    }

    (void)calculateDepthOfField(*afResults, focusRange);
}

int AiqCore::checkColorOrder(cmc_bayer_order bayerOrder, ColorOrder* colorOrder) {
    LOG2("@%s, bayerOrder = %d, colorOrder:%p", __func__, bayerOrder, colorOrder);

    int ret = OK;
    switch (bayerOrder) {
        case cmc_bayer_order_grbg:
            /* use gr r b gb constitute 2X2 array
             * gr    r
             * b     gb
             * The four channel use x y coordinate to indicate
             * gr(0, 0) r(1, 0) b(0, 1) gb(1, 1)
             */
            colorOrder->r[0] = 1U;
            colorOrder->r[1] = 0U;
            colorOrder->b[0] = 0U;
            colorOrder->b[1] = 1U;
            colorOrder->gr[0] = 0U;
            colorOrder->gr[1] = 0U;
            colorOrder->gb[0] = 1U;
            colorOrder->gb[1] = 1U;
            break;
        case cmc_bayer_order_rggb:
            colorOrder->r[0] = 0U;
            colorOrder->r[1] = 0U;
            colorOrder->b[0] = 1U;
            colorOrder->b[1] = 1U;
            colorOrder->gr[0] = 1U;
            colorOrder->gr[1] = 0U;
            colorOrder->gb[0] = 0U;
            colorOrder->gb[1] = 1U;
            break;
        case cmc_bayer_order_bggr:
            colorOrder->r[0] = 1U;
            colorOrder->r[1] = 1U;
            colorOrder->b[0] = 0U;
            colorOrder->b[1] = 0U;
            colorOrder->gr[0] = 0U;
            colorOrder->gr[1] = 1U;
            colorOrder->gb[0] = 1U;
            colorOrder->gb[1] = 0U;
            break;
        case cmc_bayer_order_gbrg:
            colorOrder->r[0] = 0U;
            colorOrder->r[1] = 1U;
            colorOrder->b[0] = 1U;
            colorOrder->b[1] = 0U;
            colorOrder->gr[0] = 1U;
            colorOrder->gr[1] = 1U;
            colorOrder->gb[0] = 0U;
            colorOrder->gb[1] = 0U;
            break;
        default:
            ret = BAD_VALUE;
            break;
    }
    return ret;
}

int AiqCore::reFormatLensShadingMap(const LSCGrid& inputLscGrid, float* dstLscGridRGGB) {
    CheckAndLogError(inputLscGrid.isBad() || (dstLscGridRGGB == nullptr), BAD_VALUE,
                     "@%s, Bad input values for lens shading map reformatting", __func__);
    LOG2("@%s, width %d, height %d", __func__, inputLscGrid.width, inputLscGrid.height);

    // Metadata spec request order [R, Geven, Godd, B]
    // the lensShading from ISP is 4 width * height block,
    // for ia_aiq_bayer_order_grbg, the four block is G, R, B, G
    const size_t size = inputLscGrid.height * inputLscGrid.width;
    for (size_t i = 0U; i < size; i++) {
        *dstLscGridRGGB = inputLscGrid.gridR[i];
        dstLscGridRGGB++;
        *dstLscGridRGGB = inputLscGrid.gridGr[i];
        dstLscGridRGGB++;
        *dstLscGridRGGB = inputLscGrid.gridGb[i];
        dstLscGridRGGB++;
        *dstLscGridRGGB = inputLscGrid.gridB[i];
        dstLscGridRGGB++;
    }

    return OK;
}

int AiqCore::storeLensShadingMap(const LSCGrid& inputLscGrid, const LSCGrid& resizeLscGrid,
                                 float* dstLscGridRGGB) {
    CheckAndLogError(inputLscGrid.isBad() || resizeLscGrid.isBad() ||
                     (dstLscGridRGGB == nullptr), BAD_VALUE,
                     "@%s, Bad input values for lens shading map storing", __func__);

    const int destWidth = resizeLscGrid.width;
    const int destHeight = resizeLscGrid.height;
    const int width = inputLscGrid.width;
    const int height = inputLscGrid.height;

    if ((width != destWidth) || (height != destHeight)) {
        // requests lensShadingMapSize must be smaller than 64*64
        // and it is a constant size.
        // Our lensShadingMapSize is dynamic based on the resolution, so need
        // to do resize for 4 channels separately

        nsecs_t startTime = CameraUtils::systemTime();
        AiqUtils::resize2dArray(inputLscGrid.gridR, width, height, resizeLscGrid.gridR, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGr, width, height, resizeLscGrid.gridGr, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridGb, width, height, resizeLscGrid.gridGb, destWidth,
                                destHeight);
        AiqUtils::resize2dArray(inputLscGrid.gridB, width, height, resizeLscGrid.gridB, destWidth,
                                destHeight);
        LOG2("resize the 2D array cost %dus",
             static_cast<unsigned>(((CameraUtils::systemTime() - startTime) / 1000U)));

        LOG2("%s:resize lens shading map from [%d,%d] to [%d,%d]", __func__, width, height,
             destWidth, destHeight);
    } else {
        const size_t size = destWidth * destHeight * sizeof(resizeLscGrid.gridR[0]);
        STDCOPY((int8_t*)resizeLscGrid.gridR, (int8_t*)inputLscGrid.gridR, size);
        STDCOPY((int8_t*)resizeLscGrid.gridGr, (int8_t*)inputLscGrid.gridGr, size);
        STDCOPY((int8_t*)resizeLscGrid.gridGb, (int8_t*)inputLscGrid.gridGb, size);
        STDCOPY((int8_t*)resizeLscGrid.gridB, (int8_t*)inputLscGrid.gridB, size);
    }

    return reFormatLensShadingMap(resizeLscGrid, dstLscGridRGGB);
}

int AiqCore::processSAResults(cca::cca_sa_results* saResults, float* lensShadingMap) {
    CheckAndLogError((saResults == nullptr) || (lensShadingMap == nullptr), BAD_VALUE,
                     "@%s, Bad input values, saResults %p, lensShadingMap %p",
                     __func__, saResults, lensShadingMap);
    LOG2("@%s, mLensShadingMapMode %d", __func__, mLensShadingMapMode);

    if (mLensShadingMapMode == LENS_SHADING_MAP_MODE_OFF) {
        return OK;
    }

    if (saResults->lsc_update) {
        ColorOrder co_ind = {};
        int ret = checkColorOrder(saResults->color_order, &co_ind);
        CheckAndLogError(ret != OK, BAD_VALUE, "Failed to checkColorOrder, ret: %d", ret);

        LSCGrid inputGrid;
        inputGrid.gridB = saResults->lsc_grid[co_ind.b[0]][co_ind.b[1]];
        inputGrid.gridR = saResults->lsc_grid[co_ind.r[0]][co_ind.r[1]];
        inputGrid.gridGr = saResults->lsc_grid[co_ind.gr[0]][co_ind.gr[1]];
        inputGrid.gridGb = saResults->lsc_grid[co_ind.gb[0]][co_ind.gb[1]];
        inputGrid.width = saResults->width;
        inputGrid.height = saResults->height;

        LSCGrid resizeGrid;
        resizeGrid.gridB = mResizeLscGridB;
        resizeGrid.gridR = mResizeLscGridR;
        resizeGrid.gridGr = mResizeLscGridGr;
        resizeGrid.gridGb = mResizeLscGridGb;
        resizeGrid.width = mLensShadingMapSize.x;
        resizeGrid.height = mLensShadingMapSize.y;

        (void)storeLensShadingMap(inputGrid, resizeGrid, mLscGridRGGB);

        // resizeGrid's width and height should be equal to inputGrid's width and height
        mLscGridRGGBLen = static_cast<uint64_t>(resizeGrid.width) *
                          static_cast<uint64_t>(resizeGrid.height) * 4U;
        size_t errCount = 0U;
        for (size_t i = 0U; i < mLscGridRGGBLen; i++) {
            if (mLscGridRGGB[i] < 1.0F) {
                mLscGridRGGB[i] = 1.0F;
                errCount++;
            }
        }
        if (errCount != 0U) {
            LOGW("Error - SA produced too small values (%zu/%zu)!", errCount, mLscGridRGGBLen);
        }
    }

    float* lsm = (mShadingMode != SHADING_MODE_OFF) ? mLscGridRGGB : mLscOffGrid;
    for (size_t i = 0U; i < mLscGridRGGBLen; i++) {
        lensShadingMap[i] = lsm[i];
    }

    return OK;
}

bool AiqCore::bypassAe(const aiq_parameter_t& param) {
    const float evShiftDelta = std::fabs(mIntel3AParameter->mAeParams.ev_shift - mLastEvShift);
    if ((mAeRunTime == 0U) || (evShiftDelta < std::numeric_limits<float>::epsilon())) {
        return false;
    }

    if (mAeForceLock || (mAeRunTime % mIntel3AParameter->mAePerTicks != 0)) {
        return true;
    }

    // run AE if manual AE or total exposure target is set
    if ((param.aeMode != AE_MODE_AUTO) || (param.powerMode != CAMERA_LOW_POWER) ||
        (param.totalExposureTarget > 0)) {
            return false;
    }

    const bool converged = mLastAeResult.exposures[0].converged;

    return skipAlgoRunning(&mAeRunRateInfo, static_cast<int32_t>(IMAGING_ALGO_AE), converged);
}

bool AiqCore::bypassAf(const aiq_parameter_t& param) {
    if (mAfForceLock) {
        return true;
    }

    if ((param.afMode == AF_MODE_OFF) || (param.powerMode != CAMERA_LOW_POWER)) {
        return false;
    }

    const bool converged = (mAiqResults->af_output.status == ia_aiq_af_status_success) &&
                     (mAiqResults->af_output.final_lens_position_reached);

    return skipAlgoRunning(&mAfRunRateInfo, static_cast<int32_t>(IMAGING_ALGO_AF), converged);
}

bool AiqCore::bypassAwb(const aiq_parameter_t& param) {
    if (mAwbForceLock || (mAwbRunTime % mIntel3AParameter->mAwbPerTicks != 0) ||
        (mIntel3AParameter->mTestPatternMode != TEST_PATTERN_OFF)) {
        return true;
    }

    if ((param.awbMode != AWB_MODE_AUTO) || (param.powerMode != CAMERA_LOW_POWER)) {
        return false;
    }

    const bool converged = mAiqResults->awb_output.distance_from_convergence < EPSILON;

    return skipAlgoRunning(&mAwbRunRateInfo, static_cast<int32_t>(IMAGING_ALGO_AWB), converged);
}

bool AiqCore::skipAlgoRunning(RunRateInfo* info, int algo, bool converged) {
    const float configRunningRate = PlatformData::getAlgoRunningRate(algo, mCameraId);
    if (configRunningRate < EPSILON) {
        return false;
    }
    LOG2("the running rate of type %d is %f", algo, configRunningRate);

    // if converged, skip algo if running rate is reached.
    if (converged) {
        info->runCcaTime++;
        if (checkRunRate(configRunningRate, info)) {
            return true;
        }

        info->runAlgoTime++;
    } else {
        info->runCcaTime = 0;
        info->runAlgoTime = 0;
    }

    return false;
}

bool AiqCore::checkRunRate(float configRunningRate, const RunRateInfo* info) {
    LOG2("configRunningRate %f, runAlgoTime %d, runCcaTime %d", configRunningRate,
         info->runAlgoTime, info->runCcaTime);
    if (info->runCcaTime == 0) {
        return true;
    }

    const float algoRunningRate = static_cast<float>(info->runAlgoTime) / info->runCcaTime;
    if ((algoRunningRate - configRunningRate) > EPSILON) {
        return true;
    }

    return false;
}

IntelCca* AiqCore::getIntelCca(TuningMode tuningMode) {
    CheckAndLogError(tuningMode >= TUNING_MODE_MAX, nullptr, "@%s, wrong tuningMode:%d", __func__,
                     tuningMode);

    return IntelCca::getInstance(mCameraId, tuningMode);
}

} /* namespace icamera */
