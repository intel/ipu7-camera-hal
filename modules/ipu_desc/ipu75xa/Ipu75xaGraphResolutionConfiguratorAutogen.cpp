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

#include "Ipu75xaGraphResolutionConfiguratorAutogen.h"

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuid(GraphResolutionConfiguratorKernelRole role)
{
    switch (role)
    {
        case GraphResolutionConfiguratorKernelRole::UpScaler:    return 25569; // upscaler_1_0
        case GraphResolutionConfiguratorKernelRole::DownScaler:  return 40299; // b2i_ds_1_1
        case GraphResolutionConfiguratorKernelRole::EspaCropper:  return 42330; // lbff_crop_espa_1_3
    }

    return 0;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuidOfOutput(HwSink hwSink, int32_t graphId, GraphLink** links)
{
    (void)graphId;
    (void)links;

    switch (hwSink)
    {
        case HwSink::ImageMpSink:    return 18789; // ofs_mp_bodr_regs_1_3
        case HwSink::ImageDpSink:    return 27847; // ofs_dp_bodr_regs_1_3
        case HwSink::ProcessedMainSink:
            switch(graphId)
            {
                case 100001:    // Bayer_NoPdaf_WithDvs_WithGdc_WithTnr
                case 100003:    // Bayer_NoPdaf_WithDvs_WithTnr
                case 100037:    // Bayer_WithPdaf2_WithDvs_WithTnr
                case 100038:    // Bayer_WithPdaf3_WithDvs_WithTnr
                case 100039:    // RgbIr_NoPdaf_WithDvs_WithTnr
                case 100040:    // Dol2Inputs_WithDvs_WithTnr
                case 100041:    // Dol3Inputs_WithDvs_WithTnr
                case 100056:    // Dol2Inputs_WithDvs_WithTnr
                case 100057:    // Dol3Inputs_WithDvs_WithTnr
                    return 5637; // gdc7_1
                case 100005:    // Bayer_NoPdaf_WithNntm_WithTnr
                case 100031:    // Dol2Inputs_NoDvs_NoTnr
                case 100032:    // Dol2Inputs_NoDvs_WithTnr
                case 100033:    // Dol3Inputs_NoDvs_NoTnr
                case 100034:    // Dol3Inputs_NoDvs_WithTnr
                case 100042:    // Bayer_WithPdaf3_WithNntm_WithTnr
                case 100052:    // Dol2Inputs_NoDvs_NoTnr
                case 100053:    // Dol2Inputs_NoDvs_WithTnr
                case 100054:    // Dol3Inputs_NoDvs_NoTnr
                case 100055:    // Dol3Inputs_NoDvs_WithTnr
                    return 46539; // nntm_1_0
                case 100044:    // Bayer_NoPdaf_NoDvs_WithTnr_WithNntm_WithImv
                case 100050:    // Bayer_NoPdaf_WithRemosaic_NoDvs_WithTnr
                case 100051:    // Bayer_NoPdaf_WithB2b_WithNntm_WithTnr
                    return 33331; // imv
            }
            break;
        case HwSink::ProcessedSecondarySink:    return 19706; // sw_scaler
        case HwSink::AeOutSink:    return 55073; // aestatistics_2_1
    }

    return 0;
}

StaticGraphStatus GraphResolutionConfiguratorHelper::getRunKernelUuidForResHistoryUpdate(std::vector<uint32_t>& kernelUuids)
{
    kernelUuids.clear();

    // Must take only one from each resolution history index, since in static graph they all share the same
    // resolution history instance

    kernelUuids.push_back(38648);  // odr_output_ps_1_3
    kernelUuids.push_back(59680);  // odr_output_me_1_3
    kernelUuids.push_back(6907);  // slim_tnr_spatial_bifd_yuvn_regs_1_3
    kernelUuids.push_back(27847);  // ofs_dp_bodr_regs_1_3
    kernelUuids.push_back(61146);  // gmv_statistics_1_0
    kernelUuids.push_back(32160);  // odr_gmv_match_1_3
    kernelUuids.push_back(55924);  // odr_gmv_feature_1_3
    kernelUuids.push_back(25579);  // slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3
    kernelUuids.push_back(48987);  // tnr7_ims_1_1
    kernelUuids.push_back(42936);  // tnr_fp_blend_bifd_rs4n_regs_1_3
    kernelUuids.push_back(32696);  // tnr7_blend_1_0
    kernelUuids.push_back(20865);  // tnr_scale_fp_bodr_yuv4n_regs_1_3
    kernelUuids.push_back(48078);  // slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3
    kernelUuids.push_back(57803);  // tnr_sp_bc_bifd_yuv4n_regs_1_3
    kernelUuids.push_back(26536);  // slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3
    kernelUuids.push_back(5637);  // gdc7_1
    kernelUuids.push_back(19706);  // sw_scaler
    kernelUuids.push_back(46539);  // nntm_1_0
    kernelUuids.push_back(33331);  // imv
    return StaticGraphStatus::SG_OK;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid()
{
    return 47358;
}

