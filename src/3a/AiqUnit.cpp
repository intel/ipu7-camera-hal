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

#define LOG_TAG AiqUnit

#include <map>
#include <string>
#include <memory>

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "CameraContext.h"
#include "PlatformData.h"

#include "AiqUnit.h"

namespace icamera {

AiqUnit::AiqUnit(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw) :
    mCameraId(cameraId),
    mAiqUnitState(AIQ_UNIT_NOT_INIT),
    mOperationMode(CAMERA_STREAM_CONFIGURATION_MODE_NORMAL),
    mCcaInitialized(false),
    mActiveStreamCount(0U) {
    mAiqEngine = new AiqEngine(cameraId, sensorHw, lensHw);
}

AiqUnit::~AiqUnit() {
    if (mAiqUnitState == AIQ_UNIT_START) {
        AiqUnit::stop();
    }
    if (mAiqUnitState == AIQ_UNIT_INIT) {
        AiqUnit::deinit();
    }

    delete mAiqEngine;
}

void AiqUnit::init() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    if (mAiqUnitState == AIQ_UNIT_NOT_INIT) {
        mAiqEngine->init();
    }

    mAiqUnitState = AIQ_UNIT_INIT;
}

void AiqUnit::deinit() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    mAiqEngine->deinit();

    deinitIntelCcaHandle();
    mAiqUnitState = AIQ_UNIT_NOT_INIT;
}

int AiqUnit::configure(const stream_config_t *streamList) {
    CheckAndLogError(streamList == nullptr, BAD_VALUE, "streamList is nullptr");

    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode,
                                                configModes);

    if ((mAiqUnitState == AIQ_UNIT_CONFIGURED) && (mOperationMode == streamList->operation_mode)) {
        std::vector<int32_t> streamIds;
        if (configModes.empty() == false) {
            std::shared_ptr<GraphConfig> graphConfig =
                CameraContext::getInstance(mCameraId)->getGraphConfig(configModes[0]);
            if (graphConfig != nullptr) {
                graphConfig->graphGetStreamIds(streamIds, false);
            }
        }

        if (streamIds.size() == mActiveStreamCount) {
            LOG2("%s: already configured in the same mode: %d", __func__, mOperationMode);
            return OK;
        }
    } else if ((mAiqUnitState != AIQ_UNIT_INIT) && (mAiqUnitState != AIQ_UNIT_STOP)) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    } else {
        LOG2("%s: configure CCA handle", __func__);
    }

    const int ret = initIntelCcaHandle(configModes);
    CheckAndLogError(ret < 0, BAD_VALUE, "@%s failed to create intel cca handle", __func__);

    mOperationMode = streamList->operation_mode;
    mAiqUnitState = AIQ_UNIT_CONFIGURED;
    return OK;
}

