/*
 * Copyright (C) 2024 Intel Corporation
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

#define LOG_TAG IntelCcaClient

#include "CcaClient.h"

#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#include "IPAClient.h"

namespace icamera {

#define SHM_NAME "shm"
#define mAlgoClient libcamera::IPAClient::getInstance()

std::vector<IntelCca::CCAHandle> IntelCca::sCcaInstance;
Mutex IntelCca::sLock;

IntelCca* IntelCca::getInstance(int cameraId, TuningMode mode) {
    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
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
    AutoMutex lock(sLock);
    for (auto& it : sCcaInstance) {
        if (cameraId == it.cameraId && it.ccaHandle.find(mode) != it.ccaHandle.end()) {
            IntelCca* cca = it.ccaHandle[mode];
            it.ccaHandle.erase(mode);
            delete cca;
        }
    }
}

IntelCca::IntelCca(int cameraId, TuningMode mode) : mCameraId(cameraId), mTuningMode(mode) {
    LOG1("<id%d> @%s, tuningMode:%d", cameraId, __func__, mode);

    std::string number = std::to_string(cameraId) + std::to_string(mode) +
                         std::to_string(reinterpret_cast<uintptr_t>(this));
    std::string structName = "/ccaStruct" + number + SHM_NAME;
    std::string initName = "/ccaInit" + number + SHM_NAME;
    std::string reinitAicName = "/ccaReinitAic" + number + SHM_NAME;
    std::string statsName = "/ccaStats" + number + SHM_NAME;
    std::string aecName = "/ccaAec" + number + SHM_NAME;
    std::string aiqName = "/ccaAiq" + number + SHM_NAME;
    std::string aicName = "/ccaAic" + number + SHM_NAME;
    std::string aicControlName = "/ccaAicControl" + number + SHM_NAME;
    std::string cmcName = "/ccaCmc" + number + SHM_NAME;
    std::string mknName = "/ccaMkn" + number + SHM_NAME;
    std::string aiqdName = "/ccaAiqd" + number + SHM_NAME;
    std::string tuningName = "/ccaTuning" + number + SHM_NAME;
    std::string deinitName = "/ccaDeinit" + number + SHM_NAME;
    std::string decodeStatsName = "/ccaDecodeStats" + number + SHM_NAME;

    mMems = {
        {structName.c_str(), sizeof(intel_cca_struct_data), &mMemStruct, false},
        {initName.c_str(), sizeof(intel_cca_init_data), &mMemInit, false},
        {reinitAicName.c_str(), sizeof(intel_cca_reinit_aic_data), &mMemReinitAic, false},
        {statsName.c_str(), sizeof(intel_cca_set_stats_data), &mMemStats, false},
        {aecName.c_str(), sizeof(intel_cca_run_aec_data), &mMemAEC, false},
        {aiqName.c_str(), sizeof(intel_cca_run_aiq_data), &mMemAIQ, false},
        {aicName.c_str(), sizeof(intel_cca_run_aic_data), &mMemAIC, false},
        {aicControlName.c_str(), sizeof(intel_cca_aic_control_data), &mMemAICControl, false},
        {cmcName.c_str(), sizeof(intel_cca_get_cmc_data), &mMemCMC, false},
        {mknName.c_str(), sizeof(intel_cca_mkn_data), &mMemMKN, false},
        {aiqdName.c_str(), sizeof(intel_cca_get_aiqd_data), &mMemAIQD, false},
        {tuningName.c_str(), sizeof(intel_cca_update_tuning_data), &mMemTuning, false},
        {deinitName.c_str(), sizeof(intel_cca_deinit_data), &mMemDeinit, false},
        {decodeStatsName.c_str(), sizeof(intel_cca_decode_stats_data), &mMemDecodeStats, false}};

    bool success = allocateAllShmMems(&mMems);
    if (!success) {
        releaseAllShmMems(mMems);
        return;
    }

    LOG1("@%s, Construct done", __func__);
}

IntelCca::~IntelCca() {
    LOG1("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    releaseAllShmMems(mMems);

    for (auto& it : mMemsOuter) {
        mAlgoClient->freeShmMem(it.second.mName, it.second.mAddr, it.second.mHandle);
    }
    mMemsOuter.clear();
}

ia_err IntelCca::init(const cca::cca_init_params& initParams) {
    LOG1("<id%d> @%s, tuningMode:%d, bitmap:0x%x", mCameraId, __func__, mTuningMode,
         initParams.bitmap);

    intel_cca_init_data* params = static_cast<intel_cca_init_data*>(mMemInit.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->inParams = initParams;

    if (!mAlgoClient) return ia_err_argument;

    int ret = mAlgoClient->initCca(mCameraId, mTuningMode, mMemInit.mHandle);

    LOG1("<id%d> @%s, tuningMode:%d done", mCameraId, __func__, mTuningMode);
    return static_cast<ia_err>(ret);
}

ia_err IntelCca::reinitAic(uint32_t aicId) {
    LOG1("<id%d> @%s, tuningMode:%d, aicId:%d", mCameraId, __func__, mTuningMode, aicId);

    intel_cca_reinit_aic_data* params =
        static_cast<intel_cca_reinit_aic_data*>(mMemReinitAic.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->aicId = aicId;

    int ret = mAlgoClient->reinitAic(mCameraId, mTuningMode, mMemReinitAic.mHandle);

    return static_cast<ia_err>(ret);
}

bool IntelCca::prepareAicConfigIPC(const cca::cca_aic_config& aicCfg,
                                   ipc_cca_aic_config* ipcCfg) {
     for (int cb = 0; cb < aicCfg.cb_num; cb++) {
        int32_t systemApiSize = 0;

        for (int kernel = 0; kernel < aicCfg.cb_config[cb].kernel_group->kernelCount; kernel++) {
            aic::ia_pac_kernel_info* kernelInfo =
                aicCfg.cb_config[cb].kernel_group->kernelList + kernel;
            systemApiSize += kernelInfo->run_kernel.system_api.size;
        }
        CheckAndLogError(systemApiSize > MAX_SYSTEM_API_DATA_SIZE_IN_PG, false,
                         "@%s: systemapi memory is too small, need %d", __func__, systemApiSize);
    }
    return true;
}

void IntelCca::prepareAicKernelOffsetIPC(const cca::cca_aic_kernel_offset& aicOffset,
                                         uint32_t* offsetPtr,
                                         ipc_cca_aic_kernel_offset* ipcOffset) {
    ipcOffset->offsetBuffer = offsetPtr;
    ipcOffset->offsetHandle = mAlgoClient->getShmMemHandle((void*)offsetPtr);
}

void IntelCca::prepareAicBufIPC(const cca::cca_aic_terminal_config& termConfig,
                                ipc_cca_aic_terminal_config* terminalConfig) {
    for (int cb = 0; cb < termConfig.cb_num; cb++) {
        for (int terminal = 0; terminal < termConfig.cb_terminal_buf[cb].num_terminal; terminal++) {
            aic::IaAicBuffer* payload =
                termConfig.cb_terminal_buf[cb].terminal_buf[terminal].payload;
            terminalConfig->cb_terminal_buf[cb].terminal_buf[terminal].payloadServerAddr = nullptr;
            if (payload->size > 0) {
                terminalConfig->cb_terminal_buf[cb].terminal_buf[terminal].payloadHandle =
                    mAlgoClient->getShmMemHandle(payload->payloadPtr);
            } else {
                terminalConfig->cb_terminal_buf[cb].terminal_buf[terminal].payloadHandle = -1;
            }
        }
    }
}

ia_err IntelCca::configAic(const cca::cca_aic_config& aicConf,
                           const cca::cca_aic_kernel_offset& kernelOffset, uint32_t* offsetPtr,
                           cca::cca_aic_terminal_config& termConfig, int32_t aicId,
                           const int32_t* statsBufToTermIds) {
    LOG2(" %s ", __func__);
    intel_cca_aic_control_data* aicControl =
        static_cast<intel_cca_aic_control_data*>(mMemAICControl.mAddr);

    bool rt = prepareAicConfigIPC(aicConf, &aicControl->config);
    CheckAndLogError(!rt, ia_err_general, "prepareAicConfigIPC fails");
    prepareAicKernelOffsetIPC(kernelOffset, offsetPtr, &aicControl->kernelOffset);
    prepareAicBufIPC(termConfig, &aicControl->termConfig);

    rt = mIpcCca.clientFlattenConfigAic(mMemAICControl.mAddr, mMemAICControl.mSize, aicConf,
                                        kernelOffset, termConfig, aicId, statsBufToTermIds);
    CheckAndLogError(!rt, ia_err_general, "clientFlattenConfigAic fails");

    int ret = mAlgoClient->configAic(mCameraId, mTuningMode, mMemAICControl.mHandle);
    mIpcCca.unFlattenTerminalConfig(aicControl->termConfig, termConfig);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::registerAicBuf(const cca::cca_aic_terminal_config& termConfig, int32_t aicId) {
    intel_cca_aic_control_data* aicControl =
        static_cast<intel_cca_aic_control_data*>(mMemAICControl.mAddr);
    aicControl->aicId = aicId;
    prepareAicBufIPC(termConfig, &aicControl->termConfig);

    mIpcCca.flattenTerminalConfig(aicControl->termConfig, termConfig);

    int ret = mAlgoClient->registerAicBuf(mCameraId, mTuningMode, mMemAICControl.mHandle);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::getAicBuf(cca::cca_aic_terminal_config& termConfig, int32_t aicId) {
    intel_cca_aic_control_data* aicControl =
        static_cast<intel_cca_aic_control_data*>(mMemAICControl.mAddr);
    aicControl->aicId = aicId;
    prepareAicBufIPC(termConfig, &aicControl->termConfig);

    mIpcCca.flattenTerminalConfig(aicControl->termConfig, termConfig);

    int ret = mAlgoClient->getAicBuf(mCameraId, mTuningMode, mMemAICControl.mHandle);
    if (ret != 0) return ia_err_argument;

    mIpcCca.unFlattenTerminalConfig(aicControl->termConfig, termConfig);
    return ia_err_none;
}

ia_err IntelCca::decodeStats(int32_t groupId, int64_t sequence, int32_t aicId,
                             cca::cca_out_stats* outStats) {
    intel_cca_decode_stats_data* decodeStats =
        static_cast<intel_cca_decode_stats_data*>(mMemDecodeStats.mAddr);

    decodeStats->groupId = groupId;
    decodeStats->sequence = sequence;
    decodeStats->aicId = aicId;

    decodeStats->statsHandle = -1;
    decodeStats->statsBuffer.data = nullptr;
    decodeStats->statsBuffer.size = 0;
    decodeStats->outStats.get_rgbs_stats = false;

    if (outStats) {
        decodeStats->outStats.get_rgbs_stats = outStats->get_rgbs_stats;
    }

    int ret = mAlgoClient->decodeStats(mCameraId, mTuningMode, mMemDecodeStats.mHandle);

    if (outStats && decodeStats->outStats.get_rgbs_stats) {
        *outStats = decodeStats->outStats;
        outStats->rgbs_grid[0].blocks_ptr = outStats->rgbs_blocks[0];
    }

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::runAIC(uint64_t frameId, const cca::cca_pal_input_params* params, uint32_t bitmap,
                        int32_t aicId) {
    intel_cca_run_aic_data* aicParams = static_cast<intel_cca_run_aic_data*>(mMemAIC.mAddr);
    aicParams->frameId = frameId;
    aicParams->inParamsHandle = mAlgoClient->getShmMemHandle((void*)params);
    aicParams->bitmap = bitmap;
    aicParams->aicId = aicId;

    int ret = mAlgoClient->runAic(mCameraId, mTuningMode, mMemAIC.mHandle);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::updateConfigurationResolutions(const cca::cca_aic_config& aicConf,
                                                int32_t aicId, bool isKeyResChanged) {
    intel_cca_aic_control_data* aicControl =
        static_cast<intel_cca_aic_control_data*>(mMemAICControl.mAddr);

    bool rt = prepareAicConfigIPC(aicConf, &aicControl->config);
    CheckAndLogError(!rt, ia_err_general, "prepareAicConfigIPC for res update fails");

    rt = mIpcCca.clientFlattenUpdateCfgRes(mMemAICControl.mAddr, mMemAICControl.mSize, aicConf,
                                           aicId, isKeyResChanged);
    CheckAndLogError(!rt, ia_err_general, "clientFlattenUpdateCfgRes fails");

    int ret = mAlgoClient->updateConfigurationResolutions(mCameraId, mTuningMode,
                                                          mMemAICControl.mHandle);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::setStatsParams(const cca::cca_stats_params& params) {
    LOG2("<id%d> @%s, tuningMode:%d, in params size:%zu", mCameraId, __func__, mTuningMode,
         sizeof(cca::cca_stats_params));

    intel_cca_set_stats_data* statsParams = static_cast<intel_cca_set_stats_data*>(mMemStats.mAddr);
    statsParams->cameraId = mCameraId;
    statsParams->tuningMode = mTuningMode;
    statsParams->inParams = params;

    int ret = mAlgoClient->setStats(mCameraId, mTuningMode, mMemStats.mHandle);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                        cca::cca_ae_results* results) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu, results size:%zu", mCameraId,
         frameId, __func__, mTuningMode, sizeof(cca::cca_ae_input_params),
         sizeof(cca::cca_ae_results));

    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    intel_cca_run_aec_data* aecParams = static_cast<intel_cca_run_aec_data*>(mMemAEC.mAddr);
    aecParams->cameraId = mCameraId;
    aecParams->tuningMode = mTuningMode;
    aecParams->frameId = frameId;
    aecParams->inParams = params;

    int ret = mAlgoClient->runAec(mCameraId, mTuningMode, mMemAEC.mHandle);
    if (ret != 0) return ia_err_general;

    *results = aecParams->results;

    return ia_err_none;
}

ia_err IntelCca::runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                        cca::cca_aiq_results* results) {
    LOG2("<id%d:req%ld> @%s, tuningMode:%d, in params size:%zu, results size:%zu", mCameraId,
         frameId, __func__, mTuningMode, sizeof(cca::cca_aiq_params), sizeof(cca::cca_aiq_results));
    CheckAndLogError(!results, ia_err_argument, "@%s, results is nullptr", __func__);

    intel_cca_run_aiq_data* aiqParams = static_cast<intel_cca_run_aiq_data*>(mMemAIQ.mAddr);
    aiqParams->cameraId = mCameraId;
    aiqParams->tuningMode = mTuningMode;
    aiqParams->frameId = frameId;
    aiqParams->inParams = params;

    int ret = mAlgoClient->runAiq(mCameraId, mTuningMode, mMemAIQ.mHandle);
    if (ret != 0) return ia_err_general;

    *results = aiqParams->results;

    return ia_err_none;
}

ia_err IntelCca::getCMC(cca::cca_cmc* cmc) {
    CheckAndLogError(!cmc, ia_err_argument, "@%s, cmc is nullptr", __func__);

    intel_cca_get_cmc_data* params = static_cast<intel_cca_get_cmc_data*>(mMemCMC.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    int ret = mAlgoClient->getCmc(mCameraId, mTuningMode, mMemCMC.mHandle);
    if (ret != 0) return static_cast<ia_err>(ret);

    *cmc = params->results;

    return ia_err_none;
}

ia_err IntelCca::getMKN(ia_mkn_trg type, cca::cca_mkn* mkn) {
    CheckAndLogError(!mkn, ia_err_argument, "@%s, mkn is nullptr", __func__);

    intel_cca_mkn_data* params = static_cast<intel_cca_mkn_data*>(mMemMKN.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->type = type;
    params->results = mkn;
    params->resultsHandle = mAlgoClient->getShmMemHandle(mkn);

    int ret = mAlgoClient->getMkn(mCameraId, mTuningMode, mMemMKN.mHandle);

    return static_cast<ia_err>(ret);
}

ia_err IntelCca::getAiqd(cca::cca_aiqd* aiqd) {
    CheckAndLogError(!aiqd, ia_err_argument, "@%s, aiqd is nullptr", __func__);

    intel_cca_get_aiqd_data* params = static_cast<intel_cca_get_aiqd_data*>(mMemAIQD.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    int ret = mAlgoClient->getAiqd(mCameraId, mTuningMode, mMemAIQD.mHandle);
    if (ret != 0) return static_cast<ia_err>(ret);

    *aiqd = params->results;

    return ia_err_none;
}

ia_err IntelCca::updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                              const cca::cca_nvm& nvm, int32_t streamId) {
    LOG2("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    intel_cca_update_tuning_data* params =
        static_cast<intel_cca_update_tuning_data*>(mMemTuning.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;
    params->lardTags = lardTags;
    params->lardParams = lardParams;
    params->nvmParams = nvm;
    params->streamId = streamId;

    int ret = mAlgoClient->updateTuning(mCameraId, mTuningMode, mMemTuning.mHandle);

    return static_cast<ia_err>(ret);
}

void IntelCca::deinit() {
    LOG1("<id%d> @%s, tuningMode:%d", mCameraId, __func__, mTuningMode);

    intel_cca_deinit_data* params = static_cast<intel_cca_deinit_data*>(mMemDeinit.mAddr);
    params->cameraId = mCameraId;
    params->tuningMode = mTuningMode;

    mAlgoClient->deinitCca(mCameraId, mTuningMode, mMemDeinit.mHandle);
}

void* IntelCca::allocMem(int streamId, const std::string& name, int index, int size) {
    std::string number = std::to_string(streamId) + std::to_string(index) +
                         std::to_string(mCameraId) + std::to_string(mTuningMode) +
                         std::to_string(reinterpret_cast<uintptr_t>(this));
    std::string finalName = name + number + SHM_NAME;

    ShmMemInfo memInfo = {};
    bool ret = mAlgoClient->allocShmMem(finalName, size, &memInfo.mAddr, memInfo.mHandle);
    CheckAndLogError(ret == false, nullptr, "%s, allocShmMem fails for pal buf", __func__);
    LOG1("<id%d> @%s, mode:%d, name:%s, index:%d, streamId:%d, size:%d, handle: %d, addr: %p",
         mCameraId, __func__, mTuningMode, name.c_str(), index, streamId, size, memInfo.mHandle,
         memInfo.mAddr);

    memInfo.mName = finalName;
    memInfo.mSize = size;

    mMemsOuter[memInfo.mAddr] = memInfo;

    return memInfo.mAddr;
}

void IntelCca::freeMem(void* addr) {
    LOG1("<id%d> @%s, tuningMode:%d, addr: %p", mCameraId, __func__, mTuningMode, addr);

    if (mMemsOuter.find(addr) != mMemsOuter.end()) {
        mAlgoClient->freeShmMem(mMemsOuter[addr].mName, mMemsOuter[addr].mAddr,
                                mMemsOuter[addr].mHandle);
        mMemsOuter.erase(addr);
        return;
    }
    LOGW("@%s, there is no addr:%p, in the mMemsOuter", __func__, addr);
}

bool IntelCca::allocateAllShmMems(std::vector<ShmMem>* mems) {
    for (auto& it : *mems) {
        ShmMemInfo* mem = it.mem;
        mem->mName = it.name;
        mem->mSize = it.size;
        bool ret = mAlgoClient->allocShmMem(mem->mName, mem->mSize, &mem->mAddr, mem->mHandle);
        if (!ret) return false;

        it.allocated = true;
    }

    return true;
}

void IntelCca::releaseAllShmMems(const std::vector<ShmMem>& mems) {
    for (auto& it : mems) {
        if (it.allocated) {
            mAlgoClient->freeShmMem((it.mem)->mName, (it.mem)->mAddr, (it.mem)->mHandle);
        }
    }
}

} /* namespace icamera */
