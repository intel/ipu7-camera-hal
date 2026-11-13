/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2023 Intel Corporation
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
	58,
	{
		{LBFF_DEVICE_ID_BXT_BLC, 0x0U, 0x24U, 0x0U},
		{LBFF_DEVICE_ID_Linearization2_0, 0x0U, 0x2174U, 0x24U},
		{LBFF_DEVICE_ID_LSC_1_2, 0x8U, 0x134U, 0x2198U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x0U, 0x334U, 0x22ccU},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x43cU, 0x44U, 0x2600U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x4bcU, 0x8cU, 0x2644U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0xa000U, 0x4U, 0x26d0U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x0U, 0x4U, 0x26d4U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0xcU, 0x8U, 0x26d8U},
		{LBFF_DEVICE_ID_PAFStatistics_1_2, 0x0U, 0x19cU, 0x26e0U},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x0U, 0x38U, 0x287cU},
		{LBFF_DEVICE_ID_CCM_3A_2_0, 0x0U, 0x44U, 0x28b4U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x0U, 0x20U, 0x28f8U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x0U, 0x2cU, 0x2918U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x30U, 0x8U, 0x2944U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x3cU, 0x30U, 0x294cU},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x0U, 0x1cU, 0x297cU},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x24U, 0x1dcU, 0x2998U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x204U, 0x2c8U, 0x2b74U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x4d0U, 0x14U, 0x2e3cU},
		{LBFF_DEVICE_ID_WB_1_1, 0x0U, 0x30U, 0x2e50U},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x0U, 0x2d4U, 0x2e80U},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x2e4U, 0x100U, 0x3154U},
		{LBFF_DEVICE_ID_BXT_Demosaic, 0x0U, 0x34U, 0x3254U},
		{LBFF_DEVICE_ID_VCSC_2_0_b, 0x0U, 0x24U, 0x3288U},
		{LBFF_DEVICE_ID_GLTM_2_0, 0x0U, 0x33cU, 0x32acU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x4U, 0xcU, 0x35e8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x20U, 0x3cU, 0x35f4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x1004U, 0x18U, 0x3630U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x102cU, 0x1a8U, 0x3648U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x2000U, 0x180U, 0x37f0U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x3004U, 0x38U, 0x3970U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x304cU, 0x19cU, 0x39a8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x4000U, 0x180U, 0x3b44U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5004U, 0x4U, 0x3cc4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5020U, 0x1cU, 0x3cc8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x504cU, 0x180U, 0x3ce4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x6000U, 0x180U, 0x3e64U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7004U, 0x1cU, 0x3fe4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7024U, 0x1cU, 0x4000U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x8004U, 0x48U, 0x401cU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x805cU, 0x148U, 0x4064U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x9000U, 0x180U, 0x41acU},
		{LBFF_DEVICE_ID_VCR_3_1, 0x0U, 0x4U, 0x432cU},
		{LBFF_DEVICE_ID_VCR_3_1, 0x40U, 0x20U, 0x4330U},
		{LBFF_DEVICE_ID_GLIM_2_0, 0x0U, 0x328U, 0x4350U},
		{LBFF_DEVICE_ID_ACM_1_1, 0x0U, 0x444U, 0x4678U},
		{LBFF_DEVICE_ID_GammaTM_V3, 0x0U, 0xc04U, 0x4abcU},
		{LBFF_DEVICE_ID_GammaTM_V3, 0xc20U, 0x1004U, 0x56c0U},
		{LBFF_DEVICE_ID_GammaTM_V3, 0x1c40U, 0x34U, 0x66c4U},
		{LBFF_DEVICE_ID_BXT_CSC, 0x0U, 0x24U, 0x66f8U},
		{LBFF_DEVICE_ID_B2I_DS_1_0_1, 0x0U, 0x8U, 0x671cU},
		{LBFF_DEVICE_ID_B2I_DS_1_0_1, 0x1cU, 0x208U, 0x6724U},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x0U, 0x4U, 0x692cU},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x14U, 0x40cU, 0x6930U},
		{LBFF_DEVICE_ID_TNR_Scale_LB, 0x0U, 0x4U, 0x6d3cU},
		{LBFF_DEVICE_ID_TNR_Scale_LB, 0x40U, 0x2cU, 0x6d40U},
		{LBFF_DEVICE_ID_GMV_Statistics_1_0, 0x0U, 0x1cU, 0x6d6cU},

	}

};

static payload_descriptor_t lbff_1_descriptors = {
	25,
	{
		{LBFF_DEVICE_ID_LSC_1_2, 0x0U, 0x8U, 0x0U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x338U, 0x104U, 0x8U},
		{LBFF_DEVICE_ID_GD_DPC_2_2, 0x488U, 0x8U, 0x10cU},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x4U, 0x4U, 0x114U},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x40U, 0xcU, 0x118U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x24U, 0x180U, 0x124U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x1c0U, 0xcU, 0x2a4U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x80U, 0xcU, 0x2b0U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x1cU, 0x8U, 0x2bcU},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x200U, 0x4U, 0x2c4U},
		{LBFF_DEVICE_ID_BNLM_3_3, 0x2d8U, 0xcU, 0x2c8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x0U, 0x4U, 0x2d4U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x10U, 0x10U, 0x2d8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x1000U, 0x4U, 0x2e8U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x101cU, 0x10U, 0x2ecU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x3000U, 0x4U, 0x2fcU},
		{LBFF_DEVICE_ID_XNR_5_2, 0x303cU, 0x10U, 0x300U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x5000U, 0x4U, 0x310U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x503cU, 0x10U, 0x314U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x7000U, 0x4U, 0x324U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x8000U, 0x4U, 0x328U},
		{LBFF_DEVICE_ID_XNR_5_2, 0x804cU, 0x10U, 0x32cU},
		{LBFF_DEVICE_ID_B2I_DS_1_0_1, 0x8U, 0x10U, 0x33cU},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x4U, 0x10U, 0x34cU},
		{LBFF_DEVICE_ID_UpScaler_1_0, 0x1010U, 0xcU, 0x35cU},

	}

};

static payload_descriptor_t lbff_2_descriptors = {
	19,
	{
		{LBFF_DEVICE_ID_ifd_pipe_1_1, 0x0U, 0x9cU, 0x0U},
		{LBFF_DEVICE_ID_ifd_lsc_1_1, 0x0U, 0x9cU, 0x9cU},
		{LBFF_DEVICE_ID_ifd_pdaf_1_1, 0x0U, 0x9cU, 0x138U},
		{LBFF_DEVICE_ID_odr_awb_std_1_1, 0x0U, 0x9cU, 0x1d4U},
		{LBFF_DEVICE_ID_odr_awb_sat_1_1, 0x0U, 0x9cU, 0x270U},
		{LBFF_DEVICE_ID_odr_awb_sve_1_1, 0x0U, 0x9cU, 0x30cU},
		{LBFF_DEVICE_ID_odr_ae_1_1, 0x0U, 0x9cU, 0x3a8U},
		{LBFF_DEVICE_ID_odr_af_std_1_1, 0x0U, 0x9cU, 0x444U},
		{LBFF_DEVICE_ID_odr_af_sve_1_1, 0x0U, 0x9cU, 0x4e0U},
		{LBFF_DEVICE_ID_odr_pdaf_1_1, 0x0U, 0x9cU, 0x57cU},
		{LBFF_DEVICE_ID_odr_ir_1_1, 0x0U, 0x9cU, 0x618U},
		{LBFF_DEVICE_ID_odr_burst_isp_1_1, 0x0U, 0x9cU, 0x6b4U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_1, 0x0U, 0xcU, 0x750U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_1, 0x4cU, 0x14U, 0x75cU},
		{LBFF_DEVICE_ID_odr_output_ps_1_1, 0x0U, 0x9cU, 0x770U},
		{LBFF_DEVICE_ID_odr_output_me_1_1, 0x0U, 0x9cU, 0x80cU},
		{LBFF_DEVICE_ID_ifd_gmv_1_1, 0x0U, 0x9cU, 0x8a8U},
		{LBFF_DEVICE_ID_odr_gmv_feature_1_1, 0x0U, 0x9cU, 0x944U},
		{LBFF_DEVICE_ID_odr_gmv_match_1_1, 0x0U, 0x9cU, 0x9e0U},

	}

};

#endif