int AiqUnit::initIntelCcaHandle(const std::vector<ConfigMode> &configModes) {
    deinitIntelCcaHandle();

    LOG1("<id%d>@%s", mCameraId, __func__);
    mTuningModes.clear();
    std::vector<int32_t> streamIds;
    for (const auto &cfg : configModes) {
        TuningMode tuningMode;
        int ret = PlatformData::getTuningModeByConfigMode(mCameraId, cfg, tuningMode);
        CheckAndLogError(ret != OK, ret, "%s: Failed to get tuningMode, cfg: %d", __func__, cfg);

        PERF_CAMERA_ATRACE_PARAM1_IMAGING("intelCca->init", 1U);

        const auto params = std::unique_ptr<cca::cca_init_params>(new cca::cca_init_params);
        // Initialize cca_cpf data
        ia_binary_data cpfData;
        ret = PlatformData::getCpf(mCameraId, tuningMode, &cpfData);
        if ((ret == OK) && (cpfData.data != nullptr)) {
            CheckAndLogError(cpfData.size > cca::MAX_CPF_LEN, UNKNOWN_ERROR,
                       "%s, AIQB buffer is too small cpfData:%d > MAX_CPF_LEN:%d",
                       __func__, cpfData.size, cca::MAX_CPF_LEN);
            MEMCPY_S(params->aiq_cpf.buf, cca::MAX_CPF_LEN, cpfData.data, cpfData.size);
            params->aiq_cpf.size = cpfData.size;
        }

        // Initialize cca_nvm data
        ia_binary_data* nvmData = PlatformData::getNvm(mCameraId);
        if (nvmData != nullptr) {
            CheckAndLogError(nvmData->size > cca::MAX_NVM_LEN,  UNKNOWN_ERROR,
                       "%s, NVM buffer is too small: nvmData:%d  MAX_NVM_LEN:%d",
                       __func__, nvmData->size, cca::MAX_NVM_LEN);
            MEMCPY_S(params->aiq_nvm.buf, cca::MAX_NVM_LEN, nvmData->data, nvmData->size);
            params->aiq_nvm.size = nvmData->size;
        }

        // Initialize cca_aiqd data
        ia_binary_data* aiqdData = PlatformData::getAiqd(mCameraId, tuningMode);
        if (aiqdData != nullptr) {
            CheckAndLogError(aiqdData->size > cca::MAX_AIQD_LEN,  UNKNOWN_ERROR,
                       "%s, AIQD buffer is too small aiqdData:%d > MAX_AIQD_LEN:%d",
                       __func__, aiqdData->size, cca::MAX_AIQD_LEN);
            MEMCPY_S(params->aiq_aiqd.buf, cca::MAX_AIQD_LEN, aiqdData->data, aiqdData->size);
            params->aiq_aiqd.size = aiqdData->size;
        }

        SensorFrameParams sensorParam = {};
        ret = PlatformData::calculateFrameParams(mCameraId, sensorParam);
        CheckAndLogError(ret != OK, ret, "%s: Failed to calculate frame params", __func__);
        AiqUtils::convertToAiqFrameParam(sensorParam, params->frameParams);

        params->frameUse = ia_aiq_frame_use_video;
        params->aiqStorageLen = MAX_SETTING_COUNT;
        // handle AE delay in AiqEngine
        params->aecFrameDelay = 0U;

        // Initialize functions which need to be started
        params->bitmap = cca::CCA_MODULE_AE | cca::CCA_MODULE_AWB |
                        cca::CCA_MODULE_PA | cca::CCA_MODULE_SA | cca::CCA_MODULE_GBCE |
                        cca::CCA_MODULE_LARD;
        if (PlatformData::getLensHwType(mCameraId) == LENS_VCM_HW) {
            params->bitmap |= cca::CCA_MODULE_AF;
        }

        std::shared_ptr<GraphConfig> graphConfig =
            CameraContext::getInstance(mCameraId)->getGraphConfig(cfg);
        if (graphConfig != nullptr) {
            streamIds.clear();
            graphConfig->graphGetStreamIds(streamIds, false);
            params->aic_stream_ids.count = streamIds.size();
            CheckAndLogError(streamIds.size() > cca::MAX_STREAM_NUM, UNKNOWN_ERROR,
                    "%s, Too many streams: %zu in graph", __func__, streamIds.size());
            for (size_t i = 0U; i < streamIds.size(); ++i) {
                params->aic_stream_ids.ids[i] = streamIds[i];
            }
        }

        IntelCca *intelCca = IntelCca::getInstance(mCameraId, tuningMode);
        CheckAndLogError(intelCca == nullptr, UNKNOWN_ERROR,
                         "Failed to get cca. mode:%d cameraId:%d", tuningMode, mCameraId);
        const ia_err iaErr = intelCca->init(*params);
        if (iaErr == ia_err_none) {
            mTuningModes.push_back(tuningMode);
        } else {
            LOGE("%s, init IntelCca fails. mode:%d cameraId:%d", __func__, tuningMode, mCameraId);
            IntelCca::releaseInstance(mCameraId, tuningMode);
            return UNKNOWN_ERROR;
        }

        ret = PlatformData::initMakernote(mCameraId, tuningMode);
        CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, PlatformData::initMakernote fails",
                         __func__);

        dumpCcaInitParam(*params);
    }

    mActiveStreamCount = streamIds.size();
    mCcaInitialized = true;
    return OK;
}

