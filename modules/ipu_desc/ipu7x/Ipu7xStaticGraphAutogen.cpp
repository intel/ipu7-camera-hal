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

#include "Ipu7xStaticGraphAutogen.h"
#define CHECK_BITMAP64_BIT(bitmap, index) ((bitmap) & ((uint64_t)1 << (index)))

/*
 * External Interfaces
 */
IStaticGraphConfig::IStaticGraphConfig(SensorMode* selectedSensorMode, VirtualSinkMapping* sinkMappingConfiguration, int32_t graphId, int32_t settingsId, ZoomKeyResolutions* zoomKeyResolutions ) :
_selectedSensorMode(selectedSensorMode), _graphId(graphId), _settingsId(settingsId)
{
    memcpy(_sinkMappingConfiguration, sinkMappingConfiguration, sizeof(VirtualSinkMapping));
    // Copy zoom key resolutions
    _zoomKeyResolutions.numberOfZoomKeyOptions = zoomKeyResolutions->numberOfZoomKeyOptions;

    if (zoomKeyResolutions->numberOfZoomKeyOptions > 0)
    {
        _zoomKeyResolutions.zoomKeyResolutionOptions = new ZoomKeyResolution[zoomKeyResolutions->numberOfZoomKeyOptions];

        memcpy(_zoomKeyResolutions.zoomKeyResolutionOptions, zoomKeyResolutions->zoomKeyResolutionOptions,
            sizeof(ZoomKeyResolution) * zoomKeyResolutions->numberOfZoomKeyOptions);
    }
    else
    {
        _zoomKeyResolutions.zoomKeyResolutionOptions = nullptr;
    }
}

StaticGraphStatus IStaticGraphConfig::getSensorMode(SensorMode** sensorMode)
{
    if (!sensorMode)
    {
        STATIC_GRAPH_LOG("Sensor mode does not exist for this setting.");
        return StaticGraphStatus::SG_OK;
    }
    *sensorMode = _selectedSensorMode;
    return StaticGraphStatus::SG_OK;
};
StaticGraphStatus IStaticGraphConfig::getZoomKeyResolutions(ZoomKeyResolutions** zoomKeyResolutions)
{
    if (!zoomKeyResolutions || _zoomKeyResolutions.numberOfZoomKeyOptions == 0)
    {
        STATIC_GRAPH_LOG("Zoom key resolutions do not exist for this setting.");
        return StaticGraphStatus::SG_ERROR;
    }
    *zoomKeyResolutions = &_zoomKeyResolutions;
    return StaticGraphStatus::SG_OK;
};

StaticGraphStatus IStaticGraphConfig::getGraphTopology(GraphTopology** topology)
{
    *topology = _selectedGraphTopology;
    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus IStaticGraphConfig::getGraphId(int32_t* graphId)
{
    if (graphId == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    *graphId = _graphId;
    return StaticGraphStatus::SG_OK;
};

StaticGraphStatus IStaticGraphConfig::getSettingsId(int32_t* settingsId)
{
    if (settingsId == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    *settingsId = _settingsId;
    return StaticGraphStatus::SG_OK;
};

StaticGraphStatus IStaticGraphConfig::getVirtualSinkConnection(VirtualSink& virtualSink, HwSink* hwSink)
{
    switch (virtualSink)
    {
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
    case VirtualSink::PostProcessingStillsSink:
        *hwSink = static_cast<HwSink>(_sinkMappingConfiguration->postProcessingStills);
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
        STATIC_GRAPH_LOG("Failed to get virtual sink mapping for virtual sink %d", static_cast<int>(virtualSink));
        return StaticGraphStatus::SG_ERROR;
    }

    return StaticGraphStatus::SG_OK;
}

GraphTopology::GraphTopology(GraphLink** links, int32_t numOfLinks, VirtualSinkMapping* sinkMappingConfiguration) :
    links(links), numOfLinks(numOfLinks), _sinkMappingConfiguration(sinkMappingConfiguration) {}

StaticGraphStatus GraphTopology::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{
    // Default impl. No inner nodes in the sub-graph nodes.
    (void)subGraphInnerNodeConfiguration;
    return StaticGraphStatus::SG_OK;
};

InnerNodeOptionsFlags GraphTopology::GetInnerOptions(SubGraphPublicInnerNodeConfiguration* publicInnerOptions)
{
    InnerNodeOptionsFlags res = None;

    if (publicInnerOptions)
    {
        res |= noGmv & (publicInnerOptions->noGmv ? -1 : 0);
        res |= no3A & (publicInnerOptions->no3A ? -1 : 0);
        res |= noMp & (publicInnerOptions->noMp ? -1 : 0);
        res |= noDp & (publicInnerOptions->noDp ? -1 : 0);
        res |= noPpp & (publicInnerOptions->noPpp ? -1 : 0);
    }

    return res;
}

/*
 * Outer Nodes
 */

void OuterNode::Init(uint8_t nodeResourceId,
    NodeTypes nodeType,
    uint32_t kernelCount,
    uint32_t nodeKernelConfigurationsOptionsCount,
    uint32_t operationMode,
    uint32_t streamId,
    uint8_t nodeNumberOfFragments)
{
    resourceId = nodeResourceId;
    type = nodeType;
    nodeKernels.kernelCount = kernelCount;
    numberOfFragments = nodeNumberOfFragments;
    kernelConfigurationsOptionsCount = nodeKernelConfigurationsOptionsCount;

    kernelListOptions = new StaticGraphPacRunKernel*[kernelConfigurationsOptionsCount];

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; i++)
    {
        if (kernelCount > 0)
        {
            kernelListOptions[i] = new StaticGraphPacRunKernel[kernelCount];
            for (uint32_t j = 0; j < kernelCount; j++)
            {
                kernelListOptions[i][j].fragment_descs = nullptr;
            }
        }
        else
        {
            kernelListOptions[i] = nullptr;
        }
    }

    selectedKernelConfigurationIndex = 0;
    nodeKernels.kernelList = kernelListOptions[0];

    nodeKernels.operationMode = operationMode;
    nodeKernels.streamId = streamId;
}
OuterNode::~OuterNode()
{
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; i++)
    {
        delete[] kernelListOptions[i];
    }

    delete[] kernelListOptions;
}
void OuterNode::InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap, StaticGraphKernelRes* resolutionInfos, uint64_t kernelsResolutionHistoryGroupBitmap, StaticGraphKernelRes* resolutionHistories, StaticGraphKernelBppConfiguration* bppInfos, uint8_t* systemApisSizes, uint8_t* systemApiData)
{
    uint8_t* systemApiDataCurrentPtr = systemApiData;
    uint32_t currentResolutionHistoryIndex = 0;
    uint32_t currentRcbIndex = 0;

    for (uint32_t i = 0; i < nodeKernels.kernelCount; ++i)
    {
        auto& runKernel = nodeKernels.kernelList[i].run_kernel;
        runKernel.kernel_uuid = kernelsUuids[i];
        runKernel.stream_id = nodeKernels.streamId;
        runKernel.enable = 1;
        runKernel.output_count = 1;
        if (CHECK_BITMAP64_BIT(kernelsRcbBitmap, i))
        {
            // RCB
            runKernel.resolution_info = &resolutionInfos[currentRcbIndex];
            currentRcbIndex++;
        }
        else
        {
            runKernel.resolution_info = nullptr;
        }

        if (CHECK_BITMAP64_BIT(kernelsResolutionHistoryGroupBitmap, i))
        {
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

        if (systemApiDataCurrentPtr)
        {
            systemApiDataCurrentPtr = systemApiDataCurrentPtr + systemApiSize;
        }

        // Metadata
        runKernel.metadata[0] = 0;
        runKernel.metadata[1] = 0;
        runKernel.metadata[2] = 0;
        runKernel.metadata[3] = 0;

    }
}

void OuterNode::SetDisabledKernels(uint64_t disabledRunKernelsBitmap)
{
    for (uint32_t i = 0; i < nodeKernels.kernelCount; ++i)
    {
        // check the i'th bit in the bitmap
        if (CHECK_BITMAP64_BIT(disabledRunKernelsBitmap, i))
        {
            nodeKernels.kernelList[i].run_kernel.enable = 2; // disabled
        }
    }
}

StaticGraphStatus OuterNode::UpdateKernelsSelectedConfiguration(uint32_t selectedIndex)
{
    if (selectedIndex >= kernelConfigurationsOptionsCount)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    nodeKernels.kernelList = kernelListOptions[selectedIndex];
    selectedKernelConfigurationIndex = selectedIndex;
    return StaticGraphStatus::SG_OK;
}
uint8_t OuterNode::GetNumberOfFragments()
{
    return numberOfFragments;
}

void IsysOuterNode::Init(IsysOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(2, NodeTypes::Isys, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {11470 /*is_odr_a*/};
    uint64_t kernelsRcbBitmap = 0x1; // { is_odr_a[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{is_odr_a}[0] }

    uint8_t systemApisSizes[1] = {0 /*is_odr_a*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerOuterNode::Init(LbffBayerOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/};
    uint64_t kernelsRcbBitmap = 0x13FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsNoTnrOuterNode::Init(BbpsNoTnrOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(1, NodeTypes::Cb, 7, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[7] = {44984 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 22660 /*cas_1_0*/, 7175 /*ofs_mp_bodr_regs_1_1*/, 6800 /*outputscaler_2_0_a*/, 51856 /*outputscaler_2_0_b*/, 30277 /*ofs_dp_bodr_regs_1_1*/, 31882 /*ofs_pp_bodr_regs_1_1*/};
    uint64_t kernelsRcbBitmap = 0x7C; // { ofs_mp_bodr_regs_1_1[2], outputscaler_2_0_a[3], outputscaler_2_0_b[4], ofs_dp_bodr_regs_1_1[5], ofs_pp_bodr_regs_1_1[6] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x60; // {{slim_tnr_spatial_bifd_yuvn_regs_1_1, cas_1_0, ofs_mp_bodr_regs_1_1, outputscaler_2_0_a, outputscaler_2_0_b}[0], {ofs_dp_bodr_regs_1_1}[1], {ofs_pp_bodr_regs_1_1}[2] }

    uint8_t systemApisSizes[7] = {156 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 0 /*cas_1_0*/, 156 /*ofs_mp_bodr_regs_1_1*/, 0 /*outputscaler_2_0_a*/, 0 /*outputscaler_2_0_b*/, 156 /*ofs_dp_bodr_regs_1_1*/, 156 /*ofs_pp_bodr_regs_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerWithGmvOuterNode::Init(LbffBayerWithGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x113FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28], gmv_statistics_1_0[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7FEE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {ifd_gmv_1_1}[13], {gmv_statistics_1_0}[14], {odr_gmv_match_1_1}[15], {odr_gmv_feature_1_1}[16] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsWithTnrOuterNode::Init(BbpsWithTnrOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(1, NodeTypes::Cb, 20, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[20] = {11500 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1*/, 33179 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1*/, 6326 /*tnr_sp_bc_bifd_yuv4n_regs_1_1*/, 48987 /*tnr7_ims_1_1*/, 54840 /*tnr7_bc_1_1*/, 48743 /*tnr_sp_bc_bodr_rs4n_regs_1_1*/, 44984 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 3133 /*tnr7_spatial_1_0*/, 27830 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1*/, 44199 /*tnr_fp_blend_bifd_rs4n_regs_1_1*/, 32696 /*tnr7_blend_1_0*/, 39844 /*tnr_fp_bodr_yuvn_regs_1_1*/, 22660 /*cas_1_0*/, 60056 /*tnr_scale_fp*/, 7175 /*ofs_mp_bodr_regs_1_1*/, 6800 /*outputscaler_2_0_a*/, 51856 /*outputscaler_2_0_b*/, 30277 /*ofs_dp_bodr_regs_1_1*/, 31882 /*ofs_pp_bodr_regs_1_1*/, 57148 /*tnr_scale_fp_bodr_yuv4n_regs_1_1*/};
    uint64_t kernelsRcbBitmap = 0x7E000; // { tnr_scale_fp[13], ofs_mp_bodr_regs_1_1[14], outputscaler_2_0_a[15], outputscaler_2_0_b[16], ofs_dp_bodr_regs_1_1[17], ofs_pp_bodr_regs_1_1[18] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0xE074E; // {{slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1}[0], {slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1}[1], {tnr_sp_bc_bifd_yuv4n_regs_1_1}[2], {tnr7_ims_1_1, tnr7_bc_1_1, tnr_sp_bc_bodr_rs4n_regs_1_1}[3], {slim_tnr_spatial_bifd_yuvn_regs_1_1, tnr7_spatial_1_0}[4], {slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1}[5], {tnr_fp_blend_bifd_rs4n_regs_1_1}[6], {tnr7_blend_1_0, tnr_fp_bodr_yuvn_regs_1_1, cas_1_0, tnr_scale_fp, ofs_mp_bodr_regs_1_1, outputscaler_2_0_a, outputscaler_2_0_b}[7], {ofs_dp_bodr_regs_1_1}[8], {ofs_pp_bodr_regs_1_1}[9], {tnr_scale_fp_bodr_yuv4n_regs_1_1}[10] }

    uint8_t systemApisSizes[20] = {156 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1*/, 156 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1*/, 156 /*tnr_sp_bc_bifd_yuv4n_regs_1_1*/, 0 /*tnr7_ims_1_1*/, 0 /*tnr7_bc_1_1*/, 156 /*tnr_sp_bc_bodr_rs4n_regs_1_1*/, 156 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 0 /*tnr7_spatial_1_0*/, 156 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1*/, 156 /*tnr_fp_blend_bifd_rs4n_regs_1_1*/, 6 /*tnr7_blend_1_0*/, 156 /*tnr_fp_bodr_yuvn_regs_1_1*/, 0 /*cas_1_0*/, 0 /*tnr_scale_fp*/, 156 /*ofs_mp_bodr_regs_1_1*/, 0 /*outputscaler_2_0_a*/, 0 /*outputscaler_2_0_b*/, 156 /*ofs_dp_bodr_regs_1_1*/, 156 /*ofs_pp_bodr_regs_1_1*/, 156 /*tnr_scale_fp_bodr_yuv4n_regs_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwGdcOuterNode::Init(SwGdcOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(4, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {5637 /*gdc7_1*/};
    uint64_t kernelsRcbBitmap = 0x1; // { gdc7_1[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{gdc7_1}[0] }

    uint8_t systemApisSizes[1] = {0 /*gdc7_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwScalerOuterNode::Init(SwScalerOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(7, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {19706 /*sw_scaler*/};
    uint64_t kernelsRcbBitmap = 0x1; // { sw_scaler[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{sw_scaler}[0] }

    uint8_t systemApisSizes[1] = {5 /*sw_scaler*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwNntmOuterNode::Init(SwNntmOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(6, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {46539 /*nntm_1_0*/};
    uint64_t kernelsRcbBitmap = 0x0; // {  }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{nntm_1_0}[0] }

    uint8_t systemApisSizes[1] = {5 /*nntm_1_0*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, nullptr, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffRgbIrOuterNode::Init(LbffRgbIrOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 34, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[34] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 33714 /*gd_dpc_2_2*/, 15021 /*rgbs_grid_1_1*/, 14488 /*rgb_ir_2_0*/, 3371 /*odr_ir_1_1*/, 20731 /*odr_awb_std_1_1*/, 2452 /*odr_awb_sve_1_1*/, 54176 /*odr_awb_sat_1_1*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/};
    uint64_t kernelsRcbBitmap = 0xFF800071; // { ifd_pipe_1_1[0], rgbs_grid_1_1[4], rgb_ir_2_0[5], odr_ir_1_1[6], ccm_3a_2_0[23], fr_grid_1_0[24], b2i_ds_1_0_1[25], upscaler_1_0[26], lbff_crop_espa_1_1[27], tnr_scale_lb[28], odr_output_ps_1_1[29], odr_output_me_1_1[30], aestatistics_2_1[31] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x3DC000FC2; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0, gd_dpc_2_2, rgbs_grid_1_1, rgb_ir_2_0}[1], {odr_ir_1_1}[2], {odr_awb_std_1_1}[3], {odr_awb_sve_1_1}[4], {odr_awb_sat_1_1}[5], {ifd_lsc_1_1}[6], {lsc_1_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[7], {upscaler_1_0}[8], {lbff_crop_espa_1_1}[9], {tnr_scale_lb, odr_output_ps_1_1}[10], {odr_output_me_1_1}[11], {aestatistics_2_1}[12], {odr_ae_1_1}[13], {odr_af_std_1_1}[14] }

    uint8_t systemApisSizes[34] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 0 /*gd_dpc_2_2*/, 24 /*rgbs_grid_1_1*/, 0 /*rgb_ir_2_0*/, 156 /*odr_ir_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sve_1_1*/, 156 /*odr_awb_sat_1_1*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][13].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrNoGmvIrStreamOuterNode::Init(LbffIrNoGmvIrStreamOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/};
    uint64_t kernelsRcbBitmap = 0x13FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsIrWithTnrOuterNode::Init(BbpsIrWithTnrOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(1, NodeTypes::Cb, 20, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[20] = {11500 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1*/, 33179 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1*/, 6326 /*tnr_sp_bc_bifd_yuv4n_regs_1_1*/, 48987 /*tnr7_ims_1_1*/, 54840 /*tnr7_bc_1_1*/, 48743 /*tnr_sp_bc_bodr_rs4n_regs_1_1*/, 44984 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 3133 /*tnr7_spatial_1_0*/, 27830 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1*/, 44199 /*tnr_fp_blend_bifd_rs4n_regs_1_1*/, 32696 /*tnr7_blend_1_0*/, 39844 /*tnr_fp_bodr_yuvn_regs_1_1*/, 22660 /*cas_1_0*/, 60056 /*tnr_scale_fp*/, 7175 /*ofs_mp_bodr_regs_1_1*/, 6800 /*outputscaler_2_0_a*/, 51856 /*outputscaler_2_0_b*/, 30277 /*ofs_dp_bodr_regs_1_1*/, 31882 /*ofs_pp_bodr_regs_1_1*/, 57148 /*tnr_scale_fp_bodr_yuv4n_regs_1_1*/};
    uint64_t kernelsRcbBitmap = 0x7E000; // { tnr_scale_fp[13], ofs_mp_bodr_regs_1_1[14], outputscaler_2_0_a[15], outputscaler_2_0_b[16], ofs_dp_bodr_regs_1_1[17], ofs_pp_bodr_regs_1_1[18] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0xE074E; // {{slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1}[0], {slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1}[1], {tnr_sp_bc_bifd_yuv4n_regs_1_1}[2], {tnr7_ims_1_1, tnr7_bc_1_1, tnr_sp_bc_bodr_rs4n_regs_1_1}[3], {slim_tnr_spatial_bifd_yuvn_regs_1_1, tnr7_spatial_1_0}[4], {slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1}[5], {tnr_fp_blend_bifd_rs4n_regs_1_1}[6], {tnr7_blend_1_0, tnr_fp_bodr_yuvn_regs_1_1, cas_1_0, tnr_scale_fp, ofs_mp_bodr_regs_1_1, outputscaler_2_0_a, outputscaler_2_0_b}[7], {ofs_dp_bodr_regs_1_1}[8], {ofs_pp_bodr_regs_1_1}[9], {tnr_scale_fp_bodr_yuv4n_regs_1_1}[10] }

    uint8_t systemApisSizes[20] = {156 /*slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1*/, 156 /*slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1*/, 156 /*tnr_sp_bc_bifd_yuv4n_regs_1_1*/, 0 /*tnr7_ims_1_1*/, 0 /*tnr7_bc_1_1*/, 156 /*tnr_sp_bc_bodr_rs4n_regs_1_1*/, 156 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 0 /*tnr7_spatial_1_0*/, 156 /*slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1*/, 156 /*tnr_fp_blend_bifd_rs4n_regs_1_1*/, 6 /*tnr7_blend_1_0*/, 156 /*tnr_fp_bodr_yuvn_regs_1_1*/, 0 /*cas_1_0*/, 0 /*tnr_scale_fp*/, 156 /*ofs_mp_bodr_regs_1_1*/, 0 /*outputscaler_2_0_a*/, 0 /*outputscaler_2_0_b*/, 156 /*ofs_dp_bodr_regs_1_1*/, 156 /*ofs_pp_bodr_regs_1_1*/, 156 /*tnr_scale_fp_bodr_yuv4n_regs_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerBurstOutNo3AOuterNode::Init(LbffBayerBurstOutNo3AOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 57981 /*odr_burst_isp_1_1*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 3971 /*ifd_pdaf_1_1*/, 43213 /*pext_1_0*/, 44308 /*pafstatistics_1_2*/, 31724 /*odr_pdaf_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x137F0001; // { ifd_pipe_1_1[0], odr_burst_isp_1_1[16], b2i_ds_1_0_1[17], upscaler_1_0[18], lbff_crop_espa_1_1[19], tnr_scale_lb[20], odr_output_ps_1_1[21], odr_output_me_1_1[22], pext_1_0[24], pafstatistics_1_2[25], gmv_statistics_1_0[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EDC0002; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0, lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, odr_burst_isp_1_1, b2i_ds_1_0_1}[1], {upscaler_1_0}[2], {lbff_crop_espa_1_1}[3], {tnr_scale_lb, odr_output_ps_1_1}[4], {odr_output_me_1_1}[5], {ifd_pdaf_1_1, pext_1_0}[6], {pafstatistics_1_2}[7], {odr_pdaf_1_1}[8], {ifd_gmv_1_1}[9], {gmv_statistics_1_0}[10], {odr_gmv_match_1_1}[11], {odr_gmv_feature_1_1}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 156 /*odr_burst_isp_1_1*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*ifd_pdaf_1_1*/, 24 /*pext_1_0*/, 8 /*pafstatistics_1_2*/, 156 /*odr_pdaf_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][6].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void BbpsIrNoTnrOuterNode::Init(BbpsIrNoTnrOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(1, NodeTypes::Cb, 7, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[7] = {44984 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 22660 /*cas_1_0*/, 7175 /*ofs_mp_bodr_regs_1_1*/, 6800 /*outputscaler_2_0_a*/, 51856 /*outputscaler_2_0_b*/, 30277 /*ofs_dp_bodr_regs_1_1*/, 31882 /*ofs_pp_bodr_regs_1_1*/};
    uint64_t kernelsRcbBitmap = 0x7C; // { ofs_mp_bodr_regs_1_1[2], outputscaler_2_0_a[3], outputscaler_2_0_b[4], ofs_dp_bodr_regs_1_1[5], ofs_pp_bodr_regs_1_1[6] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x60; // {{slim_tnr_spatial_bifd_yuvn_regs_1_1, cas_1_0, ofs_mp_bodr_regs_1_1, outputscaler_2_0_a, outputscaler_2_0_b}[0], {ofs_dp_bodr_regs_1_1}[1], {ofs_pp_bodr_regs_1_1}[2] }

    uint8_t systemApisSizes[7] = {156 /*slim_tnr_spatial_bifd_yuvn_regs_1_1*/, 0 /*cas_1_0*/, 156 /*ofs_mp_bodr_regs_1_1*/, 0 /*outputscaler_2_0_a*/, 0 /*outputscaler_2_0_b*/, 156 /*ofs_dp_bodr_regs_1_1*/, 156 /*ofs_pp_bodr_regs_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrNo2ANoGmvOuterNode::Init(LbffIrNo2ANoGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 26, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[26] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/};
    uint64_t kernelsRcbBitmap = 0xFE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], b2i_ds_1_0_1[18], upscaler_1_0[19], lbff_crop_espa_1_1[20], tnr_scale_lb[21], odr_output_ps_1_1[22], odr_output_me_1_1[23] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x3B8001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9] }

    uint8_t systemApisSizes[26] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrNoGmvOuterNode::Init(LbffIrNoGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/};
    uint64_t kernelsRcbBitmap = 0x13FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void IsysPdaf2OuterNode::Init(IsysPdaf2OuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(2, NodeTypes::Isys, 2, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[2] = {11470 /*is_odr_a*/, 55449 /*is_odr_b*/};
    uint64_t kernelsRcbBitmap = 0x3; // { is_odr_a[0], is_odr_b[1] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x2; // {{is_odr_a}[0], {is_odr_b}[1] }

    uint8_t systemApisSizes[2] = {0 /*is_odr_a*/, 0 /*is_odr_b*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf2OuterNode::Init(LbffBayerPdaf2OuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 3971 /*ifd_pdaf_1_1*/, 43213 /*pext_1_0*/, 44308 /*pafstatistics_1_2*/, 31724 /*odr_pdaf_1_1*/};
    uint64_t kernelsRcbBitmap = 0x313FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28], pext_1_0[32], pafstatistics_1_2[33] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x6FEE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {ifd_pdaf_1_1, pext_1_0}[13], {pafstatistics_1_2}[14], {odr_pdaf_1_1}[15] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 156 /*ifd_pdaf_1_1*/, 24 /*pext_1_0*/, 8 /*pafstatistics_1_2*/, 156 /*odr_pdaf_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf3OuterNode::Init(LbffBayerPdaf3OuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 34, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[34] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 43213 /*pext_1_0*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 44308 /*pafstatistics_1_2*/, 31724 /*odr_pdaf_1_1*/};
    uint64_t kernelsRcbBitmap = 0x127FE0001; // { ifd_pipe_1_1[0], pext_1_0[17], rgbs_grid_1_1[18], ccm_3a_2_0[19], fr_grid_1_0[20], b2i_ds_1_0_1[21], upscaler_1_0[22], lbff_crop_espa_1_1[23], tnr_scale_lb[24], odr_output_ps_1_1[25], odr_output_me_1_1[26], aestatistics_2_1[29], pafstatistics_1_2[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x3FDC0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, pext_1_0, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {pafstatistics_1_2}[13], {odr_pdaf_1_1}[14] }

    uint8_t systemApisSizes[34] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*pext_1_0*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 8 /*pafstatistics_1_2*/, 156 /*odr_pdaf_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void IsysDolOuterNode::Init(IsysDolOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(2, NodeTypes::Isys, 2, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[2] = {11470 /*is_odr_a*/, 50407 /*is_odr_c*/};
    uint64_t kernelsRcbBitmap = 0x3; // { is_odr_a[0], is_odr_c[1] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x2; // {{is_odr_a}[0], {is_odr_c}[1] }

    uint8_t systemApisSizes[2] = {0 /*is_odr_a*/, 0 /*is_odr_c*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwDolOuterNode::Init(SwDolOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(3, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {3700 /*dol_lite_1_0*/};
    uint64_t kernelsRcbBitmap = 0x0; // {  }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{dol_lite_1_0}[0] }

    uint8_t systemApisSizes[1] = {0 /*dol_lite_1_0*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, nullptr, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffDolOuterNode::Init(LbffDolOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 31, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[31] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/};
    uint64_t kernelsRcbBitmap = 0x13FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12] }

    uint8_t systemApisSizes[31] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void SwGtmOuterNode::Init(SwGtmOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(5, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {37003 /*tm_app*/};
    uint64_t kernelsRcbBitmap = 0x0; // {  }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{tm_app}[0] }

    uint8_t systemApisSizes[1] = {0 /*tm_app*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, nullptr, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, nullptr);
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf2WithGmvOuterNode::Init(LbffBayerPdaf2WithGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 39, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[39] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 3971 /*ifd_pdaf_1_1*/, 43213 /*pext_1_0*/, 44308 /*pafstatistics_1_2*/, 31724 /*odr_pdaf_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x1313FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28], pext_1_0[32], pafstatistics_1_2[33], gmv_statistics_1_0[36] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7EFEE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {ifd_pdaf_1_1, pext_1_0}[13], {pafstatistics_1_2}[14], {odr_pdaf_1_1}[15], {ifd_gmv_1_1}[16], {gmv_statistics_1_0}[17], {odr_gmv_match_1_1}[18], {odr_gmv_feature_1_1}[19] }

    uint8_t systemApisSizes[39] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 156 /*ifd_pdaf_1_1*/, 24 /*pext_1_0*/, 8 /*pafstatistics_1_2*/, 156 /*odr_pdaf_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffBayerPdaf3WithGmvOuterNode::Init(LbffBayerPdaf3WithGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 38, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[38] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 43213 /*pext_1_0*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 44308 /*pafstatistics_1_2*/, 31724 /*odr_pdaf_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x927FE0001; // { ifd_pipe_1_1[0], pext_1_0[17], rgbs_grid_1_1[18], ccm_3a_2_0[19], fr_grid_1_0[20], b2i_ds_1_0_1[21], upscaler_1_0[22], lbff_crop_espa_1_1[23], tnr_scale_lb[24], odr_output_ps_1_1[25], odr_output_me_1_1[26], aestatistics_2_1[29], pafstatistics_1_2[32], gmv_statistics_1_0[35] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x3FFDC0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, pext_1_0, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {pafstatistics_1_2}[13], {odr_pdaf_1_1}[14], {ifd_gmv_1_1}[15], {gmv_statistics_1_0}[16], {odr_gmv_match_1_1}[17], {odr_gmv_feature_1_1}[18] }

    uint8_t systemApisSizes[38] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*pext_1_0*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 8 /*pafstatistics_1_2*/, 156 /*odr_pdaf_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffRgbIrWithGmvOuterNode::Init(LbffRgbIrWithGmvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 38, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[38] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 33714 /*gd_dpc_2_2*/, 15021 /*rgbs_grid_1_1*/, 14488 /*rgb_ir_2_0*/, 3371 /*odr_ir_1_1*/, 20731 /*odr_awb_std_1_1*/, 2452 /*odr_awb_sve_1_1*/, 54176 /*odr_awb_sat_1_1*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x8FF800071; // { ifd_pipe_1_1[0], rgbs_grid_1_1[4], rgb_ir_2_0[5], odr_ir_1_1[6], ccm_3a_2_0[23], fr_grid_1_0[24], b2i_ds_1_0_1[25], upscaler_1_0[26], lbff_crop_espa_1_1[27], tnr_scale_lb[28], odr_output_ps_1_1[29], odr_output_me_1_1[30], aestatistics_2_1[31], gmv_statistics_1_0[35] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x3FDC000FC2; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0, gd_dpc_2_2, rgbs_grid_1_1, rgb_ir_2_0}[1], {odr_ir_1_1}[2], {odr_awb_std_1_1}[3], {odr_awb_sve_1_1}[4], {odr_awb_sat_1_1}[5], {ifd_lsc_1_1}[6], {lsc_1_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[7], {upscaler_1_0}[8], {lbff_crop_espa_1_1}[9], {tnr_scale_lb, odr_output_ps_1_1}[10], {odr_output_me_1_1}[11], {aestatistics_2_1}[12], {odr_ae_1_1}[13], {odr_af_std_1_1}[14], {ifd_gmv_1_1}[15], {gmv_statistics_1_0}[16], {odr_gmv_match_1_1}[17], {odr_gmv_feature_1_1}[18] }

    uint8_t systemApisSizes[38] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 0 /*gd_dpc_2_2*/, 24 /*rgbs_grid_1_1*/, 0 /*rgb_ir_2_0*/, 156 /*odr_ir_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sve_1_1*/, 156 /*odr_awb_sat_1_1*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][13].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void LbffIrWithGmvIrStreamOuterNode::Init(LbffIrWithGmvIrStreamOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(0, NodeTypes::Cb, 35, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[35] = {52164 /*ifd_pipe_1_1*/, 11700 /*bxt_blc*/, 10326 /*linearization2_0*/, 6070 /*ifd_lsc_1_1*/, 2144 /*lsc_1_2*/, 33714 /*gd_dpc_2_2*/, 5144 /*wb_1_1*/, 21777 /*bnlm_3_3*/, 48695 /*bxt_demosaic*/, 13708 /*vcsc_2_0_b*/, 54721 /*gltm_2_0*/, 58858 /*xnr_5_2*/, 36035 /*vcr_3_1*/, 36029 /*glim_2_0*/, 13026 /*acm_1_1*/, 57496 /*gammatm_v3*/, 31704 /*bxt_csc*/, 15021 /*rgbs_grid_1_1*/, 62344 /*ccm_3a_2_0*/, 26958 /*fr_grid_1_0*/, 20739 /*b2i_ds_1_0_1*/, 25569 /*upscaler_1_0*/, 36213 /*lbff_crop_espa_1_1*/, 33723 /*tnr_scale_lb*/, 40915 /*odr_output_ps_1_1*/, 55391 /*odr_output_me_1_1*/, 20731 /*odr_awb_std_1_1*/, 54176 /*odr_awb_sat_1_1*/, 55073 /*aestatistics_2_1*/, 50677 /*odr_ae_1_1*/, 6500 /*odr_af_std_1_1*/, 41864 /*ifd_gmv_1_1*/, 61146 /*gmv_statistics_1_0*/, 13820 /*odr_gmv_match_1_1*/, 8985 /*odr_gmv_feature_1_1*/};
    uint64_t kernelsRcbBitmap = 0x113FE0001; // { ifd_pipe_1_1[0], rgbs_grid_1_1[17], ccm_3a_2_0[18], fr_grid_1_0[19], b2i_ds_1_0_1[20], upscaler_1_0[21], lbff_crop_espa_1_1[22], tnr_scale_lb[23], odr_output_ps_1_1[24], odr_output_me_1_1[25], aestatistics_2_1[28], gmv_statistics_1_0[32] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x7FEE0001A; // {{ifd_pipe_1_1}[0], {bxt_blc, linearization2_0}[1], {ifd_lsc_1_1}[2], {lsc_1_2, gd_dpc_2_2, wb_1_1, bnlm_3_3, bxt_demosaic, vcsc_2_0_b, gltm_2_0, xnr_5_2, vcr_3_1, glim_2_0, acm_1_1, gammatm_v3, bxt_csc, rgbs_grid_1_1, ccm_3a_2_0, fr_grid_1_0, b2i_ds_1_0_1}[3], {upscaler_1_0}[4], {lbff_crop_espa_1_1}[5], {tnr_scale_lb, odr_output_ps_1_1}[6], {odr_output_me_1_1}[7], {odr_awb_std_1_1}[8], {odr_awb_sat_1_1}[9], {aestatistics_2_1}[10], {odr_ae_1_1}[11], {odr_af_std_1_1}[12], {ifd_gmv_1_1}[13], {gmv_statistics_1_0}[14], {odr_gmv_match_1_1}[15], {odr_gmv_feature_1_1}[16] }

    uint8_t systemApisSizes[35] = {156 /*ifd_pipe_1_1*/, 5 /*bxt_blc*/, 5 /*linearization2_0*/, 156 /*ifd_lsc_1_1*/, 40 /*lsc_1_2*/, 0 /*gd_dpc_2_2*/, 0 /*wb_1_1*/, 5 /*bnlm_3_3*/, 0 /*bxt_demosaic*/, 0 /*vcsc_2_0_b*/, 0 /*gltm_2_0*/, 0 /*xnr_5_2*/, 0 /*vcr_3_1*/, 0 /*glim_2_0*/, 0 /*acm_1_1*/, 5 /*gammatm_v3*/, 0 /*bxt_csc*/, 24 /*rgbs_grid_1_1*/, 5 /*ccm_3a_2_0*/, 20 /*fr_grid_1_0*/, 0 /*b2i_ds_1_0_1*/, 0 /*upscaler_1_0*/, 156 /*lbff_crop_espa_1_1*/, 0 /*tnr_scale_lb*/, 156 /*odr_output_ps_1_1*/, 156 /*odr_output_me_1_1*/, 156 /*odr_awb_std_1_1*/, 156 /*odr_awb_sat_1_1*/, 24 /*aestatistics_2_1*/, 156 /*odr_ae_1_1*/, 156 /*odr_af_std_1_1*/, 156 /*ifd_gmv_1_1*/, 0 /*gmv_statistics_1_0*/, 156 /*odr_gmv_match_1_1*/, 156 /*odr_gmv_feature_1_1*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // Metadata update
    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        kernelListOptions[i][7].run_kernel.metadata[0] = 1; // bnlm_3_3
    }

    // set default inner Node
    setInnerNode(None);
}

void SwVaiOuterNode::Init(SwVaiOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(8, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {19706 /*sw_scaler*/};
    uint64_t kernelsRcbBitmap = 0x1; // { sw_scaler[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{sw_scaler}[0] }

    uint8_t systemApisSizes[1] = {5 /*sw_scaler*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

void SwImvOuterNode::Init(SwImvOuterNodeConfiguration** selectedGraphConfiguration, uint32_t nodeKernelConfigurationsOptionsCount)
{
    OuterNode::Init(9, NodeTypes::Sw, 1, nodeKernelConfigurationsOptionsCount, selectedGraphConfiguration[0]->tuningMode, selectedGraphConfiguration[0]->streamId, 0);

    uint16_t kernelsUuids[1] = {19706 /*sw_scaler*/};
    uint64_t kernelsRcbBitmap = 0x1; // { sw_scaler[0] }
    uint64_t kernelsResolutionHistoryGroupBitmap = 0x0; // {{sw_scaler}[0] }

    uint8_t systemApisSizes[1] = {5 /*sw_scaler*/};

    for (uint32_t i = 0; i < kernelConfigurationsOptionsCount; ++i)
    {
        nodeKernels.kernelList = kernelListOptions[i];

        InitRunKernels(kernelsUuids, kernelsRcbBitmap, selectedGraphConfiguration[i]->resolutionInfos, kernelsResolutionHistoryGroupBitmap,  selectedGraphConfiguration[i]->resolutionHistories, selectedGraphConfiguration[i]->bppInfos, systemApisSizes, selectedGraphConfiguration[i]->systemApiConfiguration);
    }

    // set default inner Node
    setInnerNode(None);
}

