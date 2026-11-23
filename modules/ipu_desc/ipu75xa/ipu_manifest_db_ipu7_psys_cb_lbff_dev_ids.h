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

#ifndef IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DEV_IDS_H
#define IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DEV_IDS_H

typedef enum lbff_device_id_t {
	LBFF_DEVICE_ID_ifd_pipe_1_3 = 0U,
	LBFF_DEVICE_ID_ifd_pipe_long_1_3 = 1U,
	LBFF_DEVICE_ID_ifd_pipe_short_smth_1_3 = 2U,
	LBFF_DEVICE_ID_DOL_lite_1_1 = 3U,
	LBFF_DEVICE_ID_BXT_BLC = 4U,
	LBFF_DEVICE_ID_Linearization2_0 = 5U,
	LBFF_DEVICE_ID_ifd_lsc_1_3 = 6U,
	LBFF_DEVICE_ID_LSC_1_2 = 7U,
	LBFF_DEVICE_ID_GD_DPC_2_2 = 8U,
	LBFF_DEVICE_ID_ifd_pdaf_1_3 = 9U,
	LBFF_DEVICE_ID_PEXT_1_0 = 10U,
	LBFF_DEVICE_ID_PAFStatistics_1_2 = 11U,
	LBFF_DEVICE_ID_RGBS_Grid_1_1 = 12U,
	LBFF_DEVICE_ID_CCM_3A_2_0 = 13U,
	LBFF_DEVICE_ID_AEStatistics_2_1 = 14U,
	LBFF_DEVICE_ID_FR_Grid_1_0 = 15U,
	LBFF_DEVICE_ID_odr_awb_std_1_3 = 16U,
	LBFF_DEVICE_ID_odr_awb_sat_1_3 = 17U,
	LBFF_DEVICE_ID_odr_awb_sve_1_3 = 18U,
	LBFF_DEVICE_ID_odr_ae_1_3 = 19U,
	LBFF_DEVICE_ID_odr_af_std_1_3 = 20U,
	LBFF_DEVICE_ID_odr_af_sve_1_3 = 21U,
	LBFF_DEVICE_ID_odr_pdaf_1_3 = 22U,
	LBFF_DEVICE_ID_SVE_RGBIR_VRT_CTRL = 23U,
	LBFF_DEVICE_ID_RGB_IR_2_0 = 24U,
	LBFF_DEVICE_ID_odr_ir_1_3 = 25U,
	LBFF_DEVICE_ID_WB_1_1 = 26U,
	LBFF_DEVICE_ID_odr_burst_isp_1_3 = 27U,
	LBFF_DEVICE_ID_BNLM_3_3 = 28U,
	LBFF_DEVICE_ID_odr_bnlm_1_3 = 29U,
	LBFF_DEVICE_ID_BXT_Demosaic = 30U,
	LBFF_DEVICE_ID_VCSC_2_0_b = 31U,
	LBFF_DEVICE_ID_GLTM_2_0 = 32U,
	LBFF_DEVICE_ID_XNR_5_2 = 33U,
	LBFF_DEVICE_ID_VCR_3_1 = 34U,
	LBFF_DEVICE_ID_GLIM_2_0 = 35U,
	LBFF_DEVICE_ID_ACM_1_1 = 36U,
	LBFF_DEVICE_ID_GammaTM_V4 = 37U,
	LBFF_DEVICE_ID_CSC_1_1 = 38U,
	LBFF_DEVICE_ID_B2I_DS_1_1 = 39U,
	LBFF_DEVICE_ID_UpScaler_1_0 = 40U,
	LBFF_DEVICE_ID_lbff_crop_espa_1_3 = 41U,
	LBFF_DEVICE_ID_TNR_Scale_LB = 42U,
	LBFF_DEVICE_ID_odr_output_ps_1_3 = 43U,
	LBFF_DEVICE_ID_odr_output_me_1_3 = 44U,
	LBFF_DEVICE_ID_ifd_gmv_1_3 = 45U,
	LBFF_DEVICE_ID_GMV_Statistics_1_0 = 46U,
	LBFF_DEVICE_ID_odr_gmv_feature_1_3 = 47U,
	LBFF_DEVICE_ID_odr_gmv_match_1_3 = 48U,
	LBFF_DEVICE_ID_N = 49U,
} lbff_device_id_t;

#endif
