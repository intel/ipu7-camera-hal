/*
* INTEL CONFIDENTIAL
* Copyright (c) 2026 Intel Corporation
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

#include "Ipu8GraphResolutionConfiguratorAutogen.h"

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuid(GraphResolutionConfiguratorKernelRole role)
{
    switch (role)
    {
        case GraphResolutionConfiguratorKernelRole::UpScaler:    return 28787; // image_upscaler_1_1
        case GraphResolutionConfiguratorKernelRole::DownScaler:  return 40299; // b2i_ds_1_1
        case GraphResolutionConfiguratorKernelRole::EspaCropper:  return 65466; // lbff_crop_espa_1_4
    }

    return 0;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuidOfOutput(HwSink hwSink, int32_t graphId, GraphLink** links)
{
    (void)graphId;
    (void)links;

    switch (hwSink)
    {
        case HwSink::ImageMpSink:    return 16460; // odr_ofs_mp_1_4
        case HwSink::ImageDpSink:    return 37951; // odr_ofs_dp_1_4
        case HwSink::ProcessedMainSink:
            switch(graphId)
            {
                case 100001:    // Bayer_NoPdaf_WithDvs_NoTnr
                case 100003:    // Bayer_NoPdaf_WithDvs_WithTnr
                case 100137:    // Bayer_NoPdaf_WithDvs_WithTnr_WithSap_WithGdc
                case 100080:    // Bayer_NoPdaf_WithGdc_WithTnr
                case 100138:    // Bayer_NoPdaf_WithGdc_WithTnr_WithSap
                case 100142:    // Bayer_WithPdaf2_WithGdc_WithTnr_WithSap
                case 100143:    // Bayer_WithPdaf3_WithGdc_WithTnr_WithSap
                case 100144:    // Bayer_WithPdaf3asPdaf2_WithGdc_WithTnr_WithSap
                case 100081:    // Bayer_NoPdaf_WithGdc_WithDvs_WithTnr
                case 100005:    // Bayer_WithPdaf2_WithDvs_NoTnr
                case 100007:    // Bayer_WithPdaf2_WithDvs_WithTnr
                case 100139:    // Bayer_WithPdaf2_WithDvs_WithTnr_WithSap_WithGdc
                case 100009:    // Bayer_WithPdaf3_WithDvs_NoTnr
                case 100011:    // Bayer_WithPdaf3_WithDvs_WithTnr
                case 100140:    // Bayer_WithPdaf3_WithDvs_WithTnr_WithSap_WithGdc
                case 100013:    // Dol2Inputs_WithDvs_NoTnr
                case 100015:    // Dol2Inputs_WithDvs_WithTnr
                case 100017:    // Dol3Inputs_NoBurst_WithDvs_NoTnr
                case 100019:    // Dol3Inputs_NoBurst_WithDvs_WithTnr
                case 100021:    // RgbIr_WithDvs_NoTnr
                case 100023:    // RgbIr_WithDvs_WithTnr
                case 100040:    // Mipi_WithDvs
                case 100041:    // Mipi_WithDvs_WithTnr
                case 100028:    // Ir_WithDvs_NoTnr
                case 100030:    // Ir_WithDvs_WithTnr
                case 100032:    // Bayer_WithPdaf3asPdaf2_WithDvs_NoTnr
                case 100034:    // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr
                case 100141:    // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr_WithSap_WithGdc
                case 100101:    // Bayer_NoPdaf_WithDvs_NoTnr_WithSap
                case 100103:    // Bayer_NoPdaf_WithDvs_WithTnr_WithSap
                case 100105:    // Bayer_WithPdaf2_WithDvs_NoTnr_WithSap
                case 100107:    // Bayer_WithPdaf2_WithDvs_WithTnr_WithSap
                case 100109:    // Bayer_WithPdaf3_WithDvs_NoTnr_WithSap
                case 100111:    // Bayer_WithPdaf3_WithDvs_WithTnr_WithSap
                case 100113:    // Dol2Inputs_WithDvs_NoTnr_WithSap
                case 100146:    // Dol2Inputs_NoGmv_WithTnr_WithSap_WithGdc
                case 100115:    // Dol2Inputs_WithDvs_WithTnr_WithSap
                case 100117:    // Dol3Inputs_NoBurst_WithDvs_NoTnr_WithSap
                case 100119:    // Dol3Inputs_NoBurst_WithDvs_WithTnr_WithSap
                case 100121:    // RgbIr_WithDvs_NoTnr_WithSap
                case 100123:    // RgbIr_WithDvs_WithTnr_WithSap
                case 100128:    // Ir_WithDvs_NoTnr_WithSap
                case 100130:    // Ir_WithDvs_WithTnr_WithSap
                case 100132:    // Bayer_WithPdaf3asPdaf2_WithDvs_NoTnr_WithSap
                case 100134:    // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr_WithSap
                case 100236:    // Bayer_NoPdaf_WithDvs_NoTnr
                case 100203:    // Bayer_NoPdaf_WithDvs_WithTnr
                case 100280:    // Bayer_NoPdaf_WithGdc_WithTnr
                case 100281:    // Bayer_NoPdaf_WithGdc_WithDvs_WithTnr
                case 100205:    // Bayer_WithPdaf2_WithDvs_NoTnr
                case 100207:    // Bayer_WithPdaf2_WithDvs_WithTnr
                case 100209:    // Bayer_WithPdaf3_WithDvs_NoTnr
                case 100211:    // Bayer_WithPdaf3_WithDvs_WithTnr
                case 100213:    // Dol2Inputs_WithDvs_NoTnr
                case 100215:    // Dol2Inputs_WithDvs_WithTnr
                case 100217:    // Dol3Inputs_NoBurst_WithDvs_NoTnr
                case 100219:    // Dol3Inputs_NoBurst_WithDvs_WithTnr
                case 100221:    // RgbIr_WithDvs_NoTnr
                case 100223:    // RgbIr_WithDvs_WithTnr
                case 100240:    // Mipi_WithDvs
                case 100241:    // Mipi_WithDvs_WithTnr
                case 100228:    // Ir_WithDvs_NoTnr
                case 100230:    // Ir_WithDvs_WithTnr
                case 100232:    // Bayer_WithPdaf3asPdaf2_WithDvs_NoTnr
                case 100234:    // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr
                    return 5637; // gdc7_1
                case 100079:    // Bayer_NoPdaf_WithNntm_WithTnr
                case 100066:    // Bayer_WithPdaf2_NoGmv_WithTnr_WithNntm
                case 100067:    // Bayer_WithPdaf2_WithDvs_WithTnr_WithNntm
                case 100045:    // Bayer_WithPdaf3_WithNntm_WithTnr
                case 100012:    // Dol2Inputs_NoGmv_NoTnr
                case 100014:    // Dol2Inputs_NoGmv_WithTnr
                case 100016:    // Dol3Inputs_NoBurst_NoGmv_NoTnr
                case 100018:    // Dol3Inputs_NoBurst_NoGmv_WithTnr
                case 100135:    // Bayer_NoPdaf_WithNntm_WithTnr_WithSap
                case 100166:    // Bayer_WithPdaf2_NoGmv_WithTnr_WithSap_WithNntm
                case 100145:    // Bayer_WithPdaf2_WithDvs_WithTnr_WithSap_WithNntm
                case 100136:    // Bayer_WithPdaf3_WithNntm_WithTnr_WithSap
                case 100200:    // Bayer_WithPdaf3asPdaf2_WithNntm_WithTnr_WithSap
                case 100201:    // Bayer_WithPdaf3asPdaf2_WithNntm_WithTnr_NoSap
                case 100114:    // Dol2Inputs_NoGmv_WithTnr_WithSap
                case 100279:    // Bayer_NoPdaf_WithNntm_WithTnr
                case 100266:    // Bayer_WithPdaf2_NoGmv_WithTnr_WithNntm
                case 100267:    // Bayer_WithPdaf2_WithDvs_WithTnr_WithNntm
                case 100245:    // Bayer_WithPdaf3_WithNntm_WithTnr
                case 100212:    // Dol2Inputs_NoGmv_NoTnr
                case 100214:    // Dol2Inputs_NoGmv_WithTnr
                case 100216:    // Dol3Inputs_NoBurst_NoGmv_NoTnr
                case 100218:    // Dol3Inputs_NoBurst_NoGmv_WithTnr
                    return 46539; // nntm_1_0
            }
            break;
        case HwSink::ProcessedSecondarySink:    return 19706; // sw_scaler
        case HwSink::AeOutSink:    return 55073; // aestatistics_2_1
    }

    return 0;
}

StaticGraphStatus GraphResolutionConfiguratorHelper::getRunKernelUuidForResHistoryUpdate(std::vector<uint32_t>& kernelUuids, uint32_t startUuid)
{
    kernelUuids.clear();

    // Must take only one from each resolution history index, since in static graph they all share the same
    // resolution history instance
    if (startUuid == 0)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    if (startUuid == 65466) // ESPA Crop
    {
        kernelUuids.push_back(40280);  // gmv_statistics_1_1
        kernelUuids.push_back(7416);  // odr_gmv_feature_1_4
        kernelUuids.push_back(41148);  // odr_gmv_match_1_4
        kernelUuids.push_back(2495);  // tnr7_spatial_1_1
        kernelUuids.push_back(20119);  // tnr7_blend_1_1
        kernelUuids.push_back(65437);  // odr_tnr_scale_fp_yuv4n_1_4
        kernelUuids.push_back(23639);  // tnr7_ims_1_2
        kernelUuids.push_back(1502);  // tnr7_bc_1_2
    }
    else if (startUuid == 28787) // Upscaler
    {
        kernelUuids.push_back(9385);  // cas_1_1
        kernelUuids.push_back(37951);  // odr_ofs_dp_1_4
        kernelUuids.push_back(5637);  // gdc7_1
        kernelUuids.push_back(19706);  // sw_scaler
        kernelUuids.push_back(46539);  // nntm_1_0
    }
    return StaticGraphStatus::SG_OK;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid()
{
    return 47417;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelDownscalerSystemApiUuid()
{
    return 2102;
}

GraphResolutionConfiguratorKernelRole GraphResolutionConfiguratorHelper::getKernelRole(uint32_t kernelUuid)
{
    switch (kernelUuid)
    {
        case 28787 : return GraphResolutionConfiguratorKernelRole::UpScaler; // image_upscaler_1_1
        case 40299 : return GraphResolutionConfiguratorKernelRole::DownScaler; // b2i_ds_1_1
        case 50136 : return GraphResolutionConfiguratorKernelRole::DownScaler; // b2i_ds_output_1_1
        case 65466 : return GraphResolutionConfiguratorKernelRole::EspaCropper; // lbff_crop_espa_1_4
        case 9385 : return GraphResolutionConfiguratorKernelRole::NonRcb; // cas_1_1
        case 16460 : return GraphResolutionConfiguratorKernelRole::Output; // odr_ofs_mp_1_4
        case 37951 : return GraphResolutionConfiguratorKernelRole::Output; // odr_ofs_dp_1_4
        case 63731 : return GraphResolutionConfiguratorKernelRole::TnrOutput; // odr_tnr_sp_bc_rs4n_1_4
        case 5215 : return GraphResolutionConfiguratorKernelRole::TnrOutput; // odr_tnr_fp_yuvn_1_4
        case 65437 : return GraphResolutionConfiguratorKernelRole::TnrOutput; // odr_tnr_scale_fp_yuv4n_1_4
        case 29996 : return GraphResolutionConfiguratorKernelRole::TnrScaler; // tnr_scaler_lb_1_1
        case 20623 : return GraphResolutionConfiguratorKernelRole::TnrScaler; // tnr_scaler_fp_1_1
        case 16295 : return GraphResolutionConfiguratorKernelRole::TnrFeederFull; // ifd_tnr_fp_blend_yuvnm1_1_4
        case 7357 : return GraphResolutionConfiguratorKernelRole::TnrFeederSmall; // ifd_tnr_sp_bc_yuv4nm1_1_4
        case 62054 : return GraphResolutionConfiguratorKernelRole::TnrFeederSmall; // ifd_tnr_sp_bc_rs4nm1_1_4
        case 23639 : return GraphResolutionConfiguratorKernelRole::NonRcb; // tnr7_ims_1_2
        case 1502 : return GraphResolutionConfiguratorKernelRole::NonRcb; // tnr7_bc_1_2
        case 2495 : return GraphResolutionConfiguratorKernelRole::NonRcb; // tnr7_spatial_1_1
        case 20119 : return GraphResolutionConfiguratorKernelRole::NonRcb; // tnr7_blend_1_1
        case 13101 : return GraphResolutionConfiguratorKernelRole::Smurf; // smurf_tnr_bc_1_0
        case 42749 : return GraphResolutionConfiguratorKernelRole::Smurf; // smurf_tnr_blend_1_0
        case 37468 : return GraphResolutionConfiguratorKernelRole::Smurf; // smurf_cas_1_0
        case 35263 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_bnlm_1_4
        case 9241 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_xnr_1_4
        case 51914 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_acm_1_4
        case 47873 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_tnr_bc_1_4
        case 14619 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_tnr_blend_1_4
        case 20893 : return GraphResolutionConfiguratorKernelRole::SmurfFeeder; // ifd_segmap_cas_1_4

        default: return GraphResolutionConfiguratorKernelRole::None;
    }
}

uint32_t GraphResolutionConfiguratorHelper::getReferenceKernel(uint32_t kernelUuid)
{
    switch (kernelUuid)
    {
        case 29996 :     return 65466; // tnr_scaler_lb_1_1 from lbff_crop_espa_1_4
        case 20623 :     return 65466; // tnr_scaler_fp_1_1 from lbff_crop_espa_1_4
        case 28787 :     return 65466; // image_upscaler_1_1 from lbff_crop_espa_1_4
        case 50136 :     return 9385; // b2i_ds_output_1_1 from cas_1_1
        case 65437 :     return 20623; // odr_tnr_scale_fp_yuv4n_1_4 from tnr_scaler_fp_1_1
        case 30019 :     return 54721; // xnr_5_4 from gltm_2_0
        case 17531 :     return 36029; // acm_1_2 from glim_2_0
        case 1502 :     return 23639; // tnr7_bc_1_2 from tnr7_ims_1_2
        case 20119 :     return 16295; // tnr7_blend_1_1 from ifd_tnr_fp_blend_yuvnm1_1_4
        case 9385 :     return 28787; // cas_1_1 from image_upscaler_1_1
    }

    return 0;
}

FormatType GraphResolutionConfiguratorHelper::getFormatForDrainer(uint32_t kernelUuid)
{
    switch (kernelUuid)
    {
        case 16460 :     return FormatType::YUV420_8_SP_P; // odr_ofs_mp_1_4
        case 37951 :     return FormatType::YUV420_8_SP_P; // odr_ofs_dp_1_4
        case 63731 :     return FormatType::META_8; // odr_tnr_sp_bc_rs4n_1_4
        case 5215 :     return FormatType::YUV420_8_SP_P; // odr_tnr_fp_yuvn_1_4  HERE WE ASSUME NV12 OUTPUT!
        case 65437 :     return FormatType::YUV420_8_SP_P; // odr_tnr_scale_fp_yuv4n_1_4
    }

    return FormatType::YUV420_8_SP_P;
}

StaticGraphStatus GraphResolutionConfiguratorHelper::getSmurfRunKernelUuid(std::vector<std::pair<uint32_t, uint32_t>>& kernelUuids)
{
    kernelUuids.clear();

    std::pair <uint32_t, uint32_t> smurfPair;

    smurfPair.first = 13101;  // Smurf smurf_tnr_bc_1_0
    smurfPair.second = 1502;  // Connected To tnr7_bc_1_2
    kernelUuids.push_back(smurfPair);

    smurfPair.first = 42749;  // Smurf smurf_tnr_blend_1_0
    smurfPair.second = 20119;  // Connected To tnr7_blend_1_1
    kernelUuids.push_back(smurfPair);

    smurfPair.first = 37468;  // Smurf smurf_cas_1_0
    smurfPair.second = 9385;  // Connected To cas_1_1
    kernelUuids.push_back(smurfPair);

    return StaticGraphStatus::SG_OK;
}

