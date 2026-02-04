/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2026 Intel Corporation
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

#ifndef IPU_MANIFEST_DB_IPU8_PSYS_CB_LBFF_DESCRIPTORS_H
#define IPU_MANIFEST_DB_IPU8_PSYS_CB_LBFF_DESCRIPTORS_H

#include <stdint.h>
#include "cb_payload_descriptor.h"

static payload_descriptor_t lbff_0_descriptors = {
	91,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_2, 0x0U, 0x30U, 0x0U},
		{LBFF_DEVICE_ID_DOL_lite_1_2, 0x100U, 0xacU, 0x30U},
		{LBFF_DEVICE_ID_BXT_BLC, 0x0U, 0x24U, 0xdcU},
		{LBFF_DEVICE_ID_Linearization2_0, 0x0U, 0x2174U, 0x100U},
		{LBFF_DEVICE_ID_LSC_1_2, 0x8U, 0x134U, 0x2274U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x0U, 0x200U, 0x23a8U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x278U, 0xb4U, 0x25a8U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x330U, 0x4U, 0x265cU},
		{LBFF_DEVICE_ID_DPC_2_2, 0x43cU, 0x44U, 0x2660U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x4bcU, 0x88U, 0x26a4U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x3000U, 0x4U, 0x272cU},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x0U, 0x4U, 0x2730U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0xcU, 0x8U, 0x2734U},
		{LBFF_DEVICE_ID_PAFStatistics_1_2, 0x0U, 0x19cU, 0x273cU},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x0U, 0x38U, 0x28d8U},
		{LBFF_DEVICE_ID_CCM_3A_2_0, 0x0U, 0x44U, 0x2910U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x0U, 0x20U, 0x2954U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x0U, 0x2cU, 0x2974U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x30U, 0x8U, 0x29a0U},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x3cU, 0x30U, 0x29a8U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x0U, 0x1cU, 0x29d8U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x24U, 0x1dcU, 0x29f4U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x204U, 0x2c8U, 0x2bd0U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x4d0U, 0x14U, 0x2e98U},
		{LBFF_DEVICE_ID_GD_2_2, 0x0U, 0x78U, 0x2eacU},
		{LBFF_DEVICE_ID_GD_2_2, 0x100U, 0x4U, 0x2f24U},
		{LBFF_DEVICE_ID_GD_2_2, 0x20cU, 0x4U, 0x2f28U},
		{LBFF_DEVICE_ID_WB_1_1, 0x0U, 0x30U, 0x2f2cU},
		{LBFF_DEVICE_ID_Smurf_bnlm_1_0, 0x0U, 0x8U, 0x2f5cU},
		{LBFF_DEVICE_ID_BNLM_3_4, 0x0U, 0x188U, 0x2f64U},
		{LBFF_DEVICE_ID_BNLM_3_4, 0x18cU, 0x43cU, 0x30ecU},
		{LBFF_DEVICE_ID_BNLM_3_4, 0x5dcU, 0x104U, 0x3528U},
		{LBFF_DEVICE_ID_BXT_Demosaic, 0x0U, 0x34U, 0x362cU},
		{LBFF_DEVICE_ID_VCSC_2_0_b, 0x0U, 0x24U, 0x3660U},
		{LBFF_DEVICE_ID_GLTM_2_0, 0x0U, 0x33cU, 0x3684U},
		{LBFF_DEVICE_ID_Smurf_xnr_1_0, 0x0U, 0x8U, 0x39c0U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x4U, 0xcU, 0x39c8U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x20U, 0x5cU, 0x39d4U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x1004U, 0x18U, 0x3a30U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x102cU, 0x1ccU, 0x3a48U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x2000U, 0x180U, 0x3c14U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x3004U, 0x38U, 0x3d94U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x304cU, 0x1e4U, 0x3dccU},
		{LBFF_DEVICE_ID_XNR_5_4, 0x4000U, 0x180U, 0x3fb0U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x5004U, 0x4U, 0x4130U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x5020U, 0x1cU, 0x4134U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x504cU, 0x1c8U, 0x4150U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x6000U, 0x180U, 0x4318U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x7004U, 0x1cU, 0x4498U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x7024U, 0x20U, 0x44b4U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x8004U, 0x48U, 0x44d4U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x805cU, 0x190U, 0x451cU},
		{LBFF_DEVICE_ID_XNR_5_4, 0x9000U, 0x180U, 0x46acU},
		{LBFF_DEVICE_ID_VCR_3_1, 0x0U, 0x4U, 0x482cU},
		{LBFF_DEVICE_ID_VCR_3_1, 0x40U, 0x20U, 0x4830U},
		{LBFF_DEVICE_ID_GLIM_2_0, 0x0U, 0x328U, 0x4850U},
		{LBFF_DEVICE_ID_Smurf_acm_1_0, 0x0U, 0x8U, 0x4b78U},
		{LBFF_DEVICE_ID_ACM_1_2, 0x0U, 0x444U, 0x4b80U},
		{LBFF_DEVICE_ID_ACM_1_2, 0x470U, 0x2b0U, 0x4fc4U},
		{LBFF_DEVICE_ID_GammaTM_V4, 0x0U, 0x1c74U, 0x5274U},
		{LBFF_DEVICE_ID_CSC_1_1, 0x0U, 0x24U, 0x6ee8U},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x0U, 0x8U, 0x6f0cU},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x1cU, 0x208U, 0x6f14U},
		{LBFF_DEVICE_ID_TNR_Scaler_LB_1_1, 0x0U, 0x4U, 0x711cU},
		{LBFF_DEVICE_ID_TNR_Scaler_LB_1_1, 0x44U, 0x28U, 0x7120U},
		{LBFF_DEVICE_ID_GMV_Statistics_1_1, 0x0U, 0x1cU, 0x7148U},
		{LBFF_DEVICE_ID_TNR7_IMS_1_2, 0x0U, 0x18U, 0x7164U},
		{LBFF_DEVICE_ID_TNR7_IMS_1_2, 0x28U, 0xc0U, 0x717cU},
		{LBFF_DEVICE_ID_Smurf_tnr_bc_1_0, 0x0U, 0x8U, 0x723cU},
		{LBFF_DEVICE_ID_TNR7_BC_1_2, 0x0U, 0x20U, 0x7244U},
		{LBFF_DEVICE_ID_TNR7_BC_1_2, 0x100U, 0x54U, 0x7264U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x0U, 0x10U, 0x72b8U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x30U, 0x2cU, 0x72c8U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x60U, 0x4U, 0x72f4U},
		{LBFF_DEVICE_ID_Smurf_tnr_blend_1_0, 0x0U, 0x8U, 0x72f8U},
		{LBFF_DEVICE_ID_TNR7_BLEND_1_1, 0x0U, 0x104U, 0x7300U},
		{LBFF_DEVICE_ID_TNR7_BLEND_1_1, 0x204U, 0x148U, 0x7404U},
		{LBFF_DEVICE_ID_TNR_Scaler_FP_1_1, 0x0U, 0x4U, 0x754cU},
		{LBFF_DEVICE_ID_TNR_Scaler_FP_1_1, 0x44U, 0x28U, 0x7550U},
		{LBFF_DEVICE_ID_image_upscaler_1_1, 0x0U, 0x4U, 0x7578U},
		{LBFF_DEVICE_ID_image_upscaler_1_1, 0x1cU, 0x40cU, 0x757cU},
		{LBFF_DEVICE_ID_Smurf_cas_1_0, 0x0U, 0x8U, 0x7988U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x0U, 0x4U, 0x7990U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x8U, 0x4U, 0x7994U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x10U, 0x10U, 0x7998U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x40U, 0x168U, 0x79a8U},
		{LBFF_DEVICE_ID_bgmap_upscaler_1_1, 0x0U, 0x4U, 0x7b10U},
		{LBFF_DEVICE_ID_bgmap_upscaler_1_1, 0x1cU, 0x40cU, 0x7b14U},
		{LBFF_DEVICE_ID_BGB_1_0, 0x0U, 0x4U, 0x7f20U},
		{LBFF_DEVICE_ID_B2I_DS_output_1_1, 0x0U, 0x8U, 0x7f24U},
		{LBFF_DEVICE_ID_B2I_DS_output_1_1, 0x1cU, 0x208U, 0x7f2cU},
		
	}

};

static payload_descriptor_t lbff_1_descriptors = {
	43,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_2, 0xa0U, 0x8U, 0x0U},
		{LBFF_DEVICE_ID_LSC_1_2, 0x0U, 0x8U, 0x8U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x338U, 0x104U, 0x10U},
		{LBFF_DEVICE_ID_DPC_2_2, 0x488U, 0x8U, 0x114U},
		{LBFF_DEVICE_ID_PEXT_1_0, 0x4U, 0x4U, 0x11cU},
		{LBFF_DEVICE_ID_RGBS_Grid_1_1, 0x40U, 0xcU, 0x120U},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x24U, 0x180U, 0x12cU},
		{LBFF_DEVICE_ID_AEStatistics_2_1, 0x1c0U, 0xcU, 0x2acU},
		{LBFF_DEVICE_ID_FR_Grid_1_0, 0x80U, 0xcU, 0x2b8U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x1cU, 0x8U, 0x2c4U},
		{LBFF_DEVICE_ID_RGB_IR_2_0, 0x200U, 0x4U, 0x2ccU},
		{LBFF_DEVICE_ID_Smurf_bnlm_1_0, 0x10U, 0x18U, 0x2d0U},
		{LBFF_DEVICE_ID_BNLM_3_4, 0x5ccU, 0x10U, 0x2e8U},
		{LBFF_DEVICE_ID_Smurf_xnr_1_0, 0x10U, 0x18U, 0x2f8U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x0U, 0x4U, 0x310U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x10U, 0x10U, 0x314U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x1000U, 0x4U, 0x324U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x101cU, 0x10U, 0x328U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x3000U, 0x4U, 0x338U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x303cU, 0x10U, 0x33cU},
		{LBFF_DEVICE_ID_XNR_5_4, 0x5000U, 0x4U, 0x34cU},
		{LBFF_DEVICE_ID_XNR_5_4, 0x503cU, 0x10U, 0x350U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x7000U, 0x4U, 0x360U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x8000U, 0x4U, 0x364U},
		{LBFF_DEVICE_ID_XNR_5_4, 0x804cU, 0x10U, 0x368U},
		{LBFF_DEVICE_ID_Smurf_acm_1_0, 0x10U, 0x18U, 0x378U},
		{LBFF_DEVICE_ID_B2I_DS_1_1, 0x8U, 0x10U, 0x390U},
		{LBFF_DEVICE_ID_TNR_Scaler_LB_1_1, 0x40U, 0x4U, 0x3a0U},
		{LBFF_DEVICE_ID_GMV_Statistics_1_1, 0x1cU, 0xcU, 0x3a4U},
		{LBFF_DEVICE_ID_TNR7_IMS_1_2, 0x200U, 0x10U, 0x3b0U},
		{LBFF_DEVICE_ID_Smurf_tnr_bc_1_0, 0x10U, 0x18U, 0x3c0U},
		{LBFF_DEVICE_ID_TNR7_BC_1_2, 0x248U, 0x10U, 0x3d8U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x68U, 0x10U, 0x3e8U},
		{LBFF_DEVICE_ID_Smurf_tnr_blend_1_0, 0x10U, 0x18U, 0x3f8U},
		{LBFF_DEVICE_ID_TNR_Scaler_FP_1_1, 0x40U, 0x4U, 0x410U},
		{LBFF_DEVICE_ID_image_upscaler_1_1, 0x4U, 0x18U, 0x414U},
		{LBFF_DEVICE_ID_image_upscaler_1_1, 0x1010U, 0xcU, 0x42cU},
		{LBFF_DEVICE_ID_Smurf_cas_1_0, 0x10U, 0x18U, 0x438U},
		{LBFF_DEVICE_ID_CAS_1_1, 0xcU, 0x4U, 0x450U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x1a8U, 0x8U, 0x454U},
		{LBFF_DEVICE_ID_bgmap_upscaler_1_1, 0x4U, 0x18U, 0x45cU},
		{LBFF_DEVICE_ID_bgmap_upscaler_1_1, 0x1010U, 0xcU, 0x474U},
		{LBFF_DEVICE_ID_B2I_DS_output_1_1, 0x8U, 0x10U, 0x480U},
		
	}

};

