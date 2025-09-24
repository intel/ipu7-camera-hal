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

#define LOG_TAG IpuPacAdaptor

#include "IpuPacAdaptor.h"

#include <math.h>
#include <stdio.h>
#include <utility>
#include <memory>
#include <limits>

#include "iutils/Utils.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "CameraContext.h"
#include "AiqResultStorage.h"
#include "PlatformData.h"
#include "ia_pal_types_isp_ids_autogen.h"

namespace icamera {

IpuPacAdaptor::IpuPacAdaptor(int cameraId) :
        mPacAdaptorState(PAC_ADAPTOR_NOT_INIT),
        mCameraId(cameraId),
        mIntelCca(nullptr),
        mAiqResultStorage(nullptr),
        mLastStatsSequence(-1) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    auto cameraContext = CameraContext::getInstance(mCameraId);
    mAiqResultStorage = cameraContext->getAiqResultStorage();
}

int IpuPacAdaptor::init(std::vector<int> streamIds) {
    LOG1("<id%d>@%s", mCameraId, __func__);
    PERF_CAMERA_ATRACE();
    AutoMutex l(mPacAdaptorLock);

    mIntelCca = IntelCca::getInstance(mCameraId, TUNING_MODE_VIDEO);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    mStreamIdToInputParams.clear();
    for (const auto& id : streamIds) {
        cca::cca_pal_input_params* p = static_cast<cca::cca_pal_input_params*>(
                mIntelCca->allocMem(id, "param", id, sizeof(cca::cca_pal_input_params)));
        CheckAndLogError(p == nullptr, NO_MEMORY, "Cannot alloc memory for input parameter!");
        CLEAR(*p);
        mStreamIdToInputParams[id] = p;
    }

    {
        AutoMutex l(mIpuParamLock);
        mTerminalResult.clear();
        mTerminalData.clear();
    }

    mPacAdaptorState = PAC_ADAPTOR_INIT;
    return OK;
}

int IpuPacAdaptor::reinitAic(const int32_t aicId) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);

    const ia_err iaErr = mIntelCca->reinitAic(aicId);
    CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                     "%s, Failed to reinit aic, aicId: %d", __func__, aicId);

    return OK;
}

int IpuPacAdaptor::deinit() {
    LOG1("<id%d>@%s", mCameraId, __func__);

    AutoMutex l(mPacAdaptorLock);
    for (auto& it : mStreamIdToInputParams) {
        mIntelCca->freeMem(it.second);
    }
    mStreamIdToInputParams.clear();

    {
        AutoMutex l(mIpuParamLock);
        mTerminalResult.clear();
        mTerminalData.clear();
    }

    mPacAdaptorState = PAC_ADAPTOR_NOT_INIT;
    return OK;
}

status_t IpuPacAdaptor::pacConfig(int streamId, const cca::cca_aic_config& aicConfig,
                                  const cca::cca_aic_kernel_offset& kernelOffset,
                                  uint32_t* offsetPtr, cca::cca_aic_terminal_config* termCfg,
                                  const int32_t* statsBufToTermIds) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     INVALID_OPERATION, "%s, wrong state %d", __func__, mPacAdaptorState);

    LOG2("@%s, cb number: %u, streamId: %d", __func__, termCfg->cb_num, streamId);
    for (uint32_t i = 0U; i < termCfg->cb_num; ++i) {
        LOG2("%s, config pac for cb: %d", __func__, termCfg->cb_terminal_buf[i].group_id);
    }

    const ia_err iaErr = mIntelCca->configAic(aicConfig, kernelOffset, offsetPtr, *termCfg,
                                              streamId, statsBufToTermIds);
    CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                     "%s, Failed to configure pac, streamId: %d", __func__, streamId);

    return OK;
}

void IpuPacAdaptor::clearAicResult() {
    AutoMutex l(mPacAdaptorLock);
    mPacRunHistMap.clear();
}

