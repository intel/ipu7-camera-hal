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

#ifndef IPU_MANIFEST_DB_IPU8_PSYS_CB_LBFF_DEV_IDS_H
#define IPU_MANIFEST_DB_IPU8_PSYS_CB_LBFF_DEV_IDS_H

typedef enum lbff_device_id_t {
	LBFF_DEVICE_ID_ifd_pipe_1_4 = 0U,
	LBFF_DEVICE_ID_ifd_pipe_long_1_4 = 1U,
	LBFF_DEVICE_ID_ifd_pipe_short_smth_1_4 = 2U,
	LBFF_DEVICE_ID_DOL_lite_1_2 = 3U,
	LBFF_DEVICE_ID_BXT_BLC = 4U,
	LBFF_DEVICE_ID_Linearization2_0 = 5U,
	LBFF_DEVICE_ID_ifd_lsc_1_4 = 6U,
	LBFF_DEVICE_ID_LSC_1_2 = 7U,
	LBFF_DEVICE_ID_DPC_2_2 = 8U,
	LBFF_DEVICE_ID_ifd_pdaf_1_4 = 9U,
	LBFF_DEVICE_ID_PEXT_1_0 = 10U,
	LBFF_DEVICE_ID_PAFStatistics_1_2 = 11U,
	LBFF_DEVICE_ID_RGBS_Grid_1_1 = 12U,
	LBFF_DEVICE_ID_CCM_3A_2_0 = 13U,
	LBFF_DEVICE_ID_AEStatistics_2_1 = 14U,
	LBFF_DEVICE_ID_FR_Grid_1_0 = 15U,
	LBFF_DEVICE_ID_odr_awb_std_1_4 = 16U,
	LBFF_DEVICE_ID_odr_awb_sat_1_4 = 17U,
	LBFF_DEVICE_ID_odr_awb_sve_1_4 = 18U,
	LBFF_DEVICE_ID_odr_ae_1_4 = 19U,
	LBFF_DEVICE_ID_odr_af_std_1_4 = 20U,
	LBFF_DEVICE_ID_odr_dpc_pdaf_1_4 = 21U,
	LBFF_DEVICE_ID_odr_pdaf_1_4 = 22U,
	LBFF_DEVICE_ID_SVE_RGBIR_VRT_CTRL = 23U,
	LBFF_DEVICE_ID_RGB_IR_2_0 = 24U,
	LBFF_DEVICE_ID_odr_ir_1_4 = 25U,
	LBFF_DEVICE_ID_GD_2_2 = 26U,
	LBFF_DEVICE_ID_WB_1_1 = 27U,
	LBFF_DEVICE_ID_odr_burst_isp_1_4 = 28U,
	LBFF_DEVICE_ID_ifd_segmap_bnlm_1_4 = 29U,
	LBFF_DEVICE_ID_Smurf_bnlm_1_0 = 30U,
	LBFF_DEVICE_ID_BNLM_3_4 = 31U,
	LBFF_DEVICE_ID_odr_bnlm_1_4 = 32U,
	LBFF_DEVICE_ID_BXT_Demosaic = 33U,
	LBFF_DEVICE_ID_VCSC_2_0_b = 34U,
	LBFF_DEVICE_ID_GLTM_2_0 = 35U,
	LBFF_DEVICE_ID_ifd_segmap_xnr_1_4 = 36U,
	LBFF_DEVICE_ID_Smurf_xnr_1_0 = 37U,
	LBFF_DEVICE_ID_XNR_5_4 = 38U,
	LBFF_DEVICE_ID_VCR_3_1 = 39U,
	LBFF_DEVICE_ID_GLIM_2_0 = 40U,
	LBFF_DEVICE_ID_ifd_segmap_acm_1_4 = 41U,
	LBFF_DEVICE_ID_Smurf_acm_1_0 = 42U,
	LBFF_DEVICE_ID_ACM_1_2 = 43U,
	LBFF_DEVICE_ID_GammaTM_V4 = 44U,
	LBFF_DEVICE_ID_CSC_1_1 = 45U,
	LBFF_DEVICE_ID_B2I_DS_1_1 = 46U,
	LBFF_DEVICE_ID_lbff_crop_espa_1_4 = 47U,
	LBFF_DEVICE_ID_ifd_gmv_1_4 = 48U,
	LBFF_DEVICE_ID_GMV_Statistics_1_1 = 49U,
	LBFF_DEVICE_ID_odr_gmv_feature_1_4 = 50U,
	LBFF_DEVICE_ID_odr_gmv_match_1_4 = 51U,
	LBFF_DEVICE_ID_TNR_Scaler_LB_1_1 = 52U,
	LBFF_DEVICE_ID_tnr_delay_vrt_ctrl = 53U,
	LBFF_DEVICE_ID_ifd_tnr_sp_bc_yuv4nm1_1_4 = 54U,
	LBFF_DEVICE_ID_ifd_tnr_sp_bc_rs4nm1_1_4 = 55U,
	LBFF_DEVICE_ID_TNR7_IMS_1_2 = 56U,
	LBFF_DEVICE_ID_ifd_segmap_tnr_bc_1_4 = 57U,
	LBFF_DEVICE_ID_Smurf_tnr_bc_1_0 = 58U,
	LBFF_DEVICE_ID_TNR7_BC_1_2 = 59U,
	LBFF_DEVICE_ID_odr_tnr_sp_bc_rs4n_1_4 = 60U,
	LBFF_DEVICE_ID_TNR7_Spatial_1_1 = 61U,
	LBFF_DEVICE_ID_ifd_tnr_fp_blend_yuvnm1_1_4 = 62U,
	LBFF_DEVICE_ID_ifd_segmap_tnr_blend_1_4 = 63U,
	LBFF_DEVICE_ID_Smurf_tnr_blend_1_0 = 64U,
	LBFF_DEVICE_ID_TNR7_BLEND_1_1 = 65U,
	LBFF_DEVICE_ID_odr_tnr_fp_yuvn_1_4 = 66U,
	LBFF_DEVICE_ID_TNR_Scaler_FP_1_1 = 67U,
	LBFF_DEVICE_ID_odr_tnr_scale_fp_yuv4n_1_4 = 68U,
	LBFF_DEVICE_ID_image_upscaler_1_1 = 69U,
	LBFF_DEVICE_ID_ifd_segmap_cas_1_4 = 70U,
	LBFF_DEVICE_ID_Smurf_cas_1_0 = 71U,
	LBFF_DEVICE_ID_CAS_1_1 = 72U,
	LBFF_DEVICE_ID_ifd_bgmap_1_4 = 73U,
	LBFF_DEVICE_ID_ifd_bg_yuv_1_4 = 74U,
	LBFF_DEVICE_ID_bgmap_upscaler_1_1 = 75U,
	LBFF_DEVICE_ID_BGB_1_0 = 76U,
	LBFF_DEVICE_ID_odr_ofs_mp_1_4 = 77U,
	LBFF_DEVICE_ID_B2I_DS_output_1_1 = 78U,
	LBFF_DEVICE_ID_odr_ofs_dp_1_4 = 79U,
	LBFF_DEVICE_ID_N = 80U,
} lbff_device_id_t;

#endif