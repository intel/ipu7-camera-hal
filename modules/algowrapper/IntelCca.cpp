/*
 * Copyright (C) 2020-2025 Intel Corporation
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

#define LOG_TAG IntelCca

#include <vector>

#include "modules/algowrapper/IntelCca.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    AutoMutex lock(sLock);
    LOG2("<id%d>@%s, tuningMode:%d, cca instance size:%zu", cameraId, __func__, mode,
         sCcaInstance.size());

    for (auto &it : sCcaInstance) {
        if (cameraId == it.cameraId) {
            if (it.ccaHandle.find(mode) == it.ccaHandle.end()) {
                it.ccaHandle[mode] = new IntelCca(cameraId, mode);
            }
            return it.ccaHandle[mode];
        }
    }

    IntelCca::CCAHandle handle = {};
    handle.cameraId = cameraId;
    handle.ccaHandle[mode] = new IntelCca(cameraId, mode);
    sCcaInstance.push_back(handle);

    return handle.ccaHandle[mode];
}

void IntelCca::releaseInstance(int cameraId, TuningMode mode) {
    LOG2("<id%d>@%s, tuningMode:%d", cameraId, __func__, mode);

    AutoMutex lock(sLock);
    for (auto &it : sCcaInstance) {
        if ((cameraId == it.cameraId) && (it.ccaHandle.find(mode) != it.ccaHandle.end())) {
            IntelCca *cca = it.ccaHandle[mode];
            it.ccaHandle.erase(mode);
            delete cca;
        }
    }
}

IntelCca::IntelCca(int cameraId, TuningMode mode) :
    mCameraId(cameraId),
    mTuningMode(mode) {
    mIntelCCA = nullptr;
    LOG2("<id%d>@%s, tuningMode:%d", mCameraId, __func__, mTuningMode);
}

IntelCca::~IntelCca() {
    releaseIntelCCA();
}

cca::IntelCCA* IntelCca::getIntelCCA() {
    if (mIntelCCA == nullptr) {
        mIntelCCA = new cca::IntelCCA();
    }
    return mIntelCCA;
}

void IntelCca::releaseIntelCCA() {
    delete mIntelCCA;
    mIntelCCA = nullptr;
}
ia_err IntelCca::init(const cca::cca_init_params& initParams) {
    const ia_err ret = getIntelCCA()->init(initParams);
    LOG2("@%s, bitmap:0x%x, ret:%d, version:%s", __func__, initParams.bitmap, ret,
         getIntelCCA()->getVersion());

    return ret;
}

ia_err IntelCca::reinitAic(const int32_t aicId) {
    const ia_err ret = getIntelCCA()->reinitAic(aicId);

    LOG2("@%s, aicId:%d, ret:%d", __func__, aicId, ret);

    return ret;
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    const ia_err ret = getIntelCCA()->setStatsParams(params);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results) {
    CheckAndLogError(results == nullptr, ia_err_argument, "@%s, results is nullptr", __func__);

    const ia_err ret = getIntelCCA()->runAEC(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results) {
    CheckAndLogError(results == nullptr, ia_err_argument, "@%s, results is nullptr", __func__);

    const ia_err ret = getIntelCCA()->runAIQ(frameId, params, results);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                              const cca::cca_nvm& nvm, int32_t streamId) {
    ia_lard_input_params& lardInputParam = const_cast<ia_lard_input_params&>(lardParams);
    const ia_err ret = getIntelCCA()->updateTuning(lardTags, lardInputParam, nvm, streamId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc) {
    CheckAndLogError(cmc == nullptr, ia_err_argument, "@%s, cmc is nullptr", __func__);

    const ia_err ret = getIntelCCA()->getCMC(*cmc);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    CheckAndLogError(mkn == nullptr, ia_err_argument, "@%s, mkn is nullptr", __func__);

    const ia_err ret = getIntelCCA()->getMKN(type, *mkn);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    CheckAndLogError(aiqd == nullptr, ia_err_argument, "@%s, aiqd is nullptr", __func__);

    const ia_err ret = getIntelCCA()->getAiqd(*aiqd);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

void* IntelCca::allocMem(int streamId, const std::string& name, int index, int size) {
    LOG1("@%s, name:%s, index: %d, streamId: %d, size: %d", __func__,
         name.c_str(), index, streamId, size);

    void* ptr = nullptr;
    const int ret = posix_memalign(&ptr, PAGE_SIZE_U, PAGE_ALIGN(size));
    if (ret != 0) LOGE("alloc fail");
    return ptr;
}

void IntelCca::freeMem(void* addr) {
    LOG1("@%s addr: %p", __func__, addr);
    free(addr);
}

void IntelCca::deinit() {
    getIntelCCA()->deinit();
    releaseIntelCCA();
}

ia_err IntelCca::configAic(const cca::cca_aic_config& aicConf,
                           const cca::cca_aic_kernel_offset& kernelOffset, uint32_t* offsetPtr,
                           cca::cca_aic_terminal_config& termConfig, int32_t aicId,
                           const int32_t* statsBufToTermIds) {
    const ia_err ret = getIntelCCA()->configAIC(aicConf, kernelOffset, termConfig, aicId,
                                          statsBufToTermIds);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::registerAicBuf(const cca::cca_aic_terminal_config& termConfig, int32_t aicId) {
    const ia_err ret = getIntelCCA()->registerAICBuf(termConfig, aicId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::getAicBuf(cca::cca_aic_terminal_config& termConfig, int32_t aicId) {
    const ia_err ret = getIntelCCA()->getAICBuf(termConfig, aicId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::decodeStats(int32_t groupId, int64_t sequence, int32_t aicId,
                             cca::cca_out_stats* outStats) {
    const ia_err ret = getIntelCCA()->decodeStats(groupId, sequence, aicId);
    LOG2("@%s, ret:%d", __func__, ret);

    if ((ret == ia_err_none) && ((outStats != nullptr) && outStats->get_rgbs_stats)) {
        auto stats = getIntelCCA()->queryStatsBuf(cca::STATS_BUF_LATEST);
        if (stats != nullptr) {
            outStats->rgbs_grid[0].grid_width = stats->stats.rgbs_grids[0].grid_width;
            outStats->rgbs_grid[0].grid_height = stats->stats.rgbs_grids[0].grid_height;
            outStats->rgbs_grid[0].shading_correction = stats->stats.shading_corrected;

            const unsigned int width = stats->stats.rgbs_grids[0].grid_width;
            const unsigned int height = stats->stats.rgbs_grids[0].grid_height;
            for (unsigned int i = 0U; i < width * height; i++) {
                outStats->rgbs_blocks[0][i].avg_gr = stats->stats.rgbs_grids[0].avg[i].gr;
                outStats->rgbs_blocks[0][i].avg_r = stats->stats.rgbs_grids[0].avg[i].r;
                outStats->rgbs_blocks[0][i].avg_b = stats->stats.rgbs_grids[0].avg[i].b;
                outStats->rgbs_blocks[0][i].avg_gb = stats->stats.rgbs_grids[0].avg[i].gb;
                outStats->rgbs_blocks[0][i].sat = stats->stats.rgbs_grids[0].sat[i];
            }
        }
    }

    return ret;
}

ia_err IntelCca::runAIC(uint64_t frameId, const cca::cca_pal_input_params* params,
                         uint8_t bitmap, int32_t aicId) {
    cca::cca_multi_pal_output output = {};
    const ia_err ret = getIntelCCA()->runAIC(frameId, *params, output, bitmap, aicId);
    LOG2("@%s, ret:%d", __func__, ret);

    return ret;
}

ia_err IntelCca::updateConfigurationResolutions(const cca::cca_aic_config& aicConf,
                                                int32_t aicId, bool isKeyResChanged) {
    const ia_err ret =
        getIntelCCA()->updateConfigurationResolutions(aicConf, aicId, isKeyResChanged);
    LOG2("@%s, ret:%d ", __func__, ret);

    return ret;
}

} /* namespace icamera */