void* IpuPacAdaptor::allocateBuffer(int streamId, uint8_t contextId,
                                    uint32_t termId, size_t size) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     nullptr, "%s, wrong state %d", __func__, mPacAdaptorState);
    CheckAndLogError(mIntelCca == nullptr, nullptr, "%s, mIntelCca is nullptr", __func__);

    return allocateBufferL(streamId, contextId, termId, size);
}

void* IpuPacAdaptor::allocateBufferL(int streamId, uint8_t contextId,
                                     uint32_t termId, size_t size) {
    LOG1("<id:%d>@%s, streamId: %d, ctxId: %u, termId: %u, size: %zu",
         mCameraId, __func__, streamId, contextId, termId, size);
    std::string name = std::string("termBuf") + std::to_string(memIndex);
    memIndex++;

    void* addr = mIntelCca->allocMem(streamId, name, termId, size);
    CheckAndLogError(addr == nullptr, nullptr,
                     "%s, Failed to allocate terminal buffer. termId: %d", __func__, termId);

    return addr;
}

void IpuPacAdaptor::releaseBuffer(int streamId, uint8_t contextId, uint32_t termId, void* addr) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     VOID_VALUE, "%s, wrong state %d", __func__, mPacAdaptorState);
    CheckAndLogError(mIntelCca == nullptr, VOID_VALUE, "%s, mIntelCca is nullptr", __func__);

    releaseBufferL(streamId, contextId, termId, addr);
}

void IpuPacAdaptor::releaseBufferL(int streamId, uint8_t contextId, uint32_t termId, void* addr) {
    LOG1("<id:%d>@%s, streamId: %d, ctxId: %u, termId: %u, addr: %p",
         mCameraId, __func__, streamId, contextId, termId, addr);

    mIntelCca->freeMem(addr);
}

status_t IpuPacAdaptor::setPacTerminalData(int streamId, uint8_t contextId,
                                       const PacTerminalBufMap& bufferMap) {
    LOG1("<id:%d>@%s, streamId: %d, ctxId: %u, terminal size: %zu",
         mCameraId, __func__, streamId, contextId, bufferMap.size());

    AutoMutex paramLock(mIpuParamLock);
    // Auto create one empty instance if not exist
    std::pair<int, uint8_t> cbInstance = std::make_pair(streamId, contextId);
    mTerminalData[cbInstance] = bufferMap;

    return OK;
}

status_t IpuPacAdaptor::registerBuffer(int streamId, const cca::cca_aic_terminal_config& termCfg) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     INVALID_OPERATION, "%s, wrong state %d", __func__, mPacAdaptorState);

    LOG2("@%s, cb number: %u, streamId: %d", __func__, termCfg.cb_num, streamId);
    for (uint32_t i = 0U; i < termCfg.cb_num; ++i) {
        LOG2("%s, register buffer for cb: %d", __func__, termCfg.cb_terminal_buf[i].group_id);
    }

    const ia_err iaErr = mIntelCca->registerAicBuf(termCfg, streamId);
    CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                     "%s, Failed to register pac buffer, streamId: %d", __func__, streamId);

    return OK;
}