void AiqUnit::deinitIntelCcaHandle() {
    if (!mCcaInitialized) {
        return;
    }

    LOG1("<id%d>@%s", mCameraId, __func__);
    for (const auto &mode : mTuningModes) {
        IntelCca *intelCca = IntelCca::getInstance(mCameraId, mode);
        CheckAndLogError(intelCca == nullptr, VOID_VALUE,
                         "%s, Failed to get cca: mode(%d), cameraId(%d)",
                         __func__, mode, mCameraId);

        if (PlatformData::isAiqdEnabled(mCameraId)) {
            const auto aiqd = std::unique_ptr<cca::cca_aiqd>(new cca::cca_aiqd);
            (void)memset(aiqd.get(), 0, sizeof(cca::cca_aiqd));

            const ia_err iaErr = intelCca->getAiqd(aiqd.get());
            if (AiqUtils::convertError(iaErr) == OK) {
                ia_binary_data data = {aiqd->buf, static_cast<unsigned int>(aiqd->size)};
                PlatformData::saveAiqd(mCameraId, mode, data);
            } else {
                LOGW("@%s, failed to get aiqd data, iaErr %d", __func__, iaErr);
            }
        }

        const int ret = PlatformData::deinitMakernote(mCameraId, mode);
        if (ret != OK) {
            LOGE("@%s, PlatformData::deinitMakernote fails", __func__);
        }

        intelCca->deinit();
        IntelCca::releaseInstance(mCameraId, mode);
    }

    mCcaInitialized = false;
    mActiveStreamCount = 0U;
}

int AiqUnit::start() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    if ((mAiqUnitState != AIQ_UNIT_CONFIGURED) && (mAiqUnitState != AIQ_UNIT_STOP)) {
        LOGW("%s: configure in wrong state: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    mAiqEngine->reset();
    mAiqUnitState = AIQ_UNIT_START;

    return OK;
}

void AiqUnit::stop() {
    AutoMutex l(mAiqUnitLock);
    LOG1("<id%d>@%s", mCameraId, __func__);

    mAiqUnitState = AIQ_UNIT_STOP;
}

int AiqUnit::run3A(int64_t ccaId, int64_t applyingSeq, int64_t frameNumber, int64_t* effectSeq) {
    AutoMutex l(mAiqUnitLock);
    TRACE_LOG_PROCESS("AiqUnit", "run3A");

    if (mAiqUnitState != AIQ_UNIT_START) {
        LOGW("%s: AIQ is not started: %d", __func__, mAiqUnitState);
        return BAD_VALUE;
    }

    const int ret = mAiqEngine->run3A(ccaId, applyingSeq, frameNumber, effectSeq);
    CheckAndLogError(ret != OK, ret, "run 3A failed.");

    return OK;
}

std::vector<EventListener*> AiqUnit::getSofEventListener() {
    AutoMutex l(mAiqUnitLock);
    std::vector<EventListener*> eventListenerList;
    eventListenerList.push_back(mAiqEngine->getSofEventListener());
    return eventListenerList;
}

std::vector<EventListener*> AiqUnit::getStatsEventListener() {
    AutoMutex l(mAiqUnitLock);

    std::vector<EventListener*> eventListenerList;

    return eventListenerList;
}

void AiqUnit::dumpCcaInitParam(const cca::cca_init_params& params) {
    if (!Log::isLogTagEnabled(GET_FILE_SHIFT(AiqUnit))) {
        return;
    }

    LOG3("bitmap:%x", params.bitmap);
    LOG3("frameUse: %d", params.frameUse);
    LOG3("aecFrameDelay:%d", params.aecFrameDelay);
    LOG3("streamId num:%zu", params.aic_stream_ids.count);

    LOG3("horizontal_crop_offset:%d", params.frameParams.horizontal_crop_offset);
    LOG3("vertical_crop_offset:%d", params.frameParams.vertical_crop_offset);
    LOG3("cropped_image_width:%d", params.frameParams.cropped_image_width);
    LOG3("cropped_image_height:%d", params.frameParams.cropped_image_height);
    LOG3("horizontal_scaling_numerator:%d", params.frameParams.horizontal_scaling_numerator);
    LOG3("horizontal_scaling_denominator:%d", params.frameParams.horizontal_scaling_denominator);
    LOG3("vertical_scaling_numerator:%d", params.frameParams.vertical_scaling_numerator);
    LOG3("vertical_scaling_denominator:%d", params.frameParams.vertical_scaling_denominator);
}

} /* namespace icamera */
