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

#ifndef STATIC_GRAPH_H
#define STATIC_GRAPH_H

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "StaticGraphTypesAutogen.h"
#include "StaticGraphBinaryAutogen.h"

#define SUPPORT_KEY_RESOLUTIONS 1

enum InnerNodeOption {
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
    uint8_t resourceId;

    /**
     * \brief contextId - represents the logical Id of the node according to the use-case.
     *                    Same physical nodes in given graph topology will have a different
     * contextId
     */
    uint8_t contextId = 0;
    NodeTypes type;
    HwBitmaps bitmaps;
    StaticGraphNodeKernels nodeKernels;

    uint8_t numberOfFragments;
    OuterNode() {}
    ~OuterNode();

    void Init(uint8_t nodeResourceId, NodeTypes nodeType, uint32_t kernelCount,
              uint32_t nodeKernelConfigurationsOptionsCount, uint32_t operationMode,
              uint32_t streamId, uint8_t nodeNumberOfFragments);

    StaticGraphStatus UpdateKernelsSelectedConfiguration(uint32_t selectedIndex);

    uint8_t GetNumberOfFragments();

    void SetDisabledKernels(uint64_t disabledRunKernelsBitmap);

 protected:
    void InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap,
                        StaticGraphKernelRes* resolutionInfos,
                        uint64_t kernelsResolutionHistoryGroupBitmap,
                        StaticGraphKernelRes* resolutionHistories,
                        StaticGraphKernelBppConfiguration* bppInfos, uint8_t* systemApisSizes,
                        uint8_t* systemApiData);
    uint32_t kernelConfigurationsOptionsCount;
    uint32_t selectedKernelConfigurationIndex;
    StaticGraphPacRunKernel** kernelListOptions;
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

struct SubGraphInnerNodeConfiguration {
    SubGraphPublicInnerNodeConfiguration* imageInnerOptions = nullptr;
    SubGraphPublicInnerNodeConfiguration* irInnerOptions = nullptr;
    SubGraphPublicInnerNodeConfiguration* rawInnerOptions = nullptr;
};

class GraphTopology {
 public:
    GraphLink** links = nullptr;
    int32_t numOfLinks = 0;

    GraphTopology(GraphLink** links, int32_t numOfLinks,
                  VirtualSinkMapping* sinkMappingConfiguration);
    virtual StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration);

 protected:
    VirtualSinkMapping* _sinkMappingConfiguration = nullptr;
    static InnerNodeOptionsFlags GetInnerOptions(
        SubGraphPublicInnerNodeConfiguration* publicInnerOptions);
};

class IStaticGraphConfig {
 public:
    IStaticGraphConfig(SensorMode* selectedSensorMode, VirtualSinkMapping* sinkMappingConfiguration,
                       int32_t graphId, int32_t selectedSettingsId,
                       ZoomKeyResolutions* zoomKeyResolutions);
    virtual ~IStaticGraphConfig() {}
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

struct IsysOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct LbffBayerOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1532];
};

struct BbpsNoTnrOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[468];
};

struct LbffBayerWithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2000];
};

struct BbpsWithTnrOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[10];
    StaticGraphKernelBppConfiguration bppInfos[18];
    uint8_t systemApiConfiguration[1722];
};

struct SwGdcOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwScalerOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwNntmOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[5];
};

struct LbffRgbIrOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1844];
};

struct LbffIrNoGmvIrStreamOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1532];
};

struct BbpsIrWithTnrOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[10];
    StaticGraphKernelBppConfiguration bppInfos[18];
    uint8_t systemApiConfiguration[1722];
};

struct LbffBayerBurstOutNo3AOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1647];
};

struct BbpsIrNoTnrOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[468];
};

struct LbffIrNoGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[11];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[31];
    uint8_t systemApiConfiguration[1532];
};

struct IsysPdaf2OuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffBayerPdaf2OuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[1876];
};

struct LbffBayerPdaf3OuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1720];
};

struct IsysDolOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffDol2InputsOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[1849];
};

struct LbffDolSmoothOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[7];
    uint8_t systemApiConfiguration[327];
};

struct LbffDol3InputsOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2005];
};

struct LbffBayerPdaf2WithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2344];
};

struct LbffBayerPdaf3WithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2188];
};

struct LbffRgbIrWithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2312];
};

struct LbffIrWithGmvIrStreamOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[12];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[35];
    uint8_t systemApiConfiguration[2000];
};

struct LbffDol2InputsWithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[13];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2317];
};

struct LbffDol3InputsWithGmvOuterNodeConfiguration {
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[14];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2473];
};

struct GraphConfiguration100000 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100001 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvOuterNodeConfiguration lbffBayerWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100002 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100003 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvOuterNodeConfiguration lbffBayerWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100005 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100006 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrOuterNodeConfiguration lbffRgbIrOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    LbffIrNoGmvIrStreamOuterNodeConfiguration lbffIrNoGmvIrStreamOuterNodeConfiguration;
    BbpsIrWithTnrOuterNodeConfiguration bbpsIrWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100007 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerBurstOutNo3AOuterNodeConfiguration lbffBayerBurstOutNo3AOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[3];
};

struct GraphConfiguration100008 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrOuterNodeConfiguration lbffRgbIrOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    LbffIrNoGmvIrStreamOuterNodeConfiguration lbffIrNoGmvIrStreamOuterNodeConfiguration;
    BbpsIrNoTnrOuterNodeConfiguration bbpsIrNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100015 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerOuterNodeConfiguration lbffBayerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100016 {
    VirtualSinkMapping sinkMappingConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[3];
};

struct GraphConfiguration100025 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvOuterNodeConfiguration lbffIrNoGmvOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100026 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[2];
};

struct GraphConfiguration100027 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2OuterNodeConfiguration lbffBayerPdaf2OuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100028 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100029 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2OuterNodeConfiguration lbffBayerPdaf2OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100030 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100031 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsOuterNodeConfiguration lbffDol2InputsOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100032 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsOuterNodeConfiguration lbffDol2InputsOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100033 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsOuterNodeConfiguration lbffDol3InputsOuterNodeConfiguration;
    BbpsNoTnrOuterNodeConfiguration bbpsNoTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100034 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsOuterNodeConfiguration lbffDol3InputsOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100035 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100036 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100037 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvOuterNodeConfiguration lbffBayerPdaf2WithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100038 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvOuterNodeConfiguration lbffBayerPdaf3WithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100039 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvOuterNodeConfiguration lbffRgbIrWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    LbffIrWithGmvIrStreamOuterNodeConfiguration lbffIrWithGmvIrStreamOuterNodeConfiguration;
    BbpsIrWithTnrOuterNodeConfiguration bbpsIrWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[34];
};

struct GraphConfiguration100040 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvOuterNodeConfiguration lbffDol2InputsWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100041 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvOuterNodeConfiguration lbffDol3InputsWithGmvOuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100042 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3OuterNodeConfiguration lbffBayerPdaf3OuterNodeConfiguration;
    BbpsWithTnrOuterNodeConfiguration bbpsWithTnrOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};
#pragma pack(pop)

class IsysOuterNode : public OuterNode {
 public:
    IsysOuterNode() : OuterNode() {}
    void Init(IsysOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerOuterNode : public OuterNode {
 public:
    LbffBayerOuterNode() : OuterNode() {}
    void Init(LbffBayerOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class BbpsNoTnrOuterNode : public OuterNode {
 public:
    BbpsNoTnrOuterNode() : OuterNode() {}
    void Init(BbpsNoTnrOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerWithGmvOuterNode : public OuterNode {
 public:
    LbffBayerWithGmvOuterNode() : OuterNode() {}
    void Init(LbffBayerWithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class BbpsWithTnrOuterNode : public OuterNode {
 public:
    BbpsWithTnrOuterNode() : OuterNode() {}
    void Init(BbpsWithTnrOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwGdcOuterNode : public OuterNode {
 public:
    SwGdcOuterNode() : OuterNode() {}
    void Init(SwGdcOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwScalerOuterNode : public OuterNode {
 public:
    SwScalerOuterNode() : OuterNode() {}
    void Init(SwScalerOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwNntmOuterNode : public OuterNode {
 public:
    SwNntmOuterNode() : OuterNode() {}
    void Init(SwNntmOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrOuterNode : public OuterNode {
 public:
    LbffRgbIrOuterNode() : OuterNode() {}
    void Init(LbffRgbIrOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvIrStreamOuterNode : public OuterNode {
 public:
    LbffIrNoGmvIrStreamOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvIrStreamOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class BbpsIrWithTnrOuterNode : public OuterNode {
 public:
    BbpsIrWithTnrOuterNode() : OuterNode() {}
    void Init(BbpsIrWithTnrOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerBurstOutNo3AOuterNode : public OuterNode {
 public:
    LbffBayerBurstOutNo3AOuterNode() : OuterNode() {}
    void Init(LbffBayerBurstOutNo3AOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class BbpsIrNoTnrOuterNode : public OuterNode {
 public:
    BbpsIrNoTnrOuterNode() : OuterNode() {}
    void Init(BbpsIrNoTnrOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvOuterNode : public OuterNode {
 public:
    LbffIrNoGmvOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysPdaf2OuterNode : public OuterNode {
 public:
    IsysPdaf2OuterNode() : OuterNode() {}
    void Init(IsysPdaf2OuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2OuterNode : public OuterNode {
 public:
    LbffBayerPdaf2OuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2OuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3OuterNode : public OuterNode {
 public:
    LbffBayerPdaf3OuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3OuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysDolOuterNode : public OuterNode {
 public:
    IsysDolOuterNode() : OuterNode() {}
    void Init(IsysDolOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsOuterNode : public OuterNode {
 public:
    LbffDol2InputsOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDolSmoothOuterNode : public OuterNode {
 public:
    LbffDolSmoothOuterNode() : OuterNode() {}
    void Init(LbffDolSmoothOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsOuterNode : public OuterNode {
 public:
    LbffDol3InputsOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2WithGmvOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2WithGmvOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2WithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3WithGmvOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3WithGmvOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3WithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrWithGmvOuterNode : public OuterNode {
 public:
    LbffRgbIrWithGmvOuterNode() : OuterNode() {}
    void Init(LbffRgbIrWithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrWithGmvIrStreamOuterNode : public OuterNode {
 public:
    LbffIrWithGmvIrStreamOuterNode() : OuterNode() {}
    void Init(LbffIrWithGmvIrStreamOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsWithGmvOuterNode : public OuterNode {
 public:
    LbffDol2InputsWithGmvOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsWithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsWithGmvOuterNode : public OuterNode {
 public:
    LbffDol3InputsWithGmvOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsWithGmvOuterNodeConfiguration** selectedGraphConfigurations,
              uint32_t kernelConfigurationsOptionsCount);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};

class imageSubGraphTopology100000 : public GraphTopology {
 public:
    imageSubGraphTopology100000(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[10];
};

class StaticGraph100000 : public IStaticGraphConfig {
 public:
    StaticGraph100000(GraphConfiguration100000** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100000();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 254442951;  // autogenerated

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
    imageSubGraphTopology100001(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvOuterNode* lbffBayerWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];
};

class StaticGraph100001 : public IStaticGraphConfig {
 public:
    StaticGraph100001(GraphConfiguration100001** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100001();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1343209469;  // autogenerated

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
    imageSubGraphTopology100002(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[15];
};

class StaticGraph100002 : public IStaticGraphConfig {
 public:
    StaticGraph100002(GraphConfiguration100002** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100002();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3344665139;  // autogenerated

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
    imageSubGraphTopology100003(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvOuterNode* lbffBayerWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100003 : public IStaticGraphConfig {
 public:
    StaticGraph100003(GraphConfiguration100003** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100003();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1895591893;  // autogenerated

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
    imageSubGraphTopology100005(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100005 : public IStaticGraphConfig {
 public:
    StaticGraph100005(GraphConfiguration100005** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100005();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 218933513;  // autogenerated

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
    imageSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];
};

class irSubGraphTopology100006 : public GraphTopology {
 public:
    irSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[21];
};

class image_irSubGraphTopology100006 : public GraphTopology {
 public:
    image_irSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[29];
};

class StaticGraph100006 : public IStaticGraphConfig {
 public:
    StaticGraph100006(GraphConfiguration100006** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100006();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 936158723;  // autogenerated

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
    imageSubGraphTopology100007(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 3, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerBurstOutNo3AOuterNode* lbffBayerBurstOutNo3AOuterNode = nullptr;
    GraphLink* subGraphLinks[3];
};

class StaticGraph100007 : public IStaticGraphConfig {
 public:
    StaticGraph100007(GraphConfiguration100007** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100007();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 274832429;  // autogenerated

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
    imageSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[11];
};

class irSubGraphTopology100008 : public GraphTopology {
 public:
    irSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrNoTnrOuterNode* bbpsIrNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];
};

class image_irSubGraphTopology100008 : public GraphTopology {
 public:
    image_irSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrOuterNode* lbffRgbIrOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    LbffIrNoGmvIrStreamOuterNode* lbffIrNoGmvIrStreamOuterNode = nullptr;
    BbpsIrNoTnrOuterNode* bbpsIrNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class StaticGraph100008 : public IStaticGraphConfig {
 public:
    StaticGraph100008(GraphConfiguration100008** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100008();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3117872339;  // autogenerated

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
    imageSubGraphTopology100015(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerOuterNode* lbffBayerOuterNode = nullptr;
    GraphLink* subGraphLinks[9];
};

class StaticGraph100015 : public IStaticGraphConfig {
 public:
    StaticGraph100015(GraphConfiguration100015** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100015();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1370545417;  // autogenerated

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
    imageSubGraphTopology100016(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 3, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[3];
};

class StaticGraph100016 : public IStaticGraphConfig {
 public:
    StaticGraph100016(GraphConfiguration100016** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100016();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
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
    imageSubGraphTopology100025(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvOuterNode* lbffIrNoGmvOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[10];
};

class StaticGraph100025 : public IStaticGraphConfig {
 public:
    StaticGraph100025(GraphConfiguration100025** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100025();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 254442951;  // autogenerated

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
    rawSubGraphTopology100026(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 2, sinkMappingConfiguration) {}

    IsysOuterNode* isysOuterNode = nullptr;
    GraphLink* subGraphLinks[2];
};

class StaticGraph100026 : public IStaticGraphConfig {
 public:
    StaticGraph100026(GraphConfiguration100026** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100026();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
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
    imageSubGraphTopology100027(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2OuterNode* lbffBayerPdaf2OuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[13];
};

class StaticGraph100027 : public IStaticGraphConfig {
 public:
    StaticGraph100027(GraphConfiguration100027** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100027();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 58414011;  // autogenerated

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
    imageSubGraphTopology100028(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[11];
};

class StaticGraph100028 : public IStaticGraphConfig {
 public:
    StaticGraph100028(GraphConfiguration100028** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100028();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 690933501;  // autogenerated

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
    imageSubGraphTopology100029(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2OuterNode* lbffBayerPdaf2OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[18];
};

class StaticGraph100029 : public IStaticGraphConfig {
 public:
    StaticGraph100029(GraphConfiguration100029** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100029();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1083135791;  // autogenerated

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
    imageSubGraphTopology100030(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[16];
};

class StaticGraph100030 : public IStaticGraphConfig {
 public:
    StaticGraph100030(GraphConfiguration100030** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100030();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 802205825;  // autogenerated

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
    imageSubGraphTopology100031(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsOuterNode* lbffDol2InputsOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[18];
};

class StaticGraph100031 : public IStaticGraphConfig {
 public:
    StaticGraph100031(GraphConfiguration100031** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100031();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3403612335;  // autogenerated

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
    imageSubGraphTopology100032(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsOuterNode* lbffDol2InputsOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class StaticGraph100032 : public IStaticGraphConfig {
 public:
    StaticGraph100032(GraphConfiguration100032** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100032();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3102537819;  // autogenerated

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
    imageSubGraphTopology100033(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsOuterNode* lbffDol3InputsOuterNode = nullptr;
    BbpsNoTnrOuterNode* bbpsNoTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100033 : public IStaticGraphConfig {
 public:
    StaticGraph100033(GraphConfiguration100033** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100033();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3391467913;  // autogenerated

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
    imageSubGraphTopology100034(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsOuterNode* lbffDol3InputsOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];
};

class StaticGraph100034 : public IStaticGraphConfig {
 public:
    StaticGraph100034(GraphConfiguration100034** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100034();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3707454501;  // autogenerated

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
    rawSubGraphTopology100035(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    GraphLink* subGraphLinks[4];
};

class StaticGraph100035 : public IStaticGraphConfig {
 public:
    StaticGraph100035(GraphConfiguration100035** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100035();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
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
    rawSubGraphTopology100036(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    GraphLink* subGraphLinks[4];
};

class StaticGraph100036 : public IStaticGraphConfig {
 public:
    StaticGraph100036(GraphConfiguration100036** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100036();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
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
    imageSubGraphTopology100037(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvOuterNode* lbffBayerPdaf2WithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class StaticGraph100037 : public IStaticGraphConfig {
 public:
    StaticGraph100037(GraphConfiguration100037** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100037();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 2658466969;  // autogenerated

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
    imageSubGraphTopology100038(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvOuterNode* lbffBayerPdaf3WithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];
};

class StaticGraph100038 : public IStaticGraphConfig {
 public:
    StaticGraph100038(GraphConfiguration100038** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100038();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1409639831;  // autogenerated

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
    imageSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];
};

class irSubGraphTopology100039 : public GraphTopology {
 public:
    irSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    LbffIrWithGmvIrStreamOuterNode* lbffIrWithGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class image_irSubGraphTopology100039 : public GraphTopology {
 public:
    image_irSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 34, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvOuterNode* lbffRgbIrWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffIrWithGmvIrStreamOuterNode* lbffIrWithGmvIrStreamOuterNode = nullptr;
    BbpsIrWithTnrOuterNode* bbpsIrWithTnrOuterNode = nullptr;
    GraphLink* subGraphLinks[34];
};

class StaticGraph100039 : public IStaticGraphConfig {
 public:
    StaticGraph100039(GraphConfiguration100039** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100039();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 3095922915;  // autogenerated

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
    imageSubGraphTopology100040(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvOuterNode* lbffDol2InputsWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class StaticGraph100040 : public IStaticGraphConfig {
 public:
    StaticGraph100040(GraphConfiguration100040** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100040();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 2936873815;  // autogenerated

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
    imageSubGraphTopology100041(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvOuterNode* lbffDol3InputsWithGmvOuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];
};

class StaticGraph100041 : public IStaticGraphConfig {
 public:
    StaticGraph100041(GraphConfiguration100041** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100041();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 1123654281;  // autogenerated

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
    imageSubGraphTopology100042(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3OuterNode* lbffBayerPdaf3OuterNode = nullptr;
    BbpsWithTnrOuterNode* bbpsWithTnrOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];
};

class StaticGraph100042 : public IStaticGraphConfig {
 public:
    StaticGraph100042(GraphConfiguration100042** selectedGraphConfiguration,
                      uint32_t kernelConfigurationsOptionsCount,
                      ZoomKeyResolutions* zoomKeyResolutions,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100042();
    StaticGraphStatus updateConfiguration(uint32_t selectedIndex = 0);
    static const uint32_t hashCode = 4179556411;  // autogenerated

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

#endif
