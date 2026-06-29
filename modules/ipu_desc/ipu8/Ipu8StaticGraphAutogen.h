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

#ifndef STATIC_GRAPH_H
#define STATIC_GRAPH_H

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "Ipu8StaticGraphTypesAutogen.h"
#include "Ipu8StaticGraphBinaryAutogen.h"

#define SUPPORT_HISTORY_CHANGE_BASED_ON_GRAPH 0

#define SUPPORT_KEY_RESOLUTIONS 0

#define SUPPORT_FRAGMENTS 1

#define SUPPORT_TM_SWITCH 1

enum InnerNodeOption
{
    None = 0,
    noIr = (1 << 1),
    no3A = (1 << 2),
    noMp = (1 << 3),
    noDp = (1 << 4),
    noPdaf = (1 << 5),
};
typedef int32_t InnerNodeOptionsFlags;

struct SubGraphPublicInnerNodeConfiguration {
    bool no3A = false;
    bool noMp = false;
    bool noDp = false;
    bool noPdaf = false;
};

struct KernelFragments {
    StaticGraphFragmentDesc fragmentDescriptors[4];
};

struct TuningModeMap {
    uint8_t key = 0;
    uint8_t id = 0;
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
    HwBitmaps bitmapsNotVanished = {};
    uint64_t disabledRunKernelsBitmapNotVanished[2] = {};
    VanishOption fragmentVanishStatus[4] = {};
    uint16_t* kernelListInConfigOrder = {};
    TuningModeMap nodeTuningModes[2] = {};

    OuterNode() {}
    ~OuterNode();

    void Init(uint8_t nodeResourceId,
        NodeTypes nodeType,
        uint32_t kernelCount,
        uint32_t operationMode,
        TuningModeMap tuningModes[2],
        uint32_t streamId,
        uint8_t nodeNumberOfFragments);

    uint8_t GetNumberOfFragments();
    virtual void configVanishStatus(VanishOption vanishStatus);
    const uint16_t* getRunKernelConfigOrder();

    void SetDisabledKernels(uint64_t disabledRunKernelsBitmap[2]);

protected:
    void InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap[2], StaticGraphKernelRes* resolutionInfos, uint64_t kernelsResolutionHistoryGroupBitmap[2], uint64_t kernelFragmentDescriptorGroupBitmap[2], StaticGraphKernelRes* resolutionHistories, StaticGraphKernelBppConfiguration* bppInfos, uint8_t* systemApisSizes, uint8_t* systemApiData, KernelFragments* fragmentDescriptors, uint16_t* runKernelConfigOrder);
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
    bool isLinkVanished(GraphLink* link);

protected:
    VirtualSinkMapping* _sinkMappingConfiguration = nullptr;
    static InnerNodeOptionsFlags GetInnerOptions(SubGraphPublicInnerNodeConfiguration* publicInnerOptions);

};

class IStaticGraphConfig
{
public:
    virtual ~IStaticGraphConfig(){}
    IStaticGraphConfig(SensorMode* selectedSensorMode, VirtualSinkMapping* sinkMappingConfiguration, int32_t graphId, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    StaticGraphStatus getGraphTopology(GraphTopology** topology);
    StaticGraphStatus getSensorMode(SensorMode** sensorMode);
    StaticGraphStatus getGraphId(int32_t* id);
    StaticGraphStatus getSettingsId(int32_t* id);
    StaticGraphStatus getAdditionalFeaturesBit(int32_t* featuresBit);
    StaticGraphStatus IsInputSystemFormatUnpacked(bool* isUnpacked);
    StaticGraphStatus getVirtualSinkConnection(VirtualSink& virtualSink, HwSink* hwSink);
    StaticGraphStatus getConfigurationInformation(StaticGraphConfigurationInformation** configurationInformation);

protected:
    SensorMode* _selectedSensorMode = nullptr;
    StaticGraphConfigurationInformation* _configurationInformation = nullptr;
    GraphTopology* _selectedGraphTopology = nullptr;
    VirtualSinkMapping* _sinkMappingConfiguration = &_selectedSinkMappingConfiguration;
private:
    int32_t _graphId;
    int32_t _settingsId;
    int32_t _additionalFeaturesBit;
    bool _isIsysUnpacked;
    VirtualSinkMapping _selectedSinkMappingConfiguration;
};

#pragma pack(push, 4)

struct IsysOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[20];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[17];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[2168];
};

struct SwB2bInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwB2bOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[20];
};

struct SwB2bOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwRemosaicInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwRemosaicOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwRemosaicOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwAinrInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwAinrOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwAinrOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwGdcInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwGdcOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwGdcOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwNntmInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwNntmOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct SwNntmOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwImvInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwImvOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[2];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
    uint8_t systemApiConfiguration[12];
};

struct SwImvOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwScalerInOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct SwScalerOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[12];
};

struct SwScalerOutOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[24];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[21];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2860];
};

struct LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[31];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3498];
};

struct LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4190];
};

struct IsysWithCvOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[4];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[4];
    StaticGraphKernelBppConfiguration bppInfos[4];
    uint8_t systemApiConfiguration[54];
};

struct SwSegnetOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[31];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[24];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3048];
};

struct LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3740];
};

struct LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[47];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[37];
    StaticGraphKernelRes resolutionHistories[35];
    StaticGraphKernelBppConfiguration bppInfos[58];
    uint8_t systemApiConfiguration[4818];
};

struct LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[51];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[41];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5510];
};

struct IsysPdaf2OuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[2];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[24];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[21];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2640];
};

struct LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[28];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3332];
};

struct LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[3970];
};

struct LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[39];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[33];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[54];
    uint8_t systemApiConfiguration[4662];
};

struct IsysPdaf2WithCvOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[5];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[5];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[54];
};

struct LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3520];
};

struct LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[39];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[33];
    StaticGraphKernelRes resolutionHistories[32];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4212];
};

struct LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[51];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[41];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5290];
};

struct LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[55];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[45];
    StaticGraphKernelRes resolutionHistories[43];
    StaticGraphKernelBppConfiguration bppInfos[66];
    uint8_t systemApiConfiguration[5982];
};

struct LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2420];
};

struct LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[27];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[41];
    uint8_t systemApiConfiguration[3112];
};

struct LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3750];
};

struct LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[53];
    uint8_t systemApiConfiguration[4442];
};

struct LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3300];
};

struct LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3992];
};

struct LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[50];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[40];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[61];
    uint8_t systemApiConfiguration[5070];
};

struct LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[54];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[44];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[65];
    uint8_t systemApiConfiguration[5762];
};

struct IsysDolOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[2];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[22];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[19];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2613];
};

struct LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[26];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[23];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[41];
    uint8_t systemApiConfiguration[3305];
};

struct LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3943];
};

struct LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[53];
    uint8_t systemApiConfiguration[4635];
};

struct IsysDolWithCvOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[5];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[5];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[54];
};

struct LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3493];
};

struct LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[4185];
};

struct LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[49];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[39];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[61];
    uint8_t systemApiConfiguration[5263];
};

struct LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[53];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[43];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[65];
    uint8_t systemApiConfiguration[5955];
};

struct LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[25];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[22];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2860];
};

struct LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[29];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[26];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[43];
    uint8_t systemApiConfiguration[3552];
};

struct LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[36];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[30];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[51];
    uint8_t systemApiConfiguration[4190];
};

struct LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[40];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[34];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[55];
    uint8_t systemApiConfiguration[4882];
};

struct LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[36];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[30];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[47];
    uint8_t systemApiConfiguration[3740];
};

struct LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[40];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[34];
    StaticGraphKernelRes resolutionHistories[32];
    StaticGraphKernelBppConfiguration bppInfos[51];
    uint8_t systemApiConfiguration[4432];
};

struct LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[52];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[42];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[63];
    uint8_t systemApiConfiguration[5510];
};

struct LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[56];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[46];
    StaticGraphKernelRes resolutionHistories[43];
    StaticGraphKernelBppConfiguration bppInfos[67];
    uint8_t systemApiConfiguration[6202];
};

struct LbffDolSmoothOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[3];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[8];
    uint8_t systemApiConfiguration[476];
};

struct LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2833];
};

struct LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[27];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3525];
};

struct LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[22];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4163];
};

struct LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[54];
    uint8_t systemApiConfiguration[4855];
};

struct LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3713];
};

struct LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[31];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4405];
};

struct LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[50];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[40];
    StaticGraphKernelRes resolutionHistories[38];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5483];
};

struct LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[54];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[44];
    StaticGraphKernelRes resolutionHistories[42];
    StaticGraphKernelBppConfiguration bppInfos[66];
    uint8_t systemApiConfiguration[6175];
};

struct LbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[55];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[45];
    StaticGraphKernelRes resolutionHistories[43];
    StaticGraphKernelBppConfiguration bppInfos[66];
    uint8_t systemApiConfiguration[5982];
};

struct LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[22];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[19];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[36];
    uint8_t systemApiConfiguration[2608];
};

struct LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[19];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[16];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[32];
    uint8_t systemApiConfiguration[2148];
};

struct LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[26];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[23];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[40];
    uint8_t systemApiConfiguration[3300];
};

struct LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[48];
    uint8_t systemApiConfiguration[3938];
};

struct LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[30];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[44];
    uint8_t systemApiConfiguration[3478];
};

struct LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[52];
    uint8_t systemApiConfiguration[4630];
};

struct LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[19];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[16];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[33];
    uint8_t systemApiConfiguration[2168];
};

struct LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2860];
};

struct LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[30];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3498];
};

struct LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[4190];
};

struct LbffBayerNoGmvWithTnrWithOpacityOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[36];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[24];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[3938];
};

struct LbffDol2InputsNoGmvWithTnrWithOpacityOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[53];
    uint8_t systemApiConfiguration[4383];
};

struct LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[44];
    uint8_t systemApiConfiguration[3488];
};

struct LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[48];
    uint8_t systemApiConfiguration[4180];
};

struct LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[49];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[39];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[60];
    uint8_t systemApiConfiguration[5258];
};

struct LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[53];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[43];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[64];
    uint8_t systemApiConfiguration[5950];
};

struct LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[27];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[22];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2828];
};

struct LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[31];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[26];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[43];
    uint8_t systemApiConfiguration[3520];
};

struct LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[43];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[34];
    StaticGraphKernelRes resolutionHistories[32];
    StaticGraphKernelBppConfiguration bppInfos[55];
    uint8_t systemApiConfiguration[4598];
};

struct LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[47];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[38];
    StaticGraphKernelRes resolutionHistories[36];
    StaticGraphKernelBppConfiguration bppInfos[59];
    uint8_t systemApiConfiguration[5290];
};

struct WithCvOuterNodeConfiguration
{
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[6];
    TuningModeMap tuningModes[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[5];
    StaticGraphKernelRes resolutionHistories[6];
    StaticGraphKernelBppConfiguration bppInfos[6];
    uint8_t systemApiConfiguration[54];
};

struct GraphConfiguration200000
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200001
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200002
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200003
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200004
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200005
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200006
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[32];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200007
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[34];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200008
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200009
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200010
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200011
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200012
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200013
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[32];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200014
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[35];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200015
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[37];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200016
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200017
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200018
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200019
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200020
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200021
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200022
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200023
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[35];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200024
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200025
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200026
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200027
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200028
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration lbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200029
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration lbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200030
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[36];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200031
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration lbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[38];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200032
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200033
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200034
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200035
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[26];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200036
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200037
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200038
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[34];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200039
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[36];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200040
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200041
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[26];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200042
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200043
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200044
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration lbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200045
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration lbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[35];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200046
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration lbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[38];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration200047
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration lbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwB2bOuterNodeConfiguration swB2bOuterNodeConfiguration;
    SwRemosaicOuterNodeConfiguration swRemosaicOuterNodeConfiguration;
    SwAinrOuterNodeConfiguration swAinrOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwImvOuterNodeConfiguration swImvOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[40];
    StaticGraphSwProcessingLinkDefinition swProcessingLinkDefinitions[10];
};

struct GraphConfiguration100000
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100001
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100002
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100003
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100137
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100079
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100080
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100138
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
};

struct GraphConfiguration100142
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100162
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100143
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100144
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100081
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100004
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100005
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100006
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100066
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100007
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100067
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100139
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[32];
};

struct GraphConfiguration100169
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100008
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100009
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100010
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100011
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100140
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100045
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100012
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100013
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100014
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100015
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100016
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100017
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100018
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100019
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100020
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100021
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100022
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100023
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100024
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[8];
};

