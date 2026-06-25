/*
 * Copyright (C) 2020-2025 Intel Corporation.
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

#pragma once

#include <IntelCCA.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

#include "iutils/Thread.h"
#include "CameraTypes.h"

namespace icamera {
class IntelCca {
 public:
    IntelCca(int cameraId, TuningMode mode);
    virtual ~IntelCca();

    IntelCca(const IntelCca&) = delete;
    IntelCca& operator=(const IntelCca&) = delete;

    static IntelCca* getInstance(int cameraId, TuningMode mode);
    static void releaseInstance(int cameraId, TuningMode mode);

    ia_err init(const cca::cca_init_params& initParams);
    ia_err reinitAic(const int32_t aicId);

    ia_err setStatsParams(const cca::cca_stats_params& params);

    ia_err runAEC(uint64_t frameId, const cca::cca_ae_input_params& params,
                  cca::cca_ae_results* results);
    ia_err runAIQ(uint64_t frameId, const cca::cca_aiq_params& params,
                  cca::cca_aiq_results* results);

    ia_err updateTuning(uint8_t lardTags, const ia_lard_input_params& lardParams,
                        const cca::cca_nvm& nvm, int32_t streamId);

    ia_err getCMC(cca::cca_cmc* cmc);
    ia_err getMKN(ia_mkn_trg type, cca::cca_mkn* mkn);
    ia_err getAiqd(cca::cca_aiqd* aiqd);

    void* allocMem(int streamId, const std::string& name, int index, int size);
    void freeMem(void* addr);

    void deinit();

    ia_err configAic(const cca::cca_aic_config& aicConf,
                     const cca::cca_aic_kernel_offset& kernelOffset, uint32_t* offsetPtr,
                     cca::cca_aic_terminal_config& termConfig, int32_t aicId,
                     const int32_t* statsBufToTermIds);
    ia_err registerAicBuf(const cca::cca_aic_terminal_config& termConfig, int32_t aicId);
    ia_err getAicBuf(cca::cca_aic_terminal_config& termConfig, int32_t aicId);
    ia_err decodeStats(int32_t groupId, int64_t sequence, int32_t aicId,
                       cca::cca_out_stats* outStats);
    ia_err runAIC (uint64_t frameId, const cca::cca_pal_input_params* params,
                   uint8_t bitmap, int32_t aicId);

    ia_err updateConfigurationResolutions(const cca::cca_aic_config& aicConf,
                                          int32_t aicId, bool isKeyResChanged);

 private:
    cca::IntelCCA* getIntelCCA();
    void releaseIntelCCA();

     int mCameraId;
     TuningMode mTuningMode;

    struct CCAHandle {
        int cameraId;
        std::unordered_map<TuningMode, IntelCca*> ccaHandle;  // TuningMode to IntelCca map
    };
    static std::vector<CCAHandle> sCcaInstance;
    static Mutex sLock;

    cca::IntelCCA* mIntelCCA;
};
} /* namespace icamera */