status_t IpuPacAdaptor::storeTerminalResult(int64_t sequence, int32_t streamId) {
    AutoMutex l(mIpuParamLock);

    // get the aic buffer of all CBs for current streamId
    for (auto& data : mTerminalData) {
        if (data.first.first != streamId) {
            continue;
        }

        // According to the PacTerminalBufMap of streamId and contextId to get aic buffer
        PacTerminalBufMap& cbTermData = data.second;
        cca::cca_aic_terminal_config ccaTermConfig = {};
        ccaTermConfig.cb_num = 1U;
        cca::cca_cb_termal_buf* ccaTermBufs = &(ccaTermConfig.cb_terminal_buf[0]);
        ccaTermBufs->group_id = data.first.second;
        ccaTermBufs->num_terminal = cbTermData.size();
        CheckAndLogError(cbTermData.size() > cca::MAX_PG_TERMINAL_NUM, UNKNOWN_ERROR,
                         "%s, there are too many terminals for streamId: %d, contextId: %u",
                         __func__, streamId, data.first.second);

        int index = 0;
        aic::IaAicBuffer payloadBufs[cca::MAX_PG_TERMINAL_NUM] = {};
        for (auto& buf : cbTermData) {
            ccaTermBufs->terminal_buf[index].terminal_index = buf.first;
            ccaTermBufs->terminal_buf[index].buf_size = buf.second.size;
            ccaTermBufs->terminal_buf[index].payload = &payloadBufs[index];
            ++index;
        }

        LOG2("%s, get the aic buffer for streamId: %d, contextId: %u, terminal num: %d",
             __func__, streamId, data.first.second, index);
        const ia_err iaErr = mIntelCca->getAicBuf(ccaTermConfig, streamId);
        CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                         "<seq:%ld>%s, Failed to getAicBuf. streamId: %d, contextId: %d",
                         sequence, __func__, streamId, data.first.second);

        index = 0;
        std::pair<int, uint8_t> cbInstance = std::make_pair(streamId, data.first.second);
        for (auto& buf : cbTermData) {
            int aicDataId = payloadBufs[index].id;
            const size_t aicDataSize = payloadBufs[index].size;
            void* aicDataAddr = payloadBufs[index].payloadPtr;
            const int64_t aicDataSeq = payloadBufs[index].sequence;
            LOG3("%s, terminal id: %u, id: %u, size: %zu, addr: %p, sequence: %ld", __func__,
                 buf.first, aicDataId, aicDataSize, aicDataAddr, aicDataSeq);

            buf.second.payloadPtr = payloadBufs[index].payloadPtr;
            ++index;
        }

        // store terminal buffers with sequence id into mTerminalResult
        std::vector<CBTerminalResult>& cbTermResult = mTerminalResult[cbInstance];
        if (cbTermResult.size() > MAX_SETTING_COUNT) {
            cbTermResult.erase(cbTermResult.begin());
        }

        // Only update data if find same sequence id
        size_t i = 0U;
        for ( ; i < cbTermResult.size(); ++i) {
            if (cbTermResult[i].sequence == sequence) {
                break;
            }
        }

        if (i != cbTermResult.size()) {
            cbTermResult[i].termResult = cbTermData;
        } else {
            CBTerminalResult result = {sequence, cbTermData};
            cbTermResult.push_back(result);
        }
    }

    return OK;
}

void IpuPacAdaptor::applyMediaFormat(const AiqResult* aiqResult,
                                     ia_media_format* mediaFormat, bool* useLinearGamma,
                                     int64_t sequence) {
    CheckAndLogError((mediaFormat == nullptr) || (aiqResult == nullptr), VOID_VALUE,
                     "mediaFormat or aiqResult is nullptr");

    auto cameraContext = CameraContext::getInstance(mCameraId);
    auto dataContext = cameraContext->getDataContextBySeq(sequence);

    *mediaFormat = media_format_legacy;
    if (dataContext->mAiqParams.tonemapMode == TONEMAP_MODE_GAMMA_VALUE) {
        const float gamma = dataContext->mAiqParams.tonemapGamma;
        if (std::fabs(gamma - 1.0F) < std::numeric_limits<float>::epsilon()) {
            *useLinearGamma = true;
            *mediaFormat = media_format_custom;
            LOG2("%s: a linear 1.0 gamma value.", __func__);
        } else {
            LOGW("%s, dynamic gamma value(%f) is not supported", __func__,
                 dataContext->mAiqParams.tonemapGamma);
        }
    } else if (dataContext->mAiqParams.tonemapMode == TONEMAP_MODE_CONTRAST_CURVE) {
        const camera_tonemap_curves_t* curves = &dataContext->mAiqParams.tonemapCurves;
        CheckAndLogError((curves->rSize != curves->gSize) || (curves->bSize != curves->gSize),
                         VOID_VALUE, "%s, the size of rgb channels must be same", __func__);

        // User's curve is 2-d array: (in, out)
        // Use the gamma curve to select the media format
        const float curveX = curves->gCurve[curves->gSize / 2];
        const float curveY = curves->gCurve[curves->gSize / 2 + 1];

        if (std::fabs(curveX - curveY) < std::numeric_limits<float>::epsilon()) {
            // It's a linear gamma curves(same with gamma = 1.0)
            *useLinearGamma = true;
            *mediaFormat = media_format_custom;
            LOG2("%s: a linear gamma curve. curveX: %f, curveY: %f", __func__, curveX, curveY);
        } else if (abs(curveY - pow(curveX, (1 / 2.2))) < EPSILON) {
            // It's a standard 2_2 gamma curves
            *mediaFormat = media_format_bt709_8b;
            LOG2("%s: a 2.2 gamma curve. curveX: %f, curveY: %f", __func__, curveX, curveY);
        } else {
            LOGW("%s, dynamic gamma curve(%f, %f) is not supported", __func__, curveX, curveY);
        }
    }
}

status_t IpuPacAdaptor::runAIC(const IspSettings* ispSettings,
                               int64_t settingSequence, int32_t streamId) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     INVALID_OPERATION, "%s, wrong state %d", __func__, mPacAdaptorState);

    std::pair<int, int64_t> pacItem = std::make_pair(streamId, settingSequence);
    if (mPacRunHistMap.find(pacItem) != mPacRunHistMap.end()) {
        LOG1("%s, streamId %d, sequence %ld had run before", __func__, streamId, settingSequence);
        return OK;
    }

    auto aiqResults = const_cast<AiqResult*>(mAiqResultStorage->getAiqResult(settingSequence));
    if (aiqResults == nullptr) {
        LOGW("<seq%ld>@%s: no result! use the latest instead", settingSequence, __func__);
        aiqResults = const_cast<AiqResult*>(mAiqResultStorage->getAiqResult());
        CheckAndLogError((aiqResults == nullptr), INVALID_OPERATION,
                         "Cannot find available aiq result.");
    }
    LOG2("<id%d:streamId:%d>@%s: aiq result id %ld", mCameraId, streamId, __func__,
         aiqResults->mFrameId);

    cca::cca_pal_input_params* inputParams = mStreamIdToInputParams[streamId];
    inputParams->seq_id = settingSequence;
    inputParams->stream_id = streamId;

    bool useLinearGamma = false;
    applyMediaFormat(aiqResults, &inputParams->media_format, &useLinearGamma, settingSequence);
    LOG2("%s, media format: 0x%x, gamma lut size: %d", __func__,
         inputParams->media_format, aiqResults->mGbceResults.gamma_lut_size);

    if (STILL_STREAM_ID == streamId) {
        inputParams->force_lsc_update = true;
    }

    if (ispSettings != nullptr) {
        inputParams->nr_setting = ispSettings->nrSetting;
        inputParams->ee_setting = ispSettings->eeSetting;
        LOG2("%s: ISP NR setting, level: %d, strength: %d", __func__,
             static_cast<int>(ispSettings->nrSetting.feature_level),
             static_cast<int>(ispSettings->nrSetting.strength));

        inputParams->effects = ispSettings->effects;
        inputParams->manual_brightness = ispSettings->manualSettings.manualBrightness;
        inputParams->manual_contrast = ispSettings->manualSettings.manualContrast;
        inputParams->manual_hue = ispSettings->manualSettings.manualHue;
        inputParams->manual_saturation = ispSettings->manualSettings.manualSaturation;
        LOG2("%s: ISP EE setting, level: %d, strength: %d", __func__,
             ispSettings->eeSetting.feature_level, ispSettings->eeSetting.strength);
    }

    inputParams->custom_controls.count = aiqResults->mCustomControls.count;
    const uint32_t cnt = static_cast<uint32_t>(inputParams->custom_controls.count);
    if (cnt > 0U) {
        CheckAndLogError(cnt > cca::MAX_CUSTOM_CONTROLS_PARAM_SIZE,
                         UNKNOWN_ERROR, "%s, buffer for custom control[%d] is too small",
                         __func__, cnt);

        MEMCPY_S(inputParams->custom_controls.parameters, cnt,
                 aiqResults->mCustomControls.parameters, cca::MAX_CUSTOM_CONTROLS_PARAM_SIZE);
    }

    inputParams->manual_pa_setting = aiqResults->mPaResults;
    if (aiqResults->mGbceResults.have_manual_settings == true) {
        inputParams->manual_gbce_setting = aiqResults->mGbceResults;
        if (useLinearGamma) {
            inputParams->manual_gbce_setting.gamma_lut_size = 0U;
        }
    }

    if (CameraUtils::isUllPsysPipe(TUNING_MODE_VIDEO)) {
        // The situation that all DG passed to ISP, not sensor.
        if (!PlatformData::isUsingSensorDigitalGain(mCameraId)) {
            inputParams->manual_digital_gain =
                aiqResults->mAeResults.exposures[0].exposure[0].digital_gain;
        }

        // Fine-tune DG passed to ISP if partial ISP DG is needed.
        if (PlatformData::isUsingIspDigitalGain(mCameraId)) {
            inputParams->manual_digital_gain = PlatformData::getIspDigitalGain(mCameraId,
                    aiqResults->mAeResults.exposures[0].exposure[0].digital_gain);
        }

        LOG2("%s: set digital gain for ULL pipe: %f", __func__, inputParams->manual_digital_gain);
    } else if (PlatformData::isMultiExposureCase(mCameraId, TUNING_MODE_VIDEO) &&
               (PlatformData::getSensorGainType(mCameraId) == ISP_DG_AND_SENSOR_DIRECT_AG)) {
        inputParams->manual_digital_gain =
            aiqResults->mAeResults.exposures[0].exposure[0].digital_gain;

        LOG2("%s: all digital gain is passed to ISP, DG(%ld): %f", __func__,
             aiqResults->mSequence, aiqResults->mAeResults.exposures[0].exposure[0].digital_gain);
    }

    // TODO: Set camera_params

    ia_err iaErr = ia_err_none;
    {
        PERF_CAMERA_ATRACE_PARAM1_IMAGING("pac_run", 1);

        // HDR_FEATURE_S
        if (PlatformData::getSensorAeEnable(mCameraId)) {
            inputParams->gain_id_gaic = 1;
        }
        // HDR_FEATURE_E

        iaErr = mIntelCca->runAIC(aiqResults->mFrameId, inputParams, 0xffU, streamId);
    }
    CheckAndLogError((iaErr != ia_err_none) && (iaErr != ia_err_not_run), UNKNOWN_ERROR,
                     "Pac parameter adaptation has failed %d", iaErr);

    (void)storeTerminalResult(settingSequence, streamId);

    mPacRunHistMap[pacItem] = false;

    return OK;
}