struct GraphConfiguration100040
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100041
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100042
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100027
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100028
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration lbffIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100029
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100030
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration lbffIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100031
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100032
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100033
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100034
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100141
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100100
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100101
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100102
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100157
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithOpacityOuterNodeConfiguration lbffBayerNoGmvWithTnrWithOpacityOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100103
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
};

struct GraphConfiguration101114
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrWithOpacityOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrWithOpacityOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100135
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[27];
};

struct GraphConfiguration100104
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100105
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100106
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100166
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100107
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100145
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
};

struct GraphConfiguration100108
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100109
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100110
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100111
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100136
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100200
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100201
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100112
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration lbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100113
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration lbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[26];
};

struct GraphConfiguration100114
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100146
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100115
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration lbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100116
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration lbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100117
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration lbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100118
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration lbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100119
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration lbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
};

struct GraphConfiguration100120
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration lbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100121
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration lbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100122
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration lbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[33];
};

struct GraphConfiguration100123
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration lbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[38];
};

struct GraphConfiguration100127
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration lbffIrNoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100128
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration lbffIrWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100129
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration lbffIrNoGmvWithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100130
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration lbffIrWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[26];
};

struct GraphConfiguration100131
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100132
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100133
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100134
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100235
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100236
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100202
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100203
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100279
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100280
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100281
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100204
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100205
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100206
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100266
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100207
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100267
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100208
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100209
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100210
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100211
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100245
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100212
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100213
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100214
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100215
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100216
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100217
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100218
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100219
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration lbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100220
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100221
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100222
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100223
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[30];
};

struct GraphConfiguration100224
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    WithCvOuterNodeConfiguration withCvOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100240
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    WithCvOuterNodeConfiguration withCvOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100241
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    WithCvOuterNodeConfiguration withCvOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100242
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    WithCvOuterNodeConfiguration withCvOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100227
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100228
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration lbffIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100229
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100230
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration lbffIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100231
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100232
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100233
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100234
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100026
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[2];
};

struct GraphConfiguration100059
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100035
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100036
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100037
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100058
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100038
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[6];
};

struct GraphConfiguration101138
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[6];
};

struct GraphConfiguration100039
{
    StaticGraphConfigurationInformation configurationInformation;
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    int32_t numberOfActualLinks = 0;
    StaticGraphLinkConfiguration linkConfigurations[6];
};
#pragma pack(pop)

