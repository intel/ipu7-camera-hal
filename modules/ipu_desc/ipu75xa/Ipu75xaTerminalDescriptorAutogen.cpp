/*
* INTEL CONFIDENTIAL
* Copyright (c) 2025 Intel Corporation
* All Rights Reserved.
*
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation or its
* suppliers or licensors. Title to the Material remains with Intel
* Corporation or its suppliers and licensors. The Material may contain trade
* secrets and proprietary and confidential information of Intel Corporation
* and its suppliers and licensors, and is protected by worldwide copyright
* and trade secret laws and treaty provisions. No part of the Material may be
* used, copied, reproduced, modified, published, uploaded, posted,
* transmitted, distributed, or disclosed in any way without Intel's prior
* express written permission.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Intel in writing.
*
* Unless otherwise agreed by Intel in writing, you may not remove or alter
* this notice or any other notice embedded in Materials by Intel or Intels
* suppliers or licensors in any way.
*/

#include "Ipu75xaTerminalDescriptorAutogen.h"

TerminalDescriptor SW_ISYSTerminalDesc[] =
{
    {
        SW_ISYS_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        11470, // is_odr_a
    },
    {
        SW_ISYS_TERMINAL_CONNECT_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        11470, // is_odr_a
    },
    {
        SW_ISYS_TERMINAL_CONNECT_INPUT_PDAF,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT_PDAF",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        55449, // is_odr_b
    },
    {
        SW_ISYS_TERMINAL_CONNECT_OUTPUT_PDAF,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_PDAF",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        55449, // is_odr_b
    },
    {
        SW_ISYS_TERMINAL_CONNECT_INPUT_DOL_LONG,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT_DOL_LONG",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        50407, // is_odr_c
    },
    {
        SW_ISYS_TERMINAL_CONNECT_OUTPUT_DOL_LONG,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_DOL_LONG",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        50407, // is_odr_c
    },
};

TerminalDescriptor LBFFTerminalDesc[] =
{
    {
        LBFF_TERMINAL_LOAD_ALGO_CACHED,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_ALGO_CACHED",
        PAC_BUFFER_TYPE_PARAM_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        LBFF_TERMINAL_LOAD_ALGO_FRAG_SEQ,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_ALGO_FRAG_SEQ",
        PAC_BUFFER_TYPE_PROGRAM,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        LBFF_TERMINAL_LOAD_SYSTEM,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SYSTEM",
        PAC_BUFFER_TYPE_SYS_FRAG_SEQUENCER,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        LBFF_TERMINAL_LOAD_SR_FRAME_IN,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SR_FRAME_IN",
        PAC_BUFFER_TYPE_SR_FRAME_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN_OUT,
        0,
    },
    {
        LBFF_TERMINAL_LOAD_SR_FRAG_SEQ,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SR_FRAG_SEQ",
        PAC_BUFFER_TYPE_SR_FRAG_SEQUENCER,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN_OUT,
        0,
    },
    {
        LBFF_TERMINAL_CONNECT_MAIN_DATA_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_MAIN_DATA_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        55223, // ifd_pipe_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_DOL_LONG,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_DOL_LONG",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        52982, // ifd_pipe_long_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_DOL_SHORT_SMTH,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_DOL_SHORT_SMTH",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        49695, // ifd_pipe_short_smth_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_LSC_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_LSC_INPUT",
        PAC_BUFFER_TYPE_SPATIAL_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        27730, // ifd_lsc_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_PDAF_DATA_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_PDAF_DATA_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        6874, // ifd_pdaf_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AE_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AE_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        53496, // odr_ae_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AF_STD_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AF_STD_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        23958, // odr_af_std_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AWB_STD_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AWB_STD_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        1338, // odr_awb_std_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AWB_SAT_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AWB_SAT_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        45123, // odr_awb_sat_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_PDAF_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_PDAF_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        24208, // odr_pdaf_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_IR_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_IR_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        28176, // odr_ir_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_BURST_ISP_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_BURST_ISP_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        32658, // odr_burst_isp_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_BNLM_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_BNLM_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        56904, // odr_bnlm_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_ME_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_ME_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        59680, // odr_output_me_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_PS_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_PS_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        38648, // odr_output_ps_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_GMV_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_GMV_INPUT",
        PAC_BUFFER_TYPE_SPATIAL_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        62409, // ifd_gmv_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AWB_SVE_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AWB_SVE_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        8720, // odr_awb_sve_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_AF_SVE_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_AF_SVE_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        0, // No graphs found
    },
    {
        LBFF_TERMINAL_CONNECT_GMV_MATCH_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_GMV_MATCH_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        32160, // odr_gmv_match_1_3
    },
    {
        LBFF_TERMINAL_CONNECT_GMV_FEATURE_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_GMV_FEATURE_OUTPUT",
        PAC_BUFFER_TYPE_SPATIAL_OUT,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        55924, // odr_gmv_feature_1_3
    },
};

TerminalDescriptor BBPSTerminalDesc[] =
{
    {
        BBPS_TERMINAL_LOAD_ALGO_CACHED,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_ALGO_CACHED",
        PAC_BUFFER_TYPE_PARAM_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        BBPS_TERMINAL_LOAD_ALGO_FRAG_SEQ,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_ALGO_FRAG_SEQ",
        PAC_BUFFER_TYPE_PROGRAM,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        BBPS_TERMINAL_LOAD_SYSTEM,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SYSTEM",
        PAC_BUFFER_TYPE_SYS_FRAG_SEQUENCER,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        0,
    },
    {
        BBPS_TERMINAL_LOAD_SR_FRAME_IN,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SR_FRAME_IN",
        PAC_BUFFER_TYPE_SR_FRAME_IN,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN_OUT,
        0,
    },
    {
        BBPS_TERMINAL_LOAD_SR_FRAG_SEQ,
        TERMINAL_TYPE_LOAD,
        "TERMINAL_LOAD_SR_FRAG_SEQ",
        PAC_BUFFER_TYPE_SR_FRAG_SEQUENCER,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN_OUT,
        0,
    },
    {
        BBPS_TERMINAL_CONNECT_SLIM_TNR_BC_YUV4NM1_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_SLIM_TNR_BC_YUV4NM1_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        25579, // slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_SLIM_TNR_BC_RS4NM1_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_SLIM_TNR_BC_RS4NM1_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        48078, // slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_TNR_BC_YUV4N_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_TNR_BC_YUV4N_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        57803, // tnr_sp_bc_bifd_yuv4n_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_TNR_BC_RS4N_ODR,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_TNR_BC_RS4N_ODR",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_OUT,
        39096, // tnr_sp_bc_bodr_rs4n_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_SLIM_SPATIAL_YUVN_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_SLIM_SPATIAL_YUVN_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        6907, // slim_tnr_spatial_bifd_yuvn_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_SLIM_TNR_BLEND_YUVNM1_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_SLIM_TNR_BLEND_YUVNM1_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        26536, // slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_TNR_BLEND_RS4N_IFD,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_TNR_BLEND_RS4N_IFD",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_METADATA,
        TERMINAL_DIR_IN,
        42936, // tnr_fp_blend_bifd_rs4n_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_TNR_BLEND_YUVN_ODR,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_TNR_BLEND_YUVN_ODR",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        38465, // tnr_fp_bodr_yuvn_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_TNR_SCALE_YUV4N_ODR,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_TNR_SCALE_YUV4N_ODR",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        20865, // tnr_scale_fp_bodr_yuv4n_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_OFS_MP_YUVN_ODR,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OFS_MP_YUVN_ODR",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        18789, // ofs_mp_bodr_regs_1_3
    },
    {
        BBPS_TERMINAL_CONNECT_OFS_DP_YUVN_ODR,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OFS_DP_YUVN_ODR",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        27847, // ofs_dp_bodr_regs_1_3
    },
};

TerminalDescriptor SW_GDCTerminalDesc[] =
{
    {
        SW_GDC_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        5637, // gdc7_1
    },
    {
        SW_GDC_TERMINAL_CONNECT_OUTPUT_1,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_1",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        0, // 
    },
    {
        SW_GDC_TERMINAL_CONNECT_OUTPUT_2,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_2",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        0, // 
    },
};

TerminalDescriptor SW_SCALERTerminalDesc[] =
{
    {
        SW_SCALER_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        19706, // sw_scaler
    },
    {
        SW_SCALER_TERMINAL_CONNECT_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        19706, // sw_scaler
    },
};

TerminalDescriptor SW_NNTMTerminalDesc[] =
{
    {
        SW_NNTM_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        46539, // nntm_1_0
    },
    {
        SW_NNTM_TERMINAL_CONNECT_OUTPUT_1,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_1",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        0, // 
    },
    {
        SW_NNTM_TERMINAL_CONNECT_OUTPUT_2,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_2",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        0, // 
    },
};

TerminalDescriptor SW_IMVTerminalDesc[] =
{
    {
        SW_IMV_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        33331, // imv
    },
    {
        SW_IMV_TERMINAL_CONNECT_OUTPUT_1,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_1",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        0, // 
    },
    {
        SW_IMV_TERMINAL_CONNECT_OUTPUT_2,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT_2",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        19706, // sw_scaler
    },
};

TerminalDescriptor SW_B2BTerminalDesc[] =
{
    {
        SW_B2B_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        45372, // b2b
    },
    {
        SW_B2B_TERMINAL_CONNECT_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        45372, // b2b
    },
};

TerminalDescriptor SW_REMOSAICTerminalDesc[] =
{
    {
        SW_REMOSAIC_TERMINAL_CONNECT_INPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_INPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_IN,
        19706, // sw_scaler
    },
    {
        SW_REMOSAIC_TERMINAL_CONNECT_OUTPUT,
        TERMINAL_TYPE_CONNECT,
        "TERMINAL_CONNECT_OUTPUT",
        PAC_BUFFER_TYPE_NONE,
        TERMINAL_BUFFER_TYPE_DATA,
        TERMINAL_DIR_OUT,
        19706, // sw_scaler
    },
};

int CountOfSW_ISYSTerminalDesc = sizeof(SW_ISYSTerminalDesc) / sizeof(SW_ISYSTerminalDesc[0]);
int CountOfLBFFTerminalDesc = sizeof(LBFFTerminalDesc) / sizeof(LBFFTerminalDesc[0]);
int CountOfBBPSTerminalDesc = sizeof(BBPSTerminalDesc) / sizeof(BBPSTerminalDesc[0]);
int CountOfSW_GDCTerminalDesc = sizeof(SW_GDCTerminalDesc) / sizeof(SW_GDCTerminalDesc[0]);
int CountOfSW_SCALERTerminalDesc = sizeof(SW_SCALERTerminalDesc) / sizeof(SW_SCALERTerminalDesc[0]);
int CountOfSW_NNTMTerminalDesc = sizeof(SW_NNTMTerminalDesc) / sizeof(SW_NNTMTerminalDesc[0]);
int CountOfSW_IMVTerminalDesc = sizeof(SW_IMVTerminalDesc) / sizeof(SW_IMVTerminalDesc[0]);
int CountOfSW_B2BTerminalDesc = sizeof(SW_B2BTerminalDesc) / sizeof(SW_B2BTerminalDesc[0]);
int CountOfSW_REMOSAICTerminalDesc = sizeof(SW_REMOSAICTerminalDesc) / sizeof(SW_REMOSAICTerminalDesc[0]);
