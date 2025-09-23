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

#ifndef STATIC_GRAPH_H
#define STATIC_GRAPH_H

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "Ipu75xaStaticGraphTypesAutogen.h"
#include "Ipu75xaStaticGraphBinaryAutogen.h"

#define SUPPORT_KEY_RESOLUTIONS 1

#define SUPPORT_FRAGMENTS 0

enum InnerNodeOption
{
    None = 0,
    noBurstCapture = (1 << 1),
    noIr = (1 << 2),
    noPdaf = (1 << 3),
    noLbOutputPs = (1 << 4),
    noLbOutputMe = (1 << 5),
    noGmv = (1 << 6),
    no3A = (1 << 7),
    noMp = (1 << 8),
    noDp = (1 << 9),
};
typedef int32_t InnerNodeOptionsFlags;

struct SubGraphPublicInnerNodeConfiguration {
    bool noGmv = false;
    bool no3A = false;
    bool noMp = false;
    bool noDp = false;
};

class OuterNode {
public:
    /**
     * \brief resourceId - represents the physical ID of the node, e.g. cb_id for CB node.
     */
    uint8_t resourceId = 0;

    /**
     * \brief contextId - represents the logical Id of the node according to the use-case.
     *                    Same physical nodes in given graph topology will have a different contextId
     */
    uint8_t contextId = 0;
    NodeTypes type = NodeTypes::Cb;
    HwBitmaps bitmaps;
    StaticGraphNodeKernels nodeKernels = {};

    uint8_t numberOfFragments = 0;
    OuterNode() {}
    ~OuterNode();

    void Init(uint8_t nodeResourceId,
        NodeTypes nodeType,
        uint32_t kernelCount,
        uint32_t nodeKernelConfigurationsOptionsCount,
        uint32_t operationMode,
        uint32_t streamId,
        uint8_t nodeNumberOfFragments);

    StaticGraphStatus UpdateKernelsSelectedConfiguration(uint32_t selectedIndex);

    uint8_t GetNumberOfFragments();

    void SetDisabledKernels(uint64_t disabledRunKernelsBitmap);

protected:
    void InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap, StaticGraphKernelRes* resolutionInfos, uint64_t kernelsResolutionHistoryGroupBitmap, StaticGraphKernelRes* resolutionHistories, StaticGraphKernelBppConfiguration* bppInfos, uint8_t* systemApisSizes, uint8_t* systemApiData);
    uint32_t kernelConfigurationsOptionsCount;
    uint32_t selectedKernelConfigurationIndex;
    StaticGraphPacRunKernel **kernelListOptions;
};

struct GraphLink {
    bool isActive = true;

    GraphElementType src;
    OuterNode* srcNode = nullptr;
    GraphElementType dest;
    OuterNode* destNode = nullptr;

    uint8_t srcTerminalId = 0;
    uint8_t destTerminalId = 0;

    FormatType format;
    LinkType type;
    uint8_t frameDelay = 0;

    StaticGraphLinkConfiguration* linkConfiguration = nullptr;
    StaticGraphLinkCompressionConfiguration* linkCompressionConfiguration = nullptr;
};

struct SubGraphInnerNodeConfiguration
{
    SubGraphPublicInnerNodeConfiguration* imageInnerOptions = nullptr;
    SubGraphPublicInnerNodeConfiguration* irInnerOptions = nullptr;
    SubGraphPublicInnerNodeConfiguration* rawInnerOptions = nullptr;
};

class GraphTopology {
public:
    GraphLink** links = nullptr;
    int32_t numOfLinks = 0;

    GraphTopology(GraphLink** links, int32_t numOfLinks, VirtualSinkMapping* sinkMappingConfiguration);
    virtual StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration);

protected:
    VirtualSinkMapping* _sinkMappingConfiguration = nullptr;
    static InnerNodeOptionsFlags GetInnerOptions(SubGraphPublicInnerNodeConfiguration* publicInnerOptions);

};