/*
 * Inner Nodes Setters
 */
void IsysOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffBayerOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 31; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void BbpsNoTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 7; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp | noPpp);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000060
        bitmaps.rbm[0] = 0x60;
        // DEB - 0x000000000000000000000000000F4040
        bitmaps.deb[0] = 0xF4040;
        // TEB - 0x000000000001820F
        bitmaps.teb[0] = 0x1820F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x4;
    }
    else if (nodeRelevantInnerOptions == (noDp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000050
        bitmaps.rbm[0] = 0x50;
        // DEB - 0x000000000000000000000000000CC040
        bitmaps.deb[0] = 0xCC040;
        // TEB - 0x000000000001420F
        bitmaps.teb[0] = 0x1420F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x28;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000040
        bitmaps.rbm[0] = 0x40;
        // DEB - 0x000000000000000000000000000C4040
        bitmaps.deb[0] = 0xC4040;
        // TEB - 0x000000000001020F
        bitmaps.teb[0] = 0x1020F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2C;
    }
    else if (nodeRelevantInnerOptions == (noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000030
        bitmaps.rbm[0] = 0x30;
        // DEB - 0x0000000000000000000000000003C040
        bitmaps.deb[0] = 0x3C040;
        // TEB - 0x000000000000C20F
        bitmaps.teb[0] = 0xC20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50;
    }
    else if (nodeRelevantInnerOptions == (noMp | noPpp))
    {
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
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x54;
    }
    else if (nodeRelevantInnerOptions == (noDp | noPpp))
    {
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
        // 4 outputscaler_2_0_b- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp | noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_spatial_bifd_yuvn_regs_1_1- inner node disablement
        // 1 cas_1_0- inner node disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F;
    }
    else // default inner node
    {
        // RBM - 0x00000000000000000000000000000070
        bitmaps.rbm[0] = 0x70;
        // DEB - 0x000000000000000000000000000FC040
        bitmaps.deb[0] = 0xFC040;
        // TEB - 0x000000000001C20F
        bitmaps.teb[0] = 0x1C20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffBayerWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 35; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE01F
        bitmaps.teb[0] = 0xCE01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA3DF
        bitmaps.teb[0] = 0xCA3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA01F
        bitmaps.teb[0] = 0xCA01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC3DF
        bitmaps.teb[0] = 0xCC3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC01F
        bitmaps.teb[0] = 0xCC01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C83DF
        bitmaps.teb[0] = 0xC83DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C801F
        bitmaps.teb[0] = 0xC801F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000003EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE3DF
        bitmaps.teb[0] = 0xCE3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void BbpsWithTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 20; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp | noPpp);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000006F
        bitmaps.rbm[0] = 0x6F;
        // DEB - 0x000000000000000000000000000F7FFF
        bitmaps.deb[0] = 0xF7FFF;
        // TEB - 0x000000000001BFEF
        bitmaps.teb[0] = 0x1BFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x4000;
    }
    else if (nodeRelevantInnerOptions == (noDp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000005F
        bitmaps.rbm[0] = 0x5F;
        // DEB - 0x000000000000000000000000000CFFFF
        bitmaps.deb[0] = 0xCFFFF;
        // TEB - 0x0000000000017FEF
        bitmaps.teb[0] = 0x17FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x28000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000004F
        bitmaps.rbm[0] = 0x4F;
        // DEB - 0x000000000000000000000000000C7FFF
        bitmaps.deb[0] = 0xC7FFF;
        // TEB - 0x0000000000013FEF
        bitmaps.teb[0] = 0x13FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2C000;
    }
    else if (nodeRelevantInnerOptions == (noPpp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000003F
        bitmaps.rbm[0] = 0x3F;
        // DEB - 0x0000000000000000000000000003FFFF
        bitmaps.deb[0] = 0x3FFFF;
        // TEB - 0x000000000000FFEF
        bitmaps.teb[0] = 0xFFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noPpp))
    {
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
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x54000;
    }
    else if (nodeRelevantInnerOptions == (noDp | noPpp))
    {
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
        // 16 outputscaler_2_0_b- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp | noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1- inner node disablement
        // 1 slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1- inner node disablement
        // 2 tnr_sp_bc_bifd_yuv4n_regs_1_1- inner node disablement
        // 3 tnr7_ims_1_1- inner node disablement
        // 4 tnr7_bc_1_1- inner node disablement
        // 5 tnr_sp_bc_bodr_rs4n_regs_1_1- inner node disablement
        // 6 slim_tnr_spatial_bifd_yuvn_regs_1_1- inner node disablement
        // 7 tnr7_spatial_1_0- inner node disablement
        // 8 slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1- inner node disablement
        // 9 tnr_fp_blend_bifd_rs4n_regs_1_1- inner node disablement
        // 10 tnr7_blend_1_0- inner node disablement
        // 11 tnr_fp_bodr_yuvn_regs_1_1- inner node disablement
        // 12 cas_1_0- inner node disablement
        // 13 tnr_scale_fp- inner node disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        // 19 tnr_scale_fp_bodr_yuv4n_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000000000000007F
        bitmaps.rbm[0] = 0x7F;
        // DEB - 0x000000000000000000000000000FFFFF
        bitmaps.deb[0] = 0xFFFFF;
        // TEB - 0x000000000001FFEF
        bitmaps.teb[0] = 0x1FFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void SwGdcOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void SwScalerOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void SwNntmOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffRgbIrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 34; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noIr | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000005EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x5EA;
        // DEB - 0x0000000000000000000001FFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000681F
        bitmaps.teb[0] = 0x681F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x381800390;
    }
    else if (nodeRelevantInnerOptions == (noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000163DF
        bitmaps.teb[0] = 0x163DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x40;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3818003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000056A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x56A;
        // DEB - 0x00000000000000000000017FFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x0000000000012BDF
        bitmaps.teb[0] = 0x12BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 29 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x20000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000056A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x56A;
        // DEB - 0x00000000000000000000017FFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000281F
        bitmaps.teb[0] = 0x281F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3A1800390;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000123DF
        bitmaps.teb[0] = 0x123DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x20000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3A18003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x4EA;
        // DEB - 0x0000000000000000000000BFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x0000000000014BDF
        bitmaps.teb[0] = 0x14BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x4EA;
        // DEB - 0x0000000000000000000000BFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000481F
        bitmaps.teb[0] = 0x481F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3D1800390;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000143DF
        bitmaps.teb[0] = 0x143DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3D18003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004003A01924F
        bitmaps.rbm[0] = 0x3A01924F;
        bitmaps.rbm[1] = 0x400;
        // DEB - 0x0000000000000000000000000073FE3F
        bitmaps.deb[0] = 0x73FE3F;
        // TEB - 0x0000000000010BDF
        bitmaps.teb[0] = 0x10BDF;
        // REB - 0x0000000000000000000000000203FFFF
        bitmaps.reb[0] = 0x203FFFF;

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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E7FF000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000040000899043
        bitmaps.rbm[0] = 0x899043;
        bitmaps.rbm[1] = 0x400;
        // DEB - 0x00000000000000000000000000700027
        bitmaps.deb[0] = 0x700027;
        // TEB - 0x000000000000080F
        bitmaps.teb[0] = 0x80F;
        // REB - 0x00000000000000000000000002003FCB
        bitmaps.reb[0] = 0x2003FCB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 10 ifd_lsc_1_1- inner node disablement
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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFF90;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000003A01924F
        bitmaps.rbm[0] = 0x3A01924F;
        // DEB - 0x0000000000000000000000000033FE3F
        bitmaps.deb[0] = 0x33FE3F;
        // TEB - 0x00000000000103DF
        bitmaps.teb[0] = 0x103DF;
        // REB - 0x0000000000000000000000000203FFFF
        bitmaps.reb[0] = 0x203FFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E7FF040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 gd_dpc_2_2- inner node disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 5 rgb_ir_2_0- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 10 ifd_lsc_1_1- inner node disablement
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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000005EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x5EA;
        // DEB - 0x0000000000000000000001FFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x0000000000016BDF
        bitmaps.teb[0] = 0x16BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffIrNoGmvIrStreamOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 31; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][1].run_kernel.enable = 0; // bxt_blc
        kernelListOptions[j][2].run_kernel.enable = 0; // linearization2_0
        kernelListOptions[j][5].run_kernel.enable = 0; // gd_dpc_2_2
        kernelListOptions[j][6].run_kernel.enable = 0; // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0; // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0; // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void BbpsIrWithTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 20; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp | noPpp);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000006F
        bitmaps.rbm[0] = 0x6F;
        // DEB - 0x000000000000000000000000000F7FFF
        bitmaps.deb[0] = 0xF7FFF;
        // TEB - 0x000000000001BFEF
        bitmaps.teb[0] = 0x1BFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x4000;
    }
    else if (nodeRelevantInnerOptions == (noDp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000005F
        bitmaps.rbm[0] = 0x5F;
        // DEB - 0x000000000000000000000000000CFFFF
        bitmaps.deb[0] = 0xCFFFF;
        // TEB - 0x0000000000017FEF
        bitmaps.teb[0] = 0x17FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x28000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000004F
        bitmaps.rbm[0] = 0x4F;
        // DEB - 0x000000000000000000000000000C7FFF
        bitmaps.deb[0] = 0xC7FFF;
        // TEB - 0x0000000000013FEF
        bitmaps.teb[0] = 0x13FEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2C000;
    }
    else if (nodeRelevantInnerOptions == (noPpp))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000000000003F
        bitmaps.rbm[0] = 0x3F;
        // DEB - 0x0000000000000000000000000003FFFF
        bitmaps.deb[0] = 0x3FFFF;
        // TEB - 0x000000000000FFEF
        bitmaps.teb[0] = 0xFFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;

        // Kernels disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noPpp))
    {
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
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x54000;
    }
    else if (nodeRelevantInnerOptions == (noDp | noPpp))
    {
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
        // 16 outputscaler_2_0_b- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78000;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp | noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_sp_bc_bifd_yuv4nm1_regs_1_1- inner node disablement
        // 1 slim_tnr_sp_bc_bifd_rs4nm1_regs_1_1- inner node disablement
        // 2 tnr_sp_bc_bifd_yuv4n_regs_1_1- inner node disablement
        // 3 tnr7_ims_1_1- inner node disablement
        // 4 tnr7_bc_1_1- inner node disablement
        // 5 tnr_sp_bc_bodr_rs4n_regs_1_1- inner node disablement
        // 6 slim_tnr_spatial_bifd_yuvn_regs_1_1- inner node disablement
        // 7 tnr7_spatial_1_0- inner node disablement
        // 8 slim_tnr_fp_blend_bifd_yuvnm1_regs_1_1- inner node disablement
        // 9 tnr_fp_blend_bifd_rs4n_regs_1_1- inner node disablement
        // 10 tnr7_blend_1_0- inner node disablement
        // 11 tnr_fp_bodr_yuvn_regs_1_1- inner node disablement
        // 12 cas_1_0- inner node disablement
        // 13 tnr_scale_fp- inner node disablement
        // 14 ofs_mp_bodr_regs_1_1- inner node disablement
        // 15 outputscaler_2_0_a- inner node disablement
        // 16 outputscaler_2_0_b- inner node disablement
        // 17 ofs_dp_bodr_regs_1_1- inner node disablement
        // 18 ofs_pp_bodr_regs_1_1- inner node disablement
        // 19 tnr_scale_fp_bodr_yuv4n_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000000000000007F
        bitmaps.rbm[0] = 0x7F;
        // DEB - 0x000000000000000000000000000FFFFF
        bitmaps.deb[0] = 0xFFFFF;
        // TEB - 0x000000000001FFEF
        bitmaps.teb[0] = 0x1FFEF;
        // REB - 0x0000000000000000000000000000000F
        bitmaps.reb[0] = 0xF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffBayerBurstOutNo3AOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 31; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noBurstCapture | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noBurstCapture))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000BEA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xBEA;
        // DEB - 0x000000000000000000001FFFFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE42F
        bitmaps.teb[0] = 0xCE42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x10000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xB6E;
        // DEB - 0x000000000000000000001F7FFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CB42F
        bitmaps.teb[0] = 0xCB42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 21 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x200000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xB6A;
        // DEB - 0x000000000000000000001F7FFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA42F
        bitmaps.teb[0] = 0xCA42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x210000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xAEE;
        // DEB - 0x000000000000000000001EBFFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CD42F
        bitmaps.teb[0] = 0xCD42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x500000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xAEA;
        // DEB - 0x000000000000000000001EBFFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC42F
        bitmaps.teb[0] = 0xCC42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x510000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xA6E;
        // DEB - 0x000000000000000000001E3FFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C942F
        bitmaps.teb[0] = 0xC942F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x700000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xA6A;
        // DEB - 0x000000000000000000001E3FFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C842F
        bitmaps.teb[0] = 0xC842F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x710000;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EE;
        // DEB - 0x0000000000000000000001FFFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000742F
        bitmaps.teb[0] = 0x742F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78000000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EA;
        // DEB - 0x0000000000000000000001FFFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000642F
        bitmaps.teb[0] = 0x642F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78010000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96E;
        // DEB - 0x00000000000000000000017FFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000342F
        bitmaps.teb[0] = 0x342F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78200000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96A;
        // DEB - 0x00000000000000000000017FFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000242F
        bitmaps.teb[0] = 0x242F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78210000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EE;
        // DEB - 0x0000000000000000000000BFFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000542F
        bitmaps.teb[0] = 0x542F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78500000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EA;
        // DEB - 0x0000000000000000000000BFFE8801F7
        bitmaps.deb[0] = 0xFE8801F7;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000442F
        bitmaps.teb[0] = 0x442F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78510000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080401404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x804;
        // DEB - 0x000000000000000000000000018801F7
        bitmaps.deb[0] = 0x18801F7;
        // TEB - 0x000000000000142F
        bitmaps.teb[0] = 0x142F;
        // REB - 0x000000000000000000000000040C30FB
        bitmaps.reb[0] = 0x40C30FB;

        // Kernels disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v3- inner node disablement
        // 15 bxt_csc- inner node disablement
        // 17 b2i_ds_1_0_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787EFFC0;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080000000000
        bitmaps.rbm[1] = 0x800;
        // DEB - 0x000000000000000000000000000801C0
        bitmaps.deb[0] = 0x801C0;
        // TEB - 0x0000000000000427
        bitmaps.teb[0] = 0x427;
        // REB - 0x00000000000000000000000004040000
        bitmaps.reb[0] = 0x4040000;

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
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
        // 14 gammatm_v3- inner node disablement
        // 15 bxt_csc- inner node disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 17 b2i_ds_1_0_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787FFFFF;
    }
    else if (nodeRelevantInnerOptions == (noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EE;
        // DEB - 0x000000000000000000001FFFFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CF00F
        bitmaps.teb[0] = 0xCF00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7800000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE00F
        bitmaps.teb[0] = 0xCE00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7810000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36E;
        // DEB - 0x000000000000000000001F7FFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CB00F
        bitmaps.teb[0] = 0xCB00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7A00000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA00F
        bitmaps.teb[0] = 0xCA00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7A10000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EE;
        // DEB - 0x000000000000000000001EBFFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CD00F
        bitmaps.teb[0] = 0xCD00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D00000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC00F
        bitmaps.teb[0] = 0xCC00F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D10000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26E;
        // DEB - 0x000000000000000000001E3FFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C900F
        bitmaps.teb[0] = 0xC900F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F00000;
    }
    else if (nodeRelevantInnerOptions == (noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C800F
        bitmaps.teb[0] = 0xC800F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F10000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EE;
        // DEB - 0x0000000000000000000001FFFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000700F
        bitmaps.teb[0] = 0x700F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F800000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000600F
        bitmaps.teb[0] = 0x600F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F810000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016E01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16E;
        // DEB - 0x00000000000000000000017FFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000300F
        bitmaps.teb[0] = 0x300F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FA00000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000200F
        bitmaps.teb[0] = 0x200F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FA10000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEE;
        // DEB - 0x0000000000000000000000BFFF800037
        bitmaps.deb[0] = 0xFF800037;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000500F
        bitmaps.teb[0] = 0x500F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD00000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE800037
        bitmaps.deb[0] = 0xFE800037;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000400F
        bitmaps.teb[0] = 0x400F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD10000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000401404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x4;
        // DEB - 0x00000000000000000000000001800037
        bitmaps.deb[0] = 0x1800037;
        // TEB - 0x000000000000100F
        bitmaps.teb[0] = 0x100F;
        // REB - 0x000000000000000000000000000830FB
        bitmaps.reb[0] = 0x830FB;

        // Kernels disablement
        // 6 bnlm_3_3- inner node disablement
        // 7 bxt_demosaic- inner node disablement
        // 8 vcsc_2_0_b- inner node disablement
        // 9 gltm_2_0- inner node disablement
        // 10 xnr_5_2- inner node disablement
        // 11 vcr_3_1- inner node disablement
        // 12 glim_2_0- inner node disablement
        // 13 acm_1_1- inner node disablement
        // 14 gammatm_v3- inner node disablement
        // 15 bxt_csc- inner node disablement
        // 17 b2i_ds_1_0_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFEFFC0;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noBurstCapture | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
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
        // 14 gammatm_v3- inner node disablement
        // 15 bxt_csc- inner node disablement
        // 16 odr_burst_isp_1_1- inner node disablement
        // 17 b2i_ds_1_0_1- inner node disablement
        // 18 upscaler_1_0- inner node disablement
        // 19 lbff_crop_espa_1_1- inner node disablement
        // 20 tnr_scale_lb- inner node disablement
        // 21 odr_output_ps_1_1- inner node disablement
        // 22 odr_output_me_1_1- inner node disablement
        // 23 ifd_pdaf_1_1- inner node disablement
        // 24 pext_1_0- inner node disablement
        // 25 pafstatistics_1_2- inner node disablement
        // 26 odr_pdaf_1_1- inner node disablement
        // 27 ifd_gmv_1_1- inner node disablement
        // 28 gmv_statistics_1_0- inner node disablement
        // 29 odr_gmv_match_1_1- inner node disablement
        // 30 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x000000000000000000000BEE01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xBEE;
        // DEB - 0x000000000000000000001FFFFF8801F7
        bitmaps.deb[0] = 0xFF8801F7;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CF42F
        bitmaps.teb[0] = 0xCF42F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void BbpsIrNoTnrOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 7; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (noMp | noDp | noPpp);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (noMp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000060
        bitmaps.rbm[0] = 0x60;
        // DEB - 0x000000000000000000000000000F4040
        bitmaps.deb[0] = 0xF4040;
        // TEB - 0x000000000001820F
        bitmaps.teb[0] = 0x1820F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x4;
    }
    else if (nodeRelevantInnerOptions == (noDp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000050
        bitmaps.rbm[0] = 0x50;
        // DEB - 0x000000000000000000000000000CC040
        bitmaps.deb[0] = 0xCC040;
        // TEB - 0x000000000001420F
        bitmaps.teb[0] = 0x1420F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x28;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000040
        bitmaps.rbm[0] = 0x40;
        // DEB - 0x000000000000000000000000000C4040
        bitmaps.deb[0] = 0xC4040;
        // TEB - 0x000000000001020F
        bitmaps.teb[0] = 0x1020F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2C;
    }
    else if (nodeRelevantInnerOptions == (noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000030
        bitmaps.rbm[0] = 0x30;
        // DEB - 0x0000000000000000000000000003C040
        bitmaps.deb[0] = 0x3C040;
        // TEB - 0x000000000000C20F
        bitmaps.teb[0] = 0xC20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;

        // Kernels disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50;
    }
    else if (nodeRelevantInnerOptions == (noMp | noPpp))
    {
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
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x54;
    }
    else if (nodeRelevantInnerOptions == (noDp | noPpp))
    {
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
        // 4 outputscaler_2_0_b- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x78;
    }
    else if (nodeRelevantInnerOptions == (noMp | noDp | noPpp))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 slim_tnr_spatial_bifd_yuvn_regs_1_1- inner node disablement
        // 1 cas_1_0- inner node disablement
        // 2 ofs_mp_bodr_regs_1_1- inner node disablement
        // 3 outputscaler_2_0_a- inner node disablement
        // 4 outputscaler_2_0_b- inner node disablement
        // 5 ofs_dp_bodr_regs_1_1- inner node disablement
        // 6 ofs_pp_bodr_regs_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F;
    }
    else // default inner node
    {
        // RBM - 0x00000000000000000000000000000070
        bitmaps.rbm[0] = 0x70;
        // DEB - 0x000000000000000000000000000FC040
        bitmaps.deb[0] = 0xFC040;
        // TEB - 0x000000000001C20F
        bitmaps.teb[0] = 0x1C20F;
        // REB - 0x0000000000000000000000000000000D
        bitmaps.reb[0] = 0xD;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffIrNo2ANoGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 26; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][6].run_kernel.enable = 0; // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0; // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0; // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 24 odr_awb_std_1_1- inner node disablement
        // 25 odr_awb_sat_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3020000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A07404403
        bitmaps.rbm[0] = 0x7404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80623F
        bitmaps.deb[0] = 0xFE80623F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000231F
        bitmaps.teb[0] = 0x231F;
        // REB - 0x00000000000000000000000001D8F0FB
        bitmaps.reb[0] = 0x1D8F0FB;

        // Kernels disablement
        // 22 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x400000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 22 odr_output_ps_1_1- inner node disablement
        // 24 odr_awb_std_1_1- inner node disablement
        // 25 odr_awb_sat_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3420000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA07404403
        bitmaps.rbm[0] = 0x7404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80623F
        bitmaps.deb[0] = 0xFE80623F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000431F
        bitmaps.teb[0] = 0x431F;
        // REB - 0x00000000000000000000000001D8F0FB
        bitmaps.reb[0] = 0x1D8F0FB;

        // Kernels disablement
        // 21 tnr_scale_lb- inner node disablement
        // 23 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xA00000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 21 tnr_scale_lb- inner node disablement
        // 23 odr_output_me_1_1- inner node disablement
        // 24 odr_awb_std_1_1- inner node disablement
        // 25 odr_awb_sat_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3A20000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000006404403
        bitmaps.rbm[0] = 0x6404403;
        // DEB - 0x0000000000000000000000000000623F
        bitmaps.deb[0] = 0x623F;
        // TEB - 0x000000000000031F
        bitmaps.teb[0] = 0x31F;
        // REB - 0x0000000000000000000000000000F0FB
        bitmaps.reb[0] = 0xF0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 18 b2i_ds_1_0_1- inner node disablement
        // 19 upscaler_1_0- inner node disablement
        // 20 lbff_crop_espa_1_1- inner node disablement
        // 21 tnr_scale_lb- inner node disablement
        // 22 odr_output_ps_1_1- inner node disablement
        // 23 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFDFFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 b2i_ds_1_0_1- inner node disablement
        // 19 upscaler_1_0- inner node disablement
        // 20 lbff_crop_espa_1_1- inner node disablement
        // 21 tnr_scale_lb- inner node disablement
        // 22 odr_output_ps_1_1- inner node disablement
        // 23 odr_output_me_1_1- inner node disablement
        // 24 odr_awb_std_1_1- inner node disablement
        // 25 odr_awb_sat_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000001EA07404403
        bitmaps.rbm[0] = 0x7404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80623F
        bitmaps.deb[0] = 0xFE80623F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000631F
        bitmaps.teb[0] = 0x631F;
        // REB - 0x00000000000000000000000001D8F0FB
        bitmaps.reb[0] = 0x1D8F0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffIrNoGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 31; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][6].run_kernel.enable = 0; // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0; // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0; // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void IsysPdaf2OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffBayerPdaf2OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 35; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noPdaf);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EA;
        // DEB - 0x0000000000000000000001FFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000643F
        bitmaps.teb[0] = 0x643F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x96A;
        // DEB - 0x00000000000000000000017FFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000027FF
        bitmaps.teb[0] = 0x27FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96A;
        // DEB - 0x00000000000000000000017FFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000243F
        bitmaps.teb[0] = 0x243F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x8EA;
        // DEB - 0x0000000000000000000000BFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000047FF
        bitmaps.teb[0] = 0x47FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EA;
        // DEB - 0x0000000000000000000000BFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000443F
        bitmaps.teb[0] = 0x443F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008001E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x800;
        // DEB - 0x000000000000000000000000000B7FFF
        bitmaps.deb[0] = 0xB7FFF;
        // TEB - 0x00000000000007FF
        bitmaps.teb[0] = 0x7FF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080000000000
        bitmaps.rbm[1] = 0x800;
        // DEB - 0x000000000000000000000000000801C0
        bitmaps.deb[0] = 0x801C0;
        // TEB - 0x0000000000000427
        bitmaps.teb[0] = 0x427;
        // REB - 0x00000000000000000000000004040000
        bitmaps.reb[0] = 0x4040000;

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else if (nodeRelevantInnerOptions == (noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000009EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x9EA;
        // DEB - 0x0000000000000000000001FFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000067FF
        bitmaps.teb[0] = 0x67FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffBayerPdaf3OuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 34; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noPdaf);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EB;
        // DEB - 0x0000000000000000000001FFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000641F
        bitmaps.teb[0] = 0x641F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xF81C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x96B;
        // DEB - 0x00000000000000000000017FFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000027DF
        bitmaps.teb[0] = 0x27DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 25 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96B;
        // DEB - 0x00000000000000000000017FFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000241F
        bitmaps.teb[0] = 0x241F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFA1C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x8EB;
        // DEB - 0x0000000000000000000000BFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000047DF
        bitmaps.teb[0] = 0x47DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x5000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EB;
        // DEB - 0x0000000000000000000000BFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000441F
        bitmaps.teb[0] = 0x441F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFD1C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008011E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x801;
        // DEB - 0x000000000000000000000000000B7FBF
        bitmaps.deb[0] = 0xB7FBF;
        // TEB - 0x00000000000007DF
        bitmaps.teb[0] = 0x7DF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080100404403
        bitmaps.rbm[0] = 0x404403;
        bitmaps.rbm[1] = 0x801;
        // DEB - 0x000000000000000000000000000801BF
        bitmaps.deb[0] = 0x801BF;
        // TEB - 0x000000000000041F
        bitmaps.teb[0] = 0x41F;
        // REB - 0x000000000000000000000000040430FB
        bitmaps.reb[0] = 0x40430FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFFFDFFC0;
    }
    else if (nodeRelevantInnerOptions == (noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EB;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x300020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EB;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F81E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16B;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x302020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16B;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEB;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x305020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEB;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000011E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x1;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x307E3FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000009EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x9EB;
        // DEB - 0x0000000000000000000001FFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000067DF
        bitmaps.teb[0] = 0x67DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void IsysDolOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void SwDolOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffDolOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 31; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][6].run_kernel.enable = 0; // wb_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void SwGtmOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void LbffBayerPdaf2WithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 39; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000BEA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xBEA;
        // DEB - 0x000000000000000000001FFFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE43F
        bitmaps.teb[0] = 0xCE43F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xB6A;
        // DEB - 0x000000000000000000001F7FFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA7FF
        bitmaps.teb[0] = 0xCA7FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xB6A;
        // DEB - 0x000000000000000000001F7FFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA43F
        bitmaps.teb[0] = 0xCA43F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xAEA;
        // DEB - 0x000000000000000000001EBFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC7FF
        bitmaps.teb[0] = 0xCC7FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xAEA;
        // DEB - 0x000000000000000000001EBFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC43F
        bitmaps.teb[0] = 0xCC43F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xA6A;
        // DEB - 0x000000000000000000001E3FFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C87FF
        bitmaps.teb[0] = 0xC87FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xA6A;
        // DEB - 0x000000000000000000001E3FFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C843F
        bitmaps.teb[0] = 0xC843F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x9EA;
        // DEB - 0x0000000000000000000001FFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000067FF
        bitmaps.teb[0] = 0x67FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7800000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EA;
        // DEB - 0x0000000000000000000001FFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000643F
        bitmaps.teb[0] = 0x643F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x96A;
        // DEB - 0x00000000000000000000017FFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000027FF
        bitmaps.teb[0] = 0x27FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7801000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96A;
        // DEB - 0x00000000000000000000017FFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000243F
        bitmaps.teb[0] = 0x243F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x8EA;
        // DEB - 0x0000000000000000000000BFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000047FF
        bitmaps.teb[0] = 0x47FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7802800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EA;
        // DEB - 0x0000000000000000000000BFFE8801FF
        bitmaps.deb[0] = 0xFE8801FF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000443F
        bitmaps.teb[0] = 0x443F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008001E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x800;
        // DEB - 0x000000000000000000000000000B7FFF
        bitmaps.deb[0] = 0xB7FFF;
        // TEB - 0x00000000000007FF
        bitmaps.teb[0] = 0x7FF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7803F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080000000000
        bitmaps.rbm[1] = 0x800;
        // DEB - 0x000000000000000000000000000801C0
        bitmaps.deb[0] = 0x801C0;
        // TEB - 0x0000000000000427
        bitmaps.teb[0] = 0x427;
        // REB - 0x00000000000000000000000004040000
        bitmaps.reb[0] = 0x4040000;

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x787FFFFFFF;
    }
    else if (nodeRelevantInnerOptions == (noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE3DF
        bitmaps.teb[0] = 0xCE3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE01F
        bitmaps.teb[0] = 0xCE01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA3DF
        bitmaps.teb[0] = 0xCA3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA01F
        bitmaps.teb[0] = 0xCA01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC3DF
        bitmaps.teb[0] = 0xCC3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC01F
        bitmaps.teb[0] = 0xCC01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C83DF
        bitmaps.teb[0] = 0xC83DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x783800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C801F
        bitmaps.teb[0] = 0xC801F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FF8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F80000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFC0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F81000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFD0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F82800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFE8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F83F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_pdaf_1_1- inner node disablement
        // 32 pext_1_0- inner node disablement
        // 33 pafstatistics_1_2- inner node disablement
        // 34 odr_pdaf_1_1- inner node disablement
        // 35 ifd_gmv_1_1- inner node disablement
        // 36 gmv_statistics_1_0- inner node disablement
        // 37 odr_gmv_match_1_1- inner node disablement
        // 38 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x000000000000000000000BEA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xBEA;
        // DEB - 0x000000000000000000001FFFFE8B7FFF
        bitmaps.deb[0] = 0xFE8B7FFF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE7FF
        bitmaps.teb[0] = 0xCE7FF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffBayerPdaf3WithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 38; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv | noPdaf);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000BEB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xBEB;
        // DEB - 0x000000000000000000001FFFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE41F
        bitmaps.teb[0] = 0xCE41F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xF81C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xB6B;
        // DEB - 0x000000000000000000001F7FFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA7DF
        bitmaps.teb[0] = 0xCA7DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 25 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000B6B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xB6B;
        // DEB - 0x000000000000000000001F7FFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA41F
        bitmaps.teb[0] = 0xCA41F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFA1C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xAEB;
        // DEB - 0x000000000000000000001EBFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC7DF
        bitmaps.teb[0] = 0xCC7DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x5000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000AEB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xAEB;
        // DEB - 0x000000000000000000001EBFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC41F
        bitmaps.teb[0] = 0xCC41F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFD1C0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xA6B;
        // DEB - 0x000000000000000000001E3FFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C87DF
        bitmaps.teb[0] = 0xC87DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x000000000000000000000A6B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xA6B;
        // DEB - 0x000000000000000000001E3FFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C841F
        bitmaps.teb[0] = 0xC841F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0xFF1C0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x9EB;
        // DEB - 0x0000000000000000000001FFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000067DF
        bitmaps.teb[0] = 0x67DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000009EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x9EB;
        // DEB - 0x0000000000000000000001FFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000641F
        bitmaps.teb[0] = 0x641F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3CF81C0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x96B;
        // DEB - 0x00000000000000000000017FFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000027DF
        bitmaps.teb[0] = 0x27DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C02000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000096B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x96B;
        // DEB - 0x00000000000000000000017FFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000241F
        bitmaps.teb[0] = 0x241F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3CFA1C0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x8EB;
        // DEB - 0x0000000000000000000000BFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000047DF
        bitmaps.teb[0] = 0x47DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C05000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x8EB;
        // DEB - 0x0000000000000000000000BFFE8801BF
        bitmaps.deb[0] = 0xFE8801BF;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000441F
        bitmaps.teb[0] = 0x441F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3CFD1C0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000008011E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x801;
        // DEB - 0x000000000000000000000000000B7FBF
        bitmaps.deb[0] = 0xB7FBF;
        // TEB - 0x00000000000007DF
        bitmaps.teb[0] = 0x7DF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C07E1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000080100404403
        bitmaps.rbm[0] = 0x404403;
        bitmaps.rbm[1] = 0x801;
        // DEB - 0x000000000000000000000000000801BF
        bitmaps.deb[0] = 0x801BF;
        // TEB - 0x000000000000041F
        bitmaps.teb[0] = 0x41F;
        // REB - 0x000000000000000000000000040430FB
        bitmaps.reb[0] = 0x40430FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3CFFFDFFC0;
    }
    else if (nodeRelevantInnerOptions == (noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x3EB;
        // DEB - 0x000000000000000000001FFFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE3DF
        bitmaps.teb[0] = 0xCE3DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x300020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EB;
        // DEB - 0x000000000000000000001FFFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE01F
        bitmaps.teb[0] = 0xCE01F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F81E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x36B;
        // DEB - 0x000000000000000000001F7FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA3DF
        bitmaps.teb[0] = 0xCA3DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x302020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36B;
        // DEB - 0x000000000000000000001F7FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA01F
        bitmaps.teb[0] = 0xCA01F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x2EB;
        // DEB - 0x000000000000000000001EBFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC3DF
        bitmaps.teb[0] = 0xCC3DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x305020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EB;
        // DEB - 0x000000000000000000001EBFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC01F
        bitmaps.teb[0] = 0xCC01F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x26B;
        // DEB - 0x000000000000000000001E3FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C83DF
        bitmaps.teb[0] = 0xC83DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x307020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26B;
        // DEB - 0x000000000000000000001E3FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C801F
        bitmaps.teb[0] = 0xC801F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FF1E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EB;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F00020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EB;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FF81E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016B1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16B;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F02020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016B01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16B;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFA1E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEB;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F05020000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EB01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEB;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000005DC30FB
        bitmaps.reb[0] = 0x5DC30FB;

        // Kernels disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFD1E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000011E404403
        bitmaps.rbm[0] = 0x1E404403;
        bitmaps.rbm[1] = 0x1;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000407F0FB
        bitmaps.reb[0] = 0x407F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F07E3FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe | noPdaf))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 pext_1_0- inner node disablement
        // 18 rgbs_grid_1_1- inner node disablement
        // 19 ccm_3a_2_0- inner node disablement
        // 20 fr_grid_1_0- inner node disablement
        // 21 b2i_ds_1_0_1- inner node disablement
        // 22 upscaler_1_0- inner node disablement
        // 23 lbff_crop_espa_1_1- inner node disablement
        // 24 tnr_scale_lb- inner node disablement
        // 25 odr_output_ps_1_1- inner node disablement
        // 26 odr_output_me_1_1- inner node disablement
        // 27 odr_awb_std_1_1- inner node disablement
        // 28 odr_awb_sat_1_1- inner node disablement
        // 29 aestatistics_2_1- inner node disablement
        // 30 odr_ae_1_1- inner node disablement
        // 31 odr_af_std_1_1- inner node disablement
        // 32 pafstatistics_1_2- inner node disablement
        // 33 odr_pdaf_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x000000000000000000000BEB1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xBEB;
        // DEB - 0x000000000000000000001FFFFE8B7FBF
        bitmaps.deb[0] = 0xFE8B7FBF;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE7DF
        bitmaps.teb[0] = 0xCE7DF;
        // REB - 0x00000000000000000000000005DFF0FB
        bitmaps.reb[0] = 0x5DFF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffRgbIrWithGmvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 38; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noIr | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000007EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x7EA;
        // DEB - 0x000000000000000000001FFFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE81F
        bitmaps.teb[0] = 0xCE81F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x381800390;
    }
    else if (nodeRelevantInnerOptions == (noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000DE3DF
        bitmaps.teb[0] = 0xDE3DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x40;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE01F
        bitmaps.teb[0] = 0xCE01F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3818003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000076A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x76A;
        // DEB - 0x000000000000000000001F7FFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000DABDF
        bitmaps.teb[0] = 0xDABDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 29 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x20000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000076A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x76A;
        // DEB - 0x000000000000000000001F7FFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA81F
        bitmaps.teb[0] = 0xCA81F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3A1800390;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000DA3DF
        bitmaps.teb[0] = 0xDA3DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x20000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA01F
        bitmaps.teb[0] = 0xCA01F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3A18003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000006EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x6EA;
        // DEB - 0x000000000000000000001EBFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000DCBDF
        bitmaps.teb[0] = 0xDCBDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000006EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x6EA;
        // DEB - 0x000000000000000000001EBFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC81F
        bitmaps.teb[0] = 0xCC81F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3D1800390;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000DC3DF
        bitmaps.teb[0] = 0xDC3DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x50000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC01F
        bitmaps.teb[0] = 0xCC01F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3D18003D0;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000066A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x66A;
        // DEB - 0x000000000000000000001E3FFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000D8BDF
        bitmaps.teb[0] = 0xD8BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x70000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000066A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x66A;
        // DEB - 0x000000000000000000001E3FFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C881F
        bitmaps.teb[0] = 0xC881F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F1800390;
    }
    else if (nodeRelevantInnerOptions == (noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000D83DF
        bitmaps.teb[0] = 0xD83DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x70000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C801F
        bitmaps.teb[0] = 0xC801F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F18003D0;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000005EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x5EA;
        // DEB - 0x0000000000000000000001FFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x0000000000016BDF
        bitmaps.teb[0] = 0x16BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000005EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x5EA;
        // DEB - 0x0000000000000000000001FFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000681F
        bitmaps.teb[0] = 0x681F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F81800390;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000163DF
        bitmaps.teb[0] = 0x163DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C00000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3F818003D0;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000056A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x56A;
        // DEB - 0x00000000000000000000017FFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x0000000000012BDF
        bitmaps.teb[0] = 0x12BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C20000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000056A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x56A;
        // DEB - 0x00000000000000000000017FFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000281F
        bitmaps.teb[0] = 0x281F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FA1800390;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000123DF
        bitmaps.teb[0] = 0x123DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C20000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FA18003D0;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x4EA;
        // DEB - 0x0000000000000000000000BFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x0000000000014BDF
        bitmaps.teb[0] = 0x14BDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C50000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0x4EA;
        // DEB - 0x0000000000000000000000BFFEF0003F
        bitmaps.deb[0] = 0xFEF0003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000481F
        bitmaps.teb[0] = 0x481F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FD1800390;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFEB3FE3F
        bitmaps.deb[0] = 0xFEB3FE3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000143DF
        bitmaps.teb[0] = 0x143DF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C50000040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01019243
        bitmaps.rbm[0] = 0x1019243;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFEB0003F
        bitmaps.deb[0] = 0xFEB0003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000003D83FFB
        bitmaps.reb[0] = 0x3D83FFB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FD18003D0;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000004003A01924F
        bitmaps.rbm[0] = 0x3A01924F;
        bitmaps.rbm[1] = 0x400;
        // DEB - 0x0000000000000000000000000073FE3F
        bitmaps.deb[0] = 0x73FE3F;
        // TEB - 0x0000000000010BDF
        bitmaps.teb[0] = 0x10BDF;
        // REB - 0x0000000000000000000000000203FFFF
        bitmaps.reb[0] = 0x203FFFF;

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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C7E7FF000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000040000899043
        bitmaps.rbm[0] = 0x899043;
        bitmaps.rbm[1] = 0x400;
        // DEB - 0x00000000000000000000000000700027
        bitmaps.deb[0] = 0x700027;
        // TEB - 0x000000000000080F
        bitmaps.teb[0] = 0x80F;
        // REB - 0x00000000000000000000000002003FCB
        bitmaps.reb[0] = 0x2003FCB;

        // Kernels disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 10 ifd_lsc_1_1- inner node disablement
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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFF90;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000003A01924F
        bitmaps.rbm[0] = 0x3A01924F;
        // DEB - 0x0000000000000000000000000033FE3F
        bitmaps.deb[0] = 0x33FE3F;
        // TEB - 0x00000000000103DF
        bitmaps.teb[0] = 0x103DF;
        // REB - 0x0000000000000000000000000203FFFF
        bitmaps.reb[0] = 0x203FFFF;

        // Kernels disablement
        // 6 odr_ir_1_1- inner node disablement
        // 12 wb_1_1- inner node disablement
        // 13 bnlm_3_3- inner node disablement
        // 14 bxt_demosaic- inner node disablement
        // 15 vcsc_2_0_b- inner node disablement
        // 16 gltm_2_0- inner node disablement
        // 17 xnr_5_2- inner node disablement
        // 18 vcr_3_1- inner node disablement
        // 19 glim_2_0- inner node disablement
        // 20 acm_1_1- inner node disablement
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3C7E7FF040;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noIr | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 gd_dpc_2_2- inner node disablement
        // 4 rgbs_grid_1_1- inner node disablement
        // 5 rgb_ir_2_0- inner node disablement
        // 6 odr_ir_1_1- inner node disablement
        // 7 odr_awb_std_1_1- inner node disablement
        // 8 odr_awb_sve_1_1- inner node disablement
        // 9 odr_awb_sat_1_1- inner node disablement
        // 10 ifd_lsc_1_1- inner node disablement
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
        // 21 gammatm_v3- inner node disablement
        // 22 bxt_csc- inner node disablement
        // 23 ccm_3a_2_0- inner node disablement
        // 24 fr_grid_1_0- inner node disablement
        // 25 b2i_ds_1_0_1- inner node disablement
        // 26 upscaler_1_0- inner node disablement
        // 27 lbff_crop_espa_1_1- inner node disablement
        // 28 tnr_scale_lb- inner node disablement
        // 29 odr_output_ps_1_1- inner node disablement
        // 30 odr_output_me_1_1- inner node disablement
        // 31 aestatistics_2_1- inner node disablement
        // 32 odr_ae_1_1- inner node disablement
        // 33 odr_af_std_1_1- inner node disablement
        // 34 ifd_gmv_1_1- inner node disablement
        // 35 gmv_statistics_1_0- inner node disablement
        // 36 odr_gmv_match_1_1- inner node disablement
        // 37 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3FFFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000007EA3B01924F
        bitmaps.rbm[0] = 0x3B01924F;
        bitmaps.rbm[1] = 0x7EA;
        // DEB - 0x000000000000000000001FFFFEF3FE3F
        bitmaps.deb[0] = 0xFEF3FE3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000DEBDF
        bitmaps.teb[0] = 0xDEBDF;
        // REB - 0x00000000000000000000000003DBFFFF
        bitmaps.reb[0] = 0x3DBFFFF;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void LbffIrWithGmvIrStreamOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{
    // Kernel default enablement
    for (uint32_t j = 0; j < kernelConfigurationsOptionsCount; ++j)
    {
        for (uint8_t i = 0; i < 35; ++i)
        {
            kernelListOptions[j][i].run_kernel.enable = 1;
        }

        // Pass-through kernels
        kernelListOptions[j][6].run_kernel.enable = 0; // wb_1_1
        kernelListOptions[j][8].run_kernel.enable = 0; // bxt_demosaic
        kernelListOptions[j][14].run_kernel.enable = 0; // acm_1_1
    }

    const InnerNodeOptionsFlags nodeRelevantInnerOptions = nodeInnerOptions & (no3A | noLbOutputPs | noLbOutputMe | noGmv);
    bitmaps = HwBitmaps(); // reset HW bitmaps
    uint64_t disabledRunKernelsBitmap = 0x0;
    if (nodeRelevantInnerOptions == (no3A))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000003EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE01F
        bitmaps.teb[0] = 0xCE01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7C0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA3DF
        bitmaps.teb[0] = 0xCA3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x1000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000036A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x36A;
        // DEB - 0x000000000000000000001F7FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1F7F;
        // TEB - 0x00000000000CA01F
        bitmaps.teb[0] = 0xCA01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7D0E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC3DF
        bitmaps.teb[0] = 0xCC3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x2800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000002EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x2EA;
        // DEB - 0x000000000000000000001EBFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1EBF;
        // TEB - 0x00000000000CC01F
        bitmaps.teb[0] = 0xCC01F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7E8E0000;
    }
    else if (nodeRelevantInnerOptions == (noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C83DF
        bitmaps.teb[0] = 0xC83DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x3800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000026A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x26A;
        // DEB - 0x000000000000000000001E3FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1E3F;
        // TEB - 0x00000000000C801F
        bitmaps.teb[0] = 0xC801F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7F8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x00000000000063DF
        bitmaps.teb[0] = 0x63DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x780000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000001EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x1EA;
        // DEB - 0x0000000000000000000001FFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x1FF;
        // TEB - 0x000000000000601F
        bitmaps.teb[0] = 0x601F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FC0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x00000000000023DF
        bitmaps.teb[0] = 0x23DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x781000000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000016A01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0x16A;
        // DEB - 0x00000000000000000000017FFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0x17F;
        // TEB - 0x000000000000201F
        bitmaps.teb[0] = 0x201F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FD0E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x00000000000043DF
        bitmaps.teb[0] = 0x43DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;

        // Kernels disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x782800000;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000EA01404403
        bitmaps.rbm[0] = 0x1404403;
        bitmaps.rbm[1] = 0xEA;
        // DEB - 0x0000000000000000000000BFFE80003F
        bitmaps.deb[0] = 0xFE80003F;
        bitmaps.deb[1] = 0xBF;
        // TEB - 0x000000000000401F
        bitmaps.teb[0] = 0x401F;
        // REB - 0x00000000000000000000000001D830FB
        bitmaps.reb[0] = 0x1D830FB;

        // Kernels disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FE8E0000;
    }
    else if (nodeRelevantInnerOptions == (noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x0000000000000000000000001E404403
        bitmaps.rbm[0] = 0x1E404403;
        // DEB - 0x00000000000000000000000000037E3F
        bitmaps.deb[0] = 0x37E3F;
        // TEB - 0x00000000000003DF
        bitmaps.teb[0] = 0x3DF;
        // REB - 0x0000000000000000000000000003F0FB
        bitmaps.reb[0] = 0x3F0FB;

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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x783F1FFC0;
    }
    else if (nodeRelevantInnerOptions == (no3A | noGmv | noLbOutputPs | noLbOutputMe))
    {
        // HW bitmaps
        // RBM - 0x00000000000000000000000000000000
        // DEB - 0x00000000000000000000000000000000
        // TEB - 0x0000000000000000
        // REB - 0x00000000000000000000000000000000

        // Kernels disablement
        // 0 ifd_pipe_1_1- inner node disablement
        // 1 bxt_blc- inner node disablement
        // 2 linearization2_0- inner node disablement
        // 3 ifd_lsc_1_1- inner node disablement
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
        // 15 gammatm_v3- inner node disablement
        // 16 bxt_csc- inner node disablement
        // 17 rgbs_grid_1_1- inner node disablement
        // 18 ccm_3a_2_0- inner node disablement
        // 19 fr_grid_1_0- inner node disablement
        // 20 b2i_ds_1_0_1- inner node disablement
        // 21 upscaler_1_0- inner node disablement
        // 22 lbff_crop_espa_1_1- inner node disablement
        // 23 tnr_scale_lb- inner node disablement
        // 24 odr_output_ps_1_1- inner node disablement
        // 25 odr_output_me_1_1- inner node disablement
        // 26 odr_awb_std_1_1- inner node disablement
        // 27 odr_awb_sat_1_1- inner node disablement
        // 28 aestatistics_2_1- inner node disablement
        // 29 odr_ae_1_1- inner node disablement
        // 30 odr_af_std_1_1- inner node disablement
        // 31 ifd_gmv_1_1- inner node disablement
        // 32 gmv_statistics_1_0- inner node disablement
        // 33 odr_gmv_match_1_1- inner node disablement
        // 34 odr_gmv_feature_1_1- inner node disablement
        disabledRunKernelsBitmap = 0x7FFFFFFFF;
    }
    else // default inner node
    {
        // RBM - 0x0000000000000000000003EA1F404403
        bitmaps.rbm[0] = 0x1F404403;
        bitmaps.rbm[1] = 0x3EA;
        // DEB - 0x000000000000000000001FFFFE837E3F
        bitmaps.deb[0] = 0xFE837E3F;
        bitmaps.deb[1] = 0x1FFF;
        // TEB - 0x00000000000CE3DF
        bitmaps.teb[0] = 0xCE3DF;
        // REB - 0x00000000000000000000000001DBF0FB
        bitmaps.reb[0] = 0x1DBF0FB;
    }

    SetDisabledKernels(disabledRunKernelsBitmap);

}

void SwVaiOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

void SwImvOuterNode::setInnerNode(InnerNodeOptionsFlags nodeInnerOptions)
{

    // No inner nodes
    (void)nodeInnerOptions;
}

/*
 * Graph 100000
 */
StaticGraph100000::StaticGraph100000(GraphConfiguration100000** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100000, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100000[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
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

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 11; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100000::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100000::~StaticGraph100000()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100000::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[8]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[9]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[10]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[9]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[10]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 11; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100001
 */
StaticGraph100001::StaticGraph100001(GraphConfiguration100001** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100001, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100001[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerWithGmvOuterNodeConfiguration** lbffBayerWithGmvOuterNodeConfigurationOptions = new LbffBayerWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerWithGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerWithGmvOuterNode.Init(lbffBayerWithGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 15;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
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

    link = &_graphLinks[21];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
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

    for (uint8_t i = 0; i < 23; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100001::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100001::~StaticGraph100001()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100001::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerWithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerWithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerWithGmvOuterNode->setInnerNode(lbffBayerWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffBayerWithGmvInnerOptions & noGmv); // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[8]->isActive = !(lbffBayerWithGmvInnerOptions & noGmv); // lbff_Bayer_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffBayerWithGmvInnerOptions & noLbOutputPs); // lbff_Bayer_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[10]->isActive = !(lbffBayerWithGmvInnerOptions & noLbOutputMe); // lbff_Bayer_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 23; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[7]->isActive = (lbffBayerWithGmvInnerOptions & (noGmv|noBurstCapture|noIr|noPdaf)) != (noGmv|noBurstCapture|noIr|noPdaf); // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100002
 */
StaticGraph100002::StaticGraph100002(GraphConfiguration100002** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100002, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100002[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 18; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swScalerOuterNode->contextId = 3;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100002::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100002::~StaticGraph100002()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100002::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_scaler:terminal_connect_input
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 18; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100003
 */
StaticGraph100003::StaticGraph100003(GraphConfiguration100003** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100003, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100003[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerWithGmvOuterNodeConfiguration** lbffBayerWithGmvOuterNodeConfigurationOptions = new LbffBayerWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerWithGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerWithGmvOuterNode.Init(lbffBayerWithGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::LbffBayerWithGmv;
    link->destNode = &_lbffBayerWithGmvOuterNode;
    link->destTerminalId = 15;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerWithGmv;
    link->srcNode = &_lbffBayerWithGmvOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
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
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 22; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100003::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100003::~StaticGraph100003()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100003::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerWithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerWithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerWithGmvOuterNode->setInnerNode(lbffBayerWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerWithGmvInnerOptions & no3A); // lbff_Bayer_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffBayerWithGmvInnerOptions & noGmv); // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[8]->isActive = !(lbffBayerWithGmvInnerOptions & noGmv); // lbff_Bayer_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffBayerWithGmvInnerOptions & noLbOutputPs); // lbff_Bayer_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[10]->isActive = !(lbffBayerWithGmvInnerOptions & noLbOutputMe); // lbff_Bayer_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 22; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[7]->isActive = (lbffBayerWithGmvInnerOptions & (noGmv|noBurstCapture|noIr|noPdaf)) != (noGmv|noBurstCapture|noIr|noPdaf); // lbff_Bayer_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100004
 */
StaticGraph100004::StaticGraph100004(GraphConfiguration100004** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100004, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100004[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swGdcOuterNode.Init(swGdcOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
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
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
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

    for (uint8_t i = 0; i < 21; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGdcOuterNode = &_swGdcOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swGdcOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100004::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100004::~StaticGraph100004()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100004::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 21; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100005
 */
StaticGraph100005::StaticGraph100005(GraphConfiguration100005** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100005, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100005[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions = new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
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

    for (uint8_t i = 0; i < 21; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100005::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100005::~StaticGraph100005()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100005::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 21; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100006
 */
StaticGraph100006::StaticGraph100006(GraphConfiguration100006** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100006, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
    ,_irSubGraph(_sinkMappingConfiguration)
    ,_image_irSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100006[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrOuterNodeConfiguration** lbffRgbIrOuterNodeConfigurationOptions = new LbffRgbIrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvIrStreamOuterNodeConfiguration** lbffIrNoGmvIrStreamOuterNodeConfigurationOptions = new LbffIrNoGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrWithTnrOuterNodeConfiguration** bbpsIrWithTnrOuterNodeConfigurationOptions = new BbpsIrWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffRgbIrOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        lbffIrNoGmvIrStreamOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffIrNoGmvIrStreamOuterNodeConfiguration;
        bbpsIrWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsIrWithTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrOuterNode.Init(lbffRgbIrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrNoGmvIrStreamOuterNode.Init(lbffIrNoGmvIrStreamOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsIrWithTnrOuterNode.Init(bbpsIrWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[17] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[8] = link;
    _image_irSubGraph.links[8] = link;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 13;
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
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[16] = link;
    _image_irSubGraph.links[16] = link;

    link = &_graphLinks[18];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[18] = link;

    link = &_graphLinks[19];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[19] = link;

    link = &_graphLinks[20];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[20] = link;

    link = &_graphLinks[21];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[21] = link;

    link = &_graphLinks[22];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[22] = link;

    link = &_graphLinks[23];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[23] = link;

    link = &_graphLinks[24];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[24] = link;

    link = &_graphLinks[25];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[16] = link;
    _image_irSubGraph.links[25] = link;

    link = &_graphLinks[26];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[17] = link;
    _image_irSubGraph.links[26] = link;

    link = &_graphLinks[27];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _irSubGraph.links[18] = link;
    _image_irSubGraph.links[27] = link;

    link = &_graphLinks[28];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[19] = link;
    _image_irSubGraph.links[28] = link;

    link = &_graphLinks[29];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[20] = link;
    _image_irSubGraph.links[29] = link;

    for (uint8_t i = 0; i < 30; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.previewIr == static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.video == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.stills == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills == static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrWithTnrOuterNode->contextId = 3;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _image_irSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 3;
        _image_irSubGraph.bbpsIrWithTnrOuterNode->contextId = 4;
    }
    else
    {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100006::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffRgbIrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffIrNoGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsIrWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100006::~StaticGraph100006()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100006::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffRgbIrInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & noLbOutputPs); // lbff_RgbIr:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffRgbIrInnerOptions & noLbOutputMe); // lbff_RgbIr:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 17; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100006::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    bbpsIrWithTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[11]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[12]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[13]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[20]->isActive = !(bbpsIrWithTnrInnerOptions & noMp); // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffRgbIrInnerOptions & noIr); // lbff_RgbIr:terminal_connect_ir_output -> lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[14]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[15]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputMe); // lbff_Ir_NoGmv_IrStream:terminal_connect_me_output -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 21; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[16]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[17]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[18]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[19]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100006::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
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
    bbpsIrWithTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[29]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffRgbIrInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

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
    subGraphLinks[3]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[19]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[20]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[21]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[22]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[29]->isActive = !(bbpsIrWithTnrInnerOptions & noMp); // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[18]->isActive = !(lbffRgbIrInnerOptions & noIr); // lbff_RgbIr:terminal_connect_ir_output -> lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & noLbOutputPs); // lbff_RgbIr:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffRgbIrInnerOptions & noLbOutputMe); // lbff_RgbIr:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[23]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[24]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputMe); // lbff_Ir_NoGmv_IrStream:terminal_connect_me_output -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 30; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd
    subGraphLinks[25]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[26]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[27]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[28]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100007
 */
StaticGraph100007::StaticGraph100007(GraphConfiguration100007** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100007, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100007[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerBurstOutNo3AOuterNodeConfiguration** lbffBayerBurstOutNo3AOuterNodeConfigurationOptions = new LbffBayerBurstOutNo3AOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerBurstOutNo3AOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerBurstOutNo3AOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerBurstOutNo3AOuterNode.Init(lbffBayerBurstOutNo3AOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::LbffBayerBurstOutNo3A;
    link->srcNode = &_lbffBayerBurstOutNo3AOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 3; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100007::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerBurstOutNo3AOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100007::~StaticGraph100007()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100007::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayerBurstOutNo3A initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerBurstOutNo3AInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerBurstOutNo3AInnerOptions |= (no3A | noGmv | noIr | noLbOutputPs | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffBayerBurstOutNo3AInnerOptions |= (noLbOutputPs | noLbOutputMe | noPdaf);
    // active private inner options according to links
    if (
        subGraphLinks[2]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        lbffBayerBurstOutNo3AInnerOptions |= noBurstCapture;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerBurstOutNo3AOuterNode->setInnerNode(lbffBayerBurstOutNo3AInnerOptions);

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[2]->isActive = !(lbffBayerBurstOutNo3AInnerOptions & noBurstCapture); // lbff_Bayer_BurstOut_No3A:terminal_connect_burst_isp_output -> image_mp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 3; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100008
 */
StaticGraph100008::StaticGraph100008(GraphConfiguration100008** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100008, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
    ,_irSubGraph(_sinkMappingConfiguration)
    ,_image_irSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100008[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrOuterNodeConfiguration** lbffRgbIrOuterNodeConfigurationOptions = new LbffRgbIrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvIrStreamOuterNodeConfiguration** lbffIrNoGmvIrStreamOuterNodeConfigurationOptions = new LbffIrNoGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrNoTnrOuterNodeConfiguration** bbpsIrNoTnrOuterNodeConfigurationOptions = new BbpsIrNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffRgbIrOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
        lbffIrNoGmvIrStreamOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffIrNoGmvIrStreamOuterNodeConfiguration;
        bbpsIrNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsIrNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrOuterNode.Init(lbffRgbIrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrNoGmvIrStreamOuterNode.Init(lbffIrNoGmvIrStreamOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsIrNoTnrOuterNode.Init(bbpsIrNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[12] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIr;
    link->destNode = &_lbffRgbIrOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 14;
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
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[11] = link;
    _image_irSubGraph.links[11] = link;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffRgbIr;
    link->srcNode = &_lbffRgbIrOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::LbffIrNoGmvIrStream;
    link->destNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[13] = link;

    link = &_graphLinks[14];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[14] = link;

    link = &_graphLinks[15];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[15] = link;

    link = &_graphLinks[16];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[16] = link;

    link = &_graphLinks[17];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[17] = link;

    link = &_graphLinks[18];
    link->src = GraphElementType::LbffIrNoGmvIrStream;
    link->srcNode = &_lbffIrNoGmvIrStreamOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsIrNoTnr;
    link->destNode = &_bbpsIrNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[18] = link;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsIrNoTnr;
    link->srcNode = &_bbpsIrNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[19] = link;

    for (uint8_t i = 0; i < 20; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.previewIr == static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.video == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.stills == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills == static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrNoTnrOuterNode->contextId = 3;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrOuterNode->contextId = 1;
        _image_irSubGraph.bbpsNoTnrOuterNode->contextId = 2;
        _image_irSubGraph.lbffIrNoGmvIrStreamOuterNode->contextId = 3;
        _image_irSubGraph.bbpsIrNoTnrOuterNode->contextId = 4;
    }
    else
    {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100008::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffRgbIrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffIrNoGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsIrNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100008::~StaticGraph100008()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100008::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[9]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[10]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[11]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & noLbOutputPs); // lbff_RgbIr:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 12; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100008::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    bbpsIrNoTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrNoTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrOuterNode->setInnerNode(lbffRgbIrInnerOptions);
    lbffIrNoGmvIrStreamOuterNode->setInnerNode(lbffIrNoGmvIrStreamInnerOptions);
    bbpsIrNoTnrOuterNode->setInnerNode(bbpsIrNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[11]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[12]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[13]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[15]->isActive = !(bbpsIrNoTnrInnerOptions & noMp); // bbps_Ir_NoTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[9]->isActive = !(lbffRgbIrInnerOptions & noIr); // lbff_RgbIr:terminal_connect_ir_output -> lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[14]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 16; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100008::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    if (
        subGraphLinks[9]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[10]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
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
    bbpsIrNoTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrNoTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffIrNoGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

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
    subGraphLinks[3]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[7]->isActive = !(lbffRgbIrInnerOptions & no3A); // lbff_RgbIr:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[11]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[14]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[15]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[16]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[17]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & no3A); // lbff_Ir_NoGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[19]->isActive = !(bbpsIrNoTnrInnerOptions & noMp); // bbps_Ir_NoTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[13]->isActive = !(lbffRgbIrInnerOptions & noIr); // lbff_RgbIr:terminal_connect_ir_output -> lbff_Ir_NoGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[8]->isActive = !(lbffRgbIrInnerOptions & noLbOutputPs); // lbff_RgbIr:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[18]->isActive = !(lbffIrNoGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_NoGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 20; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100015
 */
StaticGraph100015::StaticGraph100015(GraphConfiguration100015** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100015, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100015[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 9; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100015::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100015::~StaticGraph100015()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100015::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffBayer initial inner node configuration
     */
    InnerNodeOptionsFlags lbffBayerInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffBayerInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping
    // active private inner options according to links
    if (
        subGraphLinks[7]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        lbffBayerInnerOptions |= noLbOutputPs;
    }
    if (
        subGraphLinks[8]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        lbffBayerInnerOptions |= noLbOutputMe;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> image_mp
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> image_dp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 9; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100016
 */
StaticGraph100016::StaticGraph100016(GraphConfiguration100016** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100016, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100016[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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

    link = &_graphLinks[3];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 4; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100016::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100016::~StaticGraph100016()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100016::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (
        subGraphLinks[1]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[2]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[3]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Set the selected inner nodes to the outer nodes
     */
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[1]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[2]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[3]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 4; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100024
 */
StaticGraph100024::StaticGraph100024(GraphConfiguration100024** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100024, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100024[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNo2ANoGmvOuterNodeConfiguration** lbffIrNo2ANoGmvOuterNodeConfigurationOptions = new LbffIrNo2ANoGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffIrNo2ANoGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffIrNo2ANoGmvOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrNo2ANoGmvOuterNode.Init(lbffIrNo2ANoGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffIrNo2ANoGmvOuterNodeConfigurationOptions;
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
    link->dest = GraphElementType::LbffIrNo2ANoGmv;
    link->destNode = &_lbffIrNo2ANoGmvOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffIrNo2ANoGmv;
    link->destNode = &_lbffIrNo2ANoGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffIrNo2ANoGmv;
    link->srcNode = &_lbffIrNo2ANoGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffIrNo2ANoGmv;
    link->srcNode = &_lbffIrNo2ANoGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffIrNo2ANoGmv;
    link->srcNode = &_lbffIrNo2ANoGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsNoTnr;
    link->destNode = &_bbpsNoTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::ImageMp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::ImageDp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 9; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffIrNo2ANoGmvOuterNode = &_lbffIrNo2ANoGmvOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffIrNo2ANoGmvOuterNode->contextId = 1;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 2;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100024::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffIrNo2ANoGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100024::~StaticGraph100024()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100024::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffIrNo2ANoGmv initial inner node configuration
     */
    InnerNodeOptionsFlags lbffIrNo2ANoGmvInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffIrNo2ANoGmvInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffIrNo2ANoGmvInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (
        subGraphLinks[6]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[7]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[8]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffIrNo2ANoGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffIrNo2ANoGmvOuterNode->setInnerNode(lbffIrNo2ANoGmvInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffIrNo2ANoGmvInnerOptions & no3A); // lbff_Ir_No2A_NoGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[4]->isActive = !(lbffIrNo2ANoGmvInnerOptions & no3A); // lbff_Ir_No2A_NoGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[6]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[7]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[8]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[5]->isActive = !(lbffIrNo2ANoGmvInnerOptions & noLbOutputPs); // lbff_Ir_No2A_NoGmv:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 9; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100025
 */
StaticGraph100025::StaticGraph100025(GraphConfiguration100025** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100025, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100025[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrNoGmvOuterNodeConfiguration** lbffIrNoGmvOuterNodeConfigurationOptions = new LbffIrNoGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffIrNoGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffIrNoGmvOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrNoGmvOuterNode.Init(lbffIrNoGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffIrNoGmv;
    link->destNode = &_lbffIrNoGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffIrNoGmv;
    link->srcNode = &_lbffIrNoGmvOuterNode;
    link->srcTerminalId = 14;
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

    link = &_graphLinks[10];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 11; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100025::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffIrNoGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100025::~StaticGraph100025()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100025::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[8]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[9]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[10]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffIrNoGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffIrNoGmvOuterNode->setInnerNode(lbffIrNoGmvInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffIrNoGmvInnerOptions & no3A); // lbff_Ir_NoGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffIrNoGmvInnerOptions & no3A); // lbff_Ir_NoGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffIrNoGmvInnerOptions & no3A); // lbff_Ir_NoGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffIrNoGmvInnerOptions & no3A); // lbff_Ir_NoGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[9]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[10]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffIrNoGmvInnerOptions & noLbOutputPs); // lbff_Ir_NoGmv:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 11; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100026
 */
StaticGraph100026::StaticGraph100026(GraphConfiguration100026** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100026, selectedSettingsId, zoomKeyResolutions),

    _rawSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100026[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
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

    for (uint8_t i = 0; i < 2; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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

StaticGraphStatus StaticGraph100026::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100026::~StaticGraph100026()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100027
 */
StaticGraph100027::StaticGraph100027(GraphConfiguration100027** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100027, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100027[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions = new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2OuterNodeConfiguration** lbffBayerPdaf2OuterNodeConfigurationOptions = new LbffBayerPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf2OuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2OuterNode.Init(lbffBayerPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
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
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 14;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 14; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100027::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100027::~StaticGraph100027()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100027::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[12]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2InnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2OuterNode->setInnerNode(lbffBayerPdaf2InnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[12]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[13]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffBayerPdaf2InnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf2:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffBayerPdaf2InnerOptions & noPdaf); // lbff_Bayer_Pdaf2:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 14; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100028
 */
StaticGraph100028::StaticGraph100028(GraphConfiguration100028** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100028, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100028[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions = new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
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

    link = &_graphLinks[11];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 12; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100028::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100028::~StaticGraph100028()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100028::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[9]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[10]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[9]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[10]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[11]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffBayerPdaf3InnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf3:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[7]->isActive = !(lbffBayerPdaf3InnerOptions & noPdaf); // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 12; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100029
 */
StaticGraph100029::StaticGraph100029(GraphConfiguration100029** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100029, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100029[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions = new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2OuterNodeConfiguration** lbffBayerPdaf2OuterNodeConfigurationOptions = new LbffBayerPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf2OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2OuterNode.Init(lbffBayerPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
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
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2;
    link->destNode = &_lbffBayerPdaf2OuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf2;
    link->srcNode = &_lbffBayerPdaf2OuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 19; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100029::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100029::~StaticGraph100029()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100029::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2InnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerPdaf2InnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2OuterNode->setInnerNode(lbffBayerPdaf2InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive = !(lbffBayerPdaf2InnerOptions & no3A); // lbff_Bayer_Pdaf2:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffBayerPdaf2InnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf2:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffBayerPdaf2InnerOptions & noLbOutputMe); // lbff_Bayer_Pdaf2:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[9]->isActive = !(lbffBayerPdaf2InnerOptions & noPdaf); // lbff_Bayer_Pdaf2:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 19; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100030
 */
StaticGraph100030::StaticGraph100030(GraphConfiguration100030** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100030, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100030[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions = new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 17; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100030::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100030::~StaticGraph100030()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100030::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerPdaf3InnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffBayerPdaf3InnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf3:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffBayerPdaf3InnerOptions & noLbOutputMe); // lbff_Bayer_Pdaf3:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive = !(lbffBayerPdaf3InnerOptions & noPdaf); // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 17; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100031
 */
StaticGraph100031::StaticGraph100031(GraphConfiguration100031** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100031, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100031[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions = new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwDolOuterNodeConfiguration** swDolOuterNodeConfigurationOptions = new SwDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDolOuterNodeConfiguration** lbffDolOuterNodeConfigurationOptions = new LbffDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsNoTnrOuterNodeConfiguration** bbpsNoTnrOuterNodeConfigurationOptions = new BbpsNoTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGtmOuterNodeConfiguration** swGtmOuterNodeConfigurationOptions = new SwGtmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions = new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        swDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swDolOuterNodeConfiguration;
        lbffDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffDolOuterNodeConfiguration;
        bbpsNoTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsNoTnrOuterNodeConfiguration;
        swGtmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGtmOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swDolOuterNode.Init(swDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDolOuterNode.Init(lbffDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsNoTnrOuterNode.Init(bbpsNoTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swGtmOuterNode.Init(swGtmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] swDolOuterNodeConfigurationOptions;
    delete[] lbffDolOuterNodeConfigurationOptions;
    delete[] bbpsNoTnrOuterNodeConfigurationOptions;
    delete[] swGtmOuterNodeConfigurationOptions;
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
    link->dest = GraphElementType::SwDol;
    link->destNode = &_swDolOuterNode;
    link->destTerminalId = 1;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::SwDol;
    link->destNode = &_swDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::SwDol;
    link->srcNode = &_swDolOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::LbffDol;
    link->destNode = &_lbffDolOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol;
    link->destNode = &_lbffDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 14;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[14];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGtm;
    link->destNode = &_swGtmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[15];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGtm;
    link->destNode = &_swGtmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[16];
    link->src = GraphElementType::SwGtm;
    link->srcNode = &_swGtmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::SwGtm;
    link->srcNode = &_swGtmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsNoTnr;
    link->srcNode = &_bbpsNoTnrOuterNode;
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

    for (uint8_t i = 0; i < 23; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.swDolOuterNode = &_swDolOuterNode;
    _imageSubGraph.lbffDolOuterNode = &_lbffDolOuterNode;
    _imageSubGraph.bbpsNoTnrOuterNode = &_bbpsNoTnrOuterNode;
    _imageSubGraph.swGtmOuterNode = &_swGtmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.swDolOuterNode->contextId = 1;
    _imageSubGraph.lbffDolOuterNode->contextId = 2;
    _imageSubGraph.bbpsNoTnrOuterNode->contextId = 3;
    _imageSubGraph.swGtmOuterNode->contextId = 4;
    _imageSubGraph.swScalerOuterNode->contextId = 5;
    _imageSubGraph.swNntmOuterNode->contextId = 6;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100031::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsNoTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGtmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100031::~StaticGraph100031()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100031::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDolInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDolInnerOptions |= (noGmv | noBurstCapture | noIr | noLbOutputMe | noPdaf);
    // active public options according to sink mapping
    // always active private inner options
    lbffDolInnerOptions |= (noLbOutputMe);

    /*
     * Setting Node bbpsNoTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsNoTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (
        subGraphLinks[11]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[12]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsNoTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDolInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDolOuterNode->setInnerNode(lbffDolInnerOptions);
    bbpsNoTnrOuterNode->setInnerNode(bbpsNoTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[6]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_ae_output -> ae_out
    subGraphLinks[7]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[8]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[9]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[11]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[14]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> sw_gtm:terminal_connect_input
    subGraphLinks[18]->isActive = !(bbpsNoTnrInnerOptions & noMp); // bbps_NoTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[12]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[15]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> sw_gtm:terminal_connect_input
    subGraphLinks[19]->isActive = !(bbpsNoTnrInnerOptions & noDp); // bbps_NoTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[13]->isActive = !(bbpsNoTnrInnerOptions & noPpp); // bbps_NoTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffDolInnerOptions & noLbOutputPs); // lbff_Dol:terminal_connect_ps_output -> bbps_NoTnr:bbps_slim_spatial_yuvn_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 23; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100032
 */
StaticGraph100032::StaticGraph100032(GraphConfiguration100032** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100032, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100032[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions = new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwDolOuterNodeConfiguration** swDolOuterNodeConfigurationOptions = new SwDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffDolOuterNodeConfiguration** lbffDolOuterNodeConfigurationOptions = new LbffDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGtmOuterNodeConfiguration** swGtmOuterNodeConfigurationOptions = new SwGtmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions = new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysDolOuterNodeConfiguration;
        swDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swDolOuterNodeConfiguration;
        lbffDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffDolOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGtmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGtmOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysDolOuterNode.Init(isysDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swDolOuterNode.Init(swDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffDolOuterNode.Init(lbffDolOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swGtmOuterNode.Init(swGtmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysDolOuterNodeConfigurationOptions;
    delete[] swDolOuterNodeConfigurationOptions;
    delete[] lbffDolOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swGtmOuterNodeConfigurationOptions;
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
    link->dest = GraphElementType::SwDol;
    link->destNode = &_swDolOuterNode;
    link->destTerminalId = 1;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::IsysDol;
    link->srcNode = &_isysDolOuterNode;
    link->srcTerminalId = 5;
    link->dest = GraphElementType::SwDol;
    link->destNode = &_swDolOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::SwDol;
    link->srcNode = &_swDolOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::LbffDol;
    link->destNode = &_lbffDolOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LscBuffer;
    link->dest = GraphElementType::LbffDol;
    link->destNode = &_lbffDolOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffDol;
    link->srcNode = &_lbffDolOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGtm;
    link->destNode = &_swGtmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGtm;
    link->destNode = &_swGtmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::SwGtm;
    link->srcNode = &_swGtmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[22];
    link->src = GraphElementType::SwGtm;
    link->srcNode = &_swGtmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[23];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[24];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwNntm;
    link->destNode = &_swNntmOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[25];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[26];
    link->src = GraphElementType::SwNntm;
    link->srcNode = &_swNntmOuterNode;
    link->srcTerminalId = 2;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[27];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedSecondary;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 28; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysDolOuterNode = &_isysDolOuterNode;
    _imageSubGraph.swDolOuterNode = &_swDolOuterNode;
    _imageSubGraph.lbffDolOuterNode = &_lbffDolOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swGtmOuterNode = &_swGtmOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;
    _imageSubGraph.swNntmOuterNode = &_swNntmOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysDolOuterNode->contextId = 0;
    _imageSubGraph.swDolOuterNode->contextId = 1;
    _imageSubGraph.lbffDolOuterNode->contextId = 2;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 3;
    _imageSubGraph.swGtmOuterNode->contextId = 4;
    _imageSubGraph.swScalerOuterNode->contextId = 5;
    _imageSubGraph.swNntmOuterNode->contextId = 6;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100032::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGtmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100032::~StaticGraph100032()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100032::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

    /*
     * Setting Node lbffDol initial inner node configuration
     */
    InnerNodeOptionsFlags lbffDolInnerOptions = imagePublicInnerNodeConfiguration;
    // always active public inner options
    lbffDolInnerOptions |= (noGmv | noBurstCapture | noIr | noPdaf);
    // active public options according to sink mapping

    /*
     * Setting Node bbpsWithTnr initial inner node configuration
     */
    InnerNodeOptionsFlags bbpsWithTnrInnerOptions = imagePublicInnerNodeConfiguration;
    // active public options according to sink mapping
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[23]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[24]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffDolInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffDolInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffDolOuterNode->setInnerNode(lbffDolInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[6]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_ae_output -> ae_out
    subGraphLinks[7]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[8]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[9]->isActive = !(lbffDolInnerOptions & no3A); // lbff_Dol:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gtm:terminal_connect_input
    subGraphLinks[23]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gtm:terminal_connect_input
    subGraphLinks[24]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffDolInnerOptions & noLbOutputPs); // lbff_Dol:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffDolInnerOptions & noLbOutputMe); // lbff_Dol:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 28; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100035
 */
StaticGraph100035::StaticGraph100035(GraphConfiguration100035** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100035, selectedSettingsId, zoomKeyResolutions),

    _rawSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100035[kernelConfigurationsOptionsCount];
    IsysDolOuterNodeConfiguration** isysDolOuterNodeConfigurationOptions = new IsysDolOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysDolOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysDolOuterNodeConfiguration;
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

    for (uint8_t i = 0; i < 4; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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

StaticGraphStatus StaticGraph100035::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysDolOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100035::~StaticGraph100035()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100036
 */
StaticGraph100036::StaticGraph100036(GraphConfiguration100036** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100036, selectedSettingsId, zoomKeyResolutions),

    _rawSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100036[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions = new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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

    for (uint8_t i = 0; i < 4; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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

StaticGraphStatus StaticGraph100036::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100036::~StaticGraph100036()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}
/*
 * Graph 100037
 */
StaticGraph100037::StaticGraph100037(GraphConfiguration100037** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100037, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100037[kernelConfigurationsOptionsCount];
    IsysPdaf2OuterNodeConfiguration** isysPdaf2OuterNodeConfigurationOptions = new IsysPdaf2OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf2WithGmvOuterNodeConfiguration** lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions = new LbffBayerPdaf2WithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysPdaf2OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysPdaf2OuterNodeConfiguration;
        lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf2WithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysPdaf2OuterNode.Init(isysPdaf2OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf2WithGmvOuterNode.Init(lbffBayerPdaf2WithGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
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
    link->destTerminalId = 4;
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
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::IsysPdaf2;
    link->srcNode = &_isysPdaf2OuterNode;
    link->srcTerminalId = 3;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::LbffBayerPdaf2WithGmv;
    link->destNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->destTerminalId = 15;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[13];
    link->src = GraphElementType::LbffBayerPdaf2WithGmv;
    link->srcNode = &_lbffBayerPdaf2WithGmvOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[22];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[23];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
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

    for (uint8_t i = 0; i < 25; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100037::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysPdaf2OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf2WithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100037::~StaticGraph100037()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100037::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[22]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[23]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf2WithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerPdaf2WithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf2WithGmvOuterNode->setInnerNode(lbffBayerPdaf2WithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[5]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[7]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[8]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[10]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & noGmv); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_input
    subGraphLinks[11]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & noGmv); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[21]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[22]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[23]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[12]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[13]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & noLbOutputMe); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[9]->isActive = !(lbffBayerPdaf2WithGmvInnerOptions & noPdaf); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 25; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (lbffBayerPdaf2WithGmvInnerOptions & (noGmv|noBurstCapture|noIr)) != (noGmv|noBurstCapture|noIr); // lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_Pdaf2_WithGmv:terminal_connect_gmv_input
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[16]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[17]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100038
 */
StaticGraph100038::StaticGraph100038(GraphConfiguration100038** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100038, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100038[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3WithGmvOuterNodeConfiguration** lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions = new LbffBayerPdaf3WithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf3WithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3WithGmvOuterNode.Init(lbffBayerPdaf3WithGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3WithGmv;
    link->destNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::LbffBayerPdaf3WithGmv;
    link->destNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->destTerminalId = 15;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffBayerPdaf3WithGmv;
    link->srcNode = &_lbffBayerPdaf3WithGmvOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[20];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[21];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
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

    for (uint8_t i = 0; i < 23; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100038::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf3WithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100038::~StaticGraph100038()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100038::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3WithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerPdaf3WithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3WithGmvOuterNode->setInnerNode(lbffBayerPdaf3WithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & no3A); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & noGmv); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & noGmv); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[21]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & noLbOutputMe); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive = !(lbffBayerPdaf3WithGmvInnerOptions & noPdaf); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 23; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive = (lbffBayerPdaf3WithGmvInnerOptions & (noGmv|noBurstCapture|noIr)) != (noGmv|noBurstCapture|noIr); // lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_feature_output -> lbff_Bayer_Pdaf3_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100039
 */
StaticGraph100039::StaticGraph100039(GraphConfiguration100039** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100039, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
    ,_irSubGraph(_sinkMappingConfiguration)
    ,_image_irSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100039[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffRgbIrWithGmvOuterNodeConfiguration** lbffRgbIrWithGmvOuterNodeConfigurationOptions = new LbffRgbIrWithGmvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffIrWithGmvIrStreamOuterNodeConfiguration** lbffIrWithGmvIrStreamOuterNodeConfigurationOptions = new LbffIrWithGmvIrStreamOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsIrWithTnrOuterNodeConfiguration** bbpsIrWithTnrOuterNodeConfigurationOptions = new BbpsIrWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwGdcOuterNodeConfiguration** swGdcOuterNodeConfigurationOptions = new SwGdcOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffRgbIrWithGmvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffRgbIrWithGmvOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        lbffIrWithGmvIrStreamOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffIrWithGmvIrStreamOuterNodeConfiguration;
        bbpsIrWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsIrWithTnrOuterNodeConfiguration;
        swGdcOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swGdcOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffRgbIrWithGmvOuterNode.Init(lbffRgbIrWithGmvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffIrWithGmvIrStreamOuterNode.Init(lbffIrWithGmvIrStreamOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsIrWithTnrOuterNode.Init(bbpsIrWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _imageSubGraph.links[1] = link;
    _irSubGraph.links[1] = link;
    _image_irSubGraph.links[1] = link;

    link = &_graphLinks[2];
    link->src = GraphElementType::LscBufferIr;
    link->dest = GraphElementType::LbffIrWithGmvIrStream;
    link->destNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;
    _irSubGraph.links[2] = link;
    _image_irSubGraph.links[23] = link;

    link = &_graphLinks[3];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffRgbIrWithGmv;
    link->destNode = &_lbffRgbIrWithGmvOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[2] = link;
    _irSubGraph.links[3] = link;
    _image_irSubGraph.links[2] = link;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[3] = link;
    _irSubGraph.links[4] = link;
    _image_irSubGraph.links[3] = link;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[4] = link;
    _irSubGraph.links[5] = link;
    _image_irSubGraph.links[4] = link;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[5] = link;
    _irSubGraph.links[6] = link;
    _image_irSubGraph.links[5] = link;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[6] = link;
    _irSubGraph.links[7] = link;
    _image_irSubGraph.links[6] = link;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::AwbSveOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[7] = link;
    _irSubGraph.links[8] = link;
    _image_irSubGraph.links[7] = link;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 19;
    link->dest = GraphElementType::LbffRgbIrWithGmv;
    link->destNode = &_lbffRgbIrWithGmvOuterNode;
    link->destTerminalId = 15;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _imageSubGraph.links[8] = link;
    _irSubGraph.links[9] = link;
    _image_irSubGraph.links[8] = link;

    link = &_graphLinks[10];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 18;
    link->dest = GraphElementType::GmvMatchOut;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[9] = link;
    _irSubGraph.links[10] = link;
    _image_irSubGraph.links[9] = link;

    link = &_graphLinks[11];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[10] = link;
    _image_irSubGraph.links[10] = link;

    link = &_graphLinks[12];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 13;
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
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[18] = link;
    _image_irSubGraph.links[18] = link;

    link = &_graphLinks[20];
    link->src = GraphElementType::LbffRgbIrWithGmv;
    link->srcNode = &_lbffRgbIrWithGmvOuterNode;
    link->srcTerminalId = 11;
    link->dest = GraphElementType::LbffIrWithGmvIrStream;
    link->destNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[11] = link;
    _image_irSubGraph.links[24] = link;

    link = &_graphLinks[21];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::IrAeOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[12] = link;
    _image_irSubGraph.links[25] = link;

    link = &_graphLinks[22];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::IrAfStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[13] = link;
    _image_irSubGraph.links[26] = link;

    link = &_graphLinks[23];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::IrAwbStdOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[14] = link;
    _image_irSubGraph.links[27] = link;

    link = &_graphLinks[24];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::IrAwbSatOut;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[15] = link;
    _image_irSubGraph.links[28] = link;

    link = &_graphLinks[25];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[16] = link;
    _image_irSubGraph.links[29] = link;

    link = &_graphLinks[26];
    link->src = GraphElementType::LbffIrWithGmvIrStream;
    link->srcNode = &_lbffIrWithGmvIrStreamOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 7;
    link->type = LinkType::Node2Node;
    _irSubGraph.links[17] = link;
    _image_irSubGraph.links[30] = link;

    link = &_graphLinks[27];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 12;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 10;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[18] = link;
    _image_irSubGraph.links[31] = link;

    link = &_graphLinks[28];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 13;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 5;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[19] = link;
    _image_irSubGraph.links[32] = link;

    link = &_graphLinks[29];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 11;
    link->type = LinkType::Node2Self;
    _irSubGraph.links[20] = link;
    _image_irSubGraph.links[33] = link;

    link = &_graphLinks[30];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::BbpsIrWithTnr;
    link->destNode = &_bbpsIrWithTnrOuterNode;
    link->destTerminalId = 6;
    link->type = LinkType::Node2Self;
    link->frameDelay = 1U;
    _irSubGraph.links[21] = link;
    _image_irSubGraph.links[34] = link;

    link = &_graphLinks[31];
    link->src = GraphElementType::BbpsIrWithTnr;
    link->srcNode = &_bbpsIrWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::IrMp;
    link->type = LinkType::Node2Sink;
    _irSubGraph.links[22] = link;
    _image_irSubGraph.links[35] = link;

    link = &_graphLinks[32];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[19] = link;
    _image_irSubGraph.links[19] = link;

    link = &_graphLinks[33];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[20] = link;
    _image_irSubGraph.links[20] = link;

    link = &_graphLinks[34];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 16;
    link->dest = GraphElementType::SwGdc;
    link->destNode = &_swGdcOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;
    _imageSubGraph.links[21] = link;
    _image_irSubGraph.links[21] = link;

    link = &_graphLinks[35];
    link->src = GraphElementType::SwGdc;
    link->srcNode = &_swGdcOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;
    _imageSubGraph.links[22] = link;
    _image_irSubGraph.links[22] = link;

    for (uint8_t i = 0; i < 36; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.previewIr == static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_imageSubGraph;

        // logical node IDs
        _imageSubGraph.isysOuterNode->contextId = 0;
        _imageSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _imageSubGraph.swGdcOuterNode->contextId = 3;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.video == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.stills == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills == static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_irSubGraph;

        // logical node IDs
        _irSubGraph.isysOuterNode->contextId = 0;
        _irSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _irSubGraph.lbffIrWithGmvIrStreamOuterNode->contextId = 2;
        _irSubGraph.bbpsIrWithTnrOuterNode->contextId = 3;
    }
    else if (
        // image sink group
        (_graphConfigurations[0].sinkMappingConfiguration.preview != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.video != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingVideo != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.stills != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.thumbnail != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.postProcessingStills != static_cast<int>(HwSink::Disconnected)) &&
        // raw sink group
        (_graphConfigurations[0].sinkMappingConfiguration.raw == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawPdaf == static_cast<int>(HwSink::Disconnected) &&
        _graphConfigurations[0].sinkMappingConfiguration.rawDolLong == static_cast<int>(HwSink::Disconnected)) &&
        // ir sink group
        (_graphConfigurations[0].sinkMappingConfiguration.videoIr != static_cast<int>(HwSink::Disconnected) ||
        _graphConfigurations[0].sinkMappingConfiguration.previewIr != static_cast<int>(HwSink::Disconnected)))
    {
        _selectedGraphTopology = &_image_irSubGraph;

        // logical node IDs
        _image_irSubGraph.isysOuterNode->contextId = 0;
        _image_irSubGraph.lbffRgbIrWithGmvOuterNode->contextId = 1;
        _image_irSubGraph.bbpsWithTnrOuterNode->contextId = 2;
        _image_irSubGraph.swGdcOuterNode->contextId = 3;
        _image_irSubGraph.lbffIrWithGmvIrStreamOuterNode->contextId = 4;
        _image_irSubGraph.bbpsIrWithTnrOuterNode->contextId = 5;
    }
    else
    {
        STATIC_GRAPH_LOG("Didn't found a matching sub graph for the selected virtual sinks.");
    }
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100039::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffRgbIrWithGmvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffIrWithGmvIrStreamOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsIrWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swGdcOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100039::~StaticGraph100039()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100039::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffRgbIrWithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrWithGmvOuterNode->setInnerNode(lbffRgbIrWithGmvInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[21]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> sw_gdc:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[10]->isActive = !(lbffRgbIrWithGmvInnerOptions & noLbOutputPs); // lbff_RgbIr_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffRgbIrWithGmvInnerOptions & noLbOutputMe); // lbff_RgbIr_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 23; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive = (lbffRgbIrWithGmvInnerOptions & (noGmv|noBurstCapture|noPdaf)) != (noGmv|noBurstCapture|noPdaf); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus irSubGraphTopology100039::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    bbpsIrWithTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[22]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffIrWithGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrWithGmvIrStreamInnerOptions |= noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffRgbIrWithGmvOuterNode->setInnerNode(lbffRgbIrWithGmvInnerOptions);
    lbffIrWithGmvIrStreamOuterNode->setInnerNode(lbffIrWithGmvIrStreamInnerOptions);
    bbpsIrWithTnrOuterNode->setInnerNode(bbpsIrWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[8]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[9]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[10]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[12]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[13]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[14]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[15]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[22]->isActive = !(bbpsIrWithTnrInnerOptions & noMp); // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[11]->isActive = !(lbffRgbIrWithGmvInnerOptions & noIr); // lbff_RgbIr_WithGmv:terminal_connect_ir_output -> lbff_Ir_WithGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[16]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_WithGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[17]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & noLbOutputMe); // lbff_Ir_WithGmv_IrStream:terminal_connect_me_output -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 23; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (lbffRgbIrWithGmvInnerOptions & (noGmv|noBurstCapture|noPdaf)) != (noGmv|noBurstCapture|noPdaf); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[18]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[19]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[20]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[21]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus image_irSubGraphTopology100039::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);
    InnerNodeOptionsFlags irPublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.irInnerOptions);

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
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[19]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[20]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[18]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[21]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
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
    bbpsIrWithTnrInnerOptions |= (noDp | noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[35]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsIrWithTnrInnerOptions |= noMp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffRgbIrWithGmvInnerOptions |= noIr & (-((irPublicInnerNodeConfiguration & (no3A | noMp)) == (no3A | noMp)));
    lbffRgbIrWithGmvInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffRgbIrWithGmvInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffIrWithGmvIrStreamInnerOptions |= noLbOutputPs & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));
    lbffIrWithGmvIrStreamInnerOptions |= noLbOutputMe & (-((irPublicInnerNodeConfiguration & (noMp)) == (noMp)));

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
    subGraphLinks[3]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[7]->isActive = !(lbffRgbIrWithGmvInnerOptions & no3A); // lbff_RgbIr_WithGmv:terminal_connect_awb_sve_output -> awb_sve_out
    subGraphLinks[8]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[9]->isActive = !(lbffRgbIrWithGmvInnerOptions & noGmv); // lbff_RgbIr_WithGmv:terminal_connect_gmv_match_output -> gmv_match_out
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[19]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[20]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[18]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp
    subGraphLinks[21]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> sw_gdc:terminal_connect_input
    subGraphLinks[25]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_ae_output -> ir_ae_out
    subGraphLinks[26]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_af_std_output -> ir_af_std_out
    subGraphLinks[27]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_std_output -> ir_awb_std_out
    subGraphLinks[28]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & no3A); // lbff_Ir_WithGmv_IrStream:terminal_connect_awb_sat_output -> ir_awb_sat_out
    subGraphLinks[35]->isActive = !(bbpsIrWithTnrInnerOptions & noMp); // bbps_Ir_WithTnr:bbps_ofs_mp_yuvn_odr -> ir_mp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[24]->isActive = !(lbffRgbIrWithGmvInnerOptions & noIr); // lbff_RgbIr_WithGmv:terminal_connect_ir_output -> lbff_Ir_WithGmv_IrStream:terminal_connect_main_data_input
    subGraphLinks[10]->isActive = !(lbffRgbIrWithGmvInnerOptions & noLbOutputPs); // lbff_RgbIr_WithGmv:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[11]->isActive = !(lbffRgbIrWithGmvInnerOptions & noLbOutputMe); // lbff_RgbIr_WithGmv:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[29]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & noLbOutputPs); // lbff_Ir_WithGmv_IrStream:terminal_connect_ps_output -> bbps_Ir_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[30]->isActive = !(lbffIrWithGmvIrStreamInnerOptions & noLbOutputMe); // lbff_Ir_WithGmv_IrStream:terminal_connect_me_output -> bbps_Ir_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 36; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[8]->isActive = (lbffRgbIrWithGmvInnerOptions & (noGmv|noBurstCapture|noPdaf)) != (noGmv|noBurstCapture|noPdaf); // lbff_RgbIr_WithGmv:terminal_connect_gmv_feature_output -> lbff_RgbIr_WithGmv:terminal_connect_gmv_input
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[14]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[15]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd
    subGraphLinks[31]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[32]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[33]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[34]->isActive = (bbpsIrWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_Ir_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_Ir_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100040
 */
StaticGraph100040::StaticGraph100040(GraphConfiguration100040** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100040, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100040[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerPdaf3OuterNodeConfiguration** lbffBayerPdaf3OuterNodeConfigurationOptions = new LbffBayerPdaf3OuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwNntmOuterNodeConfiguration** swNntmOuterNodeConfigurationOptions = new SwNntmOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerPdaf3OuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerPdaf3OuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swNntmOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swNntmOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerPdaf3OuterNode.Init(lbffBayerPdaf3OuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swNntmOuterNode.Init(swNntmOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayerPdaf3;
    link->destNode = &_lbffBayerPdaf3OuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 10;
    link->dest = GraphElementType::PdafOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayerPdaf3;
    link->srcNode = &_lbffBayerPdaf3OuterNode;
    link->srcTerminalId = 13;
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

    for (uint8_t i = 0; i < 21; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

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
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100040::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerPdaf3OuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swNntmOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100040::~StaticGraph100040()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100040::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    // always active public inner options
    bbpsWithTnrInnerOptions |= (noPpp);
    // active public options according to sink mapping
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerPdaf3InnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));
    lbffBayerPdaf3InnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp)) == (noMp | noDp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerPdaf3OuterNode->setInnerNode(lbffBayerPdaf3InnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerPdaf3InnerOptions & no3A); // lbff_Bayer_Pdaf3:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_nntm:terminal_connect_input
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_nntm:terminal_connect_input

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffBayerPdaf3InnerOptions & noLbOutputPs); // lbff_Bayer_Pdaf3:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffBayerPdaf3InnerOptions & noLbOutputMe); // lbff_Bayer_Pdaf3:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd
    subGraphLinks[7]->isActive = !(lbffBayerPdaf3InnerOptions & noPdaf); // lbff_Bayer_Pdaf3:terminal_connect_pdaf_output -> pdaf_out

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 21; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100041
 */
StaticGraph100041::StaticGraph100041(GraphConfiguration100041** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100041, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100041[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwVaiOuterNodeConfiguration** swVaiOuterNodeConfigurationOptions = new SwVaiOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        swVaiOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swVaiOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swVaiOuterNode.Init(swVaiOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] swVaiOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::SwVai;
    link->destNode = &_swVaiOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::SwVai;
    link->srcNode = &_swVaiOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[9];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwScaler;
    link->destNode = &_swScalerOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwScaler;
    link->srcNode = &_swScalerOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    for (uint8_t i = 0; i < 19; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.swVaiOuterNode = &_swVaiOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.swVaiOuterNode->contextId = 2;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100041::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swVaiOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100041::~StaticGraph100041()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100041::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_scaler:terminal_connect_input
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[9]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 19; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[13]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

/*
 * Graph 100042
 */
StaticGraph100042::StaticGraph100042(GraphConfiguration100042** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId) :
    IStaticGraphConfig(selectedSensorMode, sinkMappingConfiguration, 100042, selectedSettingsId, zoomKeyResolutions),

    _imageSubGraph(_sinkMappingConfiguration)
{
    // Construct outer nodes
    _graphConfigurations = new GraphConfiguration100042[kernelConfigurationsOptionsCount];
    IsysOuterNodeConfiguration** isysOuterNodeConfigurationOptions = new IsysOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    LbffBayerOuterNodeConfiguration** lbffBayerOuterNodeConfigurationOptions = new LbffBayerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    BbpsWithTnrOuterNodeConfiguration** bbpsWithTnrOuterNodeConfigurationOptions = new BbpsWithTnrOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwImvOuterNodeConfiguration** swImvOuterNodeConfigurationOptions = new SwImvOuterNodeConfiguration*[kernelConfigurationsOptionsCount];
    SwScalerOuterNodeConfiguration** swScalerOuterNodeConfigurationOptions = new SwScalerOuterNodeConfiguration*[kernelConfigurationsOptionsCount];

    for (uint32_t i=0; i < kernelConfigurationsOptionsCount; ++i)
    {
        _graphConfigurations[i] = *selectedGraphConfiguration[i];
        isysOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].isysOuterNodeConfiguration;
        lbffBayerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].lbffBayerOuterNodeConfiguration;
        bbpsWithTnrOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].bbpsWithTnrOuterNodeConfiguration;
        swImvOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swImvOuterNodeConfiguration;
        swScalerOuterNodeConfigurationOptions[i] = &_graphConfigurations[i].swScalerOuterNodeConfiguration;
    }

    _isysOuterNode.Init(isysOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _lbffBayerOuterNode.Init(lbffBayerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _bbpsWithTnrOuterNode.Init(bbpsWithTnrOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swImvOuterNode.Init(swImvOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);
    _swScalerOuterNode.Init(swScalerOuterNodeConfigurationOptions, kernelConfigurationsOptionsCount);

    delete[] isysOuterNodeConfigurationOptions;
    delete[] lbffBayerOuterNodeConfigurationOptions;
    delete[] bbpsWithTnrOuterNodeConfigurationOptions;
    delete[] swImvOuterNodeConfigurationOptions;
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
    link->destTerminalId = 4;
    link->type = LinkType::Source2Node;

    link = &_graphLinks[2];
    link->src = GraphElementType::Isys;
    link->srcNode = &_isysOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::LbffBayer;
    link->destNode = &_lbffBayerOuterNode;
    link->destTerminalId = 3;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[3];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 6;
    link->dest = GraphElementType::AeOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[4];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 7;
    link->dest = GraphElementType::AfStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[5];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 8;
    link->dest = GraphElementType::AwbStdOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[6];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 9;
    link->dest = GraphElementType::AwbSatOut;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[7];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::BbpsWithTnr;
    link->destNode = &_bbpsWithTnrOuterNode;
    link->destTerminalId = 9;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[8];
    link->src = GraphElementType::LbffBayer;
    link->srcNode = &_lbffBayerOuterNode;
    link->srcTerminalId = 13;
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
    link->srcTerminalId = 16;
    link->dest = GraphElementType::ImagePpp;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[16];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 14;
    link->dest = GraphElementType::SwImv;
    link->destNode = &_swImvOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[17];
    link->src = GraphElementType::BbpsWithTnr;
    link->srcNode = &_bbpsWithTnrOuterNode;
    link->srcTerminalId = 15;
    link->dest = GraphElementType::SwImv;
    link->destNode = &_swImvOuterNode;
    link->destTerminalId = 0;
    link->type = LinkType::Node2Node;

    link = &_graphLinks[18];
    link->src = GraphElementType::SwImv;
    link->srcNode = &_swImvOuterNode;
    link->srcTerminalId = 1;
    link->dest = GraphElementType::ProcessedMain;
    link->type = LinkType::Node2Sink;

    link = &_graphLinks[19];
    link->src = GraphElementType::SwImv;
    link->srcNode = &_swImvOuterNode;
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

    for (uint8_t i = 0; i < 21; ++i)
    {
        // apply link configuration. select configuration with maximal size
        uint32_t selectedLinkConfig = 0;
        uint32_t maxSize = _graphConfigurations[0].linkConfigurations[i].bufferSize;
        for (uint32_t j = 1; j < kernelConfigurationsOptionsCount; j++)
        {
            if (_graphConfigurations[j].linkConfigurations[i].bufferSize > maxSize)
            {
                maxSize = _graphConfigurations[j].linkConfigurations[i].bufferSize;
                selectedLinkConfig = j;
            }
        }
        _graphLinks[i].linkConfiguration = &_graphConfigurations[selectedLinkConfig].linkConfigurations[i];

        // Assign link to sub-graph
        _imageSubGraph.links[i] = &_graphLinks[i];
    }

    // add nodes for sub graph
    _imageSubGraph.isysOuterNode = &_isysOuterNode;
    _imageSubGraph.lbffBayerOuterNode = &_lbffBayerOuterNode;
    _imageSubGraph.bbpsWithTnrOuterNode = &_bbpsWithTnrOuterNode;
    _imageSubGraph.swImvOuterNode = &_swImvOuterNode;
    _imageSubGraph.swScalerOuterNode = &_swScalerOuterNode;

    // choose the selected sub graph
    _selectedGraphTopology = &_imageSubGraph;

    // logical node IDs
    _imageSubGraph.isysOuterNode->contextId = 0;
    _imageSubGraph.lbffBayerOuterNode->contextId = 1;
    _imageSubGraph.bbpsWithTnrOuterNode->contextId = 2;
    _imageSubGraph.swImvOuterNode->contextId = 3;
    _imageSubGraph.swScalerOuterNode->contextId = 4;
    // Apply a default inner nodes configuration for the selected sub graph
    SubGraphInnerNodeConfiguration defaultInnerNodeConfiguration;
    if(_selectedGraphTopology != nullptr)
    {
        _selectedGraphTopology->configInnerNodes(defaultInnerNodeConfiguration);
    }
}

StaticGraphStatus StaticGraph100042::updateConfiguration(uint32_t selectedIndex)
{
    StaticGraphStatus  res = StaticGraphStatus::SG_OK;
    res = _isysOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _lbffBayerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _bbpsWithTnrOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swImvOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    res = _swScalerOuterNode.UpdateKernelsSelectedConfiguration(selectedIndex);
    if (res != StaticGraphStatus::SG_OK)
    {
        return res;
    }
    return StaticGraphStatus::SG_OK;
}

StaticGraph100042::~StaticGraph100042()
{
    delete[] _graphConfigurations;
    delete _zoomKeyResolutions.zoomKeyResolutionOptions;
}

StaticGraphStatus imageSubGraphTopology100042::configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration)
{

    /*
     * Init sub graphs inner nodes configuration base on user request
     */
    InnerNodeOptionsFlags imagePublicInnerNodeConfiguration = GetInnerOptions(subGraphInnerNodeConfiguration.imageInnerOptions);

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
    if (
        subGraphLinks[13]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[16]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noMp;
    }
    if (
        subGraphLinks[14]->linkConfiguration->bufferSize == 0 &&
        subGraphLinks[17]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noDp;
    }
    if (
        subGraphLinks[15]->linkConfiguration->bufferSize == 0 &&
        true)
    {
        bbpsWithTnrInnerOptions |= noPpp;
    }

    /*
     * Configuring inner nodes according to the selected inner options
     */
    lbffBayerInnerOptions |= noLbOutputPs & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));
    lbffBayerInnerOptions |= noLbOutputMe & (-((imagePublicInnerNodeConfiguration & (noMp | noDp | noPpp)) == (noMp | noDp | noPpp)));

    /*
     * Set the selected inner nodes to the outer nodes
     */
    lbffBayerOuterNode->setInnerNode(lbffBayerInnerOptions);
    bbpsWithTnrOuterNode->setInnerNode(bbpsWithTnrInnerOptions);

    /*
     * Link enablement by public inner options
     */
    subGraphLinks[3]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_ae_output -> ae_out
    subGraphLinks[4]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_af_std_output -> af_std_out
    subGraphLinks[5]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_std_output -> awb_std_out
    subGraphLinks[6]->isActive = !(lbffBayerInnerOptions & no3A); // lbff_Bayer:terminal_connect_awb_sat_output -> awb_sat_out
    subGraphLinks[13]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> image_mp
    subGraphLinks[16]->isActive = !(bbpsWithTnrInnerOptions & noMp); // bbps_WithTnr:bbps_ofs_mp_yuvn_odr -> sw_imv:terminal_connect_input
    subGraphLinks[14]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> image_dp
    subGraphLinks[17]->isActive = !(bbpsWithTnrInnerOptions & noDp); // bbps_WithTnr:bbps_ofs_dp_yuvn_odr -> sw_imv:terminal_connect_input
    subGraphLinks[15]->isActive = !(bbpsWithTnrInnerOptions & noPpp); // bbps_WithTnr:bbps_ofs_pp_yuvn_odr -> image_ppp

    /*
     * Link enablement by private inner options
     */
    subGraphLinks[7]->isActive = !(lbffBayerInnerOptions & noLbOutputPs); // lbff_Bayer:terminal_connect_ps_output -> bbps_WithTnr:bbps_slim_spatial_yuvn_ifd
    subGraphLinks[8]->isActive = !(lbffBayerInnerOptions & noLbOutputMe); // lbff_Bayer:terminal_connect_me_output -> bbps_WithTnr:bbps_tnr_bc_yuv4n_ifd

    /*
     * Disable links with zero buffer size
     * (used for post processing when not all links are being used)
     */
     for (uint32_t i = 0; i < 21; i++)
     {
        if (subGraphLinks[i]->linkConfiguration->bufferSize == 0)
        {
            subGraphLinks[i]->isActive = false;
        }
     }

    /*
     * Link enablement by inner options combinations
     */
    subGraphLinks[9]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_blend_yuvn_odr -> bbps_WithTnr:bbps_slim_tnr_blend_yuvnm1_ifd
    subGraphLinks[10]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_scale_yuv4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_yuv4nm1_ifd
    subGraphLinks[11]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_tnr_blend_rs4n_ifd
    subGraphLinks[12]->isActive = (bbpsWithTnrInnerOptions & (noMp|noDp|noPpp)) != (noMp|noDp|noPpp); // bbps_WithTnr:bbps_tnr_bc_rs4n_odr -> bbps_WithTnr:bbps_slim_tnr_bc_rs4nm1_ifd

    return StaticGraphStatus::SG_OK;
}

