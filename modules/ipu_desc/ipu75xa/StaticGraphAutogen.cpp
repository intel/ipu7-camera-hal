/*
 * Copyright (C) 2023-2025 Intel Corporation.
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

#include "StaticGraphAutogen.h"
#define CHECK_BITMAP64_BIT(bitmap, index) ((bitmap) & ((uint64_t)1 << (index)))

/*
 * External Interfaces
 */
IStaticGraphConfig::IStaticGraphConfig(SensorMode* selectedSensorMode,
                                       VirtualSinkMapping* sinkMappingConfiguration,
                                       int32_t graphId, int32_t settingsId,
                                       ZoomKeyResolutions* zoomKeyResolutions)
        : _selectedSensorMode(selectedSensorMode),
          _graphId(graphId),
          _settingsId(settingsId) {
    memcpy(_sinkMappingConfiguration, sinkMappingConfiguration, sizeof(VirtualSinkMapping));
    // Copy zoom key resolutions
    _zoomKeyResolutions.numberOfZoomKeyOptions = zoomKeyResolutions->numberOfZoomKeyOptions;

    if (zoomKeyResolutions->numberOfZoomKeyOptions > 0) {
        _zoomKeyResolutions.zoomKeyResolutionOptions =
            new ZoomKeyResolution[zoomKeyResolutions->numberOfZoomKeyOptions];

        memcpy(_zoomKeyResolutions.zoomKeyResolutionOptions,
               zoomKeyResolutions->zoomKeyResolutionOptions,
               sizeof(ZoomKeyResolution) * zoomKeyResolutions->numberOfZoomKeyOptions);
    } else {
        _zoomKeyResolutions.zoomKeyResolutionOptions = nullptr;
    }
}

StaticGraphStatus IStaticGraphConfig::getSensorMode(SensorMode** sensorMode) {
    if (!sensorMode) {
        STATIC_GRAPH_LOG("Sensor mode does not exist for this setting.");
        return StaticGraphStatus::SG_OK;
    }
    *sensorMode = _selectedSensorMode;
    return StaticGraphStatus::SG_OK;
}
StaticGraphStatus IStaticGraphConfig::getZoomKeyResolutions(
    ZoomKeyResolutions** zoomKeyResolutions) {
    if (!zoomKeyResolutions || _zoomKeyResolutions.numberOfZoomKeyOptions == 0) {
        STATIC_GRAPH_LOG("Zoom key resolutions do not exist for this setting.");
        return StaticGraphStatus::SG_ERROR;
    }
    *zoomKeyResolutions = &_zoomKeyResolutions;
    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus IStaticGraphConfig::getGraphTopology(GraphTopology** topology) {
    *topology = _selectedGraphTopology;
    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus IStaticGraphConfig::getGraphId(int32_t* graphId) {
    if (graphId == nullptr) {
        return StaticGraphStatus::SG_ERROR;
    }

    *graphId = _graphId;
    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus IStaticGraphConfig::getSettingsId(int32_t* settingsId) {
    if (settingsId == nullptr) {
        return StaticGraphStatus::SG_ERROR;
    }

    *settingsId = _settingsId;
    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus IStaticGraphConfig::getVirtualSinkConnection(VirtualSink& virtualSink,
                                                               HwSink* hwSink) {
    switch (virtualSink) {
        case VirtualSink::PreviewSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->preview);
            break;
        case VirtualSink::VideoSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->video);
            break;
        case VirtualSink::PostProcessingVideoSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->postProcessingVideo);
            break;
        case VirtualSink::StillsSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->stills);
            break;
        case VirtualSink::ThumbnailSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->thumbnail);
            break;
        case VirtualSink::RawSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->raw);
            break;
        case VirtualSink::RawPdafSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->rawPdaf);
            break;
        case VirtualSink::RawDolLongSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->rawDolLong);
            break;
        case VirtualSink::VideoIrSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->videoIr);
            break;
        case VirtualSink::PreviewIrSink:
            *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->previewIr);
            break;
        default:
            STATIC_GRAPH_LOG("Failed to get virtual sink mapping for virtual sink %d",
                             static_cast<int>(virtualSink));
            return StaticGraphStatus::SG_ERROR;
    }

    return StaticGraphStatus::SG_OK;
}

GraphTopology::GraphTopology(GraphLink** links, int32_t numOfLinks,
                             VirtualSinkMapping* sinkMappingConfiguration)
        : links(links),
          numOfLinks(numOfLinks),
          _sinkMappingConfiguration(sinkMappingConfiguration) {}

StaticGraphStatus GraphTopology::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    // Default impl. No inner nodes in the sub-graph nodes.
    (void)subGraphInnerNodeConfiguration;
    return StaticGraphStatus::SG_OK;
}

InnerNodeOptionsFlags GraphTopology::GetInnerOptions(
    SubGraphPublicInnerNodeConfiguration* publicInnerOptions) {
    InnerNodeOptionsFlags res = None;

    if (publicInnerOptions) {
        res |= noGmv & (publicInnerOptions->noGmv ? -1 : 0);
        res |= no3A & (publicInnerOptions->no3A ? -1 : 0);
        res |= noMp & (publicInnerOptions->noMp ? -1 : 0);
        res |= noDp & (publicInnerOptions->noDp ? -1 : 0);
    }

    return res;
}

/*
 * Outer Nodes
 */

void OuterNode::Init(uint8_t nodeResourceId, NodeTypes nodeType, uint32_t kernelCount,
                     uint32_t nodeKernelConfigurationsOptionsCount, uint32_t operationMode,
                     uint32_t streamId, uint8_t nodeNumberOfFragments) {
    resourceId = nodeResourceId;
    type = nodeType;
    nodeKernels.kernelCount = kernelCount;
    numberOfFragments = nodeNumberOfFragments;
    kernelConfigurationsOptionsCount = nodeKernelConfigurationsOptionsCount;

    kernelListOptions = new StaticGraphPacRunKernel*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; i++) {
        if (kernelCount > 0) {
            kernelListOptions[i] = new StaticGraphPacRunKernel[kernelCount];
            for (uint32_t j = 0; j < kernelCount; j++) {
                kernelListOptions[i][j].fragment_descs = nullptr;
            }
        } else {
            kernelListOptions[i] = nullptr;
        }
    }

    selectedKernelConfigurationIndex = 0;
    nodeKernels.kernelList = kernelListOptions[0];

    nodeKernels.operationMode = operationMode;
    nodeKernels.streamId = streamId;
}
OuterNode::~OuterNode() {
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; i++) {
        delete[] kernelListOptions[i];
    }

    delete[] kernelListOptions;
}
void OuterNode::InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap,
                               StaticGraphKernelRes* resolutionInfos,
                               uint64_t kernelsResolutionHistoryGroupBitmap,
                               StaticGraphKernelRes* resolutionHistories,
                               StaticGraphKernelBppConfiguration* bppInfos,
                               uint8_t* systemApisSizes, uint8_t* systemApiData) {
    uint8_t* systemApiDataCurrentPtr = systemApiData;
    uint32_t currentResolutionHistoryIndex = 0;
    uint32_t currentRcbIndex = 0;

    for (uint32_t i = 0; i < nodeKernels.kernelCount; ++i) {
        auto& runKernel = nodeKernels.kernelList[i].run_kernel;
        runKernel.kernel_uuid = kernelsUuids[i];
        runKernel.stream_id = nodeKernels.streamId;
        runKernel.enable = 1;
        runKernel.output_count = 1;
        if (CHECK_BITMAP64_BIT(kernelsRcbBitmap, i)) {
            // RCB
            runKernel.resolution_info = &resolutionInfos[currentRcbIndex];
            currentRcbIndex++;
        } else {
            runKernel.resolution_info = nullptr;
        }

        if (CHECK_BITMAP64_BIT(kernelsResolutionHistoryGroupBitmap, i)) {
            // Next resolution history group
            currentResolutionHistoryIndex++;
        }
        runKernel.resolution_history = &resolutionHistories[currentResolutionHistoryIndex];

        runKernel.bpp_info.input_bpp = bppInfos[i].input_bpp;
        runKernel.bpp_info.output_bpp = bppInfos[i].output_bpp;

        // system API
        uint32_t systemApiSize = systemApisSizes[i];
        runKernel.system_api.size = systemApiSize;
        runKernel.system_api.data = systemApiSize != 0 ? systemApiDataCurrentPtr : nullptr;

        if (systemApiDataCurrentPtr) {
            systemApiDataCurrentPtr = systemApiDataCurrentPtr + systemApiSize;
        }

        // Metadata
        runKernel.metadata[0] = 0;
        runKernel.metadata[1] = 0;
        runKernel.metadata[2] = 0;
        runKernel.metadata[3] = 0;
    }
}

void OuterNode::SetDisabledKernels(uint64_t disabledRunKernelsBitmap) {
    for (uint32_t i = 0; i < nodeKernels.kernelCount; ++i) {
        // check the i'th bit in the bitmap
        if (CHECK_BITMAP64_BIT(disabledRunKernelsBitmap, i)) {
            nodeKernels.kernelList[i].run_kernel.enable = 2;  // disabled
        }
    }
}

StaticGraphStatus OuterNode::UpdateKernelsSelectedConfiguration(uint32_t selectedIndex) {
    if (selectedIndex >= kernelConfigurationsOptionsCount) {
        return StaticGraphStatus::SG_ERROR;
    }

    nodeKernels.kernelList = kernelListOptions[selectedIndex];
    selectedKernelConfigurationIndex = selectedIndex;
    return StaticGraphStatus::SG_OK;
}
uint8_t OuterNode::GetNumberOfFragments() {
    return numberOfFragments;
}

