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

#ifndef IPU_MANIFEST_DB_IPU7_PSYS_CB_BBPS_DESCRIPTORS_H
#define IPU_MANIFEST_DB_IPU7_PSYS_CB_BBPS_DESCRIPTORS_H

#include <stdint.h>
#include "cb_payload_descriptor.h"

static payload_descriptor_t BBPS_0_descriptors = {
	23,
	{
		{BBPS_DEVICE_ID_TNR7_IMS_1_1, 0x0U, 0x4U, 0x0U},
		{BBPS_DEVICE_ID_TNR7_IMS_1_1, 0x10U, 0xd8U, 0x4U},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0x0U, 0x4U, 0xdcU},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0x8U, 0x4U, 0xe0U},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0x1cU, 0x4U, 0xe4U},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0x24U, 0x18U, 0xe8U},
		{BBPS_DEVICE_ID_TNR7_Spatial_1_0, 0x0U, 0x10U, 0x100U},
		{BBPS_DEVICE_ID_TNR7_Spatial_1_0, 0x30U, 0x2cU, 0x110U},
		{BBPS_DEVICE_ID_TNR7_Spatial_1_0, 0x60U, 0x4U, 0x13cU},
		{BBPS_DEVICE_ID_TNR7_BLEND_1_0, 0x0U, 0x260U, 0x140U},
		{BBPS_DEVICE_ID_TNR_Scale_FP, 0x0U, 0x4U, 0x3a0U},
		{BBPS_DEVICE_ID_TNR_Scale_FP, 0x40U, 0x4U, 0x3a4U},
		{BBPS_DEVICE_ID_TNR_Scale_FP, 0x48U, 0x28U, 0x3a8U},
		{BBPS_DEVICE_ID_CAS_1_0, 0x0U, 0x4U, 0x3d0U},
		{BBPS_DEVICE_ID_CAS_1_0, 0x8U, 0x4U, 0x3d4U},
		{BBPS_DEVICE_ID_CAS_1_0, 0x10U, 0x10U, 0x3d8U},
		{BBPS_DEVICE_ID_CAS_1_0, 0x40U, 0x68U, 0x3e8U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_A, 0x0U, 0x8U, 0x450U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_A, 0x20U, 0x204U, 0x458U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_A, 0x22cU, 0x204U, 0x65cU},
		{BBPS_DEVICE_ID_OutputScaler_2_0_B, 0x0U, 0x8U, 0x860U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_B, 0x20U, 0x204U, 0x868U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_B, 0x22cU, 0x204U, 0xa6cU},

	}

};

static payload_descriptor_t BBPS_1_descriptors = {
	3,
	{
		{BBPS_DEVICE_ID_CAS_1_0, 0xcU, 0x4U, 0x0U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_A, 0x8U, 0x18U, 0x4U},
		{BBPS_DEVICE_ID_OutputScaler_2_0_B, 0x8U, 0x18U, 0x1cU},

	}

};

static payload_descriptor_t BBPS_2_descriptors = {
	12,
	{
		{BBPS_DEVICE_ID_slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1, 0x0U, 0x9cU, 0x0U},
		{BBPS_DEVICE_ID_slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1, 0x0U, 0x9cU, 0x9cU},
		{BBPS_DEVICE_ID_tnr_sp_bc_bifd_yuv4n_regs_1_1, 0x0U, 0x9cU, 0x138U},
		{BBPS_DEVICE_ID_tnr_sp_bc_bodr_rs4n_regs_1_1, 0x0U, 0x9cU, 0x1d4U},
		{BBPS_DEVICE_ID_slim_tnr_spatial_bifd_yuvn_regs_1_1, 0x0U, 0x9cU, 0x270U},
		{BBPS_DEVICE_ID_slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1, 0x0U, 0x9cU, 0x30cU},
		{BBPS_DEVICE_ID_tnr_fp_blend_bifd_rs4n_regs_1_1, 0x0U, 0x9cU, 0x3a8U},
		{BBPS_DEVICE_ID_tnr_fp_bodr_yuvn_regs_1_1, 0x0U, 0x9cU, 0x444U},
		{BBPS_DEVICE_ID_tnr_scale_fp_bodr_yuv4n_regs_1_1, 0x0U, 0x9cU, 0x4e0U},
		{BBPS_DEVICE_ID_ofs_mp_bodr_regs_1_1, 0x0U, 0x9cU, 0x57cU},
		{BBPS_DEVICE_ID_ofs_dp_bodr_regs_1_1, 0x0U, 0x9cU, 0x618U},
		{BBPS_DEVICE_ID_ofs_pp_bodr_regs_1_1, 0x0U, 0x9cU, 0x6b4U},

	}

};

static payload_descriptor_t BBPS_3_descriptors = {
	6,
	{
		{BBPS_DEVICE_ID_TNR7_IMS_1_1, 0xe8U, 0x100U, 0x0U},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0x4U, 0x4U, 0x200U},
		{BBPS_DEVICE_ID_TNR7_BC_1_1, 0xcU, 0x10U, 0x400U},
		{BBPS_DEVICE_ID_TNR7_Spatial_1_0, 0x10U, 0x20U, 0x600U},
		{BBPS_DEVICE_ID_TNR7_Spatial_1_0, 0x5cU, 0x4U, 0x800U},
		{BBPS_DEVICE_ID_CAS_1_0, 0x20U, 0x20U, 0xa00U},

	}

};

#endif