class IStaticGraphConfig
{
public:
    IStaticGraphConfig(SensorMode* selectedSensorMode, VirtualSinkMapping* sinkMappingConfiguration, int32_t graphId, int32_t selectedSettingsId, ZoomKeyResolutions* zoomKeyResolutions);
    virtual ~IStaticGraphConfig(){}
    StaticGraphStatus getGraphTopology(GraphTopology** topology);
    StaticGraphStatus getSensorMode(SensorMode** sensorMode);
    StaticGraphStatus getGraphId(int32_t* id);
    StaticGraphStatus getSettingsId(int32_t* id);
    StaticGraphStatus getVirtualSinkConnection(VirtualSink& virtualSink, HwSink* hwSink);
    StaticGraphStatus getZoomKeyResolutions(ZoomKeyResolutions** zoomKeyResolutions);
    virtual StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0) = 0;

protected:
    SensorMode* _selectedSensorMode = nullptr;
    ZoomKeyResolutions _zoomKeyResolutions;
    GraphTopology* _selectedGraphTopology = nullptr;
    VirtualSinkMapping* _sinkMappingConfiguration = &_selectedSinkMappingConfiguration;
private:
    int32_t _graphId;
    int32_t _settingsId;
    VirtualSinkMapping _selectedSinkMappingConfiguration;
};

#pragma pack(push, 4)

struct IsysOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct LbffBayerOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1537];
};

struct BbpsNoTnrOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[468];
};

struct LbffBayerWithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2005];
};

struct BbpsWithTnrOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[10];
    StaticGraphKernelBppConfiguration bppInfos[18];
    uint8_t systemApiConfiguration[1722];
};

struct SwGdcOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwScalerOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[5];
};

struct SwNntmOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[5];
};

struct LbffRgbIrOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1849];
};

struct LbffIrNoGmvIrStreamOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1537];
};

struct BbpsIrWithTnrOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[10];
    StaticGraphKernelBppConfiguration bppInfos[18];
    uint8_t systemApiConfiguration[1722];
};

struct LbffBayerBurstOutNo3AOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1652];
};

struct BbpsIrNoTnrOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[468];
};

struct LbffIrNoGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1537];
};

struct IsysPdaf2OuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffBayerPdaf2OuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[1881];
};

struct LbffBayerPdaf3OuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1725];
};

struct IsysDolOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffDol2InputsOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1854];
};

struct LbffDolSmoothOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[7];
    uint8_t systemApiConfiguration[327];
};

struct LbffDol3InputsOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2010];
};

struct LbffBayerPdaf2WithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2349];
};

struct LbffBayerPdaf3WithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2193];
};

struct LbffRgbIrWithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2317];
};

struct LbffIrWithGmvIrStreamOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2005];
};

struct LbffDol2InputsWithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2322];
};

struct LbffDol3InputsWithGmvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2478];
};

struct SwB2bOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwImvOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
    uint8_t systemApiConfiguration[5];
};

struct SwRemosaicOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[5];
};

struct LbffDol2InputsBayerStatOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1854];
};

struct LbffDol3InputsBayerStatOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2010];
};

struct LbffDol2InputsWithGmvBayerStatOuterNodeConfiguration
{
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2322];
};

struct GraphConfiguration100000
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100001
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvOuterNodeConfiguration lbffBayerWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100002
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100003
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvOuterNodeConfiguration lbffBayerWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100005
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100006
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrOuterNodeConfiguration lbffRgbIrOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    LbffIrNoGmvIrStreamOuterNodeConfiguration lbffIrNoGmvIrStreamOuterNodeConfiguration;
    BbpsIrWithTnrOuterNodeConfiguration bbpsIrWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100007
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerBurstOutNo3AOuterNodeConfiguration lbffBayerBurstOutNo3AOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[3];
};

struct GraphConfiguration100008
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrOuterNodeConfiguration lbffRgbIrOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    LbffIrNoGmvIrStreamOuterNodeConfiguration lbffIrNoGmvIrStreamOuterNodeConfiguration;
    BbpsIrNoTnrOuterNodeConfiguration bbpsIrNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100015
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100016
{
    VirtualSinkMapping sinkMappingConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[3];
};

struct GraphConfiguration100025
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvOuterNodeConfiguration lbffIrNoGmvOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100026
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[2];
};

struct GraphConfiguration100027
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2OuterNodeConfiguration lbffBayerPdaf2OuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100028
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100029
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2OuterNodeConfiguration lbffBayerPdaf2OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100030
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100031
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsOuterNodeConfiguration lbffDol2InputsOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100032
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsOuterNodeConfiguration lbffDol2InputsOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100033
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsOuterNodeConfiguration lbffDol3InputsOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100034
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsOuterNodeConfiguration lbffDol3InputsOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100035
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100036
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100037
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvOuterNodeConfiguration lbffBayerPdaf2WithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100038
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvOuterNodeConfiguration lbffBayerPdaf3WithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100039
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvOuterNodeConfiguration lbffRgbIrWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    LbffIrWithGmvIrStreamOuterNodeConfiguration lbffIrWithGmvIrStreamOuterNodeConfiguration;
    BbpsIrWithTnrOuterNodeConfiguration bbpsIrWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[34];
};

struct GraphConfiguration100040
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvOuterNodeConfiguration lbffDol2InputsWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100041
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvOuterNodeConfiguration lbffDol3InputsWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100042
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100044
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100050
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100051
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100052
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsBayerStatOuterNodeConfiguration lbffDol2InputsBayerStatOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100053
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsBayerStatOuterNodeConfiguration lbffDol2InputsBayerStatOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100054
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsBayerStatOuterNodeConfiguration lbffDol3InputsBayerStatOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100055
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsBayerStatOuterNodeConfiguration lbffDol3InputsBayerStatOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100056
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvBayerStatOuterNodeConfiguration lbffDol2InputsWithGmvBayerStatOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100057
{
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvOuterNodeConfiguration lbffDol3InputsWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};
#pragma pack(pop)

class IsysOuterNode : public OuterNode
{
public:
    IsysOuterNode(): OuterNode(){}
    void Init(IsysOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerOuterNode : public OuterNode
{
public:
    LbffBayerOuterNode(): OuterNode(){}
    void Init(LbffBayerOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class BbpsNoTnrOuterNode : public OuterNode
{
public:
    BbpsNoTnrOuterNode(): OuterNode(){}
    void Init(BbpsNoTnrOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerWithGmvOuterNode : public OuterNode
{
public:
    LbffBayerWithGmvOuterNode(): OuterNode(){}
    void Init(LbffBayerWithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class BbpsWithTnrOuterNode : public OuterNode
{
public:
    BbpsWithTnrOuterNode(): OuterNode(){}
    void Init(BbpsWithTnrOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwGdcOuterNode : public OuterNode
{
public:
    SwGdcOuterNode(): OuterNode(){}
    void Init(SwGdcOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwScalerOuterNode : public OuterNode
{
public:
    SwScalerOuterNode(): OuterNode(){}
    void Init(SwScalerOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwNntmOuterNode : public OuterNode
{
public:
    SwNntmOuterNode(): OuterNode(){}
    void Init(SwNntmOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffRgbIrOuterNode : public OuterNode
{
public:
    LbffRgbIrOuterNode(): OuterNode(){}
    void Init(LbffRgbIrOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffIrNoGmvIrStreamOuterNode : public OuterNode
{
public:
    LbffIrNoGmvIrStreamOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvIrStreamOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class BbpsIrWithTnrOuterNode : public OuterNode
{
public:
    BbpsIrWithTnrOuterNode(): OuterNode(){}
    void Init(BbpsIrWithTnrOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerBurstOutNo3AOuterNode : public OuterNode
{
public:
    LbffBayerBurstOutNo3AOuterNode(): OuterNode(){}
    void Init(LbffBayerBurstOutNo3AOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class BbpsIrNoTnrOuterNode : public OuterNode
{
public:
    BbpsIrNoTnrOuterNode(): OuterNode(){}
    void Init(BbpsIrNoTnrOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffIrNoGmvOuterNode : public OuterNode
{
public:
    LbffIrNoGmvOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class IsysPdaf2OuterNode : public OuterNode
{
public:
    IsysPdaf2OuterNode(): OuterNode(){}
    void Init(IsysPdaf2OuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerPdaf2OuterNode : public OuterNode
{
public:
    LbffBayerPdaf2OuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2OuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerPdaf3OuterNode : public OuterNode
{
public:
    LbffBayerPdaf3OuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3OuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class IsysDolOuterNode : public OuterNode
{
public:
    IsysDolOuterNode(): OuterNode(){}
    void Init(IsysDolOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol2InputsOuterNode : public OuterNode
{
public:
    LbffDol2InputsOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDolSmoothOuterNode : public OuterNode
{
public:
    LbffDolSmoothOuterNode(): OuterNode(){}
    void Init(LbffDolSmoothOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol3InputsOuterNode : public OuterNode
{
public:
    LbffDol3InputsOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerPdaf2WithGmvOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithGmvOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffBayerPdaf3WithGmvOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3WithGmvOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3WithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffRgbIrWithGmvOuterNode : public OuterNode
{
public:
    LbffRgbIrWithGmvOuterNode(): OuterNode(){}
    void Init(LbffRgbIrWithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffIrWithGmvIrStreamOuterNode : public OuterNode
{
public:
    LbffIrWithGmvIrStreamOuterNode(): OuterNode(){}
    void Init(LbffIrWithGmvIrStreamOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol2InputsWithGmvOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol3InputsWithGmvOuterNode : public OuterNode
{
public:
    LbffDol3InputsWithGmvOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsWithGmvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwB2bOuterNode : public OuterNode
{
public:
    SwB2bOuterNode(): OuterNode(){}
    void Init(SwB2bOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwImvOuterNode : public OuterNode
{
public:
    SwImvOuterNode(): OuterNode(){}
    void Init(SwImvOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class SwRemosaicOuterNode : public OuterNode
{
public:
    SwRemosaicOuterNode(): OuterNode(){}
    void Init(SwRemosaicOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol2InputsBayerStatOuterNode : public OuterNode
{
public:
    LbffDol2InputsBayerStatOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsBayerStatOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol3InputsBayerStatOuterNode : public OuterNode
{
public:
    LbffDol3InputsBayerStatOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsBayerStatOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};
class LbffDol2InputsWithGmvBayerStatOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvBayerStatOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvBayerStatOuterNodeConfiguration** selectedGraphConfigurations, uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);

};

class imageSubGraphTopology100000 : public GraphTopology {

public:
    imageSubGraphTopology100000(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[10];

};

class StaticGraph100000 : public IStaticGraphConfig
{
public:
    StaticGraph100000(GraphConfiguration100000** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100000();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2191290611;  // autogenerated

private:
    // Configuration
    GraphConfiguration100000* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100000 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[10];
};

class imageSubGraphTopology100001 : public GraphTopology {

public:
    imageSubGraphTopology100001(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvOuterNode* lbffBayerWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100001 : public IStaticGraphConfig
{
public:
    StaticGraph100001(GraphConfiguration100001** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100001();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 911553637;  // autogenerated

private:
    // Configuration
    GraphConfiguration100001* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvOuterNode _lbffBayerWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100001 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100002 : public GraphTopology {

public:
    imageSubGraphTopology100002(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class StaticGraph100002 : public IStaticGraphConfig
{
public:
    StaticGraph100002(GraphConfiguration100002** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100002();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2257098455;  // autogenerated

private:
    // Configuration
    GraphConfiguration100002* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100002 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100003 : public GraphTopology {

public:
    imageSubGraphTopology100003(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvOuterNode* lbffBayerWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100003 : public IStaticGraphConfig
{
public:
    StaticGraph100003(GraphConfiguration100003** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100003();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 835453801;  // autogenerated

private:
    // Configuration
    GraphConfiguration100003* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvOuterNode _lbffBayerWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100003 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100005 : public GraphTopology {

public:
    imageSubGraphTopology100005(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100005 : public IStaticGraphConfig
{
public:
    StaticGraph100005(GraphConfiguration100005** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100005();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 149312985;  // autogenerated

private:
    // Configuration
    GraphConfiguration100005* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100005 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100006 : public GraphTopology {

public:
    imageSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class irSubGraphTopology100006 : public GraphTopology {

public:
    irSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class image_irSubGraphTopology100006 : public GraphTopology {

public:
    image_irSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

};

class StaticGraph100006 : public IStaticGraphConfig
{
public:
    StaticGraph100006(GraphConfiguration100006** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100006();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2296144571;  // autogenerated

private:
    // Configuration
    GraphConfiguration100006* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrOuterNode _lbffRgbIrOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    LbffIrNoGmvIrStreamOuterNode _lbffIrNoGmvIrStreamOuterNode;
    BbpsIrWithTnrOuterNode _bbpsIrWithTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100006 _imageSubGraph;
    irSubGraphTopology100006 _irSubGraph;
    image_irSubGraphTopology100006 _image_irSubGraph;

    // All graph links
    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100007 : public GraphTopology {

public:
    imageSubGraphTopology100007(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 3, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerBurstOutNo3AOuterNode* lbffBayerBurstOutNo3AOuterNode = nullptr;
    GraphLink* subGraphLinks[3];

};

class StaticGraph100007 : public IStaticGraphConfig
{
public:
    StaticGraph100007(GraphConfiguration100007** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100007();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1987263145;  // autogenerated

private:
    // Configuration
    GraphConfiguration100007* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerBurstOutNo3AOuterNode _lbffBayerBurstOutNo3AOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100007 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[3];
};

class imageSubGraphTopology100008 : public GraphTopology {

public:
    imageSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class irSubGraphTopology100008 : public GraphTopology {

public:
    irSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrNoTnrOuterNode* bbpsIrNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class image_irSubGraphTopology100008 : public GraphTopology {

public:
    image_irSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrNoTnrOuterNode* bbpsIrNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100008 : public IStaticGraphConfig
{
public:
    StaticGraph100008(GraphConfiguration100008** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100008();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 4070369259;  // autogenerated

private:
    // Configuration
    GraphConfiguration100008* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrOuterNode _lbffRgbIrOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;
    LbffIrNoGmvIrStreamOuterNode _lbffIrNoGmvIrStreamOuterNode;
    BbpsIrNoTnrOuterNode _bbpsIrNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100008 _imageSubGraph;
    irSubGraphTopology100008 _irSubGraph;
    image_irSubGraphTopology100008 _image_irSubGraph;

    // All graph links
    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100015 : public GraphTopology {

public:
    imageSubGraphTopology100015(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    GraphLink* subGraphLinks[9];

};

class StaticGraph100015 : public IStaticGraphConfig
{
public:
    StaticGraph100015(GraphConfiguration100015** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100015();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 3321236285;  // autogenerated

private:
    // Configuration
    GraphConfiguration100015* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100015 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[9];
};

class imageSubGraphTopology100016 : public GraphTopology {

public:
    imageSubGraphTopology100016(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 3, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[3];

};

class StaticGraph100016 : public IStaticGraphConfig
{
public:
    StaticGraph100016(GraphConfiguration100016** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100016();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 3498640191;  // autogenerated

private:
    // Configuration
    GraphConfiguration100016* _graphConfigurations;

    /* Outer Nodes */
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100016 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[3];
};

class imageSubGraphTopology100025 : public GraphTopology {

public:
    imageSubGraphTopology100025(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvOuterNode* lbffIrNoGmvOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[10];

};

class StaticGraph100025 : public IStaticGraphConfig
{
public:
    StaticGraph100025(GraphConfiguration100025** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100025();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2191290611;  // autogenerated

private:
    // Configuration
    GraphConfiguration100025* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffIrNoGmvOuterNode _lbffIrNoGmvOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100025 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[10];
};

class rawSubGraphTopology100026 : public GraphTopology {

public:
    rawSubGraphTopology100026(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 2, sinkMappingConfiguration) {}
    IsysOuterNode* isysOuterNode = nullptr;
    GraphLink* subGraphLinks[2];

};

class StaticGraph100026 : public IStaticGraphConfig
{
public:
    StaticGraph100026(GraphConfiguration100026** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100026();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 4190204929;  // autogenerated

private:
    // Configuration
    GraphConfiguration100026* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100026 _rawSubGraph;

    // All graph links
    GraphLink _graphLinks[2];
};

class imageSubGraphTopology100027 : public GraphTopology {

public:
    imageSubGraphTopology100027(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2OuterNode* lbffBayerPdaf2OuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class StaticGraph100027 : public IStaticGraphConfig
{
public:
    StaticGraph100027(GraphConfiguration100027** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100027();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2863511927;  // autogenerated

private:
    // Configuration
    GraphConfiguration100027* _graphConfigurations;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2OuterNode _lbffBayerPdaf2OuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100027 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[13];
};

class imageSubGraphTopology100028 : public GraphTopology {

public:
    imageSubGraphTopology100028(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100028 : public IStaticGraphConfig
{
public:
    StaticGraph100028(GraphConfiguration100028** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100028();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 297092049;  // autogenerated

private:
    // Configuration
    GraphConfiguration100028* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3OuterNode _lbffBayerPdaf3OuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100028 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[11];
};

class imageSubGraphTopology100029 : public GraphTopology {

public:
    imageSubGraphTopology100029(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2OuterNode* lbffBayerPdaf2OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100029 : public IStaticGraphConfig
{
public:
    StaticGraph100029(GraphConfiguration100029** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100029();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1956517507;  // autogenerated

private:
    // Configuration
    GraphConfiguration100029* _graphConfigurations;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2OuterNode _lbffBayerPdaf2OuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100029 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100030 : public GraphTopology {

public:
    imageSubGraphTopology100030(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100030 : public IStaticGraphConfig
{
public:
    StaticGraph100030(GraphConfiguration100030** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100030();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2435910845;  // autogenerated

private:
    // Configuration
    GraphConfiguration100030* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3OuterNode _lbffBayerPdaf3OuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100030 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100031 : public GraphTopology {

public:
    imageSubGraphTopology100031(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsOuterNode* lbffDol2InputsOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100031 : public IStaticGraphConfig
{
public:
    StaticGraph100031(GraphConfiguration100031** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100031();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1681200543;  // autogenerated

private:
    // Configuration
    GraphConfiguration100031* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsOuterNode _lbffDol2InputsOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100031 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100032 : public GraphTopology {

public:
    imageSubGraphTopology100032(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsOuterNode* lbffDol2InputsOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100032 : public IStaticGraphConfig
{
public:
    StaticGraph100032(GraphConfiguration100032** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100032();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 611075083;  // autogenerated

private:
    // Configuration
    GraphConfiguration100032* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsOuterNode _lbffDol2InputsOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100032 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100033 : public GraphTopology {

public:
    imageSubGraphTopology100033(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsOuterNode* lbffDol3InputsOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100033 : public IStaticGraphConfig
{
public:
    StaticGraph100033(GraphConfiguration100033** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100033();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1734304817;  // autogenerated

private:
    // Configuration
    GraphConfiguration100033* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsOuterNode _lbffDol3InputsOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100033 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100034 : public GraphTopology {

public:
    imageSubGraphTopology100034(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsOuterNode* lbffDol3InputsOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100034 : public IStaticGraphConfig
{
public:
    StaticGraph100034(GraphConfiguration100034** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100034();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1995162237;  // autogenerated

private:
    // Configuration
    GraphConfiguration100034* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsOuterNode _lbffDol3InputsOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100034 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[25];
};

class rawSubGraphTopology100035 : public GraphTopology {

public:
    rawSubGraphTopology100035(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}
    IsysDolOuterNode* isysDolOuterNode = nullptr;
    GraphLink* subGraphLinks[4];

};

class StaticGraph100035 : public IStaticGraphConfig
{
public:
    StaticGraph100035(GraphConfiguration100035** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100035();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1527132867;  // autogenerated

private:
    // Configuration
    GraphConfiguration100035* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100035 _rawSubGraph;

    // All graph links
    GraphLink _graphLinks[4];
};

class rawSubGraphTopology100036 : public GraphTopology {

public:
    rawSubGraphTopology100036(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}
    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    GraphLink* subGraphLinks[4];

};

class StaticGraph100036 : public IStaticGraphConfig
{
public:
    StaticGraph100036(GraphConfiguration100036** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100036();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1527132867;  // autogenerated

private:
    // Configuration
    GraphConfiguration100036* _graphConfigurations;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100036 _rawSubGraph;

    // All graph links
    GraphLink _graphLinks[4];
};

class imageSubGraphTopology100037 : public GraphTopology {

public:
    imageSubGraphTopology100037(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvOuterNode* lbffBayerPdaf2WithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100037 : public IStaticGraphConfig
{
public:
    StaticGraph100037(GraphConfiguration100037** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100037();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2427208877;  // autogenerated

private:
    // Configuration
    GraphConfiguration100037* _graphConfigurations;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvOuterNode _lbffBayerPdaf2WithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100037 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100038 : public GraphTopology {

public:
    imageSubGraphTopology100038(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvOuterNode* lbffBayerPdaf3WithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100038 : public IStaticGraphConfig
{
public:
    StaticGraph100038(GraphConfiguration100038** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100038();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1204583923;  // autogenerated

private:
    // Configuration
    GraphConfiguration100038* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3WithGmvOuterNode _lbffBayerPdaf3WithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100038 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100039 : public GraphTopology {

public:
    imageSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class irSubGraphTopology100039 : public GraphTopology {

public:
    irSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    LbffIrWithGmvIrStreamOuterNode* lbffIrWithGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class image_irSubGraphTopology100039 : public GraphTopology {

public:
    image_irSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 34, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffIrWithGmvIrStreamOuterNode* lbffIrWithGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[34];

};

class StaticGraph100039 : public IStaticGraphConfig
{
public:
    StaticGraph100039(GraphConfiguration100039** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100039();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 3381848299;  // autogenerated

private:
    // Configuration
    GraphConfiguration100039* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrWithGmvOuterNode _lbffRgbIrWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    LbffIrWithGmvIrStreamOuterNode _lbffIrWithGmvIrStreamOuterNode;
    BbpsIrWithTnrOuterNode _bbpsIrWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100039 _imageSubGraph;
    irSubGraphTopology100039 _irSubGraph;
    image_irSubGraphTopology100039 _image_irSubGraph;

    // All graph links
    GraphLink _graphLinks[34];
};

class imageSubGraphTopology100040 : public GraphTopology {

public:
    imageSubGraphTopology100040(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvOuterNode* lbffDol2InputsWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100040 : public IStaticGraphConfig
{
public:
    StaticGraph100040(GraphConfiguration100040** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100040();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 288441259;  // autogenerated

private:
    // Configuration
    GraphConfiguration100040* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvOuterNode _lbffDol2InputsWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100040 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100041 : public GraphTopology {

public:
    imageSubGraphTopology100041(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvOuterNode* lbffDol3InputsWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100041 : public IStaticGraphConfig
{
public:
    StaticGraph100041(GraphConfiguration100041** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100041();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 4095848493;  // autogenerated

private:
    // Configuration
    GraphConfiguration100041* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvOuterNode _lbffDol3InputsWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100041 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100042 : public GraphTopology {

public:
    imageSubGraphTopology100042(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100042 : public IStaticGraphConfig
{
public:
    StaticGraph100042(GraphConfiguration100042** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100042();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2207768899;  // autogenerated

private:
    // Configuration
    GraphConfiguration100042* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3OuterNode _lbffBayerPdaf3OuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100042 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100044 : public GraphTopology {

public:
    imageSubGraphTopology100044(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100044 : public IStaticGraphConfig
{
public:
    StaticGraph100044(GraphConfiguration100044** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100044();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2798404829;  // autogenerated

private:
    // Configuration
    GraphConfiguration100044* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100044 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100050 : public GraphTopology {

public:
    imageSubGraphTopology100050(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100050 : public IStaticGraphConfig
{
public:
    StaticGraph100050(GraphConfiguration100050** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100050();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 3442380973;  // autogenerated

private:
    // Configuration
    GraphConfiguration100050* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwImvOuterNode _swImvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100050 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100051 : public GraphTopology {

public:
    imageSubGraphTopology100051(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100051 : public IStaticGraphConfig
{
public:
    StaticGraph100051(GraphConfiguration100051** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100051();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 2326634835;  // autogenerated

private:
    // Configuration
    GraphConfiguration100051* _graphConfigurations;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerOuterNode _lbffBayerOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100051 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100052 : public GraphTopology {

public:
    imageSubGraphTopology100052(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsBayerStatOuterNode* lbffDol2InputsBayerStatOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100052 : public IStaticGraphConfig
{
public:
    StaticGraph100052(GraphConfiguration100052** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100052();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1681200543;  // autogenerated

private:
    // Configuration
    GraphConfiguration100052* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsBayerStatOuterNode _lbffDol2InputsBayerStatOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100052 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100053 : public GraphTopology {

public:
    imageSubGraphTopology100053(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsBayerStatOuterNode* lbffDol2InputsBayerStatOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100053 : public IStaticGraphConfig
{
public:
    StaticGraph100053(GraphConfiguration100053** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100053();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 611075083;  // autogenerated

private:
    // Configuration
    GraphConfiguration100053* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsBayerStatOuterNode _lbffDol2InputsBayerStatOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100053 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100054 : public GraphTopology {

public:
    imageSubGraphTopology100054(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsBayerStatOuterNode* lbffDol3InputsBayerStatOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100054 : public IStaticGraphConfig
{
public:
    StaticGraph100054(GraphConfiguration100054** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100054();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1734304817;  // autogenerated

private:
    // Configuration
    GraphConfiguration100054* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsBayerStatOuterNode _lbffDol3InputsBayerStatOuterNode;
    BbpsNoTnrOuterNode _bbpsNoTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100054 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100055 : public GraphTopology {

public:
    imageSubGraphTopology100055(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsBayerStatOuterNode* lbffDol3InputsBayerStatOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100055 : public IStaticGraphConfig
{
public:
    StaticGraph100055(GraphConfiguration100055** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100055();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 1995162237;  // autogenerated

private:
    // Configuration
    GraphConfiguration100055* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsBayerStatOuterNode _lbffDol3InputsBayerStatOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100055 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100056 : public GraphTopology {

public:
    imageSubGraphTopology100056(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvBayerStatOuterNode* lbffDol2InputsWithGmvBayerStatOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100056 : public IStaticGraphConfig
{
public:
    StaticGraph100056(GraphConfiguration100056** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100056();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 288441259;  // autogenerated

private:
    // Configuration
    GraphConfiguration100056* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvBayerStatOuterNode _lbffDol2InputsWithGmvBayerStatOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100056 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100057 : public GraphTopology {

public:
    imageSubGraphTopology100057(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvOuterNode* lbffDol3InputsWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100057 : public IStaticGraphConfig
{
public:
    StaticGraph100057(GraphConfiguration100057** selectedGraphConfiguration, uint32_t kernelConfigurationsOptionsCount, ZoomKeyResolutions* zoomKeyResolutions, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId);
    ~StaticGraph100057();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex=0);
    static const uint32_t hashCode = 4095848493;  // autogenerated

private:
    // Configuration
    GraphConfiguration100057* _graphConfigurations;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvOuterNode _lbffDol3InputsWithGmvOuterNode;
    BbpsWithTnrOuterNode _bbpsWithTnrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100057 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[25];
};

#endif