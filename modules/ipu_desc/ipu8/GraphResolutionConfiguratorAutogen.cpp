/*
 * Copyright (C) 2025 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GraphResolutionConfiguratorAutogen.h"

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuid(
    GraphResolutionConfiguratorKernelRole role) {
    switch (role) {
        case GraphResolutionConfiguratorKernelRole::UpScaler:
            return 28787;  // image_upscaler_1_1
        case GraphResolutionConfiguratorKernelRole::DownScaler:
            return 40299;  // b2i_ds_1_1
        case GraphResolutionConfiguratorKernelRole::EspaCropper:
            return 65466;  // lbff_crop_espa_1_4
    }

    return 0;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelUuidOfOutput(HwSink hwSink, int32_t graphId,
                                                                     GraphLink** links) {
    (void)graphId;
    (void)links;

    switch (hwSink) {
        case HwSink::ImageMpSink:
            return 16460;  // odr_ofs_mp_1_4
        case HwSink::ImageDpSink:
            return 37951;  // odr_ofs_dp_1_4
        case HwSink::ProcessedMainSink:
            switch (graphId) {
                case 100001:       // Bayer_NoPdaf_WithDvs_NoTnr
                case 100003:       // Bayer_NoPdaf_WithDvs_WithTnr
                case 100080:       // Bayer_NoPdaf_WithGdc_WithTnr
                case 100081:       // Bayer_NoPdaf_WithGdc_WithDvs_WithTnr
                case 100005:       // Bayer_WithPdaf2_WithDvs_NoTnr
                case 100007:       // Bayer_WithPdaf2_WithDvs_WithTnr
                case 100009:       // Bayer_WithPdaf3_WithDvs_NoTnr
                case 100011:       // Bayer_WithPdaf3_WithDvs_WithTnr
                case 100013:       // Dol2Inputs_WithDvs_NoTnr
                case 100015:       // Dol2Inputs_WithDvs_WithTnr
                case 100017:       // Dol3Inputs_NoBurst_WithDvs_NoTnr
                case 100019:       // Dol3Inputs_NoBurst_WithDvs_WithTnr
                case 100021:       // RgbIr_WithDvs_NoTnr
                case 100023:       // RgbIr_WithDvs_WithTnr
                case 100040:       // Mipi_WithDvs
                case 100041:       // Mipi_WithDvs_WithTnr
                case 100028:       // Ir_WithDvs_NoTnr
                case 100030:       // Ir_WithDvs_WithTnr
                case 100032:       // Bayer_WithPdaf3asPdaf2_WithDvs_NoTnr
                case 100034:       // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr
                case 100101:       // Bayer_NoPdaf_WithDvs_NoTnr_WithSap
                case 100103:       // Bayer_NoPdaf_WithDvs_WithTnr_WithSap
                case 100105:       // Bayer_WithPdaf2_WithDvs_NoTnr_WithSap
                case 100107:       // Bayer_WithPdaf2_WithDvs_WithTnr_WithSap
                case 100109:       // Bayer_WithPdaf3_WithDvs_NoTnr_WithSap
                case 100111:       // Bayer_WithPdaf3_WithDvs_WithTnr_WithSap
                case 100113:       // Dol2Inputs_WithDvs_NoTnr_WithSap
                case 100115:       // Dol2Inputs_WithDvs_WithTnr_WithSap
                case 100117:       // Dol3Inputs_NoBurst_WithDvs_NoTnr_WithSap
                case 100119:       // Dol3Inputs_NoBurst_WithDvs_WithTnr_WithSap
                case 100121:       // RgbIr_WithDvs_NoTnr_WithSap
                case 100123:       // RgbIr_WithDvs_WithTnr_WithSap
                case 100128:       // Ir_WithDvs_NoTnr_WithSap
                case 100130:       // Ir_WithDvs_WithTnr_WithSap
                case 100132:       // Bayer_WithPdaf3asPdaf2_WithDvs_NoTnr_WithSap
                case 100134:       // Bayer_WithPdaf3asPdaf2_WithDvs_WithTnr_WithSap
                    return 5637;   // gdc7_1
                case 100079:       // Bayer_NoPdaf_WithNntm_WithTnr
                case 100045:       // Bayer_WithPdaf3_WithNntm_WithTnr
                case 100012:       // Dol2Inputs_NoGmv_NoTnr
                case 100014:       // Dol2Inputs_NoGmv_WithTnr
                case 100016:       // Dol3Inputs_NoBurst_NoGmv_NoTnr
                case 100018:       // Dol3Inputs_NoBurst_NoGmv_WithTnr
                    return 46539;  // nntm_1_0
            }
            break;
        case HwSink::ProcessedSecondarySink:
            return 19706;  // sw_scaler
        case HwSink::AeOutSink:
            return 55073;  // aestatistics_2_1
        default:
            return 0;
    }

    return 0;
}

StaticGraphStatus GraphResolutionConfiguratorHelper::getRunKernelUuidForResHistoryUpdate(
    std::vector<uint32_t>& kernelUuids, uint32_t startUuid) {
    kernelUuids.clear();

    // Must take only one from each resolution history index, since in static graph they all share
    // the same resolution history instance
    if (startUuid == 65466)  // ESPA Crop
    {
        kernelUuids.push_back(40280);  // gmv_statistics_1_1
        kernelUuids.push_back(7416);   // odr_gmv_feature_1_4
        kernelUuids.push_back(41148);  // odr_gmv_match_1_4
        kernelUuids.push_back(2495);   // tnr7_spatial_1_1
        kernelUuids.push_back(20119);  // tnr7_blend_1_1
        kernelUuids.push_back(65437);  // odr_tnr_scale_fp_yuv4n_1_4
        kernelUuids.push_back(23639);  // tnr7_ims_1_2
        kernelUuids.push_back(1502);   // tnr7_bc_1_2
    } else if (startUuid == 28787)     // Upscaler
    {
        kernelUuids.push_back(9385);   // cas_1_1
        kernelUuids.push_back(37951);  // odr_ofs_dp_1_4
        kernelUuids.push_back(5637);   // gdc7_1
        kernelUuids.push_back(46539);  // nntm_1_0
        kernelUuids.push_back(19706);  // sw_scaler
    }
    return StaticGraphStatus::SG_OK;
}

uint32_t GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid() {
    return 47417;
}