void IsysOuterNode::Init(IsysOuterNodeConfiguration** selectedGraphConfiguration,
                         uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(2, NodeTypes::Isys, 1, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {11470 /*is_odr_a*/};
    uint64_t kernelsRcbBitmap = 0x1;                     // { is_odr_a[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0;  // {{is_odr_a}[0] }

    uint8_t systemApisSizes[1] = {0 /*is_odr_a*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerOuterNode::Init(LbffBayerOuterNodeConfiguration** selectedGraphConfiguration,
                              uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x13FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                     // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                     // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                     // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                     // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                     // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                     // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                     // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                     // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsNoTnrOuterNode::Init(BbpsNoTnrOuterNodeConfiguration** selectedGraphConfiguration,
                              uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(1, NodeTypes::Cb, 5, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[5] = {6907 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/, 22660 /*cas_1_0*/,
                                18789 /*ofs_mp_bodr_regs_1_3*/, 6800 /*outputscaler_2_0_a*/,
                                27847 /*ofs_dp_bodr_regs_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x1C;  // { ofs_mp_bodr_regs_1_3[2], outputscaler_2_0_a[3], ofs_dp_bodr_regs_1_3[4] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x10;  // {{slim_tnr_spatial_bifd_yuvn_regs_1_3, cas_1_0, ofs_mp_bodr_regs_1_3,
               // outputscaler_2_0_a}[0], {ofs_dp_bodr_regs_1_3}[1] }

    uint8_t systemApisSizes[5] = {156 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/, 0 /*cas_1_0*/,
                                  156 /*ofs_mp_bodr_regs_1_3*/, 0 /*outputscaler_2_0_a*/,
                                  156 /*ofs_dp_bodr_regs_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerWithGmvOuterNode::Init(
    LbffBayerWithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x113FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                      // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                      // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28],
                      // gmv_statistics_1_0[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7FEE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                      // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                      // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                      // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                      // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                      // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                      // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12],
                      // {ifd_gmv_1_3}[13], {gmv_statistics_1_0}[14], {odr_gmv_match_1_3}[15],
                      // {odr_gmv_feature_1_3}[16] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsWithTnrOuterNode::Init(BbpsWithTnrOuterNodeConfiguration** selectedGraphConfiguration,
                                uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(1, NodeTypes::Cb, 18, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[18] = {25579 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3*/,
                                 48078 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3*/,
                                 57803 /*tnr_sp_bc_bifd_yuv4n_regs_1_3*/,
                                 48987 /*tnr7_ims_1_1*/,
                                 54840 /*tnr7_bc_1_1*/,
                                 39096 /*tnr_sp_bc_bodr_rs4n_regs_1_3*/,
                                 6907 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/,
                                 3133 /*tnr7_spatial_1_0*/,
                                 26536 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3*/,
                                 42936 /*tnr_fp_blend_bifd_rs4n_regs_1_3*/,
                                 32696 /*tnr7_blend_1_0*/,
                                 38465 /*tnr_fp_bodr_yuvn_regs_1_3*/,
                                 22660 /*cas_1_0*/,
                                 60056 /*tnr_scale_fp*/,
                                 18789 /*ofs_mp_bodr_regs_1_3*/,
                                 6800 /*outputscaler_2_0_a*/,
                                 27847 /*ofs_dp_bodr_regs_1_3*/,
                                 20865 /*tnr_scale_fp_bodr_yuv4n_regs_1_3*/};
    uint64_t kernelsRcbBitmap = 0x1E000;  // { tnr_scale_fp[13], ofs_mp_bodr_regs_1_3[14],
                                          // outputscaler_2_0_a[15], ofs_dp_bodr_regs_1_3[16] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3074E;  // {{slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3}[0],
                  // {slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3}[1], {tnr_sp_bc_bifd_yuv4n_regs_1_3}[2],
                  // {tnr7_ims_1_1, tnr7_bc_1_1, tnr_sp_bc_bodr_rs4n_regs_1_3}[3],
                  // {slim_tnr_spatial_bifd_yuvn_regs_1_3, tnr7_spatial_1_0}[4],
                  // {slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3}[5],
                  // {tnr_fp_blend_bifd_rs4n_regs_1_3}[6], {tnr7_blend_1_0,
                  // tnr_fp_bodr_yuvn_regs_1_3, cas_1_0, tnr_scale_fp, ofs_mp_bodr_regs_1_3,
                  // outputscaler_2_0_a}[7], {ofs_dp_bodr_regs_1_3}[8],
                  // {tnr_scale_fp_bodr_yuv4n_regs_1_3}[9] }

    uint8_t systemApisSizes[18] = {156 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3*/,
                                   156 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3*/,
                                   156 /*tnr_sp_bc_bifd_yuv4n_regs_1_3*/,
                                   0 /*tnr7_ims_1_1*/,
                                   0 /*tnr7_bc_1_1*/,
                                   156 /*tnr_sp_bc_bodr_rs4n_regs_1_3*/,
                                   156 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/,
                                   0 /*tnr7_spatial_1_0*/,
                                   156 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3*/,
                                   156 /*tnr_fp_blend_bifd_rs4n_regs_1_3*/,
                                   6 /*tnr7_blend_1_0*/,
                                   156 /*tnr_fp_bodr_yuvn_regs_1_3*/,
                                   0 /*cas_1_0*/,
                                   0 /*tnr_scale_fp*/,
                                   156 /*ofs_mp_bodr_regs_1_3*/,
                                   0 /*outputscaler_2_0_a*/,
                                   156 /*ofs_dp_bodr_regs_1_3*/,
                                   156 /*tnr_scale_fp_bodr_yuv4n_regs_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwGdcOuterNode::Init(SwGdcOuterNodeConfiguration** selectedGraphConfiguration,
                          uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(3, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {5637 /*gdc7_1*/};
    uint64_t kernelsRcbBitmap = 0x1;                     // { gdc7_1[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0;  // {{gdc7_1}[0] }

    uint8_t systemApisSizes[1] = {0 /*gdc7_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwScalerOuterNode::Init(SwScalerOuterNodeConfiguration** selectedGraphConfiguration,
                             uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(4, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {19706 /*sw_scaler*/};
    uint64_t kernelsRcbBitmap = 0x1;                     // { sw_scaler[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0;  // {{sw_scaler}[0] }

    uint8_t systemApisSizes[1] = {0 /*sw_scaler*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwNntmOuterNode::Init(SwNntmOuterNodeConfiguration** selectedGraphConfiguration,
                           uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(5, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {46539 /*nntm_1_0*/};
    uint64_t kernelsRcbBitmap = 0x0;                     // {  }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0;  // {{nntm_1_0}[0] }

    uint8_t systemApisSizes[1] = {5 /*nntm_1_0*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, nullptr, kernelsResolutionHistoryGroupBitmap,
                       selectedGraphConfiguration[i]->resolutionHistories,
                       selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
                       selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffRgbIrOuterNode::Init(LbffRgbIrOuterNodeConfiguration** selectedGraphConfiguration,
                              uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 34, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[34] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 33714 /*gd_dpc_2_2*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 14488 /*rgb_ir_2_0*/,
                                 28176 /*odr_ir_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0xFF800071;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[4], rgb_ir_2_0[5], odr_ir_1_3[6],
                     // ccm_3a_2_0[23], fr_grid_1_0[24], b2i_ds_1_1[25], upscaler_1_0[26],
                     // lbff_crop_espa_1_3[27], tnr_scale_lb[28], odr_output_ps_1_3[29],
                     // odr_output_me_1_3[30], aestatistics_2_1[31] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3DC000FC2;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0, gd_dpc_2_2, rgbs_grid_1_1,
                      // rgb_ir_2_0}[1], {odr_ir_1_3}[2], {odr_awb_std_1_3}[3],
                      // {odr_awb_sve_1_3}[4], {odr_awb_sat_1_3}[5], {ifd_lsc_1_3}[6], {lsc_1_2,
                      // wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1,
                      // glim_2_0, acm_1_1, gammatm_v4, csc_1_1, ccm_3a_2_0, fr_grid_1_0,
                      // b2i_ds_1_1}[7], {upscaler_1_0}[8], {lbff_crop_espa_1_3}[9], {tnr_scale_lb,
                      // odr_output_ps_1_3}[10], {odr_output_me_1_3}[11], {aestatistics_2_1}[12],
                      // {odr_ae_1_3}[13], {odr_af_std_1_3}[14] }

    uint8_t systemApisSizes[34] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   0 /*gd_dpc_2_2*/,
                                   24 /*rgbs_grid_1_1*/,
                                   0 /*rgb_ir_2_0*/,
                                   156 /*odr_ir_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][13].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrNoGmvIrStreamOuterNode::Init(
    LbffIrNoGmvIrStreamOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x13FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                     // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                     // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                     // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                     // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                     // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                     // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                     // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                     // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsIrWithTnrOuterNode::Init(BbpsIrWithTnrOuterNodeConfiguration** selectedGraphConfiguration,
                                  uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(1, NodeTypes::Cb, 18, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[18] = {25579 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3*/,
                                 48078 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3*/,
                                 57803 /*tnr_sp_bc_bifd_yuv4n_regs_1_3*/,
                                 48987 /*tnr7_ims_1_1*/,
                                 54840 /*tnr7_bc_1_1*/,
                                 39096 /*tnr_sp_bc_bodr_rs4n_regs_1_3*/,
                                 6907 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/,
                                 3133 /*tnr7_spatial_1_0*/,
                                 26536 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3*/,
                                 42936 /*tnr_fp_blend_bifd_rs4n_regs_1_3*/,
                                 32696 /*tnr7_blend_1_0*/,
                                 38465 /*tnr_fp_bodr_yuvn_regs_1_3*/,
                                 22660 /*cas_1_0*/,
                                 60056 /*tnr_scale_fp*/,
                                 18789 /*ofs_mp_bodr_regs_1_3*/,
                                 6800 /*outputscaler_2_0_a*/,
                                 27847 /*ofs_dp_bodr_regs_1_3*/,
                                 20865 /*tnr_scale_fp_bodr_yuv4n_regs_1_3*/};
    uint64_t kernelsRcbBitmap = 0x1E000;  // { tnr_scale_fp[13], ofs_mp_bodr_regs_1_3[14],
                                          // outputscaler_2_0_a[15], ofs_dp_bodr_regs_1_3[16] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3074E;  // {{slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3}[0],
                  // {slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3}[1], {tnr_sp_bc_bifd_yuv4n_regs_1_3}[2],
                  // {tnr7_ims_1_1, tnr7_bc_1_1, tnr_sp_bc_bodr_rs4n_regs_1_3}[3],
                  // {slim_tnr_spatial_bifd_yuvn_regs_1_3, tnr7_spatial_1_0}[4],
                  // {slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3}[5],
                  // {tnr_fp_blend_bifd_rs4n_regs_1_3}[6], {tnr7_blend_1_0,
                  // tnr_fp_bodr_yuvn_regs_1_3, cas_1_0, tnr_scale_fp, ofs_mp_bodr_regs_1_3,
                  // outputscaler_2_0_a}[7], {ofs_dp_bodr_regs_1_3}[8],
                  // {tnr_scale_fp_bodr_yuv4n_regs_1_3}[9] }

    uint8_t systemApisSizes[18] = {156 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3*/,
                                   156 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3*/,
                                   156 /*tnr_sp_bc_bifd_yuv4n_regs_1_3*/,
                                   0 /*tnr7_ims_1_1*/,
                                   0 /*tnr7_bc_1_1*/,
                                   156 /*tnr_sp_bc_bodr_rs4n_regs_1_3*/,
                                   156 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/,
                                   0 /*tnr7_spatial_1_0*/,
                                   156 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3*/,
                                   156 /*tnr_fp_blend_bifd_rs4n_regs_1_3*/,
                                   6 /*tnr7_blend_1_0*/,
                                   156 /*tnr_fp_bodr_yuvn_regs_1_3*/,
                                   0 /*cas_1_0*/,
                                   0 /*tnr_scale_fp*/,
                                   156 /*ofs_mp_bodr_regs_1_3*/,
                                   0 /*outputscaler_2_0_a*/,
                                   156 /*ofs_dp_bodr_regs_1_3*/,
                                   156 /*tnr_scale_fp_bodr_yuv4n_regs_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerBurstOutNo3AOuterNode::Init(
    LbffBayerBurstOutNo3AOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 32658 /*odr_burst_isp_1_3*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 6874 /*ifd_pdaf_1_3*/,
                                 43213 /*pext_1_0*/,
                                 44308 /*pafstatistics_1_2*/,
                                 24208 /*odr_pdaf_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x137F0001;  // { ifd_pipe_1_3[0], odr_burst_isp_1_3[16], b2i_ds_1_1[17], upscaler_1_0[18],
                     // lbff_crop_espa_1_3[19], tnr_scale_lb[20], odr_output_ps_1_3[21],
                     // odr_output_me_1_3[22], pext_1_0[24], pafstatistics_1_2[25],
                     // gmv_statistics_1_0[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EDC0002;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0, lsc_1_2, gd_dpc_2_2, wb_1_1,
                     // bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0,
                     // acm_1_1, gammatm_v4, csc_1_1, odr_burst_isp_1_3, b2i_ds_1_1}[1],
                     // {upscaler_1_0}[2], {lbff_crop_espa_1_3}[3], {tnr_scale_lb,
                     // odr_output_ps_1_3}[4], {odr_output_me_1_3}[5], {ifd_pdaf_1_3, pext_1_0}[6],
                     // {pafstatistics_1_2}[7], {odr_pdaf_1_3}[8], {ifd_gmv_1_3}[9],
                     // {gmv_statistics_1_0}[10], {odr_gmv_match_1_3}[11], {odr_gmv_feature_1_3}[12]
                     // }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   156 /*odr_burst_isp_1_3*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*ifd_pdaf_1_3*/,
                                   24 /*pext_1_0*/,
                                   8 /*pafstatistics_1_2*/,
                                   156 /*odr_pdaf_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][6].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsIrNoTnrOuterNode::Init(BbpsIrNoTnrOuterNodeConfiguration** selectedGraphConfiguration,
                                uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(1, NodeTypes::Cb, 5, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[5] = {6907 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/, 22660 /*cas_1_0*/,
                                18789 /*ofs_mp_bodr_regs_1_3*/, 6800 /*outputscaler_2_0_a*/,
                                27847 /*ofs_dp_bodr_regs_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x1C;  // { ofs_mp_bodr_regs_1_3[2], outputscaler_2_0_a[3], ofs_dp_bodr_regs_1_3[4] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x10;  // {{slim_tnr_spatial_bifd_yuvn_regs_1_3, cas_1_0, ofs_mp_bodr_regs_1_3,
               // outputscaler_2_0_a}[0], {ofs_dp_bodr_regs_1_3}[1] }

    uint8_t systemApisSizes[5] = {156 /*slim_tnr_spatial_bifd_yuvn_regs_1_3*/, 0 /*cas_1_0*/,
                                  156 /*ofs_mp_bodr_regs_1_3*/, 0 /*outputscaler_2_0_a*/,
                                  156 /*ofs_dp_bodr_regs_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrNoGmvOuterNode::Init(LbffIrNoGmvOuterNodeConfiguration** selectedGraphConfiguration,
                                uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x13FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                     // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                     // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                     // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                     // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                     // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                     // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                     // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                     // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void IsysPdaf2OuterNode::Init(IsysPdaf2OuterNodeConfiguration** selectedGraphConfiguration,
                              uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(2, NodeTypes::Isys, 2, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[2] = {11470 /*is_odr_a*/, 55449 /*is_odr_b*/};
    uint64_t kernelsRcbBitmap = 0x3;                     // { is_odr_a[0], is_odr_b[1] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x2;  // {{is_odr_a}[0], {is_odr_b}[1] }

    uint8_t systemApisSizes[2] = {0 /*is_odr_a*/, 0 /*is_odr_b*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf2OuterNode::Init(
    LbffBayerPdaf2OuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 6874 /*ifd_pdaf_1_3*/,
                                 43213 /*pext_1_0*/,
                                 44308 /*pafstatistics_1_2*/,
                                 24208 /*odr_pdaf_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x313FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                      // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                      // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28],
                      // pext_1_0[32], pafstatistics_1_2[33] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x6FEE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                      // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                      // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                      // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                      // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                      // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                      // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12],
                      // {ifd_pdaf_1_3, pext_1_0}[13], {pafstatistics_1_2}[14], {odr_pdaf_1_3}[15] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_pdaf_1_3*/,
                                   24 /*pext_1_0*/,
                                   8 /*pafstatistics_1_2*/,
                                   156 /*odr_pdaf_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf3OuterNode::Init(
    LbffBayerPdaf3OuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 34, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[34] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 43213 /*pext_1_0*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 44308 /*pafstatistics_1_2*/,
                                 24208 /*odr_pdaf_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x127FE0001;  // { ifd_pipe_1_3[0], pext_1_0[17], rgbs_grid_1_1[18], ccm_3a_2_0[19],
                      // fr_grid_1_0[20], b2i_ds_1_1[21], upscaler_1_0[22], lbff_crop_espa_1_3[23],
                      // tnr_scale_lb[24], odr_output_ps_1_3[25], odr_output_me_1_3[26],
                      // aestatistics_2_1[29], pafstatistics_1_2[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3FDC0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                      // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                      // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, pext_1_0,
                      // rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                      // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                      // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                      // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12],
                      // {pafstatistics_1_2}[13], {odr_pdaf_1_3}[14] }

    uint8_t systemApisSizes[34] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*pext_1_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   8 /*pafstatistics_1_2*/,
                                   156 /*odr_pdaf_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void IsysDolOuterNode::Init(IsysDolOuterNodeConfiguration** selectedGraphConfiguration,
                            uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(2, NodeTypes::Isys, 2, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[2] = {11470 /*is_odr_a*/, 50407 /*is_odr_c*/};
    uint64_t kernelsRcbBitmap = 0x3;                     // { is_odr_a[0], is_odr_c[1] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x2;  // {{is_odr_a}[0], {is_odr_c}[1] }

    uint8_t systemApisSizes[2] = {0 /*is_odr_a*/, 0 /*is_odr_c*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDol2InputsOuterNode::Init(
    LbffDol2InputsOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 34, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[34] = {55223 /*ifd_pipe_1_3*/,      52982 /*ifd_pipe_long_1_3*/,
                                 22000 /*dol_lite_1_1*/,      11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,  15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,        1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,    45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,  53496 /*odr_ae_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,       2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,        5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,          48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,        54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,           36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,          13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,         62703 /*csc_1_1*/,
                                 26958 /*fr_grid_1_0*/,       40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,      42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,      38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/, 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x1FC000463;  // { ifd_pipe_1_3[0], ifd_pipe_long_1_3[1], rgbs_grid_1_1[5], ccm_3a_2_0[6],
                      // aestatistics_2_1[10], fr_grid_1_0[26], b2i_ds_1_1[27], upscaler_1_0[28],
                      // lbff_crop_espa_1_3[29], tnr_scale_lb[30], odr_output_ps_1_3[31],
                      // odr_output_me_1_3[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x370003F86;  // {{ifd_pipe_1_3}[0], {ifd_pipe_long_1_3}[1], {dol_lite_1_1, bxt_blc,
                      // linearization2_0, rgbs_grid_1_1, ccm_3a_2_0}[2], {odr_awb_std_1_3}[3],
                      // {odr_awb_sve_1_3}[4], {odr_awb_sat_1_3}[5], {aestatistics_2_1}[6],
                      // {odr_ae_1_3}[7], {ifd_lsc_1_3}[8], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3,
                      // bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1,
                      // gammatm_v4, csc_1_1, fr_grid_1_0, b2i_ds_1_1}[9], {upscaler_1_0}[10],
                      // {lbff_crop_espa_1_3}[11], {tnr_scale_lb, odr_output_ps_1_3}[12],
                      // {odr_output_me_1_3}[13], {odr_af_std_1_3}[14] }

    uint8_t systemApisSizes[34] = {156 /*ifd_pipe_1_3*/,
                                   156 /*ifd_pipe_long_1_3*/,
                                   5 /*dol_lite_1_1*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][16].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDolSmoothOuterNode::Init(LbffDolSmoothOuterNodeConfiguration** selectedGraphConfiguration,
                                  uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 7, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[7] = {55223 /*ifd_pipe_1_3*/,
                                11700 /*bxt_blc*/,
                                10326 /*linearization2_0*/,
                                33714 /*gd_dpc_2_2*/,
                                5144 /*wb_1_1*/,
                                21777 /*bnlm_3_3*/,
                                56904 /*odr_bnlm_1_3*/};
    uint64_t kernelsRcbBitmap = 0x41;  // { ifd_pipe_1_3[0], odr_bnlm_1_3[6] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x2;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0, gd_dpc_2_2, wb_1_1, bnlm_3_3,
              // odr_bnlm_1_3}[1] }

    uint8_t systemApisSizes[7] = {156 /*ifd_pipe_1_3*/,
                                  5 /*bxt_blc*/,
                                  5 /*linearization2_0*/,
                                  0 /*gd_dpc_2_2*/,
                                  0 /*wb_1_1*/,
                                  5 /*bnlm_3_3*/,
                                  156 /*odr_bnlm_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][5].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDol3InputsOuterNode::Init(
    LbffDol3InputsOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {55223 /*ifd_pipe_1_3*/,
                                 52982 /*ifd_pipe_long_1_3*/,
                                 49695 /*ifd_pipe_short_smth_1_3*/,
                                 22000 /*dol_lite_1_1*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 23958 /*odr_af_std_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x3F80008C7;  // { ifd_pipe_1_3[0], ifd_pipe_long_1_3[1], ifd_pipe_short_smth_1_3[2],
                      // rgbs_grid_1_1[6], ccm_3a_2_0[7], aestatistics_2_1[11], fr_grid_1_0[27],
                      // b2i_ds_1_1[28], upscaler_1_0[29], lbff_crop_espa_1_3[30], tnr_scale_lb[31],
                      // odr_output_ps_1_3[32], odr_output_me_1_3[33] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x6E0007F0E;  // {{ifd_pipe_1_3}[0], {ifd_pipe_long_1_3}[1], {ifd_pipe_short_smth_1_3}[2],
                      // {dol_lite_1_1, bxt_blc, linearization2_0, rgbs_grid_1_1, ccm_3a_2_0}[3],
                      // {odr_awb_std_1_3}[4], {odr_awb_sve_1_3}[5], {odr_awb_sat_1_3}[6],
                      // {aestatistics_2_1}[7], {odr_ae_1_3}[8], {ifd_lsc_1_3}[9], {lsc_1_2,
                      // gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2,
                      // vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, fr_grid_1_0,
                      // b2i_ds_1_1}[10], {upscaler_1_0}[11], {lbff_crop_espa_1_3}[12],
                      // {tnr_scale_lb, odr_output_ps_1_3}[13], {odr_output_me_1_3}[14],
                      // {odr_af_std_1_3}[15] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_3*/,
                                   156 /*ifd_pipe_long_1_3*/,
                                   156 /*ifd_pipe_short_smth_1_3*/,
                                   5 /*dol_lite_1_1*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_af_std_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][17].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf2WithGmvOuterNode::Init(
    LbffBayerPdaf2WithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 39, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[39] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 6874 /*ifd_pdaf_1_3*/,
                                 43213 /*pext_1_0*/,
                                 44308 /*pafstatistics_1_2*/,
                                 24208 /*odr_pdaf_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x1313FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                       // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22],
                       // tnr_scale_lb[23], odr_output_ps_1_3[24], odr_output_me_1_3[25],
                       // aestatistics_2_1[28], pext_1_0[32], pafstatistics_1_2[33],
                       // gmv_statistics_1_0[36] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EFEE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                       // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b,
                       // gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1,
                       // rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                       // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                       // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                       // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12],
                       // {ifd_pdaf_1_3, pext_1_0}[13], {pafstatistics_1_2}[14], {odr_pdaf_1_3}[15],
                       // {ifd_gmv_1_3}[16], {gmv_statistics_1_0}[17], {odr_gmv_match_1_3}[18],
                       // {odr_gmv_feature_1_3}[19] }

    uint8_t systemApisSizes[39] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_pdaf_1_3*/,
                                   24 /*pext_1_0*/,
                                   8 /*pafstatistics_1_2*/,
                                   156 /*odr_pdaf_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf3WithGmvOuterNode::Init(
    LbffBayerPdaf3WithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 38, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[38] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 43213 /*pext_1_0*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 44308 /*pafstatistics_1_2*/,
                                 24208 /*odr_pdaf_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x927FE0001;  // { ifd_pipe_1_3[0], pext_1_0[17], rgbs_grid_1_1[18], ccm_3a_2_0[19],
                      // fr_grid_1_0[20], b2i_ds_1_1[21], upscaler_1_0[22], lbff_crop_espa_1_3[23],
                      // tnr_scale_lb[24], odr_output_ps_1_3[25], odr_output_me_1_3[26],
                      // aestatistics_2_1[29], pafstatistics_1_2[32], gmv_statistics_1_0[35] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3FFDC0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                       // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b,
                       // gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1,
                       // pext_1_0, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3],
                       // {upscaler_1_0}[4], {lbff_crop_espa_1_3}[5], {tnr_scale_lb,
                       // odr_output_ps_1_3}[6], {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8],
                       // {odr_awb_sat_1_3}[9], {aestatistics_2_1}[10], {odr_ae_1_3}[11],
                       // {odr_af_std_1_3}[12], {pafstatistics_1_2}[13], {odr_pdaf_1_3}[14],
                       // {ifd_gmv_1_3}[15], {gmv_statistics_1_0}[16], {odr_gmv_match_1_3}[17],
                       // {odr_gmv_feature_1_3}[18] }

    uint8_t systemApisSizes[38] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*pext_1_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   8 /*pafstatistics_1_2*/,
                                   156 /*odr_pdaf_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffRgbIrWithGmvOuterNode::Init(
    LbffRgbIrWithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 38, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[38] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 33714 /*gd_dpc_2_2*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 14488 /*rgb_ir_2_0*/,
                                 28176 /*odr_ir_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x8FF800071;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[4], rgb_ir_2_0[5], odr_ir_1_3[6],
                      // ccm_3a_2_0[23], fr_grid_1_0[24], b2i_ds_1_1[25], upscaler_1_0[26],
                      // lbff_crop_espa_1_3[27], tnr_scale_lb[28], odr_output_ps_1_3[29],
                      // odr_output_me_1_3[30], aestatistics_2_1[31], gmv_statistics_1_0[35] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3FDC000FC2;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0, gd_dpc_2_2, rgbs_grid_1_1,
                       // rgb_ir_2_0}[1], {odr_ir_1_3}[2], {odr_awb_std_1_3}[3],
                       // {odr_awb_sve_1_3}[4], {odr_awb_sat_1_3}[5], {ifd_lsc_1_3}[6], {lsc_1_2,
                       // wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1,
                       // glim_2_0, acm_1_1, gammatm_v4, csc_1_1, ccm_3a_2_0, fr_grid_1_0,
                       // b2i_ds_1_1}[7], {upscaler_1_0}[8], {lbff_crop_espa_1_3}[9], {tnr_scale_lb,
                       // odr_output_ps_1_3}[10], {odr_output_me_1_3}[11], {aestatistics_2_1}[12],
                       // {odr_ae_1_3}[13], {odr_af_std_1_3}[14], {ifd_gmv_1_3}[15],
                       // {gmv_statistics_1_0}[16], {odr_gmv_match_1_3}[17],
                       // {odr_gmv_feature_1_3}[18] }

    uint8_t systemApisSizes[38] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   0 /*gd_dpc_2_2*/,
                                   24 /*rgbs_grid_1_1*/,
                                   0 /*rgb_ir_2_0*/,
                                   156 /*odr_ir_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][13].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrWithGmvIrStreamOuterNode::Init(
    LbffIrWithGmvIrStreamOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {55223 /*ifd_pipe_1_3*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x113FE0001;  // { ifd_pipe_1_3[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19],
                      // b2i_ds_1_1[20], upscaler_1_0[21], lbff_crop_espa_1_3[22], tnr_scale_lb[23],
                      // odr_output_ps_1_3[24], odr_output_me_1_3[25], aestatistics_2_1[28],
                      // gmv_statistics_1_0[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7FEE0001A;  // {{ifd_pipe_1_3}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_3}[2],
                      // {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0,
                      // xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, rgbs_grid_1_1,
                      // ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_1}[3], {upscaler_1_0}[4],
                      // {lbff_crop_espa_1_3}[5], {tnr_scale_lb, odr_output_ps_1_3}[6],
                      // {odr_output_me_1_3}[7], {odr_awb_std_1_3}[8], {odr_awb_sat_1_3}[9],
                      // {aestatistics_2_1}[10], {odr_ae_1_3}[11], {odr_af_std_1_3}[12],
                      // {ifd_gmv_1_3}[13], {gmv_statistics_1_0}[14], {odr_gmv_match_1_3}[15],
                      // {odr_gmv_feature_1_3}[16] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_3*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDol2InputsWithGmvOuterNode::Init(
    LbffDol2InputsWithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 38, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[38] = {55223 /*ifd_pipe_1_3*/,      52982 /*ifd_pipe_long_1_3*/,
                                 22000 /*dol_lite_1_1*/,      11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,  15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,        1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,    45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,  53496 /*odr_ae_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,       2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,        5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,          48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,        54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,           36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,          13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,         62703 /*csc_1_1*/,
                                 26958 /*fr_grid_1_0*/,       40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,      42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,      38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/, 23958 /*odr_af_std_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,       61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/, 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x9FC000463;  // { ifd_pipe_1_3[0], ifd_pipe_long_1_3[1], rgbs_grid_1_1[5], ccm_3a_2_0[6],
                      // aestatistics_2_1[10], fr_grid_1_0[26], b2i_ds_1_1[27], upscaler_1_0[28],
                      // lbff_crop_espa_1_3[29], tnr_scale_lb[30], odr_output_ps_1_3[31],
                      // odr_output_me_1_3[32], gmv_statistics_1_0[35] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x3F70003F86;  // {{ifd_pipe_1_3}[0], {ifd_pipe_long_1_3}[1], {dol_lite_1_1, bxt_blc,
                       // linearization2_0, rgbs_grid_1_1, ccm_3a_2_0}[2], {odr_awb_std_1_3}[3],
                       // {odr_awb_sve_1_3}[4], {odr_awb_sat_1_3}[5], {aestatistics_2_1}[6],
                       // {odr_ae_1_3}[7], {ifd_lsc_1_3}[8], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3,
                       // bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1,
                       // gammatm_v4, csc_1_1, fr_grid_1_0, b2i_ds_1_1}[9], {upscaler_1_0}[10],
                       // {lbff_crop_espa_1_3}[11], {tnr_scale_lb, odr_output_ps_1_3}[12],
                       // {odr_output_me_1_3}[13], {odr_af_std_1_3}[14], {ifd_gmv_1_3}[15],
                       // {gmv_statistics_1_0}[16], {odr_gmv_match_1_3}[17],
                       // {odr_gmv_feature_1_3}[18] }

    uint8_t systemApisSizes[38] = {156 /*ifd_pipe_1_3*/,
                                   156 /*ifd_pipe_long_1_3*/,
                                   5 /*dol_lite_1_1*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][16].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDol3InputsWithGmvOuterNode::Init(
    LbffDol3InputsWithGmvOuterNodeConfiguration** selectedGraphConfiguration,
    uint32_t nodeKernelConfigurationsOptionsCount) {
    OuterNode::Init(0, NodeTypes::Cb, 39, nodeKernelConfigurationsOptionsCount,
                    selectedGraphConfiguration[0]->tuningMode,
                    selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[39] = {55223 /*ifd_pipe_1_3*/,
                                 52982 /*ifd_pipe_long_1_3*/,
                                 49695 /*ifd_pipe_short_smth_1_3*/,
                                 22000 /*dol_lite_1_1*/,
                                 11700 /*bxt_blc*/,
                                 10326 /*linearization2_0*/,
                                 15021 /*rgbs_grid_1_1*/,
                                 62344 /*ccm_3a_2_0*/,
                                 1338 /*odr_awb_std_1_3*/,
                                 8720 /*odr_awb_sve_1_3*/,
                                 45123 /*odr_awb_sat_1_3*/,
                                 55073 /*aestatistics_2_1*/,
                                 53496 /*odr_ae_1_3*/,
                                 27730 /*ifd_lsc_1_3*/,
                                 2144 /*lsc_1_2*/,
                                 33714 /*gd_dpc_2_2*/,
                                 5144 /*wb_1_1*/,
                                 21777 /*bnlm_3_3*/,
                                 48695 /*bxt_demosaic*/,
                                 13708 /*vcsc_2_0_b*/,
                                 54721 /*gltm_2_0*/,
                                 58858 /*xnr_5_2*/,
                                 36035 /*vcr_3_1*/,
                                 36029 /*glim_2_0*/,
                                 13026 /*acm_1_1*/,
                                 5394 /*gammatm_v4*/,
                                 62703 /*csc_1_1*/,
                                 26958 /*fr_grid_1_0*/,
                                 40299 /*b2i_ds_1_1*/,
                                 25569 /*upscaler_1_0*/,
                                 42330 /*lbff_crop_espa_1_3*/,
                                 33723 /*tnr_scale_lb*/,
                                 38648 /*odr_output_ps_1_3*/,
                                 59680 /*odr_output_me_1_3*/,
                                 23958 /*odr_af_std_1_3*/,
                                 62409 /*ifd_gmv_1_3*/,
                                 61146 /*gmv_statistics_1_0*/,
                                 32160 /*odr_gmv_match_1_3*/,
                                 55924 /*odr_gmv_feature_1_3*/};
    uint64_t kernelsRcbBitmap =
        0x13F80008C7;  // { ifd_pipe_1_3[0], ifd_pipe_long_1_3[1], ifd_pipe_short_smth_1_3[2],
                       // rgbs_grid_1_1[6], ccm_3a_2_0[7], aestatistics_2_1[11], fr_grid_1_0[27],
                       // b2i_ds_1_1[28], upscaler_1_0[29], lbff_crop_espa_1_3[30],
                       // tnr_scale_lb[31], odr_output_ps_1_3[32], odr_output_me_1_3[33],
                       // gmv_statistics_1_0[36] }
    uint64_t kernelsResolutionHistoryGroupBitmap =
        0x7EE0007F0E;  // {{ifd_pipe_1_3}[0], {ifd_pipe_long_1_3}[1], {ifd_pipe_short_smth_1_3}[2],
                       // {dol_lite_1_1, bxt_blc, linearization2_0, rgbs_grid_1_1, ccm_3a_2_0}[3],
                       // {odr_awb_std_1_3}[4], {odr_awb_sve_1_3}[5], {odr_awb_sat_1_3}[6],
                       // {aestatistics_2_1}[7], {odr_ae_1_3}[8], {ifd_lsc_1_3}[9], {lsc_1_2,
                       // gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2,
                       // vcr_3_1, glim_2_0, acm_1_1, gammatm_v4, csc_1_1, fr_grid_1_0,
                       // b2i_ds_1_1}[10], {upscaler_1_0}[11], {lbff_crop_espa_1_3}[12],
                       // {tnr_scale_lb, odr_output_ps_1_3}[13], {odr_output_me_1_3}[14],
                       // {odr_af_std_1_3}[15], {ifd_gmv_1_3}[16], {gmv_statistics_1_0}[17],
                       // {odr_gmv_match_1_3}[18], {odr_gmv_feature_1_3}[19] }

    uint8_t systemApisSizes[39] = {156 /*ifd_pipe_1_3*/,
                                   156 /*ifd_pipe_long_1_3*/,
                                   156 /*ifd_pipe_short_smth_1_3*/,
                                   5 /*dol_lite_1_1*/,
                                   5 /*bxt_blc*/,
                                   5 /*linearization2_0*/,
                                   24 /*rgbs_grid_1_1*/,
                                   5 /*ccm_3a_2_0*/,
                                   156 /*odr_awb_std_1_3*/,
                                   156 /*odr_awb_sve_1_3*/,
                                   156 /*odr_awb_sat_1_3*/,
                                   24 /*aestatistics_2_1*/,
                                   156 /*odr_ae_1_3*/,
                                   156 /*ifd_lsc_1_3*/,
                                   40 /*lsc_1_2*/,
                                   0 /*gd_dpc_2_2*/,
                                   0 /*wb_1_1*/,
                                   5 /*bnlm_3_3*/,
                                   0 /*bxt_demosaic*/,
                                   0 /*vcsc_2_0_b*/,
                                   0 /*gltm_2_0*/,
                                   0 /*xnr_5_2*/,
                                   0 /*vcr_3_1*/,
                                   0 /*glim_2_0*/,
                                   0 /*acm_1_1*/,
                                   0 /*gammatm_v4*/,
                                   0 /*csc_1_1*/,
                                   20 /*fr_grid_1_0*/,
                                   0 /*b2i_ds_1_1*/,
                                   0 /*upscaler_1_0*/,
                                   156 /*lbff_crop_espa_1_3*/,
                                   0 /*tnr_scale_lb*/,
                                   156 /*odr_output_ps_1_3*/,
                                   156 /*odr_output_me_1_3*/,
                                   156 /*odr_af_std_1_3*/,
                                   156 /*ifd_gmv_1_3*/,
                                   0 /*gmv_statistics_1_0*/,
                                   156 /*odr_gmv_match_1_3*/,
                                   156 /*odr_gmv_feature_1_3*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(
            kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos,
            kernelsResolutionHistoryGroupBitmap, selectedGraphConfiguration[i]->resolutionHistories,
            selectedGraphConfiguration[i]->bppInfos, systemApisSizes,
            selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        kernelListOptions[i][17].run_kernel.metadata[0] = 1;  // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

/*
 * Inner Nodes Setters
 */
void IsysOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffBayerOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 31; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void BbpsNoTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 5; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000020
        bitmaps.rbm[0] = 0x20;
        // DEB - 0x00000000000000000000000000034040
        bitmaps.deb[0] = 0x34040;
        // TEB - 0x000000000000820F
        bitmaps.teb[0] = 0x820F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x4;
    } else if (nodeRelevantInnerOptions == (noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000010
        bitmaps.rbm[0] = 0x10;
        // DEB - 0x0000000000000000000000000000C040
        bitmaps.deb[0] = 0xC040;
        // TEB - 0x000000000000420F
        bitmaps.teb[0] = 0x420F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x18;
    } else if (nodeRelevantInnerOptions == (noMp | noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_spatial_bifd_yuvn_regs_1_3- inner node disablement
        // 1 cas_1_0- inner node disablement
        // 2 ofs_mp_bodr_regs_1_3- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1F;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000000000000030
        bitmaps.rbm[0] = 0x30;
        // DEB - 0x0000000000000000000000000003C040
        bitmaps.deb[0] = 0x3C040;
        // TEB - 0x000000000000C20F
        bitmaps.teb[0] = 0xC20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffBayerWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 35; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0127
        bitmaps.teb[0] = 0x19C0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001943D27
        bitmaps.teb[0] = 0x1943D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940127
        bitmaps.teb[0] = 0x1940127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001983D27
        bitmaps.teb[0] = 0x1983D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980127
        bitmaps.teb[0] = 0x1980127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001903D27
        bitmaps.teb[0] = 0x1903D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900127
        bitmaps.teb[0] = 0x1900127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000003EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C3D27
        bitmaps.teb[0] = 0x19C3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void BbpsWithTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 18; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000002F
        bitmaps.rbm[0] = 0x2F;
        // DEB - 0x00000000000000000000000000037FFF
        bitmaps.deb[0] = 0x37FFF;
        // TEB - 0x000000000000BFEF
        bitmaps.teb[0] = 0xBFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x4000;
    } else if (nodeRelevantInnerOptions == (noDp)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000001F
        bitmaps.rbm[0] = 0x1F;
        // DEB - 0x0000000000000000000000000000FFFF
        bitmaps.deb[0] = 0xFFFF;
        // TEB - 0x0000000000007FEF
        bitmaps.teb[0] = 0x7FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x18000;
    } else if (nodeRelevantInnerOptions == (noMp | noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3- inner node disablement
        // 1 slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3- inner node disablement
        // 2 tnr_sp_bc_bifd_yuv4n_regs_1_3- inner node disablement
        // 3 tnr7_ims_1_1- inner node disablement
        // 4 tnr7_bc_1_1- inner node disablement
        // 5 tnr_sp_bc_bodr_rs4n_regs_1_3- inner node disablement
        // 6 slim_tnr_spatial_bifd_yuvn_regs_1_3- inner node disablement
        // 7 tnr7_spatial_1_0- inner node disablement
        // 8 slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3- inner node disablement
        // 9 tnr_fp_blend_bifd_rs4n_regs_1_3- inner node disablement
        // 10 tnr7_blend_1_0- inner node disablement
        // 11 tnr_fp_bodr_yuvn_regs_1_3- inner node disablement
        // 12 cas_1_0- inner node disablement
        // 13 tnr_scale_fp- inner node disablement
        // 14 ofs_mp_bodr_regs_1_3- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 ofs_dp_bodr_regs_1_3- inner node disablement
        // 17 tnr_scale_fp_bodr_yuv4n_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFF;
    } else  // default inner node
    {
        // RBM - 0x0000000000000000000000000000003F
        bitmaps.rbm[0] = 0x3F;
        // DEB - 0x0000000000000000000000000003FFFF
        bitmaps.deb[0] = 0x3FFFF;
        // TEB - 0x000000000000FFEF
        bitmaps.teb[0] = 0xFFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void SwGdcOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void SwScalerOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void SwNntmOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffRgbIrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 34; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noIr | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000DEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xDE;
        // DEB - 0x000000000000000000001FFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C8127
        bitmaps.teb[0] = 0xC8127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x381800390;
    } else if (nodeRelevantInnerOptions == (noIr)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000009EA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x9E;
        // DEB - 0x000000000000000000001FFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3D27
        bitmaps.teb[0] = 0x2C3D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x40;
    } else if (nodeRelevantInnerOptions == (no3A | noIr)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000009EA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x9E;
        // DEB - 0x000000000000000000001FFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3818003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000D6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xD6;
        // DEB - 0x0000000000000000000017FFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x000000000024BD27
        bitmaps.teb[0] = 0x24BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 29 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x20000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000D6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xD6;
        // DEB - 0x0000000000000000000017FFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000048127
        bitmaps.teb[0] = 0x48127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3A1800390;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000096A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x96;
        // DEB - 0x0000000000000000000017FFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243D27
        bitmaps.teb[0] = 0x243D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x20000040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000096A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x96;
        // DEB - 0x0000000000000000000017FFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3A18003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000CEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xCE;
        // DEB - 0x000000000000000000000BFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x000000000028BD27
        bitmaps.teb[0] = 0x28BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x50000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000CEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xCE;
        // DEB - 0x000000000000000000000BFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000088127
        bitmaps.teb[0] = 0x88127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3D1800390;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000008EA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x8E;
        // DEB - 0x000000000000000000000BFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283D27
        bitmaps.teb[0] = 0x283D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x50000040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000008EA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x8E;
        // DEB - 0x000000000000000000000BFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3D18003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000C0003A6871
        bitmaps.rbm[0] = 0x3A6871;
        bitmaps.rbm[1] = 0xC0;
        // DEB - 0x000000000000000000000000039FF1F1
        bitmaps.deb[0] = 0x39FF1F1;
        // TEB - 0x000000000020BD27
        bitmaps.teb[0] = 0x20BD27;
        // REB - 0x00000000000000000000000001807FEF
        bitmaps.reb[0] = 0x1807FEF;

        // Kernels disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E7FF000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000004000006011
        bitmaps.rbm[0] = 0x6011;
        bitmaps.rbm[1] = 0x40;
        // DEB - 0x00000000000000000000000003800131
        bitmaps.deb[0] = 0x3800131;
        // TEB - 0x0000000000008027
        bitmaps.teb[0] = 0x8027;
        // REB - 0x00000000000000000000000001800187
        bitmaps.reb[0] = 0x1800187;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 ifd_lsc_1_3- inner node disablement
        // 11 lsc_1_2- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFF90;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000080003A6871
        bitmaps.rbm[0] = 0x3A6871;
        bitmaps.rbm[1] = 0x80;
        // DEB - 0x000000000000000000000000019FF1F1
        bitmaps.deb[0] = 0x19FF1F1;
        // TEB - 0x0000000000203D27
        bitmaps.teb[0] = 0x203D27;
        // REB - 0x00000000000000000000000001807FEF
        bitmaps.reb[0] = 0x1807FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E7FF040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 gd_dpc_2_2- inner node disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 5 rgb_ir_2_0- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 ifd_lsc_1_3- inner node disablement
        // 11 lsc_1_2- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x0000000000000000000000DEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xDE;
        // DEB - 0x000000000000000000001FFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002CBD27
        bitmaps.teb[0] = 0x2CBD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffIrNoGmvIrStreamOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 31; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][1].run_kernel.enable = 0;   // bxt_blc
        kernelListOptions[j][2].run_kernel.enable = 0;   // linearization2_0
        kernelListOptions[j][5].run_kernel.enable = 0;   // gd_dpc_2_2
        kernelListOptions[j][6].run_kernel.enable = 0;   // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0;   // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0;  // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void BbpsIrWithTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 18; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000002F
        bitmaps.rbm[0] = 0x2F;
        // DEB - 0x00000000000000000000000000037FFF
        bitmaps.deb[0] = 0x37FFF;
        // TEB - 0x000000000000BFEF
        bitmaps.teb[0] = 0xBFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x4000;
    } else if (nodeRelevantInnerOptions == (noDp)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000001F
        bitmaps.rbm[0] = 0x1F;
        // DEB - 0x0000000000000000000000000000FFFF
        bitmaps.deb[0] = 0xFFFF;
        // TEB - 0x0000000000007FEF
        bitmaps.teb[0] = 0x7FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x18000;
    } else if (nodeRelevantInnerOptions == (noMp | noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_3- inner node disablement
        // 1 slim_tnr_sp_bc_bifd_rs4nm1_regs_1_3- inner node disablement
        // 2 tnr_sp_bc_bifd_yuv4n_regs_1_3- inner node disablement
        // 3 tnr7_ims_1_1- inner node disablement
        // 4 tnr7_bc_1_1- inner node disablement
        // 5 tnr_sp_bc_bodr_rs4n_regs_1_3- inner node disablement
        // 6 slim_tnr_spatial_bifd_yuvn_regs_1_3- inner node disablement
        // 7 tnr7_spatial_1_0- inner node disablement
        // 8 slim_tnr_fp_blend_bifd_yuvnm1_regs_1_3- inner node disablement
        // 9 tnr_fp_blend_bifd_rs4n_regs_1_3- inner node disablement
        // 10 tnr7_blend_1_0- inner node disablement
        // 11 tnr_fp_bodr_yuvn_regs_1_3- inner node disablement
        // 12 cas_1_0- inner node disablement
        // 13 tnr_scale_fp- inner node disablement
        // 14 ofs_mp_bodr_regs_1_3- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 ofs_dp_bodr_regs_1_3- inner node disablement
        // 17 tnr_scale_fp_bodr_yuv4n_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFF;
    } else  // default inner node
    {
        // RBM - 0x0000000000000000000000000000003F
        bitmaps.rbm[0] = 0x3F;
        // DEB - 0x0000000000000000000000000003FFFF
        bitmaps.deb[0] = 0x3FFFF;
        // TEB - 0x000000000000FFEF
        bitmaps.teb[0] = 0xFFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffBayerBurstOutNo3AOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 31; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (noBurstCapture | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noBurstCapture)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000013EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C4227
        bitmaps.teb[0] = 0x19C4227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x10000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001954227
        bitmaps.teb[0] = 0x1954227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 21 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x200000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001944227
        bitmaps.teb[0] = 0x1944227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x210000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001994227
        bitmaps.teb[0] = 0x1994227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x500000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001984227
        bitmaps.teb[0] = 0x1984227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x510000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001914227
        bitmaps.teb[0] = 0x1914227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x700000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001904227
        bitmaps.teb[0] = 0x1904227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x710000;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000D4227
        bitmaps.teb[0] = 0xD4227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78000000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C4227
        bitmaps.teb[0] = 0xC4227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78010000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000054227
        bitmaps.teb[0] = 0x54227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78200000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000044227
        bitmaps.teb[0] = 0x44227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78210000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000094227
        bitmaps.teb[0] = 0x94227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78500000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD4400FB1
        bitmaps.deb[0] = 0xD4400FB1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000084227
        bitmaps.teb[0] = 0x84227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x78510000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010010019009
        bitmaps.rbm[0] = 0x10019009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x0000000000000000000000000C400FB1
        bitmaps.deb[0] = 0xC400FB1;
        // TEB - 0x0000000000014227
        bitmaps.teb[0] = 0x14227;
        // REB - 0x000000000000000000000000020187E7
        bitmaps.reb[0] = 0x20187E7;

        // Kernels disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v4- inner node disablement
        // 15 csc_1_1- inner node disablement
        // 17 b2i_ds_1_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787EFFC0;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010000000000
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x00000000000000000000000000400E00
        bitmaps.deb[0] = 0x400E00;
        // TEB - 0x0000000000004207
        bitmaps.teb[0] = 0x4207;
        // REB - 0x00000000000000000000000002008000
        bitmaps.reb[0] = 0x2008000;

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 lsc_1_2- inner node disablement
        // 4 gd_dpc_2_2- inner node disablement
        // 5 wb_1_1- inner node disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v4- inner node disablement
        // 15 csc_1_1- inner node disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 17 b2i_ds_1_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787FFFFF;
    } else if (nodeRelevantInnerOptions == (noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019D0027
        bitmaps.teb[0] = 0x19D0027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7800000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0027
        bitmaps.teb[0] = 0x19C0027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7810000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001950027
        bitmaps.teb[0] = 0x1950027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7A00000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940027
        bitmaps.teb[0] = 0x1940027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7A10000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001990027
        bitmaps.teb[0] = 0x1990027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D00000;
    } else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980027
        bitmaps.teb[0] = 0x1980027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D10000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001910027
        bitmaps.teb[0] = 0x1910027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F00000;
    } else if (nodeRelevantInnerOptions ==
               (noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900027
        bitmaps.teb[0] = 0x1900027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F10000;
    } else if (nodeRelevantInnerOptions == (noGmv | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000D0027
        bitmaps.teb[0] = 0xD0027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F800000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0027
        bitmaps.teb[0] = 0xC0027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F810000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016B8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000050027
        bitmaps.teb[0] = 0x50027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FA00000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040027
        bitmaps.teb[0] = 0x40027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FA10000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFDC0001B1
        bitmaps.deb[0] = 0xDC0001B1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000090027
        bitmaps.teb[0] = 0x90027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD00000;
    } else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001B1
        bitmaps.deb[0] = 0xD40001B1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080027
        bitmaps.teb[0] = 0x80027;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD10000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000010019009
        bitmaps.rbm[0] = 0x10019009;
        // DEB - 0x0000000000000000000000000C0001B1
        bitmaps.deb[0] = 0xC0001B1;
        // TEB - 0x0000000000010027
        bitmaps.teb[0] = 0x10027;
        // REB - 0x000000000000000000000000000107E7
        bitmaps.reb[0] = 0x107E7;

        // Kernels disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v4- inner node disablement
        // 15 csc_1_1- inner node disablement
        // 17 b2i_ds_1_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFEFFC0;
    } else if (nodeRelevantInnerOptions ==
               (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 lsc_1_2- inner node disablement
        // 4 gd_dpc_2_2- inner node disablement
        // 5 wb_1_1- inner node disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v4- inner node disablement
        // 15 csc_1_1- inner node disablement
        // 16 odr_burst_isp_1_3- inner node disablement
        // 17 b2i_ds_1_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_3- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_3- inner node disablement
        // 22 odr_output_me_1_3- inner node disablement
        // 23 ifd_pdaf_1_3- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_3- inner node disablement
        // 27 ifd_gmv_1_3- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_3- inner node disablement
        // 30 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000013EB8019009
        bitmaps.rbm[0] = 0xB8019009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFDC400FB1
        bitmaps.deb[0] = 0xDC400FB1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019D4227
        bitmaps.teb[0] = 0x19D4227;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void BbpsIrNoTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 5; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000020
        bitmaps.rbm[0] = 0x20;
        // DEB - 0x00000000000000000000000000034040
        bitmaps.deb[0] = 0x34040;
        // TEB - 0x000000000000820F
        bitmaps.teb[0] = 0x820F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x4;
    } else if (nodeRelevantInnerOptions == (noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000010
        bitmaps.rbm[0] = 0x10;
        // DEB - 0x0000000000000000000000000000C040
        bitmaps.deb[0] = 0xC040;
        // TEB - 0x000000000000420F
        bitmaps.teb[0] = 0x420F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x18;
    } else if (nodeRelevantInnerOptions == (noMp | noDp)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_spatial_bifd_yuvn_regs_1_3- inner node disablement
        // 1 cas_1_0- inner node disablement
        // 2 ofs_mp_bodr_regs_1_3- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 ofs_dp_bodr_regs_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1F;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000000000000030
        bitmaps.rbm[0] = 0x30;
        // DEB - 0x0000000000000000000000000003C040
        bitmaps.deb[0] = 0x3C040;
        // TEB - 0x000000000000C20F
        bitmaps.teb[0] = 0xC20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffIrNoGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 31; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][6].run_kernel.enable = 0;   // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0;   // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0;  // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void IsysPdaf2OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffBayerPdaf2OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 35; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noPdaf);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C4327
        bitmaps.teb[0] = 0xC4327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000047F27
        bitmaps.teb[0] = 0x47F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000044327
        bitmaps.teb[0] = 0x44327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000087F27
        bitmaps.teb[0] = 0x87F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000084327
        bitmaps.teb[0] = 0x84327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000100001E9009
        bitmaps.rbm[0] = 0x1E9009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x000000000000000000000000005BFFF1
        bitmaps.deb[0] = 0x5BFFF1;
        // TEB - 0x0000000000007F27
        bitmaps.teb[0] = 0x7F27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010000000000
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x00000000000000000000000000400E00
        bitmaps.deb[0] = 0x400E00;
        // TEB - 0x0000000000004207
        bitmaps.teb[0] = 0x4207;
        // REB - 0x00000000000000000000000002008000
        bitmaps.reb[0] = 0x2008000;

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    } else if (nodeRelevantInnerOptions == (noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    } else if (nodeRelevantInnerOptions == (no3A | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000011EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C7F27
        bitmaps.teb[0] = 0xC7F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffBayerPdaf3OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 34; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noPdaf);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C4127
        bitmaps.teb[0] = 0xC4127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xF81C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000047D27
        bitmaps.teb[0] = 0x47D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 25 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000044127
        bitmaps.teb[0] = 0x44127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFA1C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000087D27
        bitmaps.teb[0] = 0x87D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x5000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000084127
        bitmaps.teb[0] = 0x84127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFD1C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000100041E9009
        bitmaps.rbm[0] = 0x41E9009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x000000000000000000000000005BFDF1
        bitmaps.deb[0] = 0x5BFDF1;
        // TEB - 0x0000000000007D27
        bitmaps.teb[0] = 0x7D27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010004009009
        bitmaps.rbm[0] = 0x4009009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x00000000000000000000000000400DF1
        bitmaps.deb[0] = 0x400DF1;
        // TEB - 0x0000000000004127
        bitmaps.teb[0] = 0x4127;
        // REB - 0x000000000000000000000000020087E7
        bitmaps.reb[0] = 0x20087E7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFFFDFFC0;
    } else if (nodeRelevantInnerOptions == (noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x300020000;
    } else if (nodeRelevantInnerOptions == (no3A | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F81E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x302020000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x305020000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000041E9009
        bitmaps.rbm[0] = 0x41E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x307E3FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000011EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C7D27
        bitmaps.teb[0] = 0xC7D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void IsysDolOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffDol2InputsOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 34; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][15].run_kernel.enable = 0;  // wb_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C016F
        bitmaps.teb[0] = 0xC016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x204000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243D6F
        bitmaps.teb[0] = 0x243D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x80000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x000000000004016F
        bitmaps.teb[0] = 0x4016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x284000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283D6F
        bitmaps.teb[0] = 0x283D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x140000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x000000000008016F
        bitmaps.teb[0] = 0x8016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x344000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000252960E
        bitmaps.rbm[0] = 0x252960E;
        // DEB - 0x000000000000000000000000001FF1FB
        bitmaps.deb[0] = 0x1FF1FB;
        // TEB - 0x0000000000203D6F
        bitmaps.teb[0] = 0x203D6F;
        // REB - 0x00000000000000000000000000007FF7
        bitmaps.reb[0] = 0x7FF7;

        // Kernels disablement
        // 15 wb_1_1- inner node disablement
        // 16 bnlm_3_3- inner node disablement
        // 17 bxt_demosaic- inner node disablement
        // 18 vcsc_2_0_b- inner node disablement
        // 19 gltm_2_0- inner node disablement
        // 20 xnr_5_2- inner node disablement
        // 21 vcr_3_1- inner node disablement
        // 22 glim_2_0- inner node disablement
        // 23 acm_1_1- inner node disablement
        // 24 gammatm_v4- inner node disablement
        // 25 csc_1_1- inner node disablement
        // 27 b2i_ds_1_1- inner node disablement
        // 28 upscaler_1_0- inner node disablement
        // 29 lbff_crop_espa_1_3- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1FBFF8000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 ifd_pipe_long_1_3- inner node disablement
        // 2 dol_lite_1_1- inner node disablement
        // 3 bxt_blc- inner node disablement
        // 4 linearization2_0- inner node disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 12 ifd_lsc_1_3- inner node disablement
        // 13 lsc_1_2- inner node disablement
        // 14 gd_dpc_2_2- inner node disablement
        // 15 wb_1_1- inner node disablement
        // 16 bnlm_3_3- inner node disablement
        // 17 bxt_demosaic- inner node disablement
        // 18 vcsc_2_0_b- inner node disablement
        // 19 gltm_2_0- inner node disablement
        // 20 xnr_5_2- inner node disablement
        // 21 vcr_3_1- inner node disablement
        // 22 glim_2_0- inner node disablement
        // 23 acm_1_1- inner node disablement
        // 24 gammatm_v4- inner node disablement
        // 25 csc_1_1- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 27 b2i_ds_1_1- inner node disablement
        // 28 upscaler_1_0- inner node disablement
        // 29 lbff_crop_espa_1_3- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000001EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3D6F
        bitmaps.teb[0] = 0x2C3D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffDolSmoothOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 7; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    // RBM - 0x0000000000000000000000012801A011
    bitmaps.rbm[0] = 0x2801A011;
    bitmaps.rbm[1] = 0x1;
    // DEB - 0x00000000000000000000000034000131
    bitmaps.deb[0] = 0x34000131;
    // TEB - 0x0000000000020027
    bitmaps.teb[0] = 0x20027;
    // REB - 0x000000000000000000000000000B0787
    bitmaps.reb[0] = 0xB0787;
    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffDol3InputsOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 35; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][16].run_kernel.enable = 0;  // wb_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C01EF
        bitmaps.teb[0] = 0xC01EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x408001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243DEF
        bitmaps.teb[0] = 0x243DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 32 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x100000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x00000000000401EF
        bitmaps.teb[0] = 0x401EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x508001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283DEF
        bitmaps.teb[0] = 0x283DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x280000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x00000000000801EF
        bitmaps.teb[0] = 0x801EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x688001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000252960E
        bitmaps.rbm[0] = 0x252960E;
        // DEB - 0x000000000000000000000000001FF1FF
        bitmaps.deb[0] = 0x1FF1FF;
        // TEB - 0x0000000000203DEF
        bitmaps.teb[0] = 0x203DEF;
        // REB - 0x00000000000000000000000000007FF7
        bitmaps.reb[0] = 0x7FF7;

        // Kernels disablement
        // 16 wb_1_1- inner node disablement
        // 17 bnlm_3_3- inner node disablement
        // 18 bxt_demosaic- inner node disablement
        // 19 vcsc_2_0_b- inner node disablement
        // 20 gltm_2_0- inner node disablement
        // 21 xnr_5_2- inner node disablement
        // 22 vcr_3_1- inner node disablement
        // 23 glim_2_0- inner node disablement
        // 24 acm_1_1- inner node disablement
        // 25 gammatm_v4- inner node disablement
        // 26 csc_1_1- inner node disablement
        // 28 b2i_ds_1_1- inner node disablement
        // 29 upscaler_1_0- inner node disablement
        // 30 lbff_crop_espa_1_3- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F7FF0000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 ifd_pipe_long_1_3- inner node disablement
        // 2 ifd_pipe_short_smth_1_3- inner node disablement
        // 3 dol_lite_1_1- inner node disablement
        // 4 bxt_blc- inner node disablement
        // 5 linearization2_0- inner node disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 13 ifd_lsc_1_3- inner node disablement
        // 14 lsc_1_2- inner node disablement
        // 15 gd_dpc_2_2- inner node disablement
        // 16 wb_1_1- inner node disablement
        // 17 bnlm_3_3- inner node disablement
        // 18 bxt_demosaic- inner node disablement
        // 19 vcsc_2_0_b- inner node disablement
        // 20 gltm_2_0- inner node disablement
        // 21 xnr_5_2- inner node disablement
        // 22 vcr_3_1- inner node disablement
        // 23 glim_2_0- inner node disablement
        // 24 acm_1_1- inner node disablement
        // 25 gammatm_v4- inner node disablement
        // 26 csc_1_1- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 28 b2i_ds_1_1- inner node disablement
        // 29 upscaler_1_0- inner node disablement
        // 30 lbff_crop_espa_1_3- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000001EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3DEF
        bitmaps.teb[0] = 0x2C3DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffBayerPdaf2WithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 39; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000013EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C4327
        bitmaps.teb[0] = 0x19C4327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001947F27
        bitmaps.teb[0] = 0x1947F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001944327
        bitmaps.teb[0] = 0x1944327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001987F27
        bitmaps.teb[0] = 0x1987F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001984327
        bitmaps.teb[0] = 0x1984327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001907F27
        bitmaps.teb[0] = 0x1907F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001904327
        bitmaps.teb[0] = 0x1904327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C7F27
        bitmaps.teb[0] = 0xC7F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7800000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C4327
        bitmaps.teb[0] = 0xC4327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787C0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000047F27
        bitmaps.teb[0] = 0x47F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7801000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000044327
        bitmaps.teb[0] = 0x44327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787D0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000087F27
        bitmaps.teb[0] = 0x87F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7802800000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD4400FF1
        bitmaps.deb[0] = 0xD4400FF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000084327
        bitmaps.teb[0] = 0x84327;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787E8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000100001E9009
        bitmaps.rbm[0] = 0x1E9009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x000000000000000000000000005BFFF1
        bitmaps.deb[0] = 0x5BFFF1;
        // TEB - 0x0000000000007F27
        bitmaps.teb[0] = 0x7F27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7803F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010000000000
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x00000000000000000000000000400E00
        bitmaps.deb[0] = 0x400E00;
        // TEB - 0x0000000000004207
        bitmaps.teb[0] = 0x4207;
        // REB - 0x00000000000000000000000002008000
        bitmaps.reb[0] = 0x2008000;

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x787FFFFFFF;
    } else if (nodeRelevantInnerOptions == (noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C3D27
        bitmaps.teb[0] = 0x19C3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    } else if (nodeRelevantInnerOptions == (no3A | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0127
        bitmaps.teb[0] = 0x19C0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001943D27
        bitmaps.teb[0] = 0x1943D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940127
        bitmaps.teb[0] = 0x1940127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001983D27
        bitmaps.teb[0] = 0x1983D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980127
        bitmaps.teb[0] = 0x1980127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001903D27
        bitmaps.teb[0] = 0x1903D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x783800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900127
        bitmaps.teb[0] = 0x1900127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FF8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F80000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFC0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F81000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFD0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F82800000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFE8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F83F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_pdaf_1_3- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000013EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFD45BFFF1
        bitmaps.deb[0] = 0xD45BFFF1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C7F27
        bitmaps.teb[0] = 0x19C7F27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffBayerPdaf3WithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 38; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000013EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C4127
        bitmaps.teb[0] = 0x19C4127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xF81C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001947D27
        bitmaps.teb[0] = 0x1947D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 25 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000136AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x136;
        // DEB - 0x00000000000000000001F7FFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001944127
        bitmaps.teb[0] = 0x1944127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFA1C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001987D27
        bitmaps.teb[0] = 0x1987D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x5000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000012EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x12E;
        // DEB - 0x00000000000000000001EBFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001984127
        bitmaps.teb[0] = 0x1984127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFD1C0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001907D27
        bitmaps.teb[0] = 0x1907D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000126AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x126;
        // DEB - 0x00000000000000000001E3FFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001904127
        bitmaps.teb[0] = 0x1904127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0xFF1C0000;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C7D27
        bitmaps.teb[0] = 0xC7D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000011EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x11E;
        // DEB - 0x000000000000000000001FFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C4127
        bitmaps.teb[0] = 0xC4127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3CF81C0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000047D27
        bitmaps.teb[0] = 0x47D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C02000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000116AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x116;
        // DEB - 0x0000000000000000000017FFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000044127
        bitmaps.teb[0] = 0x44127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3CFA1C0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000087D27
        bitmaps.teb[0] = 0x87D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C05000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x10E;
        // DEB - 0x000000000000000000000BFFD4400DF1
        bitmaps.deb[0] = 0xD4400DF1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000084127
        bitmaps.teb[0] = 0x84127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3CFD1C0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000100041E9009
        bitmaps.rbm[0] = 0x41E9009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x000000000000000000000000005BFDF1
        bitmaps.deb[0] = 0x5BFDF1;
        // TEB - 0x0000000000007D27
        bitmaps.teb[0] = 0x7D27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C07E1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000010004009009
        bitmaps.rbm[0] = 0x4009009;
        bitmaps.rbm[1] = 0x100;
        // DEB - 0x00000000000000000000000000400DF1
        bitmaps.deb[0] = 0x400DF1;
        // TEB - 0x0000000000004127
        bitmaps.teb[0] = 0x4127;
        // REB - 0x000000000000000000000000020087E7
        bitmaps.reb[0] = 0x20087E7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3CFFFDFFC0;
    } else if (nodeRelevantInnerOptions == (noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C3D27
        bitmaps.teb[0] = 0x19C3D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x300020000;
    } else if (nodeRelevantInnerOptions == (no3A | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0127
        bitmaps.teb[0] = 0x19C0127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F81E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001943D27
        bitmaps.teb[0] = 0x1943D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x302020000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940127
        bitmaps.teb[0] = 0x1940127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001983D27
        bitmaps.teb[0] = 0x1983D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x305020000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980127
        bitmaps.teb[0] = 0x1980127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001903D27
        bitmaps.teb[0] = 0x1903D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x307020000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900127
        bitmaps.teb[0] = 0x1900127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FF1E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F00020000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FF81E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F02020000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFA1E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F05020000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAC019009
        bitmaps.rbm[0] = 0xAC019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000027B87E7
        bitmaps.reb[0] = 0x27B87E7;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFD1E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000041E9009
        bitmaps.rbm[0] = 0x41E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x0000000000000000000000000200FFE7
        bitmaps.reb[0] = 0x200FFE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F07E3FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe | noPdaf)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_3- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_3- inner node disablement
        // 26 odr_output_me_1_3- inner node disablement
        // 27 odr_awb_std_1_3- inner node disablement
        // 28 odr_awb_sat_1_3- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_3- inner node disablement
        // 31 odr_af_std_1_3- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000013EAC1F9009
        bitmaps.rbm[0] = 0xAC1F9009;
        bitmaps.rbm[1] = 0x13E;
        // DEB - 0x00000000000000000001FFFFD45BFDF1
        bitmaps.deb[0] = 0xD45BFDF1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C7D27
        bitmaps.teb[0] = 0x19C7D27;
        // REB - 0x000000000000000000000000027BFFE7
        bitmaps.reb[0] = 0x27BFFE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffRgbIrWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 38; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noIr | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000FEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xFE;
        // DEB - 0x00000000000000000001FFFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C8127
        bitmaps.teb[0] = 0x19C8127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x381800390;
    } else if (nodeRelevantInnerOptions == (noIr)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000BEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xBE;
        // DEB - 0x00000000000000000001FFFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x0000000001BC3D27
        bitmaps.teb[0] = 0x1BC3D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x40;
    } else if (nodeRelevantInnerOptions == (no3A | noIr)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000BEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xBE;
        // DEB - 0x00000000000000000001FFFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0127
        bitmaps.teb[0] = 0x19C0127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3818003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000F6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xF6;
        // DEB - 0x00000000000000000001F7FFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001B4BD27
        bitmaps.teb[0] = 0x1B4BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 29 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x20000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000F6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xF6;
        // DEB - 0x00000000000000000001F7FFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001948127
        bitmaps.teb[0] = 0x1948127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3A1800390;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000B6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xB6;
        // DEB - 0x00000000000000000001F7FFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001B43D27
        bitmaps.teb[0] = 0x1B43D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x20000040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000B6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xB6;
        // DEB - 0x00000000000000000001F7FFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940127
        bitmaps.teb[0] = 0x1940127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3A18003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xEE;
        // DEB - 0x00000000000000000001EBFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001B8BD27
        bitmaps.teb[0] = 0x1B8BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x50000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xEE;
        // DEB - 0x00000000000000000001EBFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001988127
        bitmaps.teb[0] = 0x1988127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3D1800390;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000AEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xAE;
        // DEB - 0x00000000000000000001EBFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001B83D27
        bitmaps.teb[0] = 0x1B83D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x50000040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000AEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xAE;
        // DEB - 0x00000000000000000001EBFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980127
        bitmaps.teb[0] = 0x1980127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3D18003D0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000E6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xE6;
        // DEB - 0x00000000000000000001E3FFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001B0BD27
        bitmaps.teb[0] = 0x1B0BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x70000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000E6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xE6;
        // DEB - 0x00000000000000000001E3FFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001908127
        bitmaps.teb[0] = 0x1908127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F1800390;
    } else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000A6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xA6;
        // DEB - 0x00000000000000000001E3FFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001B03D27
        bitmaps.teb[0] = 0x1B03D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x70000040;
    } else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000A6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xA6;
        // DEB - 0x00000000000000000001E3FFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900127
        bitmaps.teb[0] = 0x1900127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F18003D0;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000DEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xDE;
        // DEB - 0x000000000000000000001FFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002CBD27
        bitmaps.teb[0] = 0x2CBD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000DEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xDE;
        // DEB - 0x000000000000000000001FFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C8127
        bitmaps.teb[0] = 0xC8127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F81800390;
    } else if (nodeRelevantInnerOptions == (noGmv | noIr)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000009EA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x9E;
        // DEB - 0x000000000000000000001FFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3D27
        bitmaps.teb[0] = 0x2C3D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000040;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000009EA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x9E;
        // DEB - 0x000000000000000000001FFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F818003D0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000D6A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xD6;
        // DEB - 0x0000000000000000000017FFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x000000000024BD27
        bitmaps.teb[0] = 0x24BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C20000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000D6A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xD6;
        // DEB - 0x0000000000000000000017FFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000048127
        bitmaps.teb[0] = 0x48127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1800390;
    } else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000096A83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x96;
        // DEB - 0x0000000000000000000017FFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243D27
        bitmaps.teb[0] = 0x243D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C20000040;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000096A8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x96;
        // DEB - 0x0000000000000000000017FFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FA18003D0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000CEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xCE;
        // DEB - 0x000000000000000000000BFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x000000000028BD27
        bitmaps.teb[0] = 0x28BD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C50000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000CEA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0xCE;
        // DEB - 0x000000000000000000000BFFD78001F1
        bitmaps.deb[0] = 0xD78001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000088127
        bitmaps.teb[0] = 0x88127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1800390;
    } else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000008EA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0x8E;
        // DEB - 0x000000000000000000000BFFD59FF1F1
        bitmaps.deb[0] = 0xD59FF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283D27
        bitmaps.teb[0] = 0x283D27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C50000040;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000008EA8016811
        bitmaps.rbm[0] = 0xA8016811;
        bitmaps.rbm[1] = 0x8E;
        // DEB - 0x000000000000000000000BFFD58001F1
        bitmaps.deb[0] = 0xD58001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x00000000000000000000000001FB07E7
        bitmaps.reb[0] = 0x1FB07E7;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FD18003D0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000C0003A6871
        bitmaps.rbm[0] = 0x3A6871;
        bitmaps.rbm[1] = 0xC0;
        // DEB - 0x000000000000000000000000039FF1F1
        bitmaps.deb[0] = 0x39FF1F1;
        // TEB - 0x000000000020BD27
        bitmaps.teb[0] = 0x20BD27;
        // REB - 0x00000000000000000000000001807FEF
        bitmaps.reb[0] = 0x1807FEF;

        // Kernels disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C7E7FF000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000004000006011
        bitmaps.rbm[0] = 0x6011;
        bitmaps.rbm[1] = 0x40;
        // DEB - 0x00000000000000000000000003800131
        bitmaps.deb[0] = 0x3800131;
        // TEB - 0x0000000000008027
        bitmaps.teb[0] = 0x8027;
        // REB - 0x00000000000000000000000001800187
        bitmaps.reb[0] = 0x1800187;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 ifd_lsc_1_3- inner node disablement
        // 11 lsc_1_2- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFF90;
    } else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000080003A6871
        bitmaps.rbm[0] = 0x3A6871;
        bitmaps.rbm[1] = 0x80;
        // DEB - 0x000000000000000000000000019FF1F1
        bitmaps.deb[0] = 0x19FF1F1;
        // TEB - 0x0000000000203D27
        bitmaps.teb[0] = 0x203D27;
        // REB - 0x00000000000000000000000001807FEF
        bitmaps.reb[0] = 0x1807FEF;

        // Kernels disablement
        // 6 odr_ir_1_3- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C7E7FF040;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 gd_dpc_2_2- inner node disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 5 rgb_ir_2_0- inner node disablement
        // 6 odr_ir_1_3- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 ifd_lsc_1_3- inner node disablement
        // 11 lsc_1_2- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v4- inner node disablement
        // 22 csc_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_3- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_3- inner node disablement
        // 30 odr_output_me_1_3- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x0000000000000000000000FEA83B6871
        bitmaps.rbm[0] = 0xA83B6871;
        bitmaps.rbm[1] = 0xFE;
        // DEB - 0x00000000000000000001FFFFD79FF1F1
        bitmaps.deb[0] = 0xD79FF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x0000000001BCBD27
        bitmaps.teb[0] = 0x1BCBD27;
        // REB - 0x00000000000000000000000001FB7FEF
        bitmaps.reb[0] = 0x1FB7FEF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffIrWithGmvIrStreamOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 35; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][1].run_kernel.enable = 0;   // bxt_blc
        kernelListOptions[j][2].run_kernel.enable = 0;   // linearization2_0
        kernelListOptions[j][5].run_kernel.enable = 0;   // gd_dpc_2_2
        kernelListOptions[j][6].run_kernel.enable = 0;   // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0;   // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0;  // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C0127
        bitmaps.teb[0] = 0x19C0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001943D27
        bitmaps.teb[0] = 0x1943D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001940127
        bitmaps.teb[0] = 0x1940127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001983D27
        bitmaps.teb[0] = 0x1983D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001980127
        bitmaps.teb[0] = 0x1980127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001903D27
        bitmaps.teb[0] = 0x1903D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001900127
        bitmaps.teb[0] = 0x1900127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C3D27
        bitmaps.teb[0] = 0xC3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C0127
        bitmaps.teb[0] = 0xC0127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000043D27
        bitmaps.teb[0] = 0x43D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000040127
        bitmaps.teb[0] = 0x40127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000083D27
        bitmaps.teb[0] = 0x83D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA8019009
        bitmaps.rbm[0] = 0xA8019009;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001F1
        bitmaps.deb[0] = 0xD40001F1;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000080127
        bitmaps.teb[0] = 0x80127;
        // REB - 0x000000000000000000000000007B07E7
        bitmaps.reb[0] = 0x7B07E7;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000000001E9009
        bitmaps.rbm[0] = 0x1E9009;
        // DEB - 0x000000000000000000000000001BF1F1
        bitmaps.deb[0] = 0x1BF1F1;
        // TEB - 0x0000000000003D27
        bitmaps.teb[0] = 0x3D27;
        // REB - 0x00000000000000000000000000007FE7
        bitmaps.reb[0] = 0x7FE7;

        // Kernels disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_3- inner node disablement
        // 4 lsc_1_2- inner node disablement
        // 5 gd_dpc_2_2- inner node disablement
        // 6 wb_1_1- inner node disablement
        // 7 bnlm_3_3- inner node disablement
        // 8 bxt_demosaic- inner node disablement
        // 9 vcsc_2_0_b- inner node disablement
        // 10 gltm_2_0- inner node disablement
        // 11 xnr_5_2- inner node disablement
        // 12 vcr_3_1- inner node disablement
        // 13 glim_2_0- inner node disablement
        // 14 acm_1_1- inner node disablement
        // 15 gammatm_v4- inner node disablement
        // 16 csc_1_1- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_3- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_3- inner node disablement
        // 25 odr_output_me_1_3- inner node disablement
        // 26 odr_awb_std_1_3- inner node disablement
        // 27 odr_awb_sat_1_3- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_3- inner node disablement
        // 30 odr_af_std_1_3- inner node disablement
        // 31 ifd_gmv_1_3- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_3- inner node disablement
        // 34 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000003EA81F9009
        bitmaps.rbm[0] = 0xA81F9009;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41BF1F1
        bitmaps.deb[0] = 0xD41BF1F1;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C3D27
        bitmaps.teb[0] = 0x19C3D27;
        // REB - 0x000000000000000000000000007B7FE7
        bitmaps.reb[0] = 0x7B7FE7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffDol2InputsWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 38; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][15].run_kernel.enable = 0;  // wb_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C016F
        bitmaps.teb[0] = 0x19C016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x204000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001B43D6F
        bitmaps.teb[0] = 0x1B43D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x80000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x000000000194016F
        bitmaps.teb[0] = 0x194016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x284000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001B83D6F
        bitmaps.teb[0] = 0x1B83D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x140000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x000000000198016F
        bitmaps.teb[0] = 0x198016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x344000FE0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001B03D6F
        bitmaps.teb[0] = 0x1B03D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x1C0000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x000000000190016F
        bitmaps.teb[0] = 0x190016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C4000FE0;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3D6F
        bitmaps.teb[0] = 0x2C3D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C016F
        bitmaps.teb[0] = 0xC016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3E04000FE0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243D6F
        bitmaps.teb[0] = 0x243D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3C80000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x000000000004016F
        bitmaps.teb[0] = 0x4016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3E84000FE0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283D6F
        bitmaps.teb[0] = 0x283D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3D40000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001FB
        bitmaps.deb[0] = 0xD40001FB;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x000000000008016F
        bitmaps.teb[0] = 0x8016F;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3F44000FE0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000252960E
        bitmaps.rbm[0] = 0x252960E;
        // DEB - 0x000000000000000000000000001FF1FB
        bitmaps.deb[0] = 0x1FF1FB;
        // TEB - 0x0000000000203D6F
        bitmaps.teb[0] = 0x203D6F;
        // REB - 0x00000000000000000000000000007FF7
        bitmaps.reb[0] = 0x7FF7;

        // Kernels disablement
        // 15 wb_1_1- inner node disablement
        // 16 bnlm_3_3- inner node disablement
        // 17 bxt_demosaic- inner node disablement
        // 18 vcsc_2_0_b- inner node disablement
        // 19 gltm_2_0- inner node disablement
        // 20 xnr_5_2- inner node disablement
        // 21 vcr_3_1- inner node disablement
        // 22 glim_2_0- inner node disablement
        // 23 acm_1_1- inner node disablement
        // 24 gammatm_v4- inner node disablement
        // 25 csc_1_1- inner node disablement
        // 27 b2i_ds_1_1- inner node disablement
        // 28 upscaler_1_0- inner node disablement
        // 29 lbff_crop_espa_1_3- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3DFBFF8000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 ifd_pipe_long_1_3- inner node disablement
        // 2 dol_lite_1_1- inner node disablement
        // 3 bxt_blc- inner node disablement
        // 4 linearization2_0- inner node disablement
        // 5 rgbs_grid_1_1- inner node disablement
        // 6 ccm_3a_2_0- inner node disablement
        // 7 odr_awb_std_1_3- inner node disablement
        // 8 odr_awb_sve_1_3- inner node disablement
        // 9 odr_awb_sat_1_3- inner node disablement
        // 10 aestatistics_2_1- inner node disablement
        // 11 odr_ae_1_3- inner node disablement
        // 12 ifd_lsc_1_3- inner node disablement
        // 13 lsc_1_2- inner node disablement
        // 14 gd_dpc_2_2- inner node disablement
        // 15 wb_1_1- inner node disablement
        // 16 bnlm_3_3- inner node disablement
        // 17 bxt_demosaic- inner node disablement
        // 18 vcsc_2_0_b- inner node disablement
        // 19 gltm_2_0- inner node disablement
        // 20 xnr_5_2- inner node disablement
        // 21 vcr_3_1- inner node disablement
        // 22 glim_2_0- inner node disablement
        // 23 acm_1_1- inner node disablement
        // 24 gammatm_v4- inner node disablement
        // 25 csc_1_1- inner node disablement
        // 26 fr_grid_1_0- inner node disablement
        // 27 b2i_ds_1_1- inner node disablement
        // 28 upscaler_1_0- inner node disablement
        // 29 lbff_crop_espa_1_3- inner node disablement
        // 30 tnr_scale_lb- inner node disablement
        // 31 odr_output_ps_1_3- inner node disablement
        // 32 odr_output_me_1_3- inner node disablement
        // 33 odr_af_std_1_3- inner node disablement
        // 34 ifd_gmv_1_3- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_3- inner node disablement
        // 37 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000003EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41FF1FB
        bitmaps.deb[0] = 0xD41FF1FB;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x0000000001BC3D6F
        bitmaps.teb[0] = 0x1BC3D6F;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

void LbffDol3InputsWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions) {
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j) {
        for (uint8_t i = 0; i < 39; ++i) {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][16].run_kernel.enable = 0;  // wb_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions =
        nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps();  // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000003EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x00000000019C01EF
        bitmaps.teb[0] = 0x19C01EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x408001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x0000000001B43DEF
        bitmaps.teb[0] = 0x1B43DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 32 odr_output_ps_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x100000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000036A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x36;
        // DEB - 0x00000000000000000001F7FFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1F7FF;
        // TEB - 0x00000000019401EF
        bitmaps.teb[0] = 0x19401EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x508001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x0000000001B83DEF
        bitmaps.teb[0] = 0x1B83DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x280000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000002EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x2E;
        // DEB - 0x00000000000000000001EBFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1EBFF;
        // TEB - 0x00000000019801EF
        bitmaps.teb[0] = 0x19801EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x688001FC0;
    } else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x0000000001B03DEF
        bitmaps.teb[0] = 0x1B03DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x380000000;
    } else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000026A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x26;
        // DEB - 0x00000000000000000001E3FFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1E3FF;
        // TEB - 0x00000000019001EF
        bitmaps.teb[0] = 0x19001EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x788001FC0;
    } else if (nodeRelevantInnerOptions == (noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000002C3DEF
        bitmaps.teb[0] = 0x2C3DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7800000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000001EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x1E;
        // DEB - 0x000000000000000000001FFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000C01EF
        bitmaps.teb[0] = 0xC01EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7C08001FC0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016AA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x0000000000243DEF
        bitmaps.teb[0] = 0x243DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7900000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs)) {
        // HW bitmaps
        // RBM - 0x000000000000000000000016A801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0x16;
        // DEB - 0x0000000000000000000017FFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0x17FF;
        // TEB - 0x00000000000401EF
        bitmaps.teb[0] = 0x401EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7D08001FC0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x0000000000283DEF
        bitmaps.teb[0] = 0x283DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;

        // Kernels disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7A80000000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000EA801900E
        bitmaps.rbm[0] = 0xA801900E;
        bitmaps.rbm[1] = 0xE;
        // DEB - 0x000000000000000000000BFFD40001FF
        bitmaps.deb[0] = 0xD40001FF;
        bitmaps.deb[1] = 0xBFF;
        // TEB - 0x00000000000801EF
        bitmaps.teb[0] = 0x801EF;
        // REB - 0x000000000000000000000000007B07F7
        bitmaps.reb[0] = 0x7B07F7;

        // Kernels disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7E88001FC0;
    } else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000252960E
        bitmaps.rbm[0] = 0x252960E;
        // DEB - 0x000000000000000000000000001FF1FF
        bitmaps.deb[0] = 0x1FF1FF;
        // TEB - 0x0000000000203DEF
        bitmaps.teb[0] = 0x203DEF;
        // REB - 0x00000000000000000000000000007FF7
        bitmaps.reb[0] = 0x7FF7;

        // Kernels disablement
        // 16 wb_1_1- inner node disablement
        // 17 bnlm_3_3- inner node disablement
        // 18 bxt_demosaic- inner node disablement
        // 19 vcsc_2_0_b- inner node disablement
        // 20 gltm_2_0- inner node disablement
        // 21 xnr_5_2- inner node disablement
        // 22 vcr_3_1- inner node disablement
        // 23 glim_2_0- inner node disablement
        // 24 acm_1_1- inner node disablement
        // 25 gammatm_v4- inner node disablement
        // 26 csc_1_1- inner node disablement
        // 28 b2i_ds_1_1- inner node disablement
        // 29 upscaler_1_0- inner node disablement
        // 30 lbff_crop_espa_1_3- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7BF7FF0000;
    } else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe)) {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_3- inner node disablement
        // 1 ifd_pipe_long_1_3- inner node disablement
        // 2 ifd_pipe_short_smth_1_3- inner node disablement
        // 3 dol_lite_1_1- inner node disablement
        // 4 bxt_blc- inner node disablement
        // 5 linearization2_0- inner node disablement
        // 6 rgbs_grid_1_1- inner node disablement
        // 7 ccm_3a_2_0- inner node disablement
        // 8 odr_awb_std_1_3- inner node disablement
        // 9 odr_awb_sve_1_3- inner node disablement
        // 10 odr_awb_sat_1_3- inner node disablement
        // 11 aestatistics_2_1- inner node disablement
        // 12 odr_ae_1_3- inner node disablement
        // 13 ifd_lsc_1_3- inner node disablement
        // 14 lsc_1_2- inner node disablement
        // 15 gd_dpc_2_2- inner node disablement
        // 16 wb_1_1- inner node disablement
        // 17 bnlm_3_3- inner node disablement
        // 18 bxt_demosaic- inner node disablement
        // 19 vcsc_2_0_b- inner node disablement
        // 20 gltm_2_0- inner node disablement
        // 21 xnr_5_2- inner node disablement
        // 22 vcr_3_1- inner node disablement
        // 23 glim_2_0- inner node disablement
        // 24 acm_1_1- inner node disablement
        // 25 gammatm_v4- inner node disablement
        // 26 csc_1_1- inner node disablement
        // 27 fr_grid_1_0- inner node disablement
        // 28 b2i_ds_1_1- inner node disablement
        // 29 upscaler_1_0- inner node disablement
        // 30 lbff_crop_espa_1_3- inner node disablement
        // 31 tnr_scale_lb- inner node disablement
        // 32 odr_output_ps_1_3- inner node disablement
        // 33 odr_output_me_1_3- inner node disablement
        // 34 odr_af_std_1_3- inner node disablement
        // 35 ifd_gmv_1_3- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_3- inner node disablement
        // 38 odr_gmv_feature_1_3- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFFF;
    } else  // default inner node
    {
        // RBM - 0x00000000000000000000003EAA53960E
        bitmaps.rbm[0] = 0xAA53960E;
        bitmaps.rbm[1] = 0x3E;
        // DEB - 0x00000000000000000001FFFFD41FF1FF
        bitmaps.deb[0] = 0xD41FF1FF;
        bitmaps.deb[1] = 0x1FFFF;
        // TEB - 0x0000000001BC3DEF
        bitmaps.teb[0] = 0x1BC3DEF;
        // REB - 0x000000000000000000000000007B7FF7
        bitmaps.reb[0] = 0x7B7FF7;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);
}

/*
 * Graph 100000
 */
StaticGraph100000::StaticGraph100000(GraphConfiguration100000** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100000,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100000[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions =
        new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 10; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100000::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100000::~StaticGraph100000() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100000::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayer initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffBayerInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[8]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[9]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[9]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive =
        !(lbffBayerInnerOptions & noLbOutputPs);  // lbff_Bayer:terminal_connect_ps_output ->
                                                  // bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 10; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100001
 */
StaticGraph100001::StaticGraph100001(GraphConfiguration100001** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100001,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100001[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerWithGmvOuterNodeConfiguration** lbffBayerWithGmvOuterNodeConfigurationOptions =
        new LbffBayerWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerWithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerWithGmvOuterNode.Init(lbffBayerWithGmvOuterNodeConfigurationOptions,
                                    kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerWithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[20];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 22; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerWithGmvOuterNode = &_lbffBayerWithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerWithGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100001::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100001::~StaticGraph100001() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100001::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerWithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerWithGmvInnerOptions |= (noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerWithGmvOuterNode->setInnerNode(lbffBayerWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffBayerWithGmvInnerOptions &
                                   noGmv);  // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[8]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          noGmv);  // lbff_Bayer_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffBayerWithGmvInnerOptions &
                                   noLbOutputPs);  // lbff_Bayer_WithGmv:terminal_connect_ps_output
                                                   // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[10]->isActive =
        !(lbffBayerWithGmvInnerOptions & noLbOutputMe);  // lbff_Bayer_WithGmv:terminal_connect_me_output
                                                         // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 22; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[7]->isActive =
        (lbffBayerWithGmvInnerOptions & (noGmv | noBurstCapture | noIr | noPdaf)) !=
        (noGmv | noBurstCapture | noIr |
         noPdaf);  // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100002
 */
StaticGraph100002::StaticGraph100002(GraphConfiguration100002** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100002,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100002[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions =
        new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 15; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100002::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100002::~StaticGraph100002() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100002::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayer initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[13]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive =
        !(lbffBayerInnerOptions & noLbOutputPs);  // lbff_Bayer:terminal_connect_ps_output ->
                                                  // bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive =
        !(lbffBayerInnerOptions & noLbOutputMe);  // lbff_Bayer:terminal_connect_me_output ->
                                                  // bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 15; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                 (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                 // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100003
 */
StaticGraph100003::StaticGraph100003(GraphConfiguration100003** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100003,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100003[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerWithGmvOuterNodeConfiguration** lbffBayerWithGmvOuterNodeConfigurationOptions =
        new LbffBayerWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerWithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerWithGmvOuterNode.Init(lbffBayerWithGmvOuterNodeConfigurationOptions,
                                    kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerWithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 20; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerWithGmvOuterNode = &_lbffBayerWithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerWithGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100003::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100003::~StaticGraph100003() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100003::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerWithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerWithGmvInnerOptions |= (noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerWithGmvOuterNode->setInnerNode(lbffBayerWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          no3A);  // lbff_Bayer_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffBayerWithGmvInnerOptions &
                                   noGmv);  // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[8]->isActive =
        !(lbffBayerWithGmvInnerOptions &
          noGmv);  // lbff_Bayer_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffBayerWithGmvInnerOptions &
                                   noLbOutputPs);  // lbff_Bayer_WithGmv:terminal_connect_ps_output
                                                   // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[10]->isActive =
        !(lbffBayerWithGmvInnerOptions & noLbOutputMe);  // lbff_Bayer_WithGmv:terminal_connect_me_output
                                                         // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 20; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[7]->isActive =
        (lbffBayerWithGmvInnerOptions & (noGmv | noBurstCapture | noIr | noPdaf)) !=
        (noGmv | noBurstCapture | noIr |
         noPdaf);  // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100005
 */
StaticGraph100005::StaticGraph100005(GraphConfiguration100005** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100005,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100005[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions =
        new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 20; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swNntmOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100005::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100005::~StaticGraph100005() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100005::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayer initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive =
        !(lbffBayerInnerOptions & noLbOutputPs);  // lbff_Bayer:terminal_connect_ps_output ->
                                                  // bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive =
        !(lbffBayerInnerOptions & noLbOutputMe);  // lbff_Bayer:terminal_connect_me_output ->
                                                  // bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 20; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                 (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                 // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100006
 */
StaticGraph100006::StaticGraph100006(GraphConfiguration100006** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100006,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration),
          _irSubGraph(_sinkMappingConfiguration),
          _image_irSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100006[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrOuterNodeConfiguration** lbffRgbIrOuterNodeConfigurationOptions =
        new LbffRgbIrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvIrStreamOuterNodeConfiguration** lbffIrNoGmvIrStreamOuterNodeConfigurationOptions =
        new LbffIrNoGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrWithTnrOuterNodeConfiguration** bbpsIrWithTnrOuterNodeConfigurationOptions =
        new BbpsIrWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffRgbIrOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        lbffIrNoGmvIrStreamOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffIrNoGmvIrStreamOuterNodeConfiguration;
        bbpsIrWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsIrWithTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrOuterNode.Init(lbffRgbIrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _lbffIrNoGmvIrStreamOuterNode.Init(lbffIrNoGmvIrStreamOuterNodeConfigurationOptions,
                                       kernelConfigurationsOptionsCount);
    _bbpsIrWithTnrOuterNode.Init(bbpsIrWithTnrOuterNodeConfigurationOptions,
                                 kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffRgbIrOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] lbffIrNoGmvIrStreamOuterNodeConfigurationOptions;
    delete[] bbpsIrWithTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[0] = link;
    _irSubGraph.links[0] = link;
    _image_irSubGraph.links[0] = link;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[16] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[8] = link;
    _image_irSubGraph.links[8] = link;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[9] = link;
    _image_irSubGraph.links[9] = link;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[10] = link;
    _image_irSubGraph.links[10] = link;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[11] = link;
    _image_irSubGraph.links[11] = link;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _imageSubGraph.links[12] = link;
    _image_irSubGraph.links[12] = link;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[13] = link;
    _image_irSubGraph.links[13] = link;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[14] = link;
    _image_irSubGraph.links[14] = link;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[15] = link;
    _image_irSubGraph.links[15] = link;

    link = &_graphLinks[17];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[17] = link;

    link = &_graphLinks[18];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[18] = link;

    link = &_graphLinks[19];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[19] = link;

    link = &_graphLinks[20];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[20] = link;

    link = &_graphLinks[21];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[21] = link;

    link = &_graphLinks[22];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[22] = link;

    link = &_graphLinks[23];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[23] = link;

    link = &_graphLinks[24];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[16] = link;
    _image_irSubGraph.links[24] = link;

    link = &_graphLinks[25];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[17] = link;
    _image_irSubGraph.links[25] = link;

    link = &_graphLinks[26];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _irSubGraph.links[18] = link;
    _image_irSubGraph.links[26] = link;

    link = &_graphLinks[27];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[19] = link;
    _image_irSubGraph.links[27] = link;

    link = &_graphLinks[28];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[20] = link;
    _image_irSubGraph.links[28] = link;

    for (uint8_t i = 0; i < 29; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _irSubGraph.isysOuterNode = &_isysOuterNode;
    _irSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _irSubGraph.lbffIrNoGmvIrStreamOuterNode = &_lbffIrNoGmvIrStreamOuterNode;
    _irSubGraph.bbpsIrWithTnrOuterNode = &_bbpsIrWithTnrOuterNode;
    _image_irSubGraph.isysOuterNode = &_isysOuterNode;
    _image_irSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _image_irSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode = &_lbffIrNoGmvIrStreamOuterNode;
    _image_irSubGraph.bbpsIrWithTnrOuterNode = &_bbpsIrWithTnrOuterNode;

    // choose the selected sub graph
    if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.previewIr ==
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.video ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.stills ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail ==
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrWithTnrOuterNode->contextId = 3;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _image_irSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 3;
        _image_irSubGraph.bbpsIrWithTnrOuterNode->contextId = 4;
    } else {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100006::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffRgbIrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffIrNoGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsIrWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100006::~StaticGraph100006() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100006::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrInnerOptions |= (noIr);

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffRgbIrInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputPs);  // lbff_RgbIr:terminal_connect_ps_output ->
                                                  // bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputMe);  // lbff_RgbIr:terminal_connect_me_output ->
                                                  // bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 16; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100006::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrInnerOptions |= (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrInnerOptions |= (noLbOutputPs | noLbOutputMe);

    /*
     * Setting Node lbffIrNoGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNoGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsIrWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrWithTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrWithTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[20]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[11]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[12]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[13]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[20]->isActive =
        !(bbpsIrWithTnrInnerOptions & noMp);  // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive =
        !(lbffRgbIrInnerOptions & noIr);  // lbff_RgbIr:terminal_connect_ir_output ->
                                          // lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[14]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[15]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputMe);  // lbff_Ir_NoGmv_IrStream:terminal_connect_me_output
                                                    // -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 21; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[16]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[17]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[18]->isActive =
        (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[19]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100006::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = None;
    // combine inner options for the node common sub graphs
    lbffRgbIrInnerOptions |= imagePublicInnerNodeConfiguration;
    lbffRgbIrInnerOptions |= irPublicInnerNodeConfiguration;

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Setting Node lbffIrNoGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNoGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsIrWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrWithTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrWithTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[28]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffRgbIrInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[18]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[19]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[20]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[21]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[28]->isActive =
        !(bbpsIrWithTnrInnerOptions & noMp);  // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[17]->isActive =
        !(lbffRgbIrInnerOptions & noIr);  // lbff_RgbIr:terminal_connect_ir_output ->
                                          // lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[8]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputPs);  // lbff_RgbIr:terminal_connect_ps_output ->
                                                  // bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputMe);  // lbff_RgbIr:terminal_connect_me_output ->
                                                  // bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[22]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[23]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputMe);  // lbff_Ir_NoGmv_IrStream:terminal_connect_me_output
                                                    // -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 29; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd
    subGraphLinks[24]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[25]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[26]->isActive =
        (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[27]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100007
 */
StaticGraph100007::StaticGraph100007(GraphConfiguration100007** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100007,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100007[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerBurstOutNo3AOuterNodeConfiguration**
        lbffBayerBurstOutNo3AOuterNodeConfigurationOptions =
            new LbffBayerBurstOutNo3AOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerBurstOutNo3AOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerBurstOutNo3AOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerBurstOutNo3AOuterNode.Init(lbffBayerBurstOutNo3AOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerBurstOutNo3AOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerBurstOutNo3A;
    link->destNode = &_lbffBayerBurstOutNo3AOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::LbffBayerBurstOutNo3A;
    link->srcNode = &_lbffBayerBurstOutNo3AOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 3; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerBurstOutNo3AOuterNode = &_lbffBayerBurstOutNo3AOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerBurstOutNo3AOuterNode->contextId = 1;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100007::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerBurstOutNo3AOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100007::~StaticGraph100007() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100007::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerBurstOutNo3A initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerBurstOutNo3AInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerBurstOutNo3AInnerOptions |=
        (no3A | noGmv | noIr | noLbOutputPs | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffBayerBurstOutNo3AInnerOptions |= (noLbOutputPs | noLbOutputMe | noPdaf);
    // active private inner options according to links
    if (subGraphLinks[2]->linkConfiguration->bufferSize == 0 && true) {
        lbffBayerBurstOutNo3AInnerOptions |= noBurstCapture;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerBurstOutNo3AOuterNode->setInnerNode(lbffBayerBurstOutNo3AInnerOptions);

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[2]->isActive = !(
        lbffBayerBurstOutNo3AInnerOptions &
        noBurstCapture);  // lbff_Bayer_BurstOut_No3A:terminal_connect_burst_isp_output -> image_mp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 3; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100008
 */
StaticGraph100008::StaticGraph100008(GraphConfiguration100008** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100008,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration),
          _irSubGraph(_sinkMappingConfiguration),
          _image_irSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100008[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrOuterNodeConfiguration** lbffRgbIrOuterNodeConfigurationOptions =
        new LbffRgbIrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvIrStreamOuterNodeConfiguration** lbffIrNoGmvIrStreamOuterNodeConfigurationOptions =
        new LbffIrNoGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrNoTnrOuterNodeConfiguration** bbpsIrNoTnrOuterNodeConfigurationOptions =
        new BbpsIrNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffRgbIrOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
        lbffIrNoGmvIrStreamOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffIrNoGmvIrStreamOuterNodeConfiguration;
        bbpsIrNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsIrNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrOuterNode.Init(lbffRgbIrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _lbffIrNoGmvIrStreamOuterNode.Init(lbffIrNoGmvIrStreamOuterNodeConfigurationOptions,
                                       kernelConfigurationsOptionsCount);
    _bbpsIrNoTnrOuterNode.Init(bbpsIrNoTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffRgbIrOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;
    delete[] lbffIrNoGmvIrStreamOuterNodeConfigurationOptions;
    delete[] bbpsIrNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[0] = link;
    _irSubGraph.links[0] = link;
    _image_irSubGraph.links[0] = link;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[11] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[8] = link;
    _image_irSubGraph.links[8] = link;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[9] = link;
    _image_irSubGraph.links[9] = link;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[10] = link;
    _image_irSubGraph.links[10] = link;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[12] = link;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[13] = link;

    link = &_graphLinks[14];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[14] = link;

    link = &_graphLinks[15];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[15] = link;

    link = &_graphLinks[16];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[16] = link;

    link = &_graphLinks[17];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsIrNoTnr;
    link->destNode = &_bbpsIrNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[17] = link;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsIrNoTnr;
    link->srcNode = &_bbpsIrNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[18] = link;

    for (uint8_t i = 0; i < 19; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;
    _irSubGraph.isysOuterNode = &_isysOuterNode;
    _irSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _irSubGraph.lbffIrNoGmvIrStreamOuterNode = &_lbffIrNoGmvIrStreamOuterNode;
    _irSubGraph.bbpsIrNoTnrOuterNode = &_bbpsIrNoTnrOuterNode;
    _image_irSubGraph.isysOuterNode = &_isysOuterNode;
    _image_irSubGraph.lbffRgbIrOuterNode = &_lbffRgbIrOuterNode;
    _image_irSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;
    _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode = &_lbffIrNoGmvIrStreamOuterNode;
    _image_irSubGraph.bbpsIrNoTnrOuterNode = &_bbpsIrNoTnrOuterNode;

    // choose the selected sub graph
    if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.previewIr ==
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.video ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.stills ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail ==
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrNoTnrOuterNode->contextId = 3;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _image_irSubGraph.bbpsNoTnrOuterNode->contextId = 2;
        _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 3;
        _image_irSubGraph.bbpsIrNoTnrOuterNode->contextId = 4;
    } else {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100008::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffRgbIrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffIrNoGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsIrNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100008::~StaticGraph100008() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100008::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrInnerOptions |= (noIr | noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[9]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[10]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputPs);  // lbff_RgbIr:terminal_connect_ps_output ->
                                                  // bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 11; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100008::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrInnerOptions |= (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrInnerOptions |= (noLbOutputPs | noLbOutputMe);

    /*
     * Setting Node lbffIrNoGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNoGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsIrNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrNoTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrNoTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrNoTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrNoTnrOuterNode->setInnerNode(bbpsIrNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[11]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[12]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[13]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[15]->isActive =
        !(bbpsIrNoTnrInnerOptions & noMp);  // bbps_Ir_NoTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive =
        !(lbffRgbIrInnerOptions & noIr);  // lbff_RgbIr:terminal_connect_ir_output ->
                                          // lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[14]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 16; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100008::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIr initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrInnerOptions = None;
    // always active private inner options
    lbffRgbIrInnerOptions |= (noLbOutputMe);
    // combine inner options for the node common sub graphs
    lbffRgbIrInnerOptions |= imagePublicInnerNodeConfiguration;
    lbffRgbIrInnerOptions |= irPublicInnerNodeConfiguration;

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[9]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[10]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Setting Node lbffIrNoGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNoGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffIrNoGmvIrStreamInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsIrNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrNoTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrNoTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrNoTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffIrNoGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrNoTnrOuterNode->setInnerNode(bbpsIrNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(
        lbffRgbIrInnerOptions & no3A);  // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[13]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[14]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[15]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[16]->isActive =
        !(lbffIrNoGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[18]->isActive =
        !(bbpsIrNoTnrInnerOptions & noMp);  // bbps_Ir_NoTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive =
        !(lbffRgbIrInnerOptions & noIr);  // lbff_RgbIr:terminal_connect_ir_output ->
                                          // lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[8]->isActive =
        !(lbffRgbIrInnerOptions & noLbOutputPs);  // lbff_RgbIr:terminal_connect_ps_output ->
                                                  // bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[17]->isActive = !(lbffIrNoGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 19; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100015
 */
StaticGraph100015::StaticGraph100015(GraphConfiguration100015** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100015,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100015[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions =
        new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 9; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100015::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100015::~StaticGraph100015() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100015::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayer initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping
    // active private inner options according to links
    if (subGraphLinks[7]->linkConfiguration->bufferSize == 0 && true) {
        lbffBayerInnerOptions |= noLbOutputPs;
    }
    if (subGraphLinks[8]->linkConfiguration->bufferSize == 0 && true) {
        lbffBayerInnerOptions |= noLbOutputMe;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(
        lbffBayerInnerOptions & no3A);  // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(
        lbffBayerInnerOptions & noLbOutputPs);  // lbff_Bayer:terminal_connect_ps_output -> image_mp
    subGraphLinks[8]->isActive = !(
        lbffBayerInnerOptions & noLbOutputMe);  // lbff_Bayer:terminal_connect_me_output -> image_dp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 9; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100016
 */
StaticGraph100016::StaticGraph100016(GraphConfiguration100016** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100016,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100016[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] bbpsNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[2];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 3; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 0;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100016::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100016::~StaticGraph100016() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100016::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[1]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[2]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[1]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[2]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 3; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100025
 */
StaticGraph100025::StaticGraph100025(GraphConfiguration100025** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100025,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100025[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvOuterNodeConfiguration** lbffIrNoGmvOuterNodeConfigurationOptions =
        new LbffIrNoGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffIrNoGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffIrNoGmvOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrNoGmvOuterNode.Init(lbffIrNoGmvOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffIrNoGmvOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffIrNoGmv;
    link->destNode = &_lbffIrNoGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffIrNoGmv;
    link->destNode = &_lbffIrNoGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 10; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffIrNoGmvOuterNode = &_lbffIrNoGmvOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffIrNoGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100025::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffIrNoGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100025::~StaticGraph100025() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100025::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffIrNoGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNoGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNoGmvInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffIrNoGmvInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[8]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[9]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffIrNoGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffIrNoGmvOuterNode->setInnerNode(lbffIrNoGmvInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffIrNoGmvInnerOptions & no3A);  // lbff_Ir_NoGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffIrNoGmvInnerOptions &
          no3A);  // lbff_Ir_NoGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffIrNoGmvInnerOptions &
          no3A);  // lbff_Ir_NoGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffIrNoGmvInnerOptions &
          no3A);  // lbff_Ir_NoGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[9]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive =
        !(lbffIrNoGmvInnerOptions & noLbOutputPs);  // lbff_Ir_NoGmv:terminal_connect_ps_output ->
                                                    // bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 10; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100026
 */
StaticGraph100026::StaticGraph100026(GraphConfiguration100026** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100026,
                             selectedSettingsId, zoomKeyResolutions),

          _rawSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100026[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::RawIsys;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 2; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _rawSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _rawSubGraph.isysOuterNode = &_isysOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_rawSubGraph;

    // logical node IDs
    _rawSubGraph.isysOuterNode->contextId = 0;
}

StaticGraphStatus StaticGraph100026::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100026::~StaticGraph100026() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100027
 */
StaticGraph100027::StaticGraph100027(GraphConfiguration100027** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100027,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100027[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions =
        new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2OuterNodeConfiguration** lbffBayerPdaf2OuterNodeConfigurationOptions =
        new LbffBayerPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf2OuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2OuterNode.Init(lbffBayerPdaf2OuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysPdaf2OuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf2OuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::PdafBuffer;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 2;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 13; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysPdaf2OuterNode = &_isysPdaf2OuterNode;
    _imageSubGraph.lbffBayerPdaf2OuterNode = &_lbffBayerPdaf2OuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysPdaf2OuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf2OuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100027::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100027::~StaticGraph100027() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100027::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf2 initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf2InnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf2InnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe);
    // active public options according to sink mapping
    // always active private inner options
    lbffBayerPdaf2InnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[11]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[12]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2InnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2OuterNode->setInnerNode(lbffBayerPdaf2InnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffBayerPdaf2InnerOptions &
                                   no3A);  // lbff_Bayer_Pdaf2:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[12]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive =
        !(lbffBayerPdaf2InnerOptions & noLbOutputPs);  // lbff_Bayer_Pdaf2:terminal_connect_ps_output
                                                       // -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf2:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 13; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100028
 */
StaticGraph100028::StaticGraph100028(GraphConfiguration100028** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100028,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100028[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions =
        new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf3OuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 11; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerPdaf3OuterNode = &_lbffBayerPdaf3OuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf3OuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100028::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100028::~StaticGraph100028() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100028::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf3 initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf3InnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf3InnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe);
    // active public options according to sink mapping
    // always active private inner options
    lbffBayerPdaf3InnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[9]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[10]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions &
                                   no3A);  // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf3InnerOptions & noLbOutputPs);  // lbff_Bayer_Pdaf3:terminal_connect_ps_output
                                                       // -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 11; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100029
 */
StaticGraph100029::StaticGraph100029(GraphConfiguration100029** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100029,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100029[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions =
        new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2OuterNodeConfiguration** lbffBayerPdaf2OuterNodeConfigurationOptions =
        new LbffBayerPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf2OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2OuterNode.Init(lbffBayerPdaf2OuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);

    delete[] isysPdaf2OuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf2OuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::PdafBuffer;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 2;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 18; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysPdaf2OuterNode = &_isysPdaf2OuterNode;
    _imageSubGraph.lbffBayerPdaf2OuterNode = &_lbffBayerPdaf2OuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysPdaf2OuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf2OuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100029::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100029::~StaticGraph100029() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100029::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf2 initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf2InnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf2InnerOptions |= (noGmv | noBurstCapture | noIr);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[17]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2InnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf2InnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2OuterNode->setInnerNode(lbffBayerPdaf2InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffBayerPdaf2InnerOptions &
                                   no3A);  // lbff_Bayer_Pdaf2:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          no3A);  // lbff_Bayer_Pdaf2:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive =
        !(lbffBayerPdaf2InnerOptions & noLbOutputPs);  // lbff_Bayer_Pdaf2:terminal_connect_ps_output
                                                       // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive =
        !(lbffBayerPdaf2InnerOptions & noLbOutputMe);  // lbff_Bayer_Pdaf2:terminal_connect_me_output
                                                       // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf2InnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf2:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 18; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100030
 */
StaticGraph100030::StaticGraph100030(GraphConfiguration100030** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100030,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100030[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions =
        new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf3OuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 16; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerPdaf3OuterNode = &_lbffBayerPdaf3OuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf3OuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100030::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100030::~StaticGraph100030() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100030::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf3 initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf3InnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf3InnerOptions |= (noGmv | noBurstCapture | noIr);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf3InnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions &
                                   no3A);  // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf3InnerOptions & noLbOutputPs);  // lbff_Bayer_Pdaf3:terminal_connect_ps_output
                                                       // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf3InnerOptions & noLbOutputMe);  // lbff_Bayer_Pdaf3:terminal_connect_me_output
                                                       // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 16; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100031
 */
StaticGraph100031::StaticGraph100031(GraphConfiguration100031** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100031,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100031[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol2InputsOuterNodeConfiguration** lbffDol2InputsOuterNodeConfigurationOptions =
        new LbffDol2InputsOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDol2InputsOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol2InputsOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDol2InputsOuterNode.Init(lbffDol2InputsOuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDol2InputsOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[15];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 18; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDol2InputsOuterNode = &_lbffDol2InputsOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDol2InputsOuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    _imageSubGraph.swNntmOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100031::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol2InputsOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100031::~StaticGraph100031() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100031::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol2Inputs initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol2InputsInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol2InputsInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffDol2InputsInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[12]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol2InputsInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol2InputsOuterNode->setInnerNode(lbffDol2InputsInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffDol2InputsInnerOptions &
                                   no3A);  // lbff_Dol2Inputs:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[11]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[13]->isActive =
        !(bbpsNoTnrInnerOptions &
          noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[12]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[14]->isActive =
        !(bbpsNoTnrInnerOptions &
          noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive =
        !(lbffDol2InputsInnerOptions & noLbOutputPs);  // lbff_Dol2Inputs:terminal_connect_ps_output
                                                       // -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 18; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100032
 */
StaticGraph100032::StaticGraph100032(GraphConfiguration100032** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100032,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100032[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol2InputsOuterNodeConfiguration** lbffDol2InputsOuterNodeConfigurationOptions =
        new LbffDol2InputsOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDol2InputsOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol2InputsOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDol2InputsOuterNode.Init(lbffDol2InputsOuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDol2InputsOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol2Inputs;
    link->destNode = &_lbffDol2InputsOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol2Inputs;
    link->srcNode = &_lbffDol2InputsOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[20];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[21];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[22];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 23; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDol2InputsOuterNode = &_lbffDol2InputsOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDol2InputsOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swNntmOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100032::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol2InputsOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100032::~StaticGraph100032() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100032::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol2Inputs initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol2InputsInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol2InputsInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol2InputsInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffDol2InputsInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol2InputsOuterNode->setInnerNode(lbffDol2InputsInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffDol2InputsInnerOptions &
                                   no3A);  // lbff_Dol2Inputs:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(lbffDol2InputsInnerOptions &
          no3A);  // lbff_Dol2Inputs:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive =
        !(lbffDol2InputsInnerOptions & noLbOutputPs);  // lbff_Dol2Inputs:terminal_connect_ps_output
                                                       // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive =
        !(lbffDol2InputsInnerOptions & noLbOutputMe);  // lbff_Dol2Inputs:terminal_connect_me_output
                                                       // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 23; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100033
 */
StaticGraph100033::StaticGraph100033(GraphConfiguration100033** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100033,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100033[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDolSmoothOuterNodeConfiguration** lbffDolSmoothOuterNodeConfigurationOptions =
        new LbffDolSmoothOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol3InputsOuterNodeConfiguration** lbffDol3InputsOuterNodeConfigurationOptions =
        new LbffDol3InputsOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions =
        new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDolSmoothOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDolSmoothOuterNodeConfiguration;
        lbffDol3InputsOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol3InputsOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDolSmoothOuterNode.Init(lbffDolSmoothOuterNodeConfigurationOptions,
                                 kernelConfigurationsOptionsCount);
    _lbffDol3InputsOuterNode.Init(lbffDol3InputsOuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDolSmoothOuterNodeConfigurationOptions;
    delete[] lbffDol3InputsOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDolSmooth;
    link->destNode = &_lbffDolSmoothOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDolSmooth;
    link->srcNode = &_lbffDolSmoothOuterNode;
    link->srcTerminalId = 17;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 20; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDolSmoothOuterNode = &_lbffDolSmoothOuterNode;
    _imageSubGraph.lbffDol3InputsOuterNode = &_lbffDol3InputsOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDolSmoothOuterNode->contextId = 1;
    _imageSubGraph.lbffDol3InputsOuterNode->contextId = 2;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 3;
    _imageSubGraph.swNntmOuterNode->contextId = 4;
    _imageSubGraph.swScalerOuterNode->contextId = 5;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100033::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDolSmoothOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol3InputsOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100033::~StaticGraph100033() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100033::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol3Inputs initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol3InputsInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol3InputsInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffDol3InputsInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 && true) {
        bbpsNoTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol3InputsInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol3InputsOuterNode->setInnerNode(lbffDol3InputsInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[7]->isActive = !(lbffDol3InputsInnerOptions &
                                   no3A);  // lbff_Dol3Inputs:terminal_connect_ae_output -> ae_out
    subGraphLinks[8]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[9]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[10]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[13]->isActive =
        !(bbpsNoTnrInnerOptions & noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive =
        !(bbpsNoTnrInnerOptions &
          noMp);  // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[14]->isActive =
        !(bbpsNoTnrInnerOptions & noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive =
        !(bbpsNoTnrInnerOptions &
          noDp);  // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive =
        !(lbffDol3InputsInnerOptions & noLbOutputPs);  // lbff_Dol3Inputs:terminal_connect_ps_output
                                                       // -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 20; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100034
 */
StaticGraph100034::StaticGraph100034(GraphConfiguration100034** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100034,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100034[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDolSmoothOuterNodeConfiguration** lbffDolSmoothOuterNodeConfigurationOptions =
        new LbffDolSmoothOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol3InputsOuterNodeConfiguration** lbffDol3InputsOuterNodeConfigurationOptions =
        new LbffDol3InputsOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDolSmoothOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDolSmoothOuterNodeConfiguration;
        lbffDol3InputsOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol3InputsOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDolSmoothOuterNode.Init(lbffDolSmoothOuterNodeConfigurationOptions,
                                 kernelConfigurationsOptionsCount);
    _lbffDol3InputsOuterNode.Init(lbffDol3InputsOuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDolSmoothOuterNodeConfigurationOptions;
    delete[] lbffDol3InputsOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDolSmooth;
    link->destNode = &_lbffDolSmoothOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDolSmooth;
    link->srcNode = &_lbffDolSmoothOuterNode;
    link->srcTerminalId = 17;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol3Inputs;
    link->destNode = &_lbffDol3InputsOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffDol3Inputs;
    link->srcNode = &_lbffDol3InputsOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[22];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[23];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[24];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 25; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDolSmoothOuterNode = &_lbffDolSmoothOuterNode;
    _imageSubGraph.lbffDol3InputsOuterNode = &_lbffDol3InputsOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDolSmoothOuterNode->contextId = 1;
    _imageSubGraph.lbffDol3InputsOuterNode->contextId = 2;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 3;
    _imageSubGraph.swNntmOuterNode->contextId = 4;
    _imageSubGraph.swScalerOuterNode->contextId = 5;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100034::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDolSmoothOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol3InputsOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100034::~StaticGraph100034() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100034::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol3Inputs initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol3InputsInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol3InputsInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol3InputsInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffDol3InputsInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol3InputsOuterNode->setInnerNode(lbffDol3InputsInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[7]->isActive = !(lbffDol3InputsInnerOptions &
                                   no3A);  // lbff_Dol3Inputs:terminal_connect_ae_output -> ae_out
    subGraphLinks[8]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[9]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[10]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive =
        !(lbffDol3InputsInnerOptions &
          no3A);  // lbff_Dol3Inputs:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[20]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[21]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive =
        !(lbffDol3InputsInnerOptions & noLbOutputPs);  // lbff_Dol3Inputs:terminal_connect_ps_output
                                                       // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[13]->isActive =
        !(lbffDol3InputsInnerOptions & noLbOutputMe);  // lbff_Dol3Inputs:terminal_connect_me_output
                                                       // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 25; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[16]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[17]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100035
 */
StaticGraph100035::StaticGraph100035(GraphConfiguration100035** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100035,
                             selectedSettingsId, zoomKeyResolutions),

          _rawSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100035[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::RawIsys;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::RawIsysDolLong;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 4; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _rawSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _rawSubGraph.isysDolOuterNode = &_isysDolOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_rawSubGraph;

    // logical node IDs
    _rawSubGraph.isysDolOuterNode->contextId = 0;
}

StaticGraphStatus StaticGraph100035::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100035::~StaticGraph100035() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100036
 */
StaticGraph100036::StaticGraph100036(GraphConfiguration100036** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100036,
                             selectedSettingsId, zoomKeyResolutions),

          _rawSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100036[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions =
        new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);

    delete[] isysPdaf2OuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::PdafBuffer;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 2;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::RawIsys;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::RawIsysPdaf;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 4; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _rawSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _rawSubGraph.isysPdaf2OuterNode = &_isysPdaf2OuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_rawSubGraph;

    // logical node IDs
    _rawSubGraph.isysPdaf2OuterNode->contextId = 0;
}

StaticGraphStatus StaticGraph100036::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100036::~StaticGraph100036() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100037
 */
StaticGraph100037::StaticGraph100037(GraphConfiguration100037** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100037,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100037[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions =
        new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2WithGmvOuterNodeConfiguration**
        lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions =
            new LbffBayerPdaf2WithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf2WithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions,
                             kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2WithGmvOuterNode.Init(lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysPdaf2OuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::PdafBuffer;
    link->dest = GraphElementType::IsysPdaf2;
    link->destNode = &_isysPdaf2OuterNode;
    link->destTerminalId = 2;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[22];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 23; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysPdaf2OuterNode = &_isysPdaf2OuterNode;
    _imageSubGraph.lbffBayerPdaf2WithGmvOuterNode = &_lbffBayerPdaf2WithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysPdaf2OuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf2WithGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100037::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf2WithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100037::~StaticGraph100037() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100037::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf2WithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf2WithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf2WithGmvInnerOptions |= (noBurstCapture | noIr);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2WithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf2WithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2WithGmvOuterNode->setInnerNode(lbffBayerPdaf2WithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          noGmv);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          noGmv);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[20]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[21]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[13]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions &
                                    noLbOutputMe);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_me_output
                                                    // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf2WithGmvInnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 23; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive =
        (lbffBayerPdaf2WithGmvInnerOptions & (noGmv | noBurstCapture | noIr)) !=
        (noGmv | noBurstCapture | noIr);  // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_feature_output
                                          // -> lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_input
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[16]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[17]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100038
 */
StaticGraph100038::StaticGraph100038(GraphConfiguration100038** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100038,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100038[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3WithGmvOuterNodeConfiguration**
        lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions =
            new LbffBayerPdaf3WithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf3WithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3WithGmvOuterNode.Init(lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf3WithGmv;
    link->destNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3WithGmv;
    link->destNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffBayerPdaf3WithGmv;
    link->destNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[20];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 21; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerPdaf3WithGmvOuterNode = &_lbffBayerPdaf3WithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf3WithGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100038::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf3WithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100038::~StaticGraph100038() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100038::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf3WithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf3WithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf3WithGmvInnerOptions |= (noBurstCapture | noIr);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3WithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf3WithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3WithGmvOuterNode->setInnerNode(lbffBayerPdaf3WithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          no3A);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          noGmv);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          noGmv);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions &
                                    noLbOutputMe);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_me_output
                                                    // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf3WithGmvInnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 21; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive =
        (lbffBayerPdaf3WithGmvInnerOptions & (noGmv | noBurstCapture | noIr)) !=
        (noGmv | noBurstCapture | noIr);  // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_feature_output
                                          // -> lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100039
 */
StaticGraph100039::StaticGraph100039(GraphConfiguration100039** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100039,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration),
          _irSubGraph(_sinkMappingConfiguration),
          _image_irSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100039[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrWithGmvOuterNodeConfiguration** lbffRgbIrWithGmvOuterNodeConfigurationOptions =
        new LbffRgbIrWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrWithGmvIrStreamOuterNodeConfiguration**
        lbffIrWithGmvIrStreamOuterNodeConfigurationOptions =
            new LbffIrWithGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrWithTnrOuterNodeConfiguration** bbpsIrWithTnrOuterNodeConfigurationOptions =
        new BbpsIrWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrWithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffRgbIrWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        lbffIrWithGmvIrStreamOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffIrWithGmvIrStreamOuterNodeConfiguration;
        bbpsIrWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsIrWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrWithGmvOuterNode.Init(lbffRgbIrWithGmvOuterNodeConfigurationOptions,
                                    kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _lbffIrWithGmvIrStreamOuterNode.Init(lbffIrWithGmvIrStreamOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);
    _bbpsIrWithTnrOuterNode.Init(bbpsIrWithTnrOuterNodeConfigurationOptions,
                                 kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffRgbIrWithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] lbffIrWithGmvIrStreamOuterNodeConfigurationOptions;
    delete[] bbpsIrWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[0] = link;
    _irSubGraph.links[0] = link;
    _image_irSubGraph.links[0] = link;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffRgbIrWithGmv;
    link->destNode = &_lbffRgbIrWithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrWithGmvIrStream;
    link->destNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[21] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIrWithGmv;
    link->destNode = &_lbffRgbIrWithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffRgbIrWithGmv;
    link->destNode = &_lbffRgbIrWithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[8] = link;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[8] = link;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[9] = link;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[9] = link;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[10] = link;
    _image_irSubGraph.links[10] = link;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[11] = link;
    _image_irSubGraph.links[11] = link;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[12] = link;
    _image_irSubGraph.links[12] = link;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[13] = link;
    _image_irSubGraph.links[13] = link;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _imageSubGraph.links[14] = link;
    _image_irSubGraph.links[14] = link;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[15] = link;
    _image_irSubGraph.links[15] = link;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[16] = link;
    _image_irSubGraph.links[16] = link;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[17] = link;
    _image_irSubGraph.links[17] = link;

    link = &_graphLinks[19];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::LbffIrWithGmvIrStream;
    link->destNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[22] = link;

    link = &_graphLinks[20];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[23] = link;

    link = &_graphLinks[21];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[24] = link;

    link = &_graphLinks[22];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[25] = link;

    link = &_graphLinks[23];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[26] = link;

    link = &_graphLinks[24];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[16] = link;
    _image_irSubGraph.links[27] = link;

    link = &_graphLinks[25];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[17] = link;
    _image_irSubGraph.links[28] = link;

    link = &_graphLinks[26];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[18] = link;
    _image_irSubGraph.links[29] = link;

    link = &_graphLinks[27];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[19] = link;
    _image_irSubGraph.links[30] = link;

    link = &_graphLinks[28];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _irSubGraph.links[20] = link;
    _image_irSubGraph.links[31] = link;

    link = &_graphLinks[29];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[21] = link;
    _image_irSubGraph.links[32] = link;

    link = &_graphLinks[30];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[22] = link;
    _image_irSubGraph.links[33] = link;

    link = &_graphLinks[31];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[18] = link;
    _image_irSubGraph.links[18] = link;

    link = &_graphLinks[32];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[19] = link;
    _image_irSubGraph.links[19] = link;

    link = &_graphLinks[33];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[20] = link;
    _image_irSubGraph.links[20] = link;

    for (uint8_t i = 0; i < 34; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffRgbIrWithGmvOuterNode = &_lbffRgbIrWithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;
    _irSubGraph.isysOuterNode = &_isysOuterNode;
    _irSubGraph.lbffRgbIrWithGmvOuterNode = &_lbffRgbIrWithGmvOuterNode;
    _irSubGraph.lbffIrWithGmvIrStreamOuterNode = &_lbffIrWithGmvIrStreamOuterNode;
    _irSubGraph.bbpsIrWithTnrOuterNode = &_bbpsIrWithTnrOuterNode;
    _image_irSubGraph.isysOuterNode = &_isysOuterNode;
    _image_irSubGraph.lbffRgbIrWithGmvOuterNode = &_lbffRgbIrWithGmvOuterNode;
    _image_irSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _image_irSubGraph.swGdcOuterNode = &_swGdcOuterNode;
    _image_irSubGraph.lbffIrWithGmvIrStreamOuterNode = &_lbffIrWithGmvIrStreamOuterNode;
    _image_irSubGraph.bbpsIrWithTnrOuterNode = &_bbpsIrWithTnrOuterNode;

    // choose the selected sub graph
    if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.previewIr ==
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _imageSubGraph.swGdcOuterNode->contextId = 3;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.video ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.stills ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail ==
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _irSubGraph.lbffIrWithGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrWithTnrOuterNode->contextId = 3;
    } else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.video !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.stills !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.thumbnail !=
             static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawPdaf ==
             static_cast<int>(HwSink::Disconnected) &&
         _graphConfigurations[0].sinkMappingConfiguration.rawDolLong ==
             static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr !=
             static_cast<int>(HwSink::Disconnected) ||
         _graphConfigurations[0].sinkMappingConfiguration.previewIr !=
             static_cast<int>(HwSink::Disconnected))) {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _image_irSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _image_irSubGraph.swGdcOuterNode->contextId = 3;
        _image_irSubGraph.lbffIrWithGmvIrStreamOuterNode->contextId = 4;
        _image_irSubGraph.bbpsIrWithTnrOuterNode->contextId = 5;
    } else {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100039::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffRgbIrWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffIrWithGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsIrWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100039::~StaticGraph100039() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100039::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffRgbIrWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrWithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrWithGmvInnerOptions |= (noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrWithGmvInnerOptions |= (noIr);

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffRgbIrWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrWithGmvOuterNode->setInnerNode(lbffRgbIrWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrWithGmvInnerOptions &
                                   noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffRgbIrWithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_RgbIr_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive =
        !(lbffRgbIrWithGmvInnerOptions & noLbOutputMe);  // lbff_RgbIr_WithGmv:terminal_connect_me_output
                                                         // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 21; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive =
        (lbffRgbIrWithGmvInnerOptions & (noGmv | noBurstCapture | noPdaf)) !=
        (noGmv | noBurstCapture | noPdaf);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100039::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIrWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrWithGmvInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffRgbIrWithGmvInnerOptions |= (noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffRgbIrWithGmvInnerOptions |= (noLbOutputPs | noLbOutputMe);

    /*
     * Setting Node lbffIrWithGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrWithGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrWithGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsIrWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrWithTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrWithTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[22]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrWithGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrWithGmvIrStreamInnerOptions |=
        noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrWithGmvOuterNode->setInnerNode(lbffRgbIrWithGmvInnerOptions);
    lbffIrWithGmvIrStreamOuterNode->setInnerNode(lbffIrWithGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[9]->isActive = !(lbffRgbIrWithGmvInnerOptions &
                                   noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[10]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[12]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[13]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[14]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[15]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[22]->isActive =
        !(bbpsIrWithTnrInnerOptions & noMp);  // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[11]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          noIr);  // lbff_RgbIr_WithGmv:terminal_connect_ir_output ->
                  // lbff_Ir_WithGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[16]->isActive = !(lbffIrWithGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_WithGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[17]->isActive = !(lbffIrWithGmvIrStreamInnerOptions &
                                    noLbOutputMe);  // lbff_Ir_WithGmv_IrStream:terminal_connect_me_output
                                                    // -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 23; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive =
        (lbffRgbIrWithGmvInnerOptions & (noGmv | noBurstCapture | noPdaf)) !=
        (noGmv | noBurstCapture | noPdaf);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[18]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[19]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[20]->isActive =
        (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[21]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100039::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

    /*
     * Setting Node lbffRgbIrWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffRgbIrWithGmvInnerOptions = None;
    // combine inner options for the node common sub graphs
    lbffRgbIrWithGmvInnerOptions |= imagePublicInnerNodeConfiguration;
    lbffRgbIrWithGmvInnerOptions |= irPublicInnerNodeConfiguration;

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Setting Node lbffIrWithGmvIrStream initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrWithGmvIrStreamInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrWithGmvIrStreamInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsIrWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsIrWithTnrInnerOptions = irPublicInnerNodeConfiguration;
    // always active public inner options
    bbpsIrWithTnrInnerOptions |= (noDp);
    // active public options according to sink mapping
    if (subGraphLinks[33]->linkConfiguration->bufferSize == 0 && true) {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |=
        noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffRgbIrWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffIrWithGmvIrStreamInnerOptions |=
        noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrWithGmvIrStreamInnerOptions |=
        noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrWithGmvOuterNode->setInnerNode(lbffRgbIrWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);
    lbffIrWithGmvIrStreamOuterNode->setInnerNode(lbffIrWithGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          no3A);  // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrWithGmvInnerOptions &
                                   noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          noGmv);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[23]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[24]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[25]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[26]->isActive =
        !(lbffIrWithGmvIrStreamInnerOptions &
          no3A);  // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[33]->isActive =
        !(bbpsIrWithTnrInnerOptions & noMp);  // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[22]->isActive =
        !(lbffRgbIrWithGmvInnerOptions &
          noIr);  // lbff_RgbIr_WithGmv:terminal_connect_ir_output ->
                  // lbff_Ir_WithGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[10]->isActive = !(lbffRgbIrWithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_RgbIr_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive =
        !(lbffRgbIrWithGmvInnerOptions & noLbOutputMe);  // lbff_RgbIr_WithGmv:terminal_connect_me_output
                                                         // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[27]->isActive = !(lbffIrWithGmvIrStreamInnerOptions &
                                    noLbOutputPs);  // lbff_Ir_WithGmv_IrStream:terminal_connect_ps_output
                                                    // -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[28]->isActive = !(lbffIrWithGmvIrStreamInnerOptions &
                                    noLbOutputMe);  // lbff_Ir_WithGmv_IrStream:terminal_connect_me_output
                                                    // -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 34; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive =
        (lbffRgbIrWithGmvInnerOptions & (noGmv | noBurstCapture | noPdaf)) !=
        (noGmv | noBurstCapture | noPdaf);  // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output
                                            // -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd
    subGraphLinks[29]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[30]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[31]->isActive =
        (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[32]->isActive = (bbpsIrWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr ->
                                                  // bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100040
 */
StaticGraph100040::StaticGraph100040(GraphConfiguration100040** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100040,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100040[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol2InputsWithGmvOuterNodeConfiguration**
        lbffDol2InputsWithGmvOuterNodeConfigurationOptions =
            new LbffDol2InputsWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDol2InputsWithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol2InputsWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDol2InputsWithGmvOuterNode.Init(lbffDol2InputsWithGmvOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDol2InputsWithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol2InputsWithGmv;
    link->destNode = &_lbffDol2InputsWithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol2InputsWithGmv;
    link->destNode = &_lbffDol2InputsWithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol2InputsWithGmv;
    link->destNode = &_lbffDol2InputsWithGmvOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffDol2InputsWithGmv;
    link->destNode = &_lbffDol2InputsWithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffDol2InputsWithGmv;
    link->srcNode = &_lbffDol2InputsWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[22];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 23; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDol2InputsWithGmvOuterNode = &_lbffDol2InputsWithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDol2InputsWithGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100040::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol2InputsWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100040::~StaticGraph100040() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100040::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol2InputsWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol2InputsWithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol2InputsWithGmvInnerOptions |= (noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol2InputsWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffDol2InputsWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol2InputsWithGmvOuterNode->setInnerNode(lbffDol2InputsWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol2Inputs_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol2Inputs_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol2Inputs_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol2Inputs_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol2Inputs_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[10]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          noGmv);  // lbff_Dol2Inputs_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Dol2Inputs_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive =
        !(lbffDol2InputsWithGmvInnerOptions &
          noGmv);  // lbff_Dol2Inputs_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[18]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[20]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[19]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[21]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive = !(lbffDol2InputsWithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_Dol2Inputs_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[13]->isActive = !(lbffDol2InputsWithGmvInnerOptions &
                                    noLbOutputMe);  // lbff_Dol2Inputs_WithGmv:terminal_connect_me_output
                                                    // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 23; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive =
        (lbffDol2InputsWithGmvInnerOptions & (noGmv | noBurstCapture | noIr | noPdaf)) !=
        (noGmv | noBurstCapture | noIr |
         noPdaf);  // lbff_Dol2Inputs_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Dol2Inputs_WithGmv:terminal_connect_gmv_input
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[16]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[17]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100041
 */
StaticGraph100041::StaticGraph100041(GraphConfiguration100041** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100041,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100041[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions =
        new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDolSmoothOuterNodeConfiguration** lbffDolSmoothOuterNodeConfigurationOptions =
        new LbffDolSmoothOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDol3InputsWithGmvOuterNodeConfiguration**
        lbffDol3InputsWithGmvOuterNodeConfigurationOptions =
            new LbffDol3InputsWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions =
        new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        lbffDolSmoothOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDolSmoothOuterNodeConfiguration;
        lbffDol3InputsWithGmvOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffDol3InputsWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDolSmoothOuterNode.Init(lbffDolSmoothOuterNodeConfigurationOptions,
                                 kernelConfigurationsOptionsCount);
    _lbffDol3InputsWithGmvOuterNode.Init(lbffDol3InputsWithGmvOuterNodeConfigurationOptions,
                                         kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] lbffDolSmoothOuterNodeConfigurationOptions;
    delete[] lbffDol3InputsWithGmvOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGdcOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::SensorDolLongExposure;
    link->dest = GraphElementType::IsysDol;
    link->destNode = &_isysDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDolSmooth;
    link->destNode = &_lbffDolSmoothOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffDol3InputsWithGmv;
    link->destNode = &_lbffDol3InputsWithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::LbffDol3InputsWithGmv;
    link->destNode = &_lbffDol3InputsWithGmvOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffDolSmooth;
    link->srcNode = &_lbffDolSmoothOuterNode;
    link->srcTerminalId = 17;
    link->dest = GraphElementType::LbffDol3InputsWithGmv;
    link->destNode = &_lbffDol3InputsWithGmvOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol3InputsWithGmv;
    link->destNode = &_lbffDol3InputsWithGmvOuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 21;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 24;
    link->dest = GraphElementType::LbffDol3InputsWithGmv;
    link->destNode = &_lbffDol3InputsWithGmvOuterNode;
    link->destTerminalId = 20;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 23;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[14];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[15];
    link->src = GraphElementType::LbffDol3InputsWithGmv;
    link->srcNode = &_lbffDol3InputsWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[22];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[23];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[24];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 25; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.lbffDolSmoothOuterNode = &_lbffDolSmoothOuterNode;
    _imageSubGraph.lbffDol3InputsWithGmvOuterNode = &_lbffDol3InputsWithGmvOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.lbffDolSmoothOuterNode->contextId = 1;
    _imageSubGraph.lbffDol3InputsWithGmvOuterNode->contextId = 2;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 3;
    _imageSubGraph.swGdcOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100041::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDolSmoothOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffDol3InputsWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100041::~StaticGraph100041() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100041::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol3InputsWithGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDol3InputsWithGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDol3InputsWithGmvInnerOptions |= (noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[22]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[21]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[23]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDol3InputsWithGmvInnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffDol3InputsWithGmvInnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDol3InputsWithGmvOuterNode->setInnerNode(lbffDol3InputsWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[7]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol3Inputs_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[8]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol3Inputs_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[9]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol3Inputs_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[10]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol3Inputs_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          no3A);  // lbff_Dol3Inputs_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[12]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          noGmv);  // lbff_Dol3Inputs_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Dol3Inputs_WithGmv:terminal_connect_gmv_input
    subGraphLinks[13]->isActive =
        !(lbffDol3InputsWithGmvInnerOptions &
          noGmv);  // lbff_Dol3Inputs_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[20]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[22]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[21]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[23]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[14]->isActive = !(lbffDol3InputsWithGmvInnerOptions &
                                    noLbOutputPs);  // lbff_Dol3Inputs_WithGmv:terminal_connect_ps_output
                                                    // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[15]->isActive = !(lbffDol3InputsWithGmvInnerOptions &
                                    noLbOutputMe);  // lbff_Dol3Inputs_WithGmv:terminal_connect_me_output
                                                    // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 25; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[12]->isActive =
        (lbffDol3InputsWithGmvInnerOptions & (noGmv | noBurstCapture | noIr | noPdaf)) !=
        (noGmv | noBurstCapture | noIr |
         noPdaf);  // lbff_Dol3Inputs_WithGmv:terminal_connect_gmv_feature_output ->
                   // lbff_Dol3Inputs_WithGmv:terminal_connect_gmv_input
    subGraphLinks[16]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[17]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[18]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[19]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100042
 */
StaticGraph100042::StaticGraph100042(GraphConfiguration100042** selectedGraphConfiguration,
                                     uint32_t kernelConfigurationsOptionsCount,
                                     ZoomKeyResolutions* zoomKeyResolutions,
                                     VirtualSinkMapping* sinkMappingConfiguration,
                                     SensorMode* selectedSensorMode, int32_t selectedSettingsId)
        : IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100042,
                             selectedSettingsId, zoomKeyResolutions),

          _imageSubGraph(_sinkMappingConfiguration) {
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100042[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions =
        new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions =
        new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions =
        new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions =
        new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions =
        new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i) {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] =
            &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions,
                                  kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions,
                               kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions,
                            kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerPdaf3OuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swNntmOuterNodeConfigurationOptions;
    delete[] swScalerOuterNodeConfigurationOptions;

    // Use default configuration
    updateConfiguration(0);

    // Declare all the links in the graph
    GraphLink* link = nullptr;
    link = &_graphLinks[0];
    link->src = GraphElementType::Sensor;
    link->dest = GraphElementType::Isys;
    link->destNode = &_isysOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[1];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 8;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[12];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;

    link = &_graphLinks[13];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[20];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 21; ++i) {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++) {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize) {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration =
            &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerPdaf3OuterNode = &_lbffBayerPdaf3OuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerPdaf3OuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swNntmOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if (_selectedGraphTopology != nullptr) {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100042::updateConfiguration(uint32_t selectedIndex) {
    StaticGraphStatus res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK) {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100042::~StaticGraph100042() {
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100042::configInnerNodes(
    SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) {
    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration =
        GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerPdaf3 initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerPdaf3InnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerPdaf3InnerOptions |= (noGmv | noBurstCapture | noIr);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 && true) {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |=
        noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf3InnerOptions |=
        noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions &
                                   no3A);  // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          no3A);  // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive =
        !(bbpsWithTnrInnerOptions & noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive =
        !(bbpsWithTnrInnerOptions &
          noMp);  // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[15]->isActive =
        !(bbpsWithTnrInnerOptions & noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[17]->isActive =
        !(bbpsWithTnrInnerOptions &
          noDp);  // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive =
        !(lbffBayerPdaf3InnerOptions & noLbOutputPs);  // lbff_Bayer_Pdaf3:terminal_connect_ps_output
                                                       // -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive =
        !(lbffBayerPdaf3InnerOptions & noLbOutputMe);  // lbff_Bayer_Pdaf3:terminal_connect_me_output
                                                       // -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive =
        !(lbffBayerPdaf3InnerOptions &
          noPdaf);  // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
    for (uint32_t i = 0; i < 21; i++) {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0) {
            subGraphLinks[i]->isActive = false;
        }
    }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_blend_yuvn_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
                                  (noMp | noDp);  // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr ->
                                                  // bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp | noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive =
        (bbpsWithTnrInnerOptions & (noMp | noDp)) !=
        (noMp |
         noDp);  // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}