status_t IpuPacAdaptor::updateResolutionSettings(int streamId,
                                                 const cca::cca_aic_config& aicConfig,
                                                 bool isKeyResChanged) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     INVALID_OPERATION, "%s, wrong state %d", __func__, mPacAdaptorState);

    const ia_err iaErr = mIntelCca->updateConfigurationResolutions(aicConfig, streamId,
                                                                   isKeyResChanged);
    CheckAndLogError(iaErr != ia_err_none, UNKNOWN_ERROR,
                     "%s, Failed to configure pac, streamId: %d", __func__, streamId);

    return OK;
}

status_t IpuPacAdaptor::getAllBuffers(int streamId, uint8_t contextId, int64_t sequenceId,
                                      PacTerminalBufMap& bufferMap) {
    AutoMutex l(mIpuParamLock);

    std::pair<int, uint8_t> cbInstance = std::make_pair(streamId, contextId);
    if (mTerminalResult.find(cbInstance) == mTerminalResult.end()) {
        LOG2("%s, Failed to find data instance for streamId: %d, contextId: %u",
             __func__, streamId, contextId);
        return BAD_INDEX;
    }

    std::vector<CBTerminalResult>& cbTermResult = mTerminalResult[cbInstance];
    // Get the latest result when sequence is -1
    if (sequenceId == -1) {
        bufferMap = cbTermResult.back().termResult;
        return OK;
    } else {
        for (size_t i = 0U; i < cbTermResult.size(); ++i) {
            if (cbTermResult[i].sequence == sequenceId) {
                bufferMap = cbTermResult[i].termResult;
                return OK;
            }
        }
    }

    LOG2("%s, can't find the terminal buffer for sequence: %ld", __func__, sequenceId);
    return INVALID_OPERATION;;
}

