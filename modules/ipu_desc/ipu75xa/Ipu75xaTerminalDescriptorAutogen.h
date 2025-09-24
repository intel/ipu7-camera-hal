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
#pragma once

typedef enum _TerminalType
{
    TERMINAL_TYPE_LOAD,
    TERMINAL_TYPE_CONNECT,
    TERMINAL_TYPE_N
} TerminalType;

typedef enum _PacBufferType
{
    PAC_BUFFER_TYPE_PARAM_IN,
    PAC_BUFFER_TYPE_PARAM_OUT,
    PAC_BUFFER_TYPE_PROGRAM,
    PAC_BUFFER_TYPE_SPATIAL_IN,
    PAC_BUFFER_TYPE_SPATIAL_OUT,
    PAC_BUFFER_TYPE_FRAME_IN,
    PAC_BUFFER_TYPE_FRAME_OUT,
    PAC_BUFFER_TYPE_SYS_FRAG_SEQUENCER,
    PAC_BUFFER_TYPE_SR_FRAME_IN,
    PAC_BUFFER_TYPE_SR_FRAG_SEQUENCER,
    PAC_BUFFER_TYPE_NONE,
} PacBufferType;

typedef enum _TerminalBufferType
{
    TERMINAL_BUFFER_TYPE_DATA,
    TERMINAL_BUFFER_TYPE_METADATA,
    TERMINAL_BUFFER_TYPE_N
} TerminalBufferType;

typedef enum _TerminalDirection
{
    TERMINAL_DIR_IN,
    TERMINAL_DIR_OUT,
    TERMINAL_DIR_IN_OUT,
    TERMINAL_DIR_N
} TerminalDirection;

typedef struct _TerminalDescriptor
{
    int                 TerminalId;
    TerminalType        TerminalType;
    const char         *TerminalName;
    PacBufferType       PacBufferType;
    TerminalBufferType  TerminalBufferType;
    TerminalDirection   TerminalDirection;
    int                 TerminalLinkedKernel;
} TerminalDescriptor;

enum SW_ISYSTerminalID
{
    SW_ISYS_TERMINAL_CONNECT_INPUT,
    SW_ISYS_TERMINAL_CONNECT_OUTPUT,
    SW_ISYS_TERMINAL_CONNECT_INPUT_PDAF,
    SW_ISYS_TERMINAL_CONNECT_OUTPUT_PDAF,
    SW_ISYS_TERMINAL_CONNECT_INPUT_DOL_LONG,
    SW_ISYS_TERMINAL_CONNECT_OUTPUT_DOL_LONG,
};

enum LBFFTerminalID
{
    LBFF_TERMINAL_LOAD_ALGO_CACHED,
    LBFF_TERMINAL_LOAD_ALGO_FRAG_SEQ,
    LBFF_TERMINAL_LOAD_SYSTEM,
    LBFF_TERMINAL_LOAD_SR_FRAME_IN,
    LBFF_TERMINAL_LOAD_SR_FRAG_SEQ,
    LBFF_TERMINAL_CONNECT_MAIN_DATA_INPUT,
    LBFF_TERMINAL_CONNECT_DOL_LONG,
    LBFF_TERMINAL_CONNECT_DOL_SHORT_SMTH,
    LBFF_TERMINAL_CONNECT_LSC_INPUT,
    LBFF_TERMINAL_CONNECT_PDAF_DATA_INPUT,
    LBFF_TERMINAL_CONNECT_AE_OUTPUT,
    LBFF_TERMINAL_CONNECT_AF_STD_OUTPUT,
    LBFF_TERMINAL_CONNECT_AWB_STD_OUTPUT,
    LBFF_TERMINAL_CONNECT_AWB_SAT_OUTPUT,
    LBFF_TERMINAL_CONNECT_PDAF_OUTPUT,
    LBFF_TERMINAL_CONNECT_IR_OUTPUT,
    LBFF_TERMINAL_CONNECT_BURST_ISP_OUTPUT,
    LBFF_TERMINAL_CONNECT_BNLM_OUTPUT,
    LBFF_TERMINAL_CONNECT_ME_OUTPUT,
    LBFF_TERMINAL_CONNECT_PS_OUTPUT,
    LBFF_TERMINAL_CONNECT_GMV_INPUT,
    LBFF_TERMINAL_CONNECT_AWB_SVE_OUTPUT,
    LBFF_TERMINAL_CONNECT_AF_SVE_OUTPUT,
    LBFF_TERMINAL_CONNECT_GMV_MATCH_OUTPUT,
    LBFF_TERMINAL_CONNECT_GMV_FEATURE_OUTPUT,
};

enum BBPSTerminalID
{
    BBPS_TERMINAL_LOAD_ALGO_CACHED,
    BBPS_TERMINAL_LOAD_ALGO_FRAG_SEQ,
    BBPS_TERMINAL_LOAD_SYSTEM,
    BBPS_TERMINAL_LOAD_SR_FRAME_IN,
    BBPS_TERMINAL_LOAD_SR_FRAG_SEQ,
    BBPS_TERMINAL_CONNECT_SLIM_TNR_BC_YUV4NM1_IFD,
    BBPS_TERMINAL_CONNECT_SLIM_TNR_BC_RS4NM1_IFD,
    BBPS_TERMINAL_CONNECT_TNR_BC_YUV4N_IFD,
    BBPS_TERMINAL_CONNECT_TNR_BC_RS4N_ODR,
    BBPS_TERMINAL_CONNECT_SLIM_SPATIAL_YUVN_IFD,
    BBPS_TERMINAL_CONNECT_SLIM_TNR_BLEND_YUVNM1_IFD,
    BBPS_TERMINAL_CONNECT_TNR_BLEND_RS4N_IFD,
    BBPS_TERMINAL_CONNECT_TNR_BLEND_YUVN_ODR,
    BBPS_TERMINAL_CONNECT_TNR_SCALE_YUV4N_ODR,
    BBPS_TERMINAL_CONNECT_OFS_MP_YUVN_ODR,
    BBPS_TERMINAL_CONNECT_OFS_DP_YUVN_ODR,
};

enum SW_GDCTerminalID
{
    SW_GDC_TERMINAL_CONNECT_INPUT,
    SW_GDC_TERMINAL_CONNECT_OUTPUT_1,
    SW_GDC_TERMINAL_CONNECT_OUTPUT_2,
};

enum SW_SCALERTerminalID
{
    SW_SCALER_TERMINAL_CONNECT_INPUT,
    SW_SCALER_TERMINAL_CONNECT_OUTPUT,
};

enum SW_NNTMTerminalID
{
    SW_NNTM_TERMINAL_CONNECT_INPUT,
    SW_NNTM_TERMINAL_CONNECT_OUTPUT_1,
    SW_NNTM_TERMINAL_CONNECT_OUTPUT_2,
};

enum SW_IMVTerminalID
{
    SW_IMV_TERMINAL_CONNECT_INPUT,
    SW_IMV_TERMINAL_CONNECT_OUTPUT_1,
    SW_IMV_TERMINAL_CONNECT_OUTPUT_2,
};

enum SW_B2BTerminalID
{
    SW_B2B_TERMINAL_CONNECT_INPUT,
    SW_B2B_TERMINAL_CONNECT_OUTPUT,
};

enum SW_REMOSAICTerminalID
{
    SW_REMOSAIC_TERMINAL_CONNECT_INPUT,
    SW_REMOSAIC_TERMINAL_CONNECT_OUTPUT,
};

extern TerminalDescriptor SW_ISYSTerminalDesc[];
extern TerminalDescriptor LBFFTerminalDesc[];
extern TerminalDescriptor BBPSTerminalDesc[];
extern TerminalDescriptor SW_GDCTerminalDesc[];
extern TerminalDescriptor SW_SCALERTerminalDesc[];
extern TerminalDescriptor SW_NNTMTerminalDesc[];
extern TerminalDescriptor SW_IMVTerminalDesc[];
extern TerminalDescriptor SW_B2BTerminalDesc[];
extern TerminalDescriptor SW_REMOSAICTerminalDesc[];

extern int CountOfSW_ISYSTerminalDesc;
extern int CountOfLBFFTerminalDesc;
extern int CountOfBBPSTerminalDesc;
extern int CountOfSW_GDCTerminalDesc;
extern int CountOfSW_SCALERTerminalDesc;
extern int CountOfSW_NNTMTerminalDesc;
extern int CountOfSW_IMVTerminalDesc;
extern int CountOfSW_B2BTerminalDesc;
extern int CountOfSW_REMOSAICTerminalDesc;