static payload_descriptor_t lbff_2_descriptors = {
	36,
	{
		{LBFF_DEVICE_ID_ifd_pipe_1_4, 0x0U, 0xc4U, 0x0U},
		{LBFF_DEVICE_ID_ifd_pipe_long_1_4, 0x0U, 0xc4U, 0xc4U},
		{LBFF_DEVICE_ID_ifd_pipe_short_smth_1_4, 0x0U, 0xc4U, 0x188U},
		{LBFF_DEVICE_ID_ifd_lsc_1_4, 0x0U, 0xc4U, 0x24cU},
		{LBFF_DEVICE_ID_ifd_pdaf_1_4, 0x0U, 0xc4U, 0x310U},
		{LBFF_DEVICE_ID_odr_awb_std_1_4, 0x0U, 0xc4U, 0x3d4U},
		{LBFF_DEVICE_ID_odr_awb_sat_1_4, 0x0U, 0xc4U, 0x498U},
		{LBFF_DEVICE_ID_odr_awb_sve_1_4, 0x0U, 0xc4U, 0x55cU},
		{LBFF_DEVICE_ID_odr_ae_1_4, 0x0U, 0xc4U, 0x620U},
		{LBFF_DEVICE_ID_odr_af_std_1_4, 0x0U, 0xc4U, 0x6e4U},
		{LBFF_DEVICE_ID_odr_dpc_pdaf_1_4, 0x0U, 0xc4U, 0x7a8U},
		{LBFF_DEVICE_ID_odr_pdaf_1_4, 0x0U, 0xc4U, 0x86cU},
		{LBFF_DEVICE_ID_odr_ir_1_4, 0x0U, 0xc4U, 0x930U},
		{LBFF_DEVICE_ID_odr_burst_isp_1_4, 0x0U, 0xc4U, 0x9f4U},
		{LBFF_DEVICE_ID_ifd_segmap_bnlm_1_4, 0x0U, 0xc4U, 0xab8U},
		{LBFF_DEVICE_ID_odr_bnlm_1_4, 0x0U, 0xc4U, 0xb7cU},
		{LBFF_DEVICE_ID_ifd_segmap_xnr_1_4, 0x0U, 0xc4U, 0xc40U},
		{LBFF_DEVICE_ID_ifd_segmap_acm_1_4, 0x0U, 0xc4U, 0xd04U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_4, 0x0U, 0xcU, 0xdc8U},
		{LBFF_DEVICE_ID_lbff_crop_espa_1_4, 0x4cU, 0x14U, 0xdd4U},
		{LBFF_DEVICE_ID_ifd_gmv_1_4, 0x0U, 0xc4U, 0xde8U},
		{LBFF_DEVICE_ID_odr_gmv_feature_1_4, 0x0U, 0xc4U, 0xeacU},
		{LBFF_DEVICE_ID_odr_gmv_match_1_4, 0x0U, 0xc4U, 0xf70U},
		{LBFF_DEVICE_ID_ifd_tnr_sp_bc_yuv4nm1_1_4, 0x0U, 0xc4U, 0x1034U},
		{LBFF_DEVICE_ID_ifd_tnr_sp_bc_rs4nm1_1_4, 0x0U, 0xc4U, 0x10f8U},
		{LBFF_DEVICE_ID_ifd_segmap_tnr_bc_1_4, 0x0U, 0xc4U, 0x11bcU},
		{LBFF_DEVICE_ID_odr_tnr_sp_bc_rs4n_1_4, 0x0U, 0xc4U, 0x1280U},
		{LBFF_DEVICE_ID_ifd_tnr_fp_blend_yuvnm1_1_4, 0x0U, 0xc4U, 0x1344U},
		{LBFF_DEVICE_ID_ifd_segmap_tnr_blend_1_4, 0x0U, 0xc4U, 0x1408U},
		{LBFF_DEVICE_ID_odr_tnr_fp_yuvn_1_4, 0x0U, 0xc4U, 0x14ccU},
		{LBFF_DEVICE_ID_odr_tnr_scale_fp_yuv4n_1_4, 0x0U, 0xc4U, 0x1590U},
		{LBFF_DEVICE_ID_ifd_segmap_cas_1_4, 0x0U, 0xc4U, 0x1654U},
		{LBFF_DEVICE_ID_ifd_bgmap_1_4, 0x0U, 0xc4U, 0x1718U},
		{LBFF_DEVICE_ID_ifd_bg_yuv_1_4, 0x0U, 0xc4U, 0x17dcU},
		{LBFF_DEVICE_ID_odr_ofs_mp_1_4, 0x0U, 0xc4U, 0x18a0U},
		{LBFF_DEVICE_ID_odr_ofs_dp_1_4, 0x0U, 0xc4U, 0x1964U},
		
	}

};

static payload_descriptor_t lbff_3_descriptors = {
	6,
	{
		{LBFF_DEVICE_ID_DOL_lite_1_2, 0x200U, 0x164U, 0x0U},
		{LBFF_DEVICE_ID_TNR7_IMS_1_2, 0xe8U, 0x100U, 0x200U},
		{LBFF_DEVICE_ID_TNR7_BC_1_2, 0x300U, 0x14U, 0x400U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x10U, 0x20U, 0x600U},
		{LBFF_DEVICE_ID_TNR7_Spatial_1_1, 0x5cU, 0x4U, 0x800U},
		{LBFF_DEVICE_ID_CAS_1_1, 0x20U, 0x20U, 0xa00U},
		
	}

};

#endif