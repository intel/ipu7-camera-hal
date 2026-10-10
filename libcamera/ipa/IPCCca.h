/*
 * Copyright (C) 2024 Intel Corporation.
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

#include "IntelCCA.h"

namespace icamera {

struct intel_cca_struct_data {
    int cameraId;
    int tuningMode;
};

struct intel_cca_init_data {
    int cameraId;
    int tuningMode;

    cca::cca_init_params inParams;
};

struct intel_cca_reinit_aic_data {
    int cameraId;
    int tuningMode;

    uint32_t aicId;
};

struct intel_cca_set_stats_data {
    int cameraId;
    int tuningMode;

    cca::cca_stats_params inParams;
};

struct intel_cca_run_aec_data {
    int cameraId;
    int tuningMode;

    uint64_t frameId;
    cca::cca_ae_input_params inParams;

    cca::cca_ae_results results;
};

struct intel_cca_run_aiq_data {
    int cameraId;
    int tuningMode;

    uint64_t frameId;
    cca::cca_aiq_params inParams;

    cca::cca_aiq_results results;
};

// See systemApiConfiguration in StaticGraphAutogen.h
#define MAX_SYSTEM_API_DATA_SIZE_IN_PG (8092)

typedef struct {
    aic::ImagingKernelGroup kernelGroup;

    // Save aic::ImagingKernelGroup::kernelList
    aic::ia_pac_kernel_info kernelList[cca::MAX_KERNEL_NUM_IN_PG];

    // Save aic::ia_pac_kernel_info::fragment_descs
    aic::IaAicFragmentDesc fragmentDescs[cca::MAX_KERNEL_NUM_IN_PG];

    // Save related memebrs in aic::ia_pac_kernel_info::ia_isp_bxt_run_kernels_t
    ia_isp_bxt_resolution_info_t resolutionInfo[cca::MAX_KERNEL_NUM_IN_PG];
    ia_isp_bxt_resolution_info_t resolutionHistory[cca::MAX_KERNEL_NUM_IN_PG];
    uint8_t systemApiData[MAX_SYSTEM_API_DATA_SIZE_IN_PG];
} intel_cca_cb_config_data;

typedef struct {
    uint32_t cbNum;
    cca::cca_cb_config cbConfig[cca::MAX_PG_NUM];

    intel_cca_cb_config_data data[cca::MAX_PG_NUM];
} ipc_cca_aic_config;

typedef struct {
    int uuid;
    aic::IaAicBufferTypes terminal_type;
    uint32_t terminal_index;
    uint32_t offsets;
    uint32_t sizes;
    uint32_t num_offsets;
    uint32_t fragment;
} ipc_cca_kernel_offset;

typedef struct {
    int group_id;
    uint32_t num_kernels;
    ipc_cca_kernel_offset kernels_offset[cca::MAX_KERNEL_NUM_IN_PG];
} ipc_cca_cb_kernel_offset;

typedef struct {
    uint32_t cb_num;
    ipc_cca_cb_kernel_offset cb_kernel_offset[cca::MAX_PG_NUM];
    uint32_t offsetHandle;
    uint32_t* offsetBuffer;
} ipc_cca_aic_kernel_offset;

typedef struct {
    uint32_t terminal_index;
    size_t buf_size;
    aic::IaAicBuffer payload;
    uint32_t fragment_index;
    uint32_t payloadHandle;
    void* payloadServerAddr;
} ipc_cca_terminal_buf;

typedef struct {
    int group_id;
    uint32_t num_terminal;
    ipc_cca_terminal_buf terminal_buf[cca::MAX_PG_TERMINAL_NUM];
} ipc_cca_cb_termal_buf;

typedef struct {
    uint32_t cb_num;
    ipc_cca_cb_termal_buf cb_terminal_buf[cca::MAX_PG_NUM];
} ipc_cca_aic_terminal_config;

struct intel_cca_aic_control_data {
    int cameraId;
    int tuningMode;

    ipc_cca_aic_config config;
    ipc_cca_aic_kernel_offset kernelOffset;
    ipc_cca_aic_terminal_config termConfig;
    int32_t aicId;
    int32_t statsBufToTermIds[NUM_STATISTICS_BUFFER_TYPES];

    bool isKeyResChanged;
};

struct intel_cca_run_aic_data {
    int cameraId;
    int tuningMode;

    uint64_t frameId;
    cca::cca_pal_input_params* inParams;
    int32_t inParamsHandle;
    int32_t aicId;
    uint32_t bitmap;
};

struct intel_cca_get_cmc_data {
    int cameraId;
    int tuningMode;

    cca::cca_cmc results;
};

struct intel_cca_get_aiqd_data {
    int cameraId;
    int tuningMode;

    cca::cca_aiqd results;
};

struct intel_cca_mkn_data {
    int cameraId;
    int tuningMode;

    ia_mkn_trg type;

    cca::cca_mkn* results;
    int32_t resultsHandle;
};

struct intel_cca_update_tuning_data {
    int cameraId;
    int tuningMode;

    uint8_t lardTags;
    ia_lard_input_params lardParams;
    cca::cca_nvm nvmParams;
    int32_t streamId;
};

struct intel_cca_deinit_data {
    int cameraId;
    int tuningMode;
};

struct intel_cca_decode_stats_data {
    int cameraId;
    int tuningMode;

    int32_t groupId;
    int32_t aicId;
    int32_t statsHandle;
    uint32_t bitmap;
    int64_t sequence;
    ia_binary_data statsBuffer;
    cca::cca_out_stats outStats;
    ia_isp_bxt_statistics_query_results_t results;
};

struct intel_cca_get_pal_data_size {
    int cameraId;
    int tuningMode;

    cca::cca_program_group pg;

    uint32_t returnSize;
};

class IPCCca {
 public:
    IPCCca() {}
    virtual ~IPCCca() {}

    bool clientFlattenConfigAic(void* pData, uint32_t size, const cca::cca_aic_config& aicConf,
                                const cca::cca_aic_kernel_offset& kernelOffset,
                                cca::cca_aic_terminal_config& termConfig, int32_t aicId,
                                const int32_t* statsBufToTermIds);
    bool serverUnflattenConfigAic(void* pData, uint32_t size, cca::cca_aic_config& aicConf,
                                  cca::cca_aic_kernel_offset& kernelOffset,
                                  cca::cca_aic_terminal_config& termConfig, int32_t& aicId,
                                  int32_t*& statsBufToTermIds);
    void unFlattenTerminalConfig(ipc_cca_aic_terminal_config& terminalConfig,
                                 cca::cca_aic_terminal_config& termConfig);
    void flattenTerminalConfig(ipc_cca_aic_terminal_config& terminalConfig,
                               const cca::cca_aic_terminal_config& termConfig);
    bool clientFlattenUpdateCfgRes(void* pData, uint32_t size,
                                   const cca::cca_aic_config& aicConf, int32_t aicId,
                                   bool isKeyResChanged);
    bool serverUnflattenUpdateCfgRes(void* pData, uint32_t size,
                                     cca::cca_aic_config& aicConf, int32_t& aicId,
                                     bool& isKeyResChanged);
 private:
    void flattenAicConfig(const cca::cca_aic_config& aicCfg, ipc_cca_aic_config* ipcCfg);
    void unflattenAicConfig(ipc_cca_aic_config& ipcCfg, cca::cca_aic_config& aicCfg);
};
} /* namespace icamera */