bool IpuPacAdaptor::isStatsUsed(int streamId, int64_t sequence) {
    // Don't use statistics if it is older than last one
    if ((mLastStatsSequence >= 0) && (sequence <= mLastStatsSequence)) {
        return false;
    }

    /**
     * Normally only statistics from Video pipe is used.
     * The statistics from Still pipe is used when working in Still pipe cases.
     */
    if ((streamId == STILL_STREAM_ID) && (!PlatformData::isStillOnlyPipeEnabled(mCameraId))) {
        return false;
    }

    return true;
}

status_t IpuPacAdaptor::decodeStats(int streamId, uint8_t contextId, int64_t sequenceId,
                                    unsigned long long timestamp) {
    AutoMutex l(mPacAdaptorLock);
    CheckAndLogError(mIntelCca == nullptr, UNKNOWN_ERROR, "%s, mIntelCca is nullptr", __func__);
    CheckAndLogError(mPacAdaptorState != PAC_ADAPTOR_INIT,
                     INVALID_OPERATION, "%s, wrong state %d", __func__, mPacAdaptorState);

    std::pair<int, int64_t> pacItem = std::make_pair(streamId, sequenceId);
    if (mPacRunHistMap.find(pacItem) == mPacRunHistMap.end()) {
        LOG1("%s, no stream %d and sequence %ld found", __func__, streamId, sequenceId);
        return OK;
    } else if (mPacRunHistMap[pacItem]) {
        LOG1("%s, stream %d and sequence %ld decoded", __func__, streamId, sequenceId);
        return OK;
    }

    const bool statsUsed = isStatsUsed(streamId, sequenceId);

    cca::cca_out_stats outStatsTemp;
    cca::cca_out_stats* outStats = &outStatsTemp;
    outStats->get_rgbs_stats = false;

    LOG2("<seq:%ld>@%s, decode 3A stats. streamId: %d, contextId: %d, statsUsed: %d",
         sequenceId, __func__, streamId, contextId, statsUsed);

    if (statsUsed) {
        auto cameraContext = CameraContext::getInstance(mCameraId);
        auto dataContext = cameraContext->getDataContextBySeq(sequenceId);
        auto aiqResult = const_cast<AiqResult*>(mAiqResultStorage->getAiqResult(sequenceId));
        if ((aiqResult != nullptr) && dataContext->mAiqParams.callbackRgbs) {
            outStats = &aiqResult->mOutStats;
            outStats->get_rgbs_stats = true;
        }
    }

    const ia_err iaErr = mIntelCca->decodeStats(contextId, sequenceId, streamId, outStats);
    if (iaErr == ia_err::ia_err_none) {
        if (statsUsed) {
            AiqStatistics* aiqStatistics = mAiqResultStorage->acquireAiqStatistics();
            aiqStatistics->mSequence = sequenceId;
            aiqStatistics->mTimestamp = timestamp;
            aiqStatistics->mTuningMode = TUNING_MODE_VIDEO;

            mAiqResultStorage->updateAiqStatistics(sequenceId);
        }
        mLastStatsSequence = sequenceId;
    } else {
        LOGE("<seq:%ld>%s, Failed to decode stats. streamId: %d, contextId: %d", sequenceId,
             __func__, streamId, contextId);
    }

    mPacRunHistMap[pacItem] = true;
    if (mPacRunHistMap.size() >= MAX_CACHE_PAC_HIST) {
        for (auto iter = mPacRunHistMap.begin(); iter != mPacRunHistMap.end(); iter++) {
            if (iter->second) {
                mPacRunHistMap.erase(iter);
                break;
            }
        }
    }

    return (iaErr == ia_err_none) ? OK : UNKNOWN_ERROR;
}

} // namespace icamera
