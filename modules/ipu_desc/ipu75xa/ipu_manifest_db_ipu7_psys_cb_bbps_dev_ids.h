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

#ifndef IPU_MANIFEST_DB_IPU7_PSYS_CB_BBPS_DEV_IDS_H
#define IPU_MANIFEST_DB_IPU7_PSYS_CB_BBPS_DEV_IDS_H

typedef enum BBPS_device_id_t {
	BBPS_DEVICE_ID_slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3 = 0U,
	BBPS_DEVICE_ID_slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3 = 1U,
	BBPS_DEVICE_ID_tnr_sp_bc_bifd_yuv4n_regs_1_3 = 2U,
	BBPS_DEVICE_ID_TNR7_IMS_1_1 = 3U,
	BBPS_DEVICE_ID_TNR7_BC_1_1 = 4U,
	BBPS_DEVICE_ID_tnr_sp_bc_bodr_rs4n_regs_1_3 = 5U,
	BBPS_DEVICE_ID_slim_tnr_spatial_bifd_yuvn_regs_1_3 = 6U,
	BBPS_DEVICE_ID_TNR7_Spatial_1_0 = 7U,
	BBPS_DEVICE_ID_slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3 = 8U,
	BBPS_DEVICE_ID_TNR7_BLEND_1_0 = 9U,
	BBPS_DEVICE_ID_tnr_fp_blend_bifd_rs4n_regs_1_3 = 10U,
	BBPS_DEVICE_ID_tnr_fp_bodr_yuvn_regs_1_3 = 11U,
	BBPS_DEVICE_ID_TNR_Scale_FP = 12U,
	BBPS_DEVICE_ID_tnr_scale_fp_bodr_yuv4n_regs_1_3 = 13U,
	BBPS_DEVICE_ID_CAS_1_0 = 14U,
	BBPS_DEVICE_ID_ofs_mp_bodr_regs_1_3 = 15U,
	BBPS_DEVICE_ID_OutputScaler_2_0_A = 16U,
	BBPS_DEVICE_ID_ofs_dp_bodr_regs_1_3 = 17U,
	BBPS_DEVICE_ID_N = 18U,
} BBPS_device_id_t;

#endif
