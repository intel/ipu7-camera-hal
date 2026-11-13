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

#ifndef IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DEV_IDS_H
#define IPU_MANIFEST_DB_IPU7_PSYS_CB_LBFF_DEV_IDS_H

typedef enum lbff_device_id_t {
	LBFF_DEVICE_ID_ifd_pipe_1_1 = 0U,
	LBFF_DEVICE_ID_BXT_BLC = 1U,
	LBFF_DEVICE_ID_Linearization2_0 = 2U,
	LBFF_DEVICE_ID_ifd_lsc_1_1 = 3U,
	LBFF_DEVICE_ID_LSC_1_2 = 4U,
	LBFF_DEVICE_ID_GD_DPC_2_2 = 5U,
	LBFF_DEVICE_ID_ifd_pdaf_1_1 = 6U,
	LBFF_DEVICE_ID_PEXT_1_0 = 7U,
	LBFF_DEVICE_ID_PAFStatistics_1_2 = 8U,
	LBFF_DEVICE_ID_RGBS_Grid_1_1 = 9U,
	LBFF_DEVICE_ID_CCM_3A_2_0 = 10U,
	LBFF_DEVICE_ID_AEStatistics_2_1 = 11U,
	LBFF_DEVICE_ID_FR_Grid_1_0 = 12U,
	LBFF_DEVICE_ID_odr_awb_std_1_1 = 13U,
	LBFF_DEVICE_ID_odr_awb_sat_1_1 = 14U,
	LBFF_DEVICE_ID_odr_awb_sve_1_1 = 15U,
	LBFF_DEVICE_ID_odr_ae_1_1 = 16U,
	LBFF_DEVICE_ID_odr_af_std_1_1 = 17U,
	LBFF_DEVICE_ID_odr_af_sve_1_1 = 18U,
	LBFF_DEVICE_ID_odr_pdaf_1_1 = 19U,
	LBFF_DEVICE_ID_SVE_RGBIR_VRT_CTRL = 20U,
	LBFF_DEVICE_ID_RGB_IR_2_0 = 21U,
	LBFF_DEVICE_ID_odr_ir_1_1 = 22U,
	LBFF_DEVICE_ID_WB_1_1 = 23U,
	LBFF_DEVICE_ID_odr_burst_isp_1_1 = 24U,
	LBFF_DEVICE_ID_BNLM_3_3 = 25U,
	LBFF_DEVICE_ID_BXT_Demosaic = 26U,
	LBFF_DEVICE_ID_VCSC_2_0_b = 27U,
	LBFF_DEVICE_ID_GLTM_2_0 = 28U,
	LBFF_DEVICE_ID_XNR_5_2 = 29U,
	LBFF_DEVICE_ID_VCR_3_1 = 30U,
	LBFF_DEVICE_ID_GLIM_2_0 = 31U,
	LBFF_DEVICE_ID_ACM_1_1 = 32U,
	LBFF_DEVICE_ID_GammaTM_V3 = 33U,
	LBFF_DEVICE_ID_BXT_CSC = 34U,
	LBFF_DEVICE_ID_B2I_DS_1_0_1 = 35U,
	LBFF_DEVICE_ID_UpScaler_1_0 = 36U,
	LBFF_DEVICE_ID_lbff_crop_espa_1_1 = 37U,
	LBFF_DEVICE_ID_TNR_Scale_LB = 38U,
	LBFF_DEVICE_ID_odr_output_ps_1_1 = 39U,
	LBFF_DEVICE_ID_odr_output_me_1_1 = 40U,
	LBFF_DEVICE_ID_ifd_gmv_1_1 = 41U,
	LBFF_DEVICE_ID_GMV_Statistics_1_0 = 42U,
	LBFF_DEVICE_ID_odr_gmv_feature_1_1 = 43U,
	LBFF_DEVICE_ID_odr_gmv_match_1_1 = 44U,
	LBFF_DEVICE_ID_N = 45U,
} lbff_device_id_t;

#endif