class IsysOuterNode : public OuterNode
{
public:
    IsysOuterNode(): OuterNode(){}
    void Init(IsysOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwB2bInOuterNode : public OuterNode
{
public:
    SwB2bInOuterNode(): OuterNode(){}
    void Init(SwB2bInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwB2bOuterNode : public OuterNode
{
public:
    SwB2bOuterNode(): OuterNode(){}
    void Init(SwB2bOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwB2bOutOuterNode : public OuterNode
{
public:
    SwB2bOutOuterNode(): OuterNode(){}
    void Init(SwB2bOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwRemosaicInOuterNode : public OuterNode
{
public:
    SwRemosaicInOuterNode(): OuterNode(){}
    void Init(SwRemosaicInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwRemosaicOuterNode : public OuterNode
{
public:
    SwRemosaicOuterNode(): OuterNode(){}
    void Init(SwRemosaicOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwRemosaicOutOuterNode : public OuterNode
{
public:
    SwRemosaicOutOuterNode(): OuterNode(){}
    void Init(SwRemosaicOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwAinrInOuterNode : public OuterNode
{
public:
    SwAinrInOuterNode(): OuterNode(){}
    void Init(SwAinrInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwAinrOuterNode : public OuterNode
{
public:
    SwAinrOuterNode(): OuterNode(){}
    void Init(SwAinrOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwAinrOutOuterNode : public OuterNode
{
public:
    SwAinrOutOuterNode(): OuterNode(){}
    void Init(SwAinrOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwGdcInOuterNode : public OuterNode
{
public:
    SwGdcInOuterNode(): OuterNode(){}
    void Init(SwGdcInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwGdcOuterNode : public OuterNode
{
public:
    SwGdcOuterNode(): OuterNode(){}
    void Init(SwGdcOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwGdcOutOuterNode : public OuterNode
{
public:
    SwGdcOutOuterNode(): OuterNode(){}
    void Init(SwGdcOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwNntmInOuterNode : public OuterNode
{
public:
    SwNntmInOuterNode(): OuterNode(){}
    void Init(SwNntmInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwNntmOuterNode : public OuterNode
{
public:
    SwNntmOuterNode(): OuterNode(){}
    void Init(SwNntmOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwNntmOutOuterNode : public OuterNode
{
public:
    SwNntmOutOuterNode(): OuterNode(){}
    void Init(SwNntmOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwImvInOuterNode : public OuterNode
{
public:
    SwImvInOuterNode(): OuterNode(){}
    void Init(SwImvInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwImvOuterNode : public OuterNode
{
public:
    SwImvOuterNode(): OuterNode(){}
    void Init(SwImvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwImvOutOuterNode : public OuterNode
{
public:
    SwImvOutOuterNode(): OuterNode(){}
    void Init(SwImvOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwScalerInOuterNode : public OuterNode
{
public:
    SwScalerInOuterNode(): OuterNode(){}
    void Init(SwScalerInOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwScalerOuterNode : public OuterNode
{
public:
    SwScalerOuterNode(): OuterNode(){}
    void Init(SwScalerOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwScalerOutOuterNode : public OuterNode
{
public:
    SwScalerOutOuterNode(): OuterNode(){}
    void Init(SwScalerOutOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerWithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerWithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerWithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerWithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class IsysWithCvOuterNode : public OuterNode
{
public:
    IsysWithCvOuterNode(): OuterNode(){}
    void Init(IsysWithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class SwSegnetOuterNode : public OuterNode
{
public:
    SwSegnetOuterNode(): OuterNode(){}
    void Init(SwSegnetOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerNoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerNoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerWithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerWithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerNoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerNoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerWithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerWithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class IsysPdaf2OuterNode : public OuterNode
{
public:
    IsysPdaf2OuterNode(): OuterNode(){}
    void Init(IsysPdaf2OuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class IsysPdaf2WithCvOuterNode : public OuterNode
{
public:
    IsysPdaf2WithCvOuterNode(): OuterNode(){}
    void Init(IsysPdaf2WithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class IsysDolOuterNode : public OuterNode
{
public:
    IsysDolOuterNode(): OuterNode(){}
    void Init(IsysDolOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsWithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsWithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class IsysDolWithCvOuterNode : public OuterNode
{
public:
    IsysDolWithCvOuterNode(): OuterNode(){}
    void Init(IsysDolWithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsNoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsWithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsNoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsWithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDolSmoothOuterNode : public OuterNode
{
public:
    LbffDolSmoothOuterNode(): OuterNode(){}
    void Init(LbffDolSmoothOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsWithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsWithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsNoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsWithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsNoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol3InputsWithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerPdaf2WithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffBayerPdaf2WithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffBayerPdaf2WithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrIrNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrWithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrWithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrIrNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrWithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffRgbIrWithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrNoGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffIrNoGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrWithGmvNoTnrNoSapOuterNode : public OuterNode
{
public:
    LbffIrWithGmvNoTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrNoGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffIrNoGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrWithGmvWithTnrNoSapOuterNode : public OuterNode
{
public:
    LbffIrWithGmvWithTnrNoSapOuterNode(): OuterNode(){}
    void Init(LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffBayerNoGmvWithTnrWithOpacityOuterNode : public OuterNode
{
public:
    LbffBayerNoGmvWithTnrWithOpacityOuterNode(): OuterNode(){}
    void Init(LbffBayerNoGmvWithTnrWithOpacityOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffDol2InputsNoGmvWithTnrWithOpacityOuterNode : public OuterNode
{
public:
    LbffDol2InputsNoGmvWithTnrWithOpacityOuterNode(): OuterNode(){}
    void Init(LbffDol2InputsNoGmvWithTnrWithOpacityOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrNoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffRgbIrNoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrWithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffRgbIrWithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrNoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffRgbIrNoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffRgbIrWithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffRgbIrWithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrNoGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffIrNoGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrWithGmvNoTnrWithSapOuterNode : public OuterNode
{
public:
    LbffIrWithGmvNoTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrNoGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffIrNoGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class LbffIrWithGmvWithTnrWithSapOuterNode : public OuterNode
{
public:
    LbffIrWithGmvWithTnrWithSapOuterNode(): OuterNode(){}
    void Init(LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};
class WithCvOuterNode : public OuterNode
{
public:
    WithCvOuterNode(): OuterNode(){}
    void Init(WithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
    void configVanishStatus(VanishOption vanishStatus) override;

};

class imageSubGraphTopology200000 : public GraphTopology {

public:
    imageSubGraphTopology200000(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph200000 : public IStaticGraphConfig
{
public:
    StaticGraph200000(GraphConfiguration200000* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200000();
    static const uint32_t hashCode = 4193146247;  // autogenerated

private:
    // Configuration
    GraphConfiguration200000 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvNoTnrNoSapOuterNode _lbffBayerNoGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200000 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology200001 : public GraphTopology {

public:
    imageSubGraphTopology200001(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph200001 : public IStaticGraphConfig
{
public:
    StaticGraph200001(GraphConfiguration200001* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200001();
    static const uint32_t hashCode = 3288532247;  // autogenerated

private:
    // Configuration
    GraphConfiguration200001 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvNoTnrNoSapOuterNode _lbffBayerWithGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200001 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology200002 : public GraphTopology {

public:
    imageSubGraphTopology200002(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph200002 : public IStaticGraphConfig
{
public:
    StaticGraph200002(GraphConfiguration200002* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200002();
    static const uint32_t hashCode = 691605543;  // autogenerated

private:
    // Configuration
    GraphConfiguration200002 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200002 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology200003 : public GraphTopology {

public:
    imageSubGraphTopology200003(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph200003 : public IStaticGraphConfig
{
public:
    StaticGraph200003(GraphConfiguration200003* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200003();
    static const uint32_t hashCode = 3615244151;  // autogenerated

private:
    // Configuration
    GraphConfiguration200003 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200003 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology200004 : public GraphTopology {

public:
    imageSubGraphTopology200004(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvNoTnrWithSapOuterNode* lbffBayerNoGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200004 : public IStaticGraphConfig
{
public:
    StaticGraph200004(GraphConfiguration200004* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200004();
    static const uint32_t hashCode = 3992658056;  // autogenerated

private:
    // Configuration
    GraphConfiguration200004 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvNoTnrWithSapOuterNode _lbffBayerNoGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200004 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology200005 : public GraphTopology {

public:
    imageSubGraphTopology200005(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvNoTnrWithSapOuterNode* lbffBayerWithGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200005 : public IStaticGraphConfig
{
public:
    StaticGraph200005(GraphConfiguration200005* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200005();
    static const uint32_t hashCode = 4031159184;  // autogenerated

private:
    // Configuration
    GraphConfiguration200005 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerWithGmvNoTnrWithSapOuterNode _lbffBayerWithGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200005 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology200006 : public GraphTopology {

public:
    imageSubGraphTopology200006(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 32, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithSapOuterNode* lbffBayerNoGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[32];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200006 : public IStaticGraphConfig
{
public:
    StaticGraph200006(GraphConfiguration200006* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200006();
    static const uint32_t hashCode = 4034951006;  // autogenerated

private:
    // Configuration
    GraphConfiguration200006 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvWithTnrWithSapOuterNode _lbffBayerNoGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200006 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[32];
};

class imageSubGraphTopology200007 : public GraphTopology {

public:
    imageSubGraphTopology200007(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 34, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvWithTnrWithSapOuterNode* lbffBayerWithGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[34];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200007 : public IStaticGraphConfig
{
public:
    StaticGraph200007(GraphConfiguration200007* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200007();
    static const uint32_t hashCode = 2405410902;  // autogenerated

private:
    // Configuration
    GraphConfiguration200007 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerWithGmvWithTnrWithSapOuterNode _lbffBayerWithGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200007 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[34];
};

class imageSubGraphTopology200008 : public GraphTopology {

public:
    imageSubGraphTopology200008(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph200008 : public IStaticGraphConfig
{
public:
    StaticGraph200008(GraphConfiguration200008* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200008();
    static const uint32_t hashCode = 2268347828;  // autogenerated

private:
    // Configuration
    GraphConfiguration200008 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200008 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology200009 : public GraphTopology {

public:
    imageSubGraphTopology200009(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph200009 : public IStaticGraphConfig
{
public:
    StaticGraph200009(GraphConfiguration200009* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200009();
    static const uint32_t hashCode = 2729877820;  // autogenerated

private:
    // Configuration
    GraphConfiguration200009 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200009 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology200010 : public GraphTopology {

public:
    imageSubGraphTopology200010(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph200010 : public IStaticGraphConfig
{
public:
    StaticGraph200010(GraphConfiguration200010* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200010();
    static const uint32_t hashCode = 2893799632;  // autogenerated

private:
    // Configuration
    GraphConfiguration200010 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200010 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology200011 : public GraphTopology {

public:
    imageSubGraphTopology200011(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

};

class StaticGraph200011 : public IStaticGraphConfig
{
public:
    StaticGraph200011(GraphConfiguration200011* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200011();
    static const uint32_t hashCode = 456036856;  // autogenerated

private:
    // Configuration
    GraphConfiguration200011 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200011 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology200012 : public GraphTopology {

public:
    imageSubGraphTopology200012(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200012 : public IStaticGraphConfig
{
public:
    StaticGraph200012(GraphConfiguration200012* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200012();
    static const uint32_t hashCode = 271672635;  // autogenerated

private:
    // Configuration
    GraphConfiguration200012 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf2NoGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200012 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology200013 : public GraphTopology {

public:
    imageSubGraphTopology200013(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 32, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf2WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[32];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200013 : public IStaticGraphConfig
{
public:
    StaticGraph200013(GraphConfiguration200013* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200013();
    static const uint32_t hashCode = 1333520811;  // autogenerated

private:
    // Configuration
    GraphConfiguration200013 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf2WithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200013 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[32];
};

class imageSubGraphTopology200014 : public GraphTopology {

public:
    imageSubGraphTopology200014(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 35, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[35];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200014 : public IStaticGraphConfig
{
public:
    StaticGraph200014(GraphConfiguration200014* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200014();
    static const uint32_t hashCode = 537894889;  // autogenerated

private:
    // Configuration
    GraphConfiguration200014 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200014 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[35];
};

class imageSubGraphTopology200015 : public GraphTopology {

public:
    imageSubGraphTopology200015(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 37, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[37];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200015 : public IStaticGraphConfig
{
public:
    StaticGraph200015(GraphConfiguration200015* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200015();
    static const uint32_t hashCode = 3358791529;  // autogenerated

private:
    // Configuration
    GraphConfiguration200015 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200015 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[37];
};

class imageSubGraphTopology200016 : public GraphTopology {

public:
    imageSubGraphTopology200016(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph200016 : public IStaticGraphConfig
{
public:
    StaticGraph200016(GraphConfiguration200016* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200016();
    static const uint32_t hashCode = 1555116574;  // autogenerated

private:
    // Configuration
    GraphConfiguration200016 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200016 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology200017 : public GraphTopology {

public:
    imageSubGraphTopology200017(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph200017 : public IStaticGraphConfig
{
public:
    StaticGraph200017(GraphConfiguration200017* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200017();
    static const uint32_t hashCode = 3257661974;  // autogenerated

private:
    // Configuration
    GraphConfiguration200017 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200017 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology200018 : public GraphTopology {

public:
    imageSubGraphTopology200018(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph200018 : public IStaticGraphConfig
{
public:
    StaticGraph200018(GraphConfiguration200018* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200018();
    static const uint32_t hashCode = 1216700882;  // autogenerated

private:
    // Configuration
    GraphConfiguration200018 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200018 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology200019 : public GraphTopology {

public:
    imageSubGraphTopology200019(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph200019 : public IStaticGraphConfig
{
public:
    StaticGraph200019(GraphConfiguration200019* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200019();
    static const uint32_t hashCode = 3050003466;  // autogenerated

private:
    // Configuration
    GraphConfiguration200019 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200019 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology200020 : public GraphTopology {

public:
    imageSubGraphTopology200020(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf3NoGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200020 : public IStaticGraphConfig
{
public:
    StaticGraph200020(GraphConfiguration200020* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200020();
    static const uint32_t hashCode = 2901109361;  // autogenerated

private:
    // Configuration
    GraphConfiguration200020 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf3NoGmvNoTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200020 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology200021 : public GraphTopology {

public:
    imageSubGraphTopology200021(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf3WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200021 : public IStaticGraphConfig
{
public:
    StaticGraph200021(GraphConfiguration200021* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200021();
    static const uint32_t hashCode = 3646552305;  // autogenerated

private:
    // Configuration
    GraphConfiguration200021 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf3WithGmvNoTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200021 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology200022 : public GraphTopology {

public:
    imageSubGraphTopology200022(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200022 : public IStaticGraphConfig
{
public:
    StaticGraph200022(GraphConfiguration200022* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200022();
    static const uint32_t hashCode = 3944654851;  // autogenerated

private:
    // Configuration
    GraphConfiguration200022 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200022 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology200023 : public GraphTopology {

public:
    imageSubGraphTopology200023(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 35, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[35];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200023 : public IStaticGraphConfig
{
public:
    StaticGraph200023(GraphConfiguration200023* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200023();
    static const uint32_t hashCode = 2699563891;  // autogenerated

private:
    // Configuration
    GraphConfiguration200023 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200023 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[35];
};

class imageSubGraphTopology200024 : public GraphTopology {

public:
    imageSubGraphTopology200024(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode* lbffDol2InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph200024 : public IStaticGraphConfig
{
public:
    StaticGraph200024(GraphConfiguration200024* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200024();
    static const uint32_t hashCode = 3783302055;  // autogenerated

private:
    // Configuration
    GraphConfiguration200024 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode _lbffDol2InputsNoGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200024 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology200025 : public GraphTopology {

public:
    imageSubGraphTopology200025(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode* lbffDol2InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph200025 : public IStaticGraphConfig
{
public:
    StaticGraph200025(GraphConfiguration200025* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200025();
    static const uint32_t hashCode = 251624183;  // autogenerated

private:
    // Configuration
    GraphConfiguration200025 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode _lbffDol2InputsWithGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200025 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology200026 : public GraphTopology {

public:
    imageSubGraphTopology200026(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode* lbffDol2InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph200026 : public IStaticGraphConfig
{
public:
    StaticGraph200026(GraphConfiguration200026* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200026();
    static const uint32_t hashCode = 4037804759;  // autogenerated

private:
    // Configuration
    GraphConfiguration200026 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode _lbffDol2InputsNoGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200026 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology200027 : public GraphTopology {

public:
    imageSubGraphTopology200027(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode* lbffDol2InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

};

class StaticGraph200027 : public IStaticGraphConfig
{
public:
    StaticGraph200027(GraphConfiguration200027* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200027();
    static const uint32_t hashCode = 2359525479;  // autogenerated

private:
    // Configuration
    GraphConfiguration200027 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode _lbffDol2InputsWithGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200027 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology200028 : public GraphTopology {

public:
    imageSubGraphTopology200028(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode* lbffDol2InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200028 : public IStaticGraphConfig
{
public:
    StaticGraph200028(GraphConfiguration200028* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200028();
    static const uint32_t hashCode = 93548272;  // autogenerated

private:
    // Configuration
    GraphConfiguration200028 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode _lbffDol2InputsNoGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200028 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology200029 : public GraphTopology {

public:
    imageSubGraphTopology200029(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode* lbffDol2InputsWithGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200029 : public IStaticGraphConfig
{
public:
    StaticGraph200029(GraphConfiguration200029* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200029();
    static const uint32_t hashCode = 3524836440;  // autogenerated

private:
    // Configuration
    GraphConfiguration200029 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode _lbffDol2InputsWithGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200029 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology200030 : public GraphTopology {

public:
    imageSubGraphTopology200030(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 36, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode* lbffDol2InputsNoGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[36];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200030 : public IStaticGraphConfig
{
public:
    StaticGraph200030(GraphConfiguration200030* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200030();
    static const uint32_t hashCode = 3969246966;  // autogenerated

private:
    // Configuration
    GraphConfiguration200030 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode _lbffDol2InputsNoGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200030 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[36];
};

class imageSubGraphTopology200031 : public GraphTopology {

public:
    imageSubGraphTopology200031(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 38, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode* lbffDol2InputsWithGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[38];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200031 : public IStaticGraphConfig
{
public:
    StaticGraph200031(GraphConfiguration200031* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200031();
    static const uint32_t hashCode = 3946358798;  // autogenerated

private:
    // Configuration
    GraphConfiguration200031 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode _lbffDol2InputsWithGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200031 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[38];
};

class imageSubGraphTopology200032 : public GraphTopology {

public:
    imageSubGraphTopology200032(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph200032 : public IStaticGraphConfig
{
public:
    StaticGraph200032(GraphConfiguration200032* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200032();
    static const uint32_t hashCode = 3774286530;  // autogenerated

private:
    // Configuration
    GraphConfiguration200032 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200032 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology200033 : public GraphTopology {

public:
    imageSubGraphTopology200033(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph200033 : public IStaticGraphConfig
{
public:
    StaticGraph200033(GraphConfiguration200033* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200033();
    static const uint32_t hashCode = 1167337402;  // autogenerated

private:
    // Configuration
    GraphConfiguration200033 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200033 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology200034 : public GraphTopology {

public:
    imageSubGraphTopology200034(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph200034 : public IStaticGraphConfig
{
public:
    StaticGraph200034(GraphConfiguration200034* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200034();
    static const uint32_t hashCode = 2279440806;  // autogenerated

private:
    // Configuration
    GraphConfiguration200034 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200034 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology200035 : public GraphTopology {

public:
    imageSubGraphTopology200035(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

};

class StaticGraph200035 : public IStaticGraphConfig
{
public:
    StaticGraph200035(GraphConfiguration200035* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200035();
    static const uint32_t hashCode = 749036350;  // autogenerated

private:
    // Configuration
    GraphConfiguration200035 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200035 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[26];
};

class imageSubGraphTopology200036 : public GraphTopology {

public:
    imageSubGraphTopology200036(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200036 : public IStaticGraphConfig
{
public:
    StaticGraph200036(GraphConfiguration200036* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200036();
    static const uint32_t hashCode = 4138369545;  // autogenerated

private:
    // Configuration
    GraphConfiguration200036 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200036 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology200037 : public GraphTopology {

public:
    imageSubGraphTopology200037(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200037 : public IStaticGraphConfig
{
public:
    StaticGraph200037(GraphConfiguration200037* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200037();
    static const uint32_t hashCode = 494240265;  // autogenerated

private:
    // Configuration
    GraphConfiguration200037 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200037 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology200038 : public GraphTopology {

public:
    imageSubGraphTopology200038(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 34, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[34];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200038 : public IStaticGraphConfig
{
public:
    StaticGraph200038(GraphConfiguration200038* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200038();
    static const uint32_t hashCode = 650937483;  // autogenerated

private:
    // Configuration
    GraphConfiguration200038 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200038 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[34];
};

class imageSubGraphTopology200039 : public GraphTopology {

public:
    imageSubGraphTopology200039(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 36, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[36];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200039 : public IStaticGraphConfig
{
public:
    StaticGraph200039(GraphConfiguration200039* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200039();
    static const uint32_t hashCode = 4055385467;  // autogenerated

private:
    // Configuration
    GraphConfiguration200039 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200039 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[36];
};

class imageSubGraphTopology200040 : public GraphTopology {

public:
    imageSubGraphTopology200040(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode* lbffDol3InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph200040 : public IStaticGraphConfig
{
public:
    StaticGraph200040(GraphConfiguration200040* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200040();
    static const uint32_t hashCode = 3498184540;  // autogenerated

private:
    // Configuration
    GraphConfiguration200040 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode _lbffDol3InputsNoGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200040 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology200041 : public GraphTopology {

public:
    imageSubGraphTopology200041(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode* lbffDol3InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

};

class StaticGraph200041 : public IStaticGraphConfig
{
public:
    StaticGraph200041(GraphConfiguration200041* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200041();
    static const uint32_t hashCode = 39707076;  // autogenerated

private:
    // Configuration
    GraphConfiguration200041 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode _lbffDol3InputsWithGmvNoTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200041 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[26];
};

class imageSubGraphTopology200042 : public GraphTopology {

public:
    imageSubGraphTopology200042(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode* lbffDol3InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

};

class StaticGraph200042 : public IStaticGraphConfig
{
public:
    StaticGraph200042(GraphConfiguration200042* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200042();
    static const uint32_t hashCode = 1858599960;  // autogenerated

private:
    // Configuration
    GraphConfiguration200042 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode _lbffDol3InputsNoGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200042 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology200043 : public GraphTopology {

public:
    imageSubGraphTopology200043(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode* lbffDol3InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

};

class StaticGraph200043 : public IStaticGraphConfig
{
public:
    StaticGraph200043(GraphConfiguration200043* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200043();
    static const uint32_t hashCode = 2374557088;  // autogenerated

private:
    // Configuration
    GraphConfiguration200043 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode _lbffDol3InputsWithGmvWithTnrNoSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200043 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology200044 : public GraphTopology {

public:
    imageSubGraphTopology200044(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode* lbffDol3InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200044 : public IStaticGraphConfig
{
public:
    StaticGraph200044(GraphConfiguration200044* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200044();
    static const uint32_t hashCode = 1799386923;  // autogenerated

private:
    // Configuration
    GraphConfiguration200044 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode _lbffDol3InputsNoGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200044 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology200045 : public GraphTopology {

public:
    imageSubGraphTopology200045(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 35, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode* lbffDol3InputsWithGmvNoTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[35];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200045 : public IStaticGraphConfig
{
public:
    StaticGraph200045(GraphConfiguration200045* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200045();
    static const uint32_t hashCode = 2855375451;  // autogenerated

private:
    // Configuration
    GraphConfiguration200045 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode _lbffDol3InputsWithGmvNoTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200045 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[35];
};

class imageSubGraphTopology200046 : public GraphTopology {

public:
    imageSubGraphTopology200046(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 38, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode* lbffDol3InputsNoGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[38];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200046 : public IStaticGraphConfig
{
public:
    StaticGraph200046(GraphConfiguration200046* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200046();
    static const uint32_t hashCode = 2901032385;  // autogenerated

private:
    // Configuration
    GraphConfiguration200046 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode _lbffDol3InputsNoGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200046 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[38];
};

class imageSubGraphTopology200047 : public GraphTopology {

public:
    imageSubGraphTopology200047(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 40, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode* lbffDol3InputsWithGmvWithTnrWithSapOuterNode = nullptr;
    SwB2bOuterNode* swB2bOuterNode = nullptr;
    SwRemosaicOuterNode* swRemosaicOuterNode = nullptr;
    SwAinrOuterNode* swAinrOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwImvOuterNode* swImvOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[40];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph200047 : public IStaticGraphConfig
{
public:
    StaticGraph200047(GraphConfiguration200047* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph200047();
    static const uint32_t hashCode = 1122107393;  // autogenerated

private:
    // Configuration
    GraphConfiguration200047 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode _lbffDol3InputsWithGmvWithTnrWithSapOuterNode;
    SwB2bOuterNode _swB2bOuterNode;
    SwRemosaicOuterNode _swRemosaicOuterNode;
    SwAinrOuterNode _swAinrOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwImvOuterNode _swImvOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology200047 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[40];
};

class imageSubGraphTopology100000 : public GraphTopology {

public:
    imageSubGraphTopology100000(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[9];

};

class StaticGraph100000 : public IStaticGraphConfig
{
public:
    StaticGraph100000(GraphConfiguration100000* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100000();
    static const uint32_t hashCode = 1522444746;  // autogenerated

private:
    // Configuration
    GraphConfiguration100000 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvNoTnrNoSapOuterNode _lbffBayerNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100000 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[9];
};

class imageSubGraphTopology100001 : public GraphTopology {

public:
    imageSubGraphTopology100001(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100001 : public IStaticGraphConfig
{
public:
    StaticGraph100001(GraphConfiguration100001* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100001();
    static const uint32_t hashCode = 1105844649;  // autogenerated

private:
    // Configuration
    GraphConfiguration100001 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvNoTnrNoSapOuterNode _lbffBayerWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100001 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100002 : public GraphTopology {

public:
    imageSubGraphTopology100002(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];

};

class StaticGraph100002 : public IStaticGraphConfig
{
public:
    StaticGraph100002(GraphConfiguration100002* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100002();
    static const uint32_t hashCode = 1992788174;  // autogenerated

private:
    // Configuration
    GraphConfiguration100002 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100002 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[12];
};

class imageSubGraphTopology100003 : public GraphTopology {

public:
    imageSubGraphTopology100003(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100003 : public IStaticGraphConfig
{
public:
    StaticGraph100003(GraphConfiguration100003* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100003();
    static const uint32_t hashCode = 1767402465;  // autogenerated

private:
    // Configuration
    GraphConfiguration100003 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100003 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100137 : public GraphTopology {

public:
    imageSubGraphTopology100137(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvWithTnrWithSapOuterNode* lbffBayerWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100137 : public IStaticGraphConfig
{
public:
    StaticGraph100137(GraphConfiguration100137* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100137();
    static const uint32_t hashCode = 45191375;  // autogenerated

private:
    // Configuration
    GraphConfiguration100137 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerWithGmvWithTnrWithSapOuterNode _lbffBayerWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100137 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100079 : public GraphTopology {

public:
    imageSubGraphTopology100079(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100079 : public IStaticGraphConfig
{
public:
    StaticGraph100079(GraphConfiguration100079* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100079();
    static const uint32_t hashCode = 773132404;  // autogenerated

private:
    // Configuration
    GraphConfiguration100079 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100079 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100080 : public GraphTopology {

public:
    imageSubGraphTopology100080(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100080 : public IStaticGraphConfig
{
public:
    StaticGraph100080(GraphConfiguration100080* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100080();
    static const uint32_t hashCode = 2756497462;  // autogenerated

private:
    // Configuration
    GraphConfiguration100080 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100080 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100138 : public GraphTopology {

public:
    imageSubGraphTopology100138(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithSapOuterNode* lbffBayerNoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100138 : public IStaticGraphConfig
{
public:
    StaticGraph100138(GraphConfiguration100138* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100138();
    static const uint32_t hashCode = 432961535;  // autogenerated

private:
    // Configuration
    GraphConfiguration100138 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvWithTnrWithSapOuterNode _lbffBayerNoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100138 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology100142 : public GraphTopology {

public:
    imageSubGraphTopology100142(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100142 : public IStaticGraphConfig
{
public:
    StaticGraph100142(GraphConfiguration100142* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100142();
    static const uint32_t hashCode = 1906423777;  // autogenerated

private:
    // Configuration
    GraphConfiguration100142 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100142 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100162 : public GraphTopology {

public:
    imageSubGraphTopology100162(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithTnrWithSapOuterNode* lbffBayerPdaf2WithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100162 : public IStaticGraphConfig
{
public:
    StaticGraph100162(GraphConfiguration100162* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100162();
    static const uint32_t hashCode = 2206653037;  // autogenerated

private:
    // Configuration
    GraphConfiguration100162 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithTnrWithSapOuterNode _lbffBayerPdaf2WithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100162 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100143 : public GraphTopology {

public:
    imageSubGraphTopology100143(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100143 : public IStaticGraphConfig
{
public:
    StaticGraph100143(GraphConfiguration100143* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100143();
    static const uint32_t hashCode = 66198243;  // autogenerated

private:
    // Configuration
    GraphConfiguration100143 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100143 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100144 : public GraphTopology {

public:
    imageSubGraphTopology100144(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100144 : public IStaticGraphConfig
{
public:
    StaticGraph100144(GraphConfiguration100144* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100144();
    static const uint32_t hashCode = 3020442763;  // autogenerated

private:
    // Configuration
    GraphConfiguration100144 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100144 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100081 : public GraphTopology {

public:
    imageSubGraphTopology100081(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100081 : public IStaticGraphConfig
{
public:
    StaticGraph100081(GraphConfiguration100081* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100081();
    static const uint32_t hashCode = 3051061038;  // autogenerated

private:
    // Configuration
    GraphConfiguration100081 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100081 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100004 : public GraphTopology {

public:
    imageSubGraphTopology100004(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];

};

class StaticGraph100004 : public IStaticGraphConfig
{
public:
    StaticGraph100004(GraphConfiguration100004* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100004();
    static const uint32_t hashCode = 1094581504;  // autogenerated

private:
    // Configuration
    GraphConfiguration100004 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100004 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[12];
};

class imageSubGraphTopology100005 : public GraphTopology {

public:
    imageSubGraphTopology100005(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100005 : public IStaticGraphConfig
{
public:
    StaticGraph100005(GraphConfiguration100005* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100005();
    static const uint32_t hashCode = 1029296263;  // autogenerated

private:
    // Configuration
    GraphConfiguration100005 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100005 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100006 : public GraphTopology {

public:
    imageSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class StaticGraph100006 : public IStaticGraphConfig
{
public:
    StaticGraph100006(GraphConfiguration100006* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100006();
    static const uint32_t hashCode = 399719836;  // autogenerated

private:
    // Configuration
    GraphConfiguration100006 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100006 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100066 : public GraphTopology {

public:
    imageSubGraphTopology100066(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100066 : public IStaticGraphConfig
{
public:
    StaticGraph100066(GraphConfiguration100066* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100066();
    static const uint32_t hashCode = 2605096182;  // autogenerated

private:
    // Configuration
    GraphConfiguration100066 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100066 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100007 : public GraphTopology {

public:
    imageSubGraphTopology100007(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100007 : public IStaticGraphConfig
{
public:
    StaticGraph100007(GraphConfiguration100007* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100007();
    static const uint32_t hashCode = 3834807223;  // autogenerated

private:
    // Configuration
    GraphConfiguration100007 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100007 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100067 : public GraphTopology {

public:
    imageSubGraphTopology100067(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100067 : public IStaticGraphConfig
{
public:
    StaticGraph100067(GraphConfiguration100067* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100067();
    static const uint32_t hashCode = 2210564785;  // autogenerated

private:
    // Configuration
    GraphConfiguration100067 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2OuterNode _isysPdaf2OuterNode;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100067 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100139 : public GraphTopology {

public:
    imageSubGraphTopology100139(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 32, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[32];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100139 : public IStaticGraphConfig
{
public:
    StaticGraph100139(GraphConfiguration100139* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100139();
    static const uint32_t hashCode = 1969300705;  // autogenerated

private:
    // Configuration
    GraphConfiguration100139 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100139 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[32];
};

class imageSubGraphTopology100169 : public GraphTopology {

public:
    imageSubGraphTopology100169(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithTnrWithSapOuterNode* lbffBayerPdaf2WithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100169 : public IStaticGraphConfig
{
public:
    StaticGraph100169(GraphConfiguration100169* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100169();
    static const uint32_t hashCode = 2206653037;  // autogenerated

private:
    // Configuration
    GraphConfiguration100169 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithTnrWithSapOuterNode _lbffBayerPdaf2WithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100169 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100008 : public GraphTopology {

public:
    imageSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[10];

};

class StaticGraph100008 : public IStaticGraphConfig
{
public:
    StaticGraph100008(GraphConfiguration100008* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100008();
    static const uint32_t hashCode = 3083944874;  // autogenerated

private:
    // Configuration
    GraphConfiguration100008 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100008 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[10];
};

class imageSubGraphTopology100009 : public GraphTopology {

public:
    imageSubGraphTopology100009(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class StaticGraph100009 : public IStaticGraphConfig
{
public:
    StaticGraph100009(GraphConfiguration100009* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100009();
    static const uint32_t hashCode = 2745120197;  // autogenerated

private:
    // Configuration
    GraphConfiguration100009 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100009 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100010 : public GraphTopology {

public:
    imageSubGraphTopology100010(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class StaticGraph100010 : public IStaticGraphConfig
{
public:
    StaticGraph100010(GraphConfiguration100010* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100010();
    static const uint32_t hashCode = 1015515006;  // autogenerated

private:
    // Configuration
    GraphConfiguration100010 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100010 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[13];
};

class imageSubGraphTopology100011 : public GraphTopology {

public:
    imageSubGraphTopology100011(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100011 : public IStaticGraphConfig
{
public:
    StaticGraph100011(GraphConfiguration100011* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100011();
    static const uint32_t hashCode = 1880667005;  // autogenerated

private:
    // Configuration
    GraphConfiguration100011 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100011 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100140 : public GraphTopology {

public:
    imageSubGraphTopology100140(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100140 : public IStaticGraphConfig
{
public:
    StaticGraph100140(GraphConfiguration100140* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100140();
    static const uint32_t hashCode = 2826678547;  // autogenerated

private:
    // Configuration
    GraphConfiguration100140 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100140 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100045 : public GraphTopology {

public:
    imageSubGraphTopology100045(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100045 : public IStaticGraphConfig
{
public:
    StaticGraph100045(GraphConfiguration100045* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100045();
    static const uint32_t hashCode = 2339897452;  // autogenerated

private:
    // Configuration
    GraphConfiguration100045 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100045 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100012 : public GraphTopology {

public:
    imageSubGraphTopology100012(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode* lbffDol2InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100012 : public IStaticGraphConfig
{
public:
    StaticGraph100012(GraphConfiguration100012* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100012();
    static const uint32_t hashCode = 215738356;  // autogenerated

private:
    // Configuration
    GraphConfiguration100012 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode _lbffDol2InputsNoGmvNoTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100012 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100013 : public GraphTopology {

public:
    imageSubGraphTopology100013(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode* lbffDol2InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100013 : public IStaticGraphConfig
{
public:
    StaticGraph100013(GraphConfiguration100013* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100013();
    static const uint32_t hashCode = 3893270369;  // autogenerated

private:
    // Configuration
    GraphConfiguration100013 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode _lbffDol2InputsWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100013 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100014 : public GraphTopology {

public:
    imageSubGraphTopology100014(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode* lbffDol2InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100014 : public IStaticGraphConfig
{
public:
    StaticGraph100014(GraphConfiguration100014* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100014();
    static const uint32_t hashCode = 4055897344;  // autogenerated

private:
    // Configuration
    GraphConfiguration100014 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode _lbffDol2InputsNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100014 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100015 : public GraphTopology {

public:
    imageSubGraphTopology100015(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode* lbffDol2InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100015 : public IStaticGraphConfig
{
public:
    StaticGraph100015(GraphConfiguration100015* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100015();
    static const uint32_t hashCode = 3271382345;  // autogenerated

private:
    // Configuration
    GraphConfiguration100015 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode _lbffDol2InputsWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100015 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100016 : public GraphTopology {

public:
    imageSubGraphTopology100016(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode* lbffDol3InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100016 : public IStaticGraphConfig
{
public:
    StaticGraph100016(GraphConfiguration100016* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100016();
    static const uint32_t hashCode = 45477583;  // autogenerated

private:
    // Configuration
    GraphConfiguration100016 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode _lbffDol3InputsNoGmvNoTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100016 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100017 : public GraphTopology {

public:
    imageSubGraphTopology100017(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode* lbffDol3InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100017 : public IStaticGraphConfig
{
public:
    StaticGraph100017(GraphConfiguration100017* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100017();
    static const uint32_t hashCode = 953964422;  // autogenerated

private:
    // Configuration
    GraphConfiguration100017 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode _lbffDol3InputsWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100017 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100018 : public GraphTopology {

public:
    imageSubGraphTopology100018(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode* lbffDol3InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100018 : public IStaticGraphConfig
{
public:
    StaticGraph100018(GraphConfiguration100018* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100018();
    static const uint32_t hashCode = 3766431663;  // autogenerated

private:
    // Configuration
    GraphConfiguration100018 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode _lbffDol3InputsNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100018 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100019 : public GraphTopology {

public:
    imageSubGraphTopology100019(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode* lbffDol3InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100019 : public IStaticGraphConfig
{
public:
    StaticGraph100019(GraphConfiguration100019* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100019();
    static const uint32_t hashCode = 2984534170;  // autogenerated

private:
    // Configuration
    GraphConfiguration100019 _graphConfiguration;

    /* Outer Nodes */
    IsysDolOuterNode _isysDolOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode _lbffDol3InputsWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100019 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100020 : public GraphTopology {

public:
    imageSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[10];

};

class irSubGraphTopology100020 : public GraphTopology {

public:
    irSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class image_irSubGraphTopology100020 : public GraphTopology {

public:
    image_irSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100020 : public IStaticGraphConfig
{
public:
    StaticGraph100020(GraphConfiguration100020* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100020();
    static const uint32_t hashCode = 4245298421;  // autogenerated

private:
    // Configuration
    GraphConfiguration100020 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrNoGmvNoTnrNoSapOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100020 _imageSubGraph;
    irSubGraphTopology100020 _irSubGraph;
    image_irSubGraphTopology100020 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100021 : public GraphTopology {

public:
    imageSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class irSubGraphTopology100021 : public GraphTopology {

public:
    irSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class image_irSubGraphTopology100021 : public GraphTopology {

public:
    image_irSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100021 : public IStaticGraphConfig
{
public:
    StaticGraph100021(GraphConfiguration100021* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100021();
    static const uint32_t hashCode = 1464818630;  // autogenerated

private:
    // Configuration
    GraphConfiguration100021 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode _lbffRgbIrWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100021 _imageSubGraph;
    irSubGraphTopology100021 _irSubGraph;
    image_irSubGraphTopology100021 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100022 : public GraphTopology {

public:
    imageSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class irSubGraphTopology100022 : public GraphTopology {

public:
    irSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class image_irSubGraphTopology100022 : public GraphTopology {

public:
    image_irSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

};

class StaticGraph100022 : public IStaticGraphConfig
{
public:
    StaticGraph100022(GraphConfiguration100022* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100022();
    static const uint32_t hashCode = 1167817829;  // autogenerated

private:
    // Configuration
    GraphConfiguration100022 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrNoGmvWithTnrNoSapOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100022 _imageSubGraph;
    irSubGraphTopology100022 _irSubGraph;
    image_irSubGraphTopology100022 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100023 : public GraphTopology {

public:
    imageSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class irSubGraphTopology100023 : public GraphTopology {

public:
    irSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class image_irSubGraphTopology100023 : public GraphTopology {

public:
    image_irSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

};

class StaticGraph100023 : public IStaticGraphConfig
{
public:
    StaticGraph100023(GraphConfiguration100023* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100023();
    static const uint32_t hashCode = 1474625774;  // autogenerated

private:
    // Configuration
    GraphConfiguration100023 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode _lbffRgbIrWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100023 _imageSubGraph;
    irSubGraphTopology100023 _irSubGraph;
    image_irSubGraphTopology100023 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100024 : public GraphTopology {

public:
    imageSubGraphTopology100024(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 8, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[8];

};

class StaticGraph100024 : public IStaticGraphConfig
{
public:
    StaticGraph100024(GraphConfiguration100024* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100024();
    static const uint32_t hashCode = 3882192597;  // autogenerated

private:
    // Configuration
    GraphConfiguration100024 _graphConfiguration;

    /* Outer Nodes */
    LbffBayerNoGmvNoTnrNoSapOuterNode _lbffBayerNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100024 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[8];
};

class imageSubGraphTopology100040 : public GraphTopology {

public:
    imageSubGraphTopology100040(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class StaticGraph100040 : public IStaticGraphConfig
{
public:
    StaticGraph100040(GraphConfiguration100040* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100040();
    static const uint32_t hashCode = 3246908538;  // autogenerated

private:
    // Configuration
    GraphConfiguration100040 _graphConfiguration;

    /* Outer Nodes */
    LbffBayerWithGmvNoTnrNoSapOuterNode _lbffBayerWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100040 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[13];
};

class imageSubGraphTopology100041 : public GraphTopology {

public:
    imageSubGraphTopology100041(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100041 : public IStaticGraphConfig
{
public:
    StaticGraph100041(GraphConfiguration100041* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100041();
    static const uint32_t hashCode = 4081602974;  // autogenerated

private:
    // Configuration
    GraphConfiguration100041 _graphConfiguration;

    /* Outer Nodes */
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100041 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100042 : public GraphTopology {

public:
    imageSubGraphTopology100042(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100042 : public IStaticGraphConfig
{
public:
    StaticGraph100042(GraphConfiguration100042* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100042();
    static const uint32_t hashCode = 887432477;  // autogenerated

private:
    // Configuration
    GraphConfiguration100042 _graphConfiguration;

    /* Outer Nodes */
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100042 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[11];
};

class imageSubGraphTopology100027 : public GraphTopology {

public:
    imageSubGraphTopology100027(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvNoTnrNoSapOuterNode* lbffIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[9];

};

class StaticGraph100027 : public IStaticGraphConfig
{
public:
    StaticGraph100027(GraphConfiguration100027* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100027();
    static const uint32_t hashCode = 2928001630;  // autogenerated

private:
    // Configuration
    GraphConfiguration100027 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffIrNoGmvNoTnrNoSapOuterNode _lbffIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100027 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[9];
};

class imageSubGraphTopology100028 : public GraphTopology {

public:
    imageSubGraphTopology100028(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrWithGmvNoTnrNoSapOuterNode* lbffIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100028 : public IStaticGraphConfig
{
public:
    StaticGraph100028(GraphConfiguration100028* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100028();
    static const uint32_t hashCode = 2011278349;  // autogenerated

private:
    // Configuration
    GraphConfiguration100028 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffIrWithGmvNoTnrNoSapOuterNode _lbffIrWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100028 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100029 : public GraphTopology {

public:
    imageSubGraphTopology100029(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvWithTnrNoSapOuterNode* lbffIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];

};

class StaticGraph100029 : public IStaticGraphConfig
{
public:
    StaticGraph100029(GraphConfiguration100029* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100029();
    static const uint32_t hashCode = 922691010;  // autogenerated

private:
    // Configuration
    GraphConfiguration100029 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffIrNoGmvWithTnrNoSapOuterNode _lbffIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100029 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[12];
};

class imageSubGraphTopology100030 : public GraphTopology {

public:
    imageSubGraphTopology100030(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrWithGmvWithTnrNoSapOuterNode* lbffIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100030 : public IStaticGraphConfig
{
public:
    StaticGraph100030(GraphConfiguration100030* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100030();
    static const uint32_t hashCode = 3057898757;  // autogenerated

private:
    // Configuration
    GraphConfiguration100030 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffIrWithGmvWithTnrNoSapOuterNode _lbffIrWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100030 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100031 : public GraphTopology {

public:
    imageSubGraphTopology100031(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100031 : public IStaticGraphConfig
{
public:
    StaticGraph100031(GraphConfiguration100031* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100031();
    static const uint32_t hashCode = 3600854270;  // autogenerated

private:
    // Configuration
    GraphConfiguration100031 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100031 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[11];
};

class imageSubGraphTopology100032 : public GraphTopology {

public:
    imageSubGraphTopology100032(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100032 : public IStaticGraphConfig
{
public:
    StaticGraph100032(GraphConfiguration100032* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100032();
    static const uint32_t hashCode = 2018298469;  // autogenerated

private:
    // Configuration
    GraphConfiguration100032 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100032 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100033 : public GraphTopology {

public:
    imageSubGraphTopology100033(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100033 : public IStaticGraphConfig
{
public:
    StaticGraph100033(GraphConfiguration100033* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100033();
    static const uint32_t hashCode = 1273478754;  // autogenerated

private:
    // Configuration
    GraphConfiguration100033 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100033 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100034 : public GraphTopology {

public:
    imageSubGraphTopology100034(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100034 : public IStaticGraphConfig
{
public:
    StaticGraph100034(GraphConfiguration100034* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100034();
    static const uint32_t hashCode = 1962191709;  // autogenerated

private:
    // Configuration
    GraphConfiguration100034 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100034 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100141 : public GraphTopology {

public:
    imageSubGraphTopology100141(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100141 : public IStaticGraphConfig
{
public:
    StaticGraph100141(GraphConfiguration100141* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100141();
    static const uint32_t hashCode = 459279291;  // autogenerated

private:
    // Configuration
    GraphConfiguration100141 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100141 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100100 : public GraphTopology {

public:
    imageSubGraphTopology100100(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvNoTnrWithSapOuterNode* lbffBayerNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100100 : public IStaticGraphConfig
{
public:
    StaticGraph100100(GraphConfiguration100100* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100100();
    static const uint32_t hashCode = 3556363137;  // autogenerated

private:
    // Configuration
    GraphConfiguration100100 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvNoTnrWithSapOuterNode _lbffBayerNoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100100 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100101 : public GraphTopology {

public:
    imageSubGraphTopology100101(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvNoTnrWithSapOuterNode* lbffBayerWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100101 : public IStaticGraphConfig
{
public:
    StaticGraph100101(GraphConfiguration100101* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100101();
    static const uint32_t hashCode = 3566296298;  // autogenerated

private:
    // Configuration
    GraphConfiguration100101 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerWithGmvNoTnrWithSapOuterNode _lbffBayerWithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100101 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100102 : public GraphTopology {

public:
    imageSubGraphTopology100102(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithSapOuterNode* lbffBayerNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100102 : public IStaticGraphConfig
{
public:
    StaticGraph100102(GraphConfiguration100102* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100102();
    static const uint32_t hashCode = 2998053651;  // autogenerated

private:
    // Configuration
    GraphConfiguration100102 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvWithTnrWithSapOuterNode _lbffBayerNoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100102 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100157 : public GraphTopology {

public:
    imageSubGraphTopology100157(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithOpacityOuterNode* lbffBayerNoGmvWithTnrWithOpacityOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100157 : public IStaticGraphConfig
{
public:
    StaticGraph100157(GraphConfiguration100157* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100157();
    static const uint32_t hashCode = 117824145;  // autogenerated

private:
    // Configuration
    GraphConfiguration100157 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvWithTnrWithOpacityOuterNode _lbffBayerNoGmvWithTnrWithOpacityOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100157 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100103 : public GraphTopology {

public:
    imageSubGraphTopology100103(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvWithTnrWithSapOuterNode* lbffBayerWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100103 : public IStaticGraphConfig
{
public:
    StaticGraph100103(GraphConfiguration100103* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100103();
    static const uint32_t hashCode = 4074792460;  // autogenerated

private:
    // Configuration
    GraphConfiguration100103 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerWithGmvWithTnrWithSapOuterNode _lbffBayerWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100103 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology101114 : public GraphTopology {

public:
    imageSubGraphTopology101114(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrWithOpacityOuterNode* lbffDol2InputsNoGmvWithTnrWithOpacityOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph101114 : public IStaticGraphConfig
{
public:
    StaticGraph101114(GraphConfiguration101114* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph101114();
    static const uint32_t hashCode = 1694801375;  // autogenerated

private:
    // Configuration
    GraphConfiguration101114 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvWithTnrWithOpacityOuterNode _lbffDol2InputsNoGmvWithTnrWithOpacityOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology101114 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100135 : public GraphTopology {

public:
    imageSubGraphTopology100135(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithSapOuterNode* lbffBayerNoGmvWithTnrWithSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100135 : public IStaticGraphConfig
{
public:
    StaticGraph100135(GraphConfiguration100135* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100135();
    static const uint32_t hashCode = 3029162029;  // autogenerated

private:
    // Configuration
    GraphConfiguration100135 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerNoGmvWithTnrWithSapOuterNode _lbffBayerNoGmvWithTnrWithSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100135 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[27];
};

class imageSubGraphTopology100104 : public GraphTopology {

public:
    imageSubGraphTopology100104(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100104 : public IStaticGraphConfig
{
public:
    StaticGraph100104(GraphConfiguration100104* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100104();
    static const uint32_t hashCode = 1806917903;  // autogenerated

private:
    // Configuration
    GraphConfiguration100104 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf2NoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100104 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100105 : public GraphTopology {

public:
    imageSubGraphTopology100105(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf2WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100105 : public IStaticGraphConfig
{
public:
    StaticGraph100105(GraphConfiguration100105* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100105();
    static const uint32_t hashCode = 1551375760;  // autogenerated

private:
    // Configuration
    GraphConfiguration100105 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf2WithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100105 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100106 : public GraphTopology {

public:
    imageSubGraphTopology100106(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100106 : public IStaticGraphConfig
{
public:
    StaticGraph100106(GraphConfiguration100106* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100106();
    static const uint32_t hashCode = 597674829;  // autogenerated

private:
    // Configuration
    GraphConfiguration100106 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100106 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100166 : public GraphTopology {

public:
    imageSubGraphTopology100166(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100166 : public IStaticGraphConfig
{
public:
    StaticGraph100166(GraphConfiguration100166* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100166();
    static const uint32_t hashCode = 2536205451;  // autogenerated

private:
    // Configuration
    GraphConfiguration100166 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100166 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100107 : public GraphTopology {

public:
    imageSubGraphTopology100107(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100107 : public IStaticGraphConfig
{
public:
    StaticGraph100107(GraphConfiguration100107* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100107();
    static const uint32_t hashCode = 1366707606;  // autogenerated

private:
    // Configuration
    GraphConfiguration100107 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100107 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100145 : public GraphTopology {

public:
    imageSubGraphTopology100145(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100145 : public IStaticGraphConfig
{
public:
    StaticGraph100145(GraphConfiguration100145* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100145();
    static const uint32_t hashCode = 344075652;  // autogenerated

private:
    // Configuration
    GraphConfiguration100145 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100145 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology100108 : public GraphTopology {

public:
    imageSubGraphTopology100108(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf3NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100108 : public IStaticGraphConfig
{
public:
    StaticGraph100108(GraphConfiguration100108* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100108();
    static const uint32_t hashCode = 2133957557;  // autogenerated

private:
    // Configuration
    GraphConfiguration100108 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf3NoGmvNoTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100108 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100109 : public GraphTopology {

public:
    imageSubGraphTopology100109(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf3WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100109 : public IStaticGraphConfig
{
public:
    StaticGraph100109(GraphConfiguration100109* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100109();
    static const uint32_t hashCode = 2702714386;  // autogenerated

private:
    // Configuration
    GraphConfiguration100109 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf3WithGmvNoTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100109 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100110 : public GraphTopology {

public:
    imageSubGraphTopology100110(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100110 : public IStaticGraphConfig
{
public:
    StaticGraph100110(GraphConfiguration100110* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100110();
    static const uint32_t hashCode = 2511889479;  // autogenerated

private:
    // Configuration
    GraphConfiguration100110 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100110 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100111 : public GraphTopology {

public:
    imageSubGraphTopology100111(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100111 : public IStaticGraphConfig
{
public:
    StaticGraph100111(GraphConfiguration100111* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100111();
    static const uint32_t hashCode = 2606404484;  // autogenerated

private:
    // Configuration
    GraphConfiguration100111 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100111 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100136 : public GraphTopology {

public:
    imageSubGraphTopology100136(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100136 : public IStaticGraphConfig
{
public:
    StaticGraph100136(GraphConfiguration100136* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100136();
    static const uint32_t hashCode = 834725913;  // autogenerated

private:
    // Configuration
    GraphConfiguration100136 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100136 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100200 : public GraphTopology {

public:
    imageSubGraphTopology100200(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100200 : public IStaticGraphConfig
{
public:
    StaticGraph100200(GraphConfiguration100200* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100200();
    static const uint32_t hashCode = 3630993513;  // autogenerated

private:
    // Configuration
    GraphConfiguration100200 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100200 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100201 : public GraphTopology {

public:
    imageSubGraphTopology100201(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100201 : public IStaticGraphConfig
{
public:
    StaticGraph100201(GraphConfiguration100201* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100201();
    static const uint32_t hashCode = 2977685764;  // autogenerated

private:
    // Configuration
    GraphConfiguration100201 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100201 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100112 : public GraphTopology {

public:
    imageSubGraphTopology100112(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode* lbffDol2InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100112 : public IStaticGraphConfig
{
public:
    StaticGraph100112(GraphConfiguration100112* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100112();
    static const uint32_t hashCode = 1497638649;  // autogenerated

private:
    // Configuration
    GraphConfiguration100112 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode _lbffDol2InputsNoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100112 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100113 : public GraphTopology {

public:
    imageSubGraphTopology100113(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode* lbffDol2InputsWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100113 : public IStaticGraphConfig
{
public:
    StaticGraph100113(GraphConfiguration100113* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100113();
    static const uint32_t hashCode = 1545153954;  // autogenerated

private:
    // Configuration
    GraphConfiguration100113 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode _lbffDol2InputsWithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100113 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[26];
};

class imageSubGraphTopology100114 : public GraphTopology {

public:
    imageSubGraphTopology100114(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode* lbffDol2InputsNoGmvWithTnrWithSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100114 : public IStaticGraphConfig
{
public:
    StaticGraph100114(GraphConfiguration100114* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100114();
    static const uint32_t hashCode = 274750405;  // autogenerated

private:
    // Configuration
    GraphConfiguration100114 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode _lbffDol2InputsNoGmvWithTnrWithSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100114 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100146 : public GraphTopology {

public:
    imageSubGraphTopology100146(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode* lbffDol2InputsNoGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100146 : public IStaticGraphConfig
{
public:
    StaticGraph100146(GraphConfiguration100146* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100146();
    static const uint32_t hashCode = 2727273223;  // autogenerated

private:
    // Configuration
    GraphConfiguration100146 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode _lbffDol2InputsNoGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100146 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100115 : public GraphTopology {

public:
    imageSubGraphTopology100115(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode* lbffDol2InputsWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100115 : public IStaticGraphConfig
{
public:
    StaticGraph100115(GraphConfiguration100115* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100115();
    static const uint32_t hashCode = 1251250420;  // autogenerated

private:
    // Configuration
    GraphConfiguration100115 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode _lbffDol2InputsWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100115 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100116 : public GraphTopology {

public:
    imageSubGraphTopology100116(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode* lbffDol3InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100116 : public IStaticGraphConfig
{
public:
    StaticGraph100116(GraphConfiguration100116* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100116();
    static const uint32_t hashCode = 1189982662;  // autogenerated

private:
    // Configuration
    GraphConfiguration100116 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode _lbffDol3InputsNoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100116 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100117 : public GraphTopology {

public:
    imageSubGraphTopology100117(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode* lbffDol3InputsWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100117 : public IStaticGraphConfig
{
public:
    StaticGraph100117(GraphConfiguration100117* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100117();
    static const uint32_t hashCode = 2476175693;  // autogenerated

private:
    // Configuration
    GraphConfiguration100117 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode _lbffDol3InputsWithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100117 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100118 : public GraphTopology {

public:
    imageSubGraphTopology100118(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode* lbffDol3InputsNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100118 : public IStaticGraphConfig
{
public:
    StaticGraph100118(GraphConfiguration100118* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100118();
    static const uint32_t hashCode = 2508084080;  // autogenerated

private:
    // Configuration
    GraphConfiguration100118 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode _lbffDol3InputsNoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100118 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100119 : public GraphTopology {

public:
    imageSubGraphTopology100119(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode* lbffDol3InputsWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100119 : public IStaticGraphConfig
{
public:
    StaticGraph100119(GraphConfiguration100119* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100119();
    static const uint32_t hashCode = 591912599;  // autogenerated

private:
    // Configuration
    GraphConfiguration100119 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode _lbffDol3InputsWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100119 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology100120 : public GraphTopology {

public:
    imageSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100120 : public GraphTopology {

public:
    irSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100120 : public GraphTopology {

public:
    image_irSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100120 : public IStaticGraphConfig
{
public:
    StaticGraph100120(GraphConfiguration100120* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100120();
    static const uint32_t hashCode = 204985626;  // autogenerated

private:
    // Configuration
    GraphConfiguration100120 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode _lbffRgbIrNoGmvNoTnrWithSapOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100120 _imageSubGraph;
    irSubGraphTopology100120 _irSubGraph;
    image_irSubGraphTopology100120 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100121 : public GraphTopology {

public:
    imageSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100121 : public GraphTopology {

public:
    irSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100121 : public GraphTopology {

public:
    image_irSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100121 : public IStaticGraphConfig
{
public:
    StaticGraph100121(GraphConfiguration100121* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100121();
    static const uint32_t hashCode = 2675250681;  // autogenerated

private:
    // Configuration
    GraphConfiguration100121 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode _lbffRgbIrWithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100121 _imageSubGraph;
    irSubGraphTopology100121 _irSubGraph;
    image_irSubGraphTopology100121 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100122 : public GraphTopology {

public:
    imageSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100122 : public GraphTopology {

public:
    irSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100122 : public GraphTopology {

public:
    image_irSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100122 : public IStaticGraphConfig
{
public:
    StaticGraph100122(GraphConfiguration100122* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100122();
    static const uint32_t hashCode = 2950517008;  // autogenerated

private:
    // Configuration
    GraphConfiguration100122 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode _lbffRgbIrNoGmvWithTnrWithSapOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100122 _imageSubGraph;
    irSubGraphTopology100122 _irSubGraph;
    image_irSubGraphTopology100122 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[33];
};

class imageSubGraphTopology100123 : public GraphTopology {

public:
    imageSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100123 : public GraphTopology {

public:
    irSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 35, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[35];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100123 : public GraphTopology {

public:
    image_irSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 38, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[38];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100123 : public IStaticGraphConfig
{
public:
    StaticGraph100123(GraphConfiguration100123* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100123();
    static const uint32_t hashCode = 523833107;  // autogenerated

private:
    // Configuration
    GraphConfiguration100123 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode _lbffRgbIrWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100123 _imageSubGraph;
    irSubGraphTopology100123 _irSubGraph;
    image_irSubGraphTopology100123 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[38];
};

class imageSubGraphTopology100127 : public GraphTopology {

public:
    imageSubGraphTopology100127(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrNoGmvNoTnrWithSapOuterNode* lbffIrNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100127 : public IStaticGraphConfig
{
public:
    StaticGraph100127(GraphConfiguration100127* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100127();
    static const uint32_t hashCode = 923306631;  // autogenerated

private:
    // Configuration
    GraphConfiguration100127 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffIrNoGmvNoTnrWithSapOuterNode _lbffIrNoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100127 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100128 : public GraphTopology {

public:
    imageSubGraphTopology100128(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrWithGmvNoTnrWithSapOuterNode* lbffIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100128 : public IStaticGraphConfig
{
public:
    StaticGraph100128(GraphConfiguration100128* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100128();
    static const uint32_t hashCode = 3291442376;  // autogenerated

private:
    // Configuration
    GraphConfiguration100128 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffIrWithGmvNoTnrWithSapOuterNode _lbffIrWithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100128 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100129 : public GraphTopology {

public:
    imageSubGraphTopology100129(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrNoGmvWithTnrWithSapOuterNode* lbffIrNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100129 : public IStaticGraphConfig
{
public:
    StaticGraph100129(GraphConfiguration100129* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100129();
    static const uint32_t hashCode = 443369629;  // autogenerated

private:
    // Configuration
    GraphConfiguration100129 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffIrNoGmvWithTnrWithSapOuterNode _lbffIrNoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100129 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100130 : public GraphTopology {

public:
    imageSubGraphTopology100130(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrWithGmvWithTnrWithSapOuterNode* lbffIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100130 : public IStaticGraphConfig
{
public:
    StaticGraph100130(GraphConfiguration100130* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100130();
    static const uint32_t hashCode = 3111694518;  // autogenerated

private:
    // Configuration
    GraphConfiguration100130 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffIrWithGmvWithTnrWithSapOuterNode _lbffIrWithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100130 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[26];
};

class imageSubGraphTopology100131 : public GraphTopology {

public:
    imageSubGraphTopology100131(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100131 : public IStaticGraphConfig
{
public:
    StaticGraph100131(GraphConfiguration100131* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100131();
    static const uint32_t hashCode = 3745450829;  // autogenerated

private:
    // Configuration
    GraphConfiguration100131 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100131 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100132 : public GraphTopology {

public:
    imageSubGraphTopology100132(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100132 : public IStaticGraphConfig
{
public:
    StaticGraph100132(GraphConfiguration100132* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100132();
    static const uint32_t hashCode = 2780416462;  // autogenerated

private:
    // Configuration
    GraphConfiguration100132 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100132 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100133 : public GraphTopology {

public:
    imageSubGraphTopology100133(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100133 : public IStaticGraphConfig
{
public:
    StaticGraph100133(GraphConfiguration100133* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100133();
    static const uint32_t hashCode = 986156287;  // autogenerated

private:
    // Configuration
    GraphConfiguration100133 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100133 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100134 : public GraphTopology {

public:
    imageSubGraphTopology100134(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100134 : public IStaticGraphConfig
{
public:
    StaticGraph100134(GraphConfiguration100134* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100134();
    static const uint32_t hashCode = 1100112176;  // autogenerated

private:
    // Configuration
    GraphConfiguration100134 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100134 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100235 : public GraphTopology {

public:
    imageSubGraphTopology100235(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100235 : public IStaticGraphConfig
{
public:
    StaticGraph100235(GraphConfiguration100235* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100235();
    static const uint32_t hashCode = 2533625086;  // autogenerated

private:
    // Configuration
    GraphConfiguration100235 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerNoGmvNoTnrNoSapOuterNode _lbffBayerNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100235 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[11];
};

class imageSubGraphTopology100236 : public GraphTopology {

public:
    imageSubGraphTopology100236(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100236 : public IStaticGraphConfig
{
public:
    StaticGraph100236(GraphConfiguration100236* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100236();
    static const uint32_t hashCode = 3706116901;  // autogenerated

private:
    // Configuration
    GraphConfiguration100236 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerWithGmvNoTnrNoSapOuterNode _lbffBayerWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100236 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100202 : public GraphTopology {

public:
    imageSubGraphTopology100202(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100202 : public IStaticGraphConfig
{
public:
    StaticGraph100202(GraphConfiguration100202* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100202();
    static const uint32_t hashCode = 269812994;  // autogenerated

private:
    // Configuration
    GraphConfiguration100202 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100202 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100203 : public GraphTopology {

public:
    imageSubGraphTopology100203(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100203 : public IStaticGraphConfig
{
public:
    StaticGraph100203(GraphConfiguration100203* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100203();
    static const uint32_t hashCode = 2586179389;  // autogenerated

private:
    // Configuration
    GraphConfiguration100203 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100203 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100279 : public GraphTopology {

public:
    imageSubGraphTopology100279(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100279 : public IStaticGraphConfig
{
public:
    StaticGraph100279(GraphConfiguration100279* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100279();
    static const uint32_t hashCode = 1755203480;  // autogenerated

private:
    // Configuration
    GraphConfiguration100279 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100279 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100280 : public GraphTopology {

public:
    imageSubGraphTopology100280(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100280 : public IStaticGraphConfig
{
public:
    StaticGraph100280(GraphConfiguration100280* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100280();
    static const uint32_t hashCode = 467449978;  // autogenerated

private:
    // Configuration
    GraphConfiguration100280 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100280 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100281 : public GraphTopology {

public:
    imageSubGraphTopology100281(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100281 : public IStaticGraphConfig
{
public:
    StaticGraph100281(GraphConfiguration100281* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100281();
    static const uint32_t hashCode = 7327794;  // autogenerated

private:
    // Configuration
    GraphConfiguration100281 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100281 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100204 : public GraphTopology {

public:
    imageSubGraphTopology100204(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100204 : public IStaticGraphConfig
{
public:
    StaticGraph100204(GraphConfiguration100204* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100204();
    static const uint32_t hashCode = 997851588;  // autogenerated

private:
    // Configuration
    GraphConfiguration100204 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100204 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100205 : public GraphTopology {

public:
    imageSubGraphTopology100205(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100205 : public IStaticGraphConfig
{
public:
    StaticGraph100205(GraphConfiguration100205* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100205();
    static const uint32_t hashCode = 3755277027;  // autogenerated

private:
    // Configuration
    GraphConfiguration100205 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100205 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100206 : public GraphTopology {

public:
    imageSubGraphTopology100206(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100206 : public IStaticGraphConfig
{
public:
    StaticGraph100206(GraphConfiguration100206* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100206();
    static const uint32_t hashCode = 3549933872;  // autogenerated

private:
    // Configuration
    GraphConfiguration100206 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100206 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100266 : public GraphTopology {

public:
    imageSubGraphTopology100266(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100266 : public IStaticGraphConfig
{
public:
    StaticGraph100266(GraphConfiguration100266* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100266();
    static const uint32_t hashCode = 534773418;  // autogenerated

private:
    // Configuration
    GraphConfiguration100266 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100266 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100207 : public GraphTopology {

public:
    imageSubGraphTopology100207(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100207 : public IStaticGraphConfig
{
public:
    StaticGraph100207(GraphConfiguration100207* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100207();
    static const uint32_t hashCode = 2439336771;  // autogenerated

private:
    // Configuration
    GraphConfiguration100207 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100207 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100267 : public GraphTopology {

public:
    imageSubGraphTopology100267(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100267 : public IStaticGraphConfig
{
public:
    StaticGraph100267(GraphConfiguration100267* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100267();
    static const uint32_t hashCode = 432090845;  // autogenerated

private:
    // Configuration
    GraphConfiguration100267 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100267 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100208 : public GraphTopology {

public:
    imageSubGraphTopology100208(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];

};

class StaticGraph100208 : public IStaticGraphConfig
{
public:
    StaticGraph100208(GraphConfiguration100208* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100208();
    static const uint32_t hashCode = 3622930326;  // autogenerated

private:
    // Configuration
    GraphConfiguration100208 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100208 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[12];
};

class imageSubGraphTopology100209 : public GraphTopology {

public:
    imageSubGraphTopology100209(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class StaticGraph100209 : public IStaticGraphConfig
{
public:
    StaticGraph100209(GraphConfiguration100209* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100209();
    static const uint32_t hashCode = 2679332857;  // autogenerated

private:
    // Configuration
    GraphConfiguration100209 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100209 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100210 : public GraphTopology {

public:
    imageSubGraphTopology100210(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class StaticGraph100210 : public IStaticGraphConfig
{
public:
    StaticGraph100210(GraphConfiguration100210* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100210();
    static const uint32_t hashCode = 2711112138;  // autogenerated

private:
    // Configuration
    GraphConfiguration100210 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100210 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100211 : public GraphTopology {

public:
    imageSubGraphTopology100211(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100211 : public IStaticGraphConfig
{
public:
    StaticGraph100211(GraphConfiguration100211* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100211();
    static const uint32_t hashCode = 1748768337;  // autogenerated

private:
    // Configuration
    GraphConfiguration100211 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100211 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100245 : public GraphTopology {

public:
    imageSubGraphTopology100245(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class StaticGraph100245 : public IStaticGraphConfig
{
public:
    StaticGraph100245(GraphConfiguration100245* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100245();
    static const uint32_t hashCode = 1036862344;  // autogenerated

private:
    // Configuration
    GraphConfiguration100245 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100245 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100212 : public GraphTopology {

public:
    imageSubGraphTopology100212(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode* lbffDol2InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100212 : public IStaticGraphConfig
{
public:
    StaticGraph100212(GraphConfiguration100212* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100212();
    static const uint32_t hashCode = 4128543248;  // autogenerated

private:
    // Configuration
    GraphConfiguration100212 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode _lbffDol2InputsNoGmvNoTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100212 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100213 : public GraphTopology {

public:
    imageSubGraphTopology100213(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode* lbffDol2InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100213 : public IStaticGraphConfig
{
public:
    StaticGraph100213(GraphConfiguration100213* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100213();
    static const uint32_t hashCode = 3028697525;  // autogenerated

private:
    // Configuration
    GraphConfiguration100213 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode _lbffDol2InputsWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100213 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100214 : public GraphTopology {

public:
    imageSubGraphTopology100214(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode* lbffDol2InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100214 : public IStaticGraphConfig
{
public:
    StaticGraph100214(GraphConfiguration100214* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100214();
    static const uint32_t hashCode = 1088099756;  // autogenerated

private:
    // Configuration
    GraphConfiguration100214 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode _lbffDol2InputsNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100214 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100215 : public GraphTopology {

public:
    imageSubGraphTopology100215(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode* lbffDol2InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

};

class StaticGraph100215 : public IStaticGraphConfig
{
public:
    StaticGraph100215(GraphConfiguration100215* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100215();
    static const uint32_t hashCode = 3292651501;  // autogenerated

private:
    // Configuration
    GraphConfiguration100215 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode _lbffDol2InputsWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100215 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100216 : public GraphTopology {

public:
    imageSubGraphTopology100216(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode* lbffDol3InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100216 : public IStaticGraphConfig
{
public:
    StaticGraph100216(GraphConfiguration100216* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100216();
    static const uint32_t hashCode = 2055501779;  // autogenerated

private:
    // Configuration
    GraphConfiguration100216 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode _lbffDol3InputsNoGmvNoTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100216 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100217 : public GraphTopology {

public:
    imageSubGraphTopology100217(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode* lbffDol3InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100217 : public IStaticGraphConfig
{
public:
    StaticGraph100217(GraphConfiguration100217* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100217();
    static const uint32_t hashCode = 4013164018;  // autogenerated

private:
    // Configuration
    GraphConfiguration100217 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode _lbffDol3InputsWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100217 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100218 : public GraphTopology {

public:
    imageSubGraphTopology100218(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode* lbffDol3InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph100218 : public IStaticGraphConfig
{
public:
    StaticGraph100218(GraphConfiguration100218* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100218();
    static const uint32_t hashCode = 3208020355;  // autogenerated

private:
    // Configuration
    GraphConfiguration100218 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode _lbffDol3InputsNoGmvWithTnrNoSapOuterNode;
    SwNntmOuterNode _swNntmOuterNode;
    SwScalerOuterNode _swScalerOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100218 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100219 : public GraphTopology {

public:
    imageSubGraphTopology100219(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode* lbffDol3InputsWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph100219 : public IStaticGraphConfig
{
public:
    StaticGraph100219(GraphConfiguration100219* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100219();
    static const uint32_t hashCode = 1606204982;  // autogenerated

private:
    // Configuration
    GraphConfiguration100219 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    LbffDolSmoothOuterNode _lbffDolSmoothOuterNode;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode _lbffDol3InputsWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100219 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100220 : public GraphTopology {

public:
    imageSubGraphTopology100220(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];

};

class irSubGraphTopology100220 : public GraphTopology {

public:
    irSubGraphTopology100220(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class image_irSubGraphTopology100220 : public GraphTopology {

public:
    image_irSubGraphTopology100220(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100220 : public IStaticGraphConfig
{
public:
    StaticGraph100220(GraphConfiguration100220* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100220();
    static const uint32_t hashCode = 3260527689;  // autogenerated

private:
    // Configuration
    GraphConfiguration100220 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrNoGmvNoTnrNoSapOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100220 _imageSubGraph;
    irSubGraphTopology100220 _irSubGraph;
    image_irSubGraphTopology100220 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100221 : public GraphTopology {

public:
    imageSubGraphTopology100221(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

};

class irSubGraphTopology100221 : public GraphTopology {

public:
    irSubGraphTopology100221(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class image_irSubGraphTopology100221 : public GraphTopology {

public:
    image_irSubGraphTopology100221(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

};

class StaticGraph100221 : public IStaticGraphConfig
{
public:
    StaticGraph100221(GraphConfiguration100221* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100221();
    static const uint32_t hashCode = 2898871730;  // autogenerated

private:
    // Configuration
    GraphConfiguration100221 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode _lbffRgbIrWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode _lbffRgbIrIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100221 _imageSubGraph;
    irSubGraphTopology100221 _irSubGraph;
    image_irSubGraphTopology100221 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100222 : public GraphTopology {

public:
    imageSubGraphTopology100222(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

};

class irSubGraphTopology100222 : public GraphTopology {

public:
    irSubGraphTopology100222(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class image_irSubGraphTopology100222 : public GraphTopology {

public:
    image_irSubGraphTopology100222(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

};

class StaticGraph100222 : public IStaticGraphConfig
{
public:
    StaticGraph100222(GraphConfiguration100222* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100222();
    static const uint32_t hashCode = 654739193;  // autogenerated

private:
    // Configuration
    GraphConfiguration100222 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrNoGmvWithTnrNoSapOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100222 _imageSubGraph;
    irSubGraphTopology100222 _irSubGraph;
    image_irSubGraphTopology100222 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100223 : public GraphTopology {

public:
    imageSubGraphTopology100223(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

};

class irSubGraphTopology100223 : public GraphTopology {

public:
    irSubGraphTopology100223(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

};

class image_irSubGraphTopology100223 : public GraphTopology {

public:
    image_irSubGraphTopology100223(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 30, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[30];

};

class StaticGraph100223 : public IStaticGraphConfig
{
public:
    StaticGraph100223(GraphConfiguration100223* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100223();
    static const uint32_t hashCode = 3363320522;  // autogenerated

private:
    // Configuration
    GraphConfiguration100223 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode _lbffRgbIrWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode _lbffRgbIrIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100223 _imageSubGraph;
    irSubGraphTopology100223 _irSubGraph;
    image_irSubGraphTopology100223 _image_irSubGraph;

    // All graph links

    GraphLink _graphLinks[30];
};

class imageSubGraphTopology100224 : public GraphTopology {

public:
    imageSubGraphTopology100224(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 8, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[8];

};

class StaticGraph100224 : public IStaticGraphConfig
{
public:
    StaticGraph100224(GraphConfiguration100224* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100224();
    static const uint32_t hashCode = 1855472042;  // autogenerated

private:
    // Configuration
    GraphConfiguration100224 _graphConfiguration;

    /* Outer Nodes */
    WithCvOuterNode _withCvOuterNode;
    LbffBayerNoGmvNoTnrNoSapOuterNode _lbffBayerNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100224 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[10];
};

class imageSubGraphTopology100240 : public GraphTopology {

public:
    imageSubGraphTopology100240(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class StaticGraph100240 : public IStaticGraphConfig
{
public:
    StaticGraph100240(GraphConfiguration100240* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100240();
    static const uint32_t hashCode = 4169757189;  // autogenerated

private:
    // Configuration
    GraphConfiguration100240 _graphConfiguration;

    /* Outer Nodes */
    WithCvOuterNode _withCvOuterNode;
    LbffBayerWithGmvNoTnrNoSapOuterNode _lbffBayerWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100240 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100241 : public GraphTopology {

public:
    imageSubGraphTopology100241(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100241 : public IStaticGraphConfig
{
public:
    StaticGraph100241(GraphConfiguration100241* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100241();
    static const uint32_t hashCode = 4276738301;  // autogenerated

private:
    // Configuration
    GraphConfiguration100241 _graphConfiguration;

    /* Outer Nodes */
    WithCvOuterNode _withCvOuterNode;
    LbffBayerWithGmvWithTnrNoSapOuterNode _lbffBayerWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100241 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100242 : public GraphTopology {

public:
    imageSubGraphTopology100242(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100242 : public IStaticGraphConfig
{
public:
    StaticGraph100242(GraphConfiguration100242* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100242();
    static const uint32_t hashCode = 3420854654;  // autogenerated

private:
    // Configuration
    GraphConfiguration100242 _graphConfiguration;

    /* Outer Nodes */
    WithCvOuterNode _withCvOuterNode;
    LbffBayerNoGmvWithTnrNoSapOuterNode _lbffBayerNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100242 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[13];
};

class imageSubGraphTopology100227 : public GraphTopology {

public:
    imageSubGraphTopology100227(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffIrNoGmvNoTnrNoSapOuterNode* lbffIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];

};

class StaticGraph100227 : public IStaticGraphConfig
{
public:
    StaticGraph100227(GraphConfiguration100227* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100227();
    static const uint32_t hashCode = 1362805226;  // autogenerated

private:
    // Configuration
    GraphConfiguration100227 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffIrNoGmvNoTnrNoSapOuterNode _lbffIrNoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100227 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[11];
};

class imageSubGraphTopology100228 : public GraphTopology {

public:
    imageSubGraphTopology100228(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffIrWithGmvNoTnrNoSapOuterNode* lbffIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100228 : public IStaticGraphConfig
{
public:
    StaticGraph100228(GraphConfiguration100228* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100228();
    static const uint32_t hashCode = 3728741665;  // autogenerated

private:
    // Configuration
    GraphConfiguration100228 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffIrWithGmvNoTnrNoSapOuterNode _lbffIrWithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100228 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100229 : public GraphTopology {

public:
    imageSubGraphTopology100229(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffIrNoGmvWithTnrNoSapOuterNode* lbffIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[14];

};

class StaticGraph100229 : public IStaticGraphConfig
{
public:
    StaticGraph100229(GraphConfiguration100229* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100229();
    static const uint32_t hashCode = 421618222;  // autogenerated

private:
    // Configuration
    GraphConfiguration100229 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffIrNoGmvWithTnrNoSapOuterNode _lbffIrNoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100229 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[14];
};

class imageSubGraphTopology100230 : public GraphTopology {

public:
    imageSubGraphTopology100230(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffIrWithGmvWithTnrNoSapOuterNode* lbffIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

};

class StaticGraph100230 : public IStaticGraphConfig
{
public:
    StaticGraph100230(GraphConfiguration100230* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100230();
    static const uint32_t hashCode = 2638994777;  // autogenerated

private:
    // Configuration
    GraphConfiguration100230 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffIrWithGmvWithTnrNoSapOuterNode _lbffIrWithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100230 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100231 : public GraphTopology {

public:
    imageSubGraphTopology100231(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[13];

};

class StaticGraph100231 : public IStaticGraphConfig
{
public:
    StaticGraph100231(GraphConfiguration100231* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100231();
    static const uint32_t hashCode = 2840555034;  // autogenerated

private:
    // Configuration
    GraphConfiguration100231 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100231 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[13];
};

class imageSubGraphTopology100232 : public GraphTopology {

public:
    imageSubGraphTopology100232(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

};

class StaticGraph100232 : public IStaticGraphConfig
{
public:
    StaticGraph100232(GraphConfiguration100232* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100232();
    static const uint32_t hashCode = 642246825;  // autogenerated

private:
    // Configuration
    GraphConfiguration100232 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100232 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100233 : public GraphTopology {

public:
    imageSubGraphTopology100233(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

};

class StaticGraph100233 : public IStaticGraphConfig
{
public:
    StaticGraph100233(GraphConfiguration100233* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100233();
    static const uint32_t hashCode = 3590707710;  // autogenerated

private:
    // Configuration
    GraphConfiguration100233 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100233 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100234 : public GraphTopology {

public:
    imageSubGraphTopology100234(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

};

class StaticGraph100234 : public IStaticGraphConfig
{
public:
    StaticGraph100234(GraphConfiguration100234* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100234();
    static const uint32_t hashCode = 3104810881;  // autogenerated

private:
    // Configuration
    GraphConfiguration100234 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode _lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100234 _imageSubGraph;

    // All graph links

    GraphLink _graphLinks[21];
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
    StaticGraph100026(GraphConfiguration100026* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100026();
    static const uint32_t hashCode = 3562265251;  // autogenerated

private:
    // Configuration
    GraphConfiguration100026 _graphConfiguration;

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

class rawSubGraphTopology100059 : public GraphTopology {

public:
    rawSubGraphTopology100059(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}
    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[4];

};

class StaticGraph100059 : public IStaticGraphConfig
{
public:
    StaticGraph100059(GraphConfiguration100059* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100059();
    static const uint32_t hashCode = 2170417703;  // autogenerated

private:
    // Configuration
    GraphConfiguration100059 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100059 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[4];
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
    StaticGraph100035(GraphConfiguration100035* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100035();
    static const uint32_t hashCode = 1941820013;  // autogenerated

private:
    // Configuration
    GraphConfiguration100035 _graphConfiguration;

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
    StaticGraph100036(GraphConfiguration100036* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100036();
    static const uint32_t hashCode = 1941820013;  // autogenerated

private:
    // Configuration
    GraphConfiguration100036 _graphConfiguration;

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

class rawSubGraphTopology100037 : public GraphTopology {

public:
    rawSubGraphTopology100037(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}
    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[4];

};

class StaticGraph100037 : public IStaticGraphConfig
{
public:
    StaticGraph100037(GraphConfiguration100037* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100037();
    static const uint32_t hashCode = 2170417703;  // autogenerated

private:
    // Configuration
    GraphConfiguration100037 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100037 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[4];
};

class rawSubGraphTopology100058 : public GraphTopology {

public:
    rawSubGraphTopology100058(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}
    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[4];

};

class StaticGraph100058 : public IStaticGraphConfig
{
public:
    StaticGraph100058(GraphConfiguration100058* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100058();
    static const uint32_t hashCode = 2170417703;  // autogenerated

private:
    // Configuration
    GraphConfiguration100058 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100058 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[4];
};

class rawSubGraphTopology100038 : public GraphTopology {

public:
    rawSubGraphTopology100038(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 6, sinkMappingConfiguration) {}
    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[6];

};

class StaticGraph100038 : public IStaticGraphConfig
{
public:
    StaticGraph100038(GraphConfiguration100038* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100038();
    static const uint32_t hashCode = 88788097;  // autogenerated

private:
    // Configuration
    GraphConfiguration100038 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100038 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[6];
};

class rawSubGraphTopology101138 : public GraphTopology {

public:
    rawSubGraphTopology101138(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 6, sinkMappingConfiguration) {}
    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[6];

};

class StaticGraph101138 : public IStaticGraphConfig
{
public:
    StaticGraph101138(GraphConfiguration101138* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph101138();
    static const uint32_t hashCode = 88788097;  // autogenerated

private:
    // Configuration
    GraphConfiguration101138 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology101138 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[6];
};

class rawSubGraphTopology100039 : public GraphTopology {

public:
    rawSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration) : GraphTopology(subGraphLinks, 6, sinkMappingConfiguration) {}
    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[6];

};

class StaticGraph100039 : public IStaticGraphConfig
{
public:
    StaticGraph100039(GraphConfiguration100039* selectedGraphConfiguration, VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode, int32_t selectedSettingsId, int32_t additionalFeaturesBit, bool isIsysUnpacked, StaticGraphConfigurationInformation* configurationInformation);
    ~StaticGraph100039();
    static const uint32_t hashCode = 88788097;  // autogenerated

private:
    // Configuration
    GraphConfiguration100039 _graphConfiguration;

    /* Outer Nodes */
    IsysPdaf2WithCvOuterNode _isysPdaf2WithCvOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    rawSubGraphTopology100039 _rawSubGraph;

    // All graph links

    GraphLink _graphLinks[6];
};

#endif