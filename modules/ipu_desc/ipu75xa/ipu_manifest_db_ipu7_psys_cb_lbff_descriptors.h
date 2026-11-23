/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2024 Intel Corporation
 * All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.The Material may contain trade
 * secrets and proprietary and confidential information of Intel Corporation
 * and its suppliers and licensors, and is protected by worldwide copyright
 * and trade secret laws and treaty provisions.No part of the Material may be
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

#ifndef IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DESCRIPTORS_H
#define IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DESCRIPTORS_H

#include <stdint.h>
#include "cb_payload_descriptor.h"

static payload_descriptor_t lbff_0_descriptors = {
	60,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_1, 0x0U, 0x30U, 0x0U},
		{LBFF_DEVICE_ID_DOL_lite_1_1, 0x100U, 0xacU, 0x30U},
		{LBFF_DEVICE_ID_BXT_BLC, 0x0U, 0x24U, 0xdcU},
		{LBFF_DEVICE_ID_Linearization2_0, 0x0U, 0x2174U, 0x100U},
		{LBFF_DEVICE_ID_LSC_1_2, 0x8U, 0x134U, 0x2274U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x0U, 0x334U, 0x23a8U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x43cU, 0x44U, 0x26dcU},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x4bcU, 0x8cU, 0x2720U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0xa000U, 0x4U, 0x27acU},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x0U, 0x4U, 0x27b0U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0xcU, 0x8U, 0x27b4U},
		{LBFF_DEVICE_ID_PAFStatistics_1_2, 0x0U, 0x19cU, 0x27bcU},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x0U, 0x38U, 0x2958U},
		{LBFF_DEVICE_ID_CCM_3A_2_0, 0x0U, 0x44U, 0x2990U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x0U, 0x20U, 0x29d4U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x0U, 0x2cU, 0x29f4U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x30U, 0x8U, 0x2a20U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x3cU, 0x30U, 0x2a28U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x0U, 0x1cU, 0x2a58U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x24U, 0x1dcU, 0x2a74U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x204U, 0x2c8U, 0x2c50U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x4d0U, 0x14U, 0x2f18U},
		{LBFF_DEVICE_ID_WB_1_1, 0x0U, 0x30U, 0x2f2cU},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x0U, 0x2d4U, 0x2f5cU},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x2e4U, 0x100U, 0x3230U},
		{LBFF_DEVICE_ID_BXT_Demosaic, 0x0U, 0x34U, 0x3330U},
		{LBFF_DEVICE_ID_VCSC_2_0_b, 0x0U, 0x24U, 0x3364U},
		{LBFF_DEVICE_ID_GLTM_2_0, 0x0U, 0x33cU, 0x3388U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x4U, 0xcU, 0x36c4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x20U, 0x3cU, 0x36d0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x1004U, 0x18U, 0x370cU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x102cU, 0x1a8U, 0x3724U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x2000U, 0x180U, 0x38ccU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x3004U, 0x38U, 0x3a4cU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x304cU, 0x19cU, 0x3a84U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x4000U, 0x180U, 0x3c20U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5004U, 0x4U, 0x3da0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5020U, 0x1cU, 0x3da4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x504cU, 0x180U, 0x3dc0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x6000U, 0x180U, 0x3f40U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7004U, 0x1cU, 0x40c0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7024U, 0x1cU, 0x40dcU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x8004U, 0x48U, 0x40f8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x805cU, 0x148U, 0x4140U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x9000U, 0x180U, 0x4288U},
		{LBFF_DEVICE_ID_VCR_3_1, 0x0U, 0x4U, 0x4408U},
		{LBFF_DEVICE_ID_VCR_3_1, 0x40U, 0x20U, 0x440cU},
		{LBFF_DEVICE_ID_GLIM_2_0, 0x0U, 0x328U, 0x442cU},
		{LBFF_DEVICE_ID_ACM_1_1, 0x0U, 0x444U, 0x4754U},
		{LBFF_DEVICE_ID_GammaTM_V4, 0x0U, 0x4U, 0x4b98U},
		{LBFF_DEVICE_ID_GammaTM_V4, 0x8U, 0x1c64U, 0x4b9cU},
		{LBFF_DEVICE_ID_GammaTM_V4, 0x1c70U, 0xcU, 0x6800U},
		{LBFF_DEVICE_ID_CSC_1_1, 0x0U, 0x24U, 0x680cU},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x0U, 0x8U, 0x6830U},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x1cU, 0x208U, 0x6838U},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x0U, 0x4U, 0x6a40U},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x14U, 0x40cU, 0x6a44U},
		{LBFF_DEVICE_ID_TNR_Scale_LB, 0x0U, 0x4U, 0x6e50U},
		{LBFF_DEVICE_ID_TNR_Scale_LB, 0x40U, 0x2cU, 0x6e54U},
		{LBFF_DEVICE_ID_GMV_Statistics_1_0, 0x0U, 0x1cU, 0x6e80U},

	}

};

static payload_descriptor_t lbff_1_descriptors = {
	26,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_1, 0xa0U, 0x4U, 0x0U},
		{LBFF_DEVICE_ID_LSC_1_2, 0x0U, 0x8U, 0x4U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x338U, 0x104U, 0xcU},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x488U, 0x8U, 0x110U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x4U, 0x4U, 0x118U},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x40U, 0xcU, 0x11cU},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x24U, 0x180U, 0x128U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x1c0U, 0xcU, 0x2a8U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x80U, 0xcU, 0x2b4U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x1cU, 0x8U, 0x2c0U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x200U, 0x4U, 0x2c8U},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x2d8U, 0xcU, 0x2ccU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x0U, 0x4U, 0x2d8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x10U, 0x10U, 0x2dcU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x1000U, 0x4U, 0x2ecU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x101cU, 0x10U, 0x2f0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x3000U, 0x4U, 0x300U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x303cU, 0x10U, 0x304U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5000U, 0x4U, 0x314U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x503cU, 0x10U, 0x318U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7000U, 0x4U, 0x328U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x8000U, 0x4U, 0x32cU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x804cU, 0x10U, 0x330U},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x8U, 0x10U, 0x340U},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x4U, 0x10U, 0x350U},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x1010U, 0xcU, 0x360U},

	}

};

static payload_descriptor_t lbff_2_descriptors = {
	22,
	{
		{LBFF_DEVICE_ID_ifd_pipe_1_3, 0x0U, 0x9cU, 0x0U},
		{LBFF_DEVICE_ID_ifd_pipe_long_1_3, 0x0U, 0x9cU, 0x9cU},
		{LBFF_DEVICE_ID_ifd_pipe_short_smth_1_3, 0x0U, 0x9cU, 0x138U},
		{LBFF_DEVICE_ID_ifd_lsc_1_3, 0x0U, 0x9cU, 0x1d4U},
		{LBFF_DEVICE_ID_ifd_pdaf_1_3, 0x0U, 0x9cU, 0x270U},
		{LBFF_DEVICE_ID_odr_awb_std_1_3, 0x0U, 0x9cU, 0x30cU},
		{LBFF_DEVICE_ID_odr_awb_sat_1_3, 0x0U, 0x9cU, 0x3a8U},
		{LBFF_DEVICE_ID_odr_awb_sve_1_3, 0x0U, 0x9cU, 0x444U},
		{LBFF_DEVICE_ID_odr_ae_1_3, 0x0U, 0x9cU, 0x4e0U},
		{LBFF_DEVICE_ID_odr_af_std_1_3, 0x0U, 0x9cU, 0x57cU},
		{LBFF_DEVICE_ID_odr_af_sve_1_3, 0x0U, 0x9cU, 0x618U},
		{LBFF_DEVICE_ID_odr_pdaf_1_3, 0x0U, 0x9cU, 0x6b4U},
		{LBFF_DEVICE_ID_odr_ir_1_3, 0x0U, 0x9cU, 0x750U},
		{LBFF_DEVICE_ID_odr_burst_isp_1_3, 0x0U, 0x9cU, 0x7ecU},
		{LBFF_DEVICE_ID_odr_bnlm_1_3, 0x0U, 0x9cU, 0x888U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_3, 0x0U, 0xcU, 0x924U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_3, 0x4cU, 0x14U, 0x930U},
		{LBFF_DEVICE_ID_odr_output_ps_1_3, 0x0U, 0x9cU, 0x944U},
		{LBFF_DEVICE_ID_odr_output_me_1_3, 0x0U, 0x9cU, 0x9e0U},
		{LBFF_DEVICE_ID_ifd_gmv_1_3, 0x0U, 0x9cU, 0xa7cU},
		{LBFF_DEVICE_ID_odr_gmv_feature_1_3, 0x0U, 0x9cU, 0xb18U},
		{LBFF_DEVICE_ID_odr_gmv_match_1_3, 0x0U, 0x9cU, 0xbb4U},

	}

};

static payload_descriptor_t lbff_3_descriptors = {
	1,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_1, 0x200U, 0x164U, 0x0U},

	}

};

#endif
