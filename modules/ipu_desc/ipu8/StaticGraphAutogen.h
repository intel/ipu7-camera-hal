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

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "StaticGraphBinaryAutogen.h"
#include "StaticGraphTypesAutogen.h"

#define SUPPORT_KEY_RESOLUTIONS 0

enum InnerNodeOption {
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
              uint32_t operationMode, uint32_t streamId, uint8_t nodeNumberOfFragments);

    uint8_t GetNumberOfFragments();

    void SetDisabledKernels(uint64_t disabledRunKernelsBitmap[2]);

 protected:
    void InitRunKernels(uint16_t* kernelsUuids, uint64_t kernelsRcbBitmap[2],
                        StaticGraphKernelRes* resolutionInfos,
                        uint64_t kernelsResolutionHistoryGroupBitmap[2],
                        uint64_t kernelFragmentDescriptorGroupBitmap[2],
                        StaticGraphKernelRes* resolutionHistories,
                        StaticGraphKernelBppConfiguration* bppInfos, uint8_t* systemApisSizes,
                        uint8_t* systemApiData, KernelFragments* fragmentDescriptors);
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
    virtual ~IStaticGraphConfig() {}
    IStaticGraphConfig(SensorMode* selectedSensorMode, VirtualSinkMapping* sinkMappingConfiguration,
                       int32_t graphId, int32_t selectedSettingsId);
    StaticGraphStatus getGraphTopology(GraphTopology** topology);
    StaticGraphStatus getSensorMode(SensorMode** sensorMode);
    StaticGraphStatus getGraphId(int32_t* id);
    StaticGraphStatus getSettingsId(int32_t* id);
    StaticGraphStatus getVirtualSinkConnection(VirtualSink& virtualSink, HwSink* hwSink);

 protected:
    SensorMode* _selectedSensorMode = nullptr;
    GraphTopology* _selectedGraphTopology = nullptr;
    VirtualSinkMapping* _sinkMappingConfiguration = &_selectedSinkMappingConfiguration;

 private:
    int32_t _graphId;
    int32_t _settingsId;
    VirtualSinkMapping _selectedSinkMappingConfiguration;
};

#pragma pack(push, 4)

struct IsysOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[20];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[17];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[34];
    uint8_t systemApiConfiguration[2144];
};

struct LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[24];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[21];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2824];
};

struct SwGdcOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[31];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3474];
};

struct LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4154];
};

struct SwNntmOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
    uint8_t systemApiConfiguration[5];
};

struct SwScalerOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[1];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[1];
    StaticGraphKernelRes resolutionHistories[1];
    StaticGraphKernelBppConfiguration bppInfos[1];
};

struct IsysPdaf2OuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[24];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[21];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2616];
};

struct LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[28];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3296];
};

struct LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[3946];
};

struct LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[39];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[33];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[54];
    uint8_t systemApiConfiguration[4626];
};

struct LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2396];
};

struct LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[27];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[41];
    uint8_t systemApiConfiguration[3076];
};

struct LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3726];
};

struct LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[53];
    uint8_t systemApiConfiguration[4406];
};

struct IsysDolOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[2];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[2];
};

struct LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[22];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[19];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2589];
};

struct LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[26];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[23];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[41];
    uint8_t systemApiConfiguration[3269];
};

struct LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3919];
};

struct LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[53];
    uint8_t systemApiConfiguration[4599];
};

struct LbffDolSmoothOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[3];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[2];
    StaticGraphKernelRes resolutionHistories[2];
    StaticGraphKernelBppConfiguration bppInfos[8];
    uint8_t systemApiConfiguration[476];
};

struct LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[16];
    StaticGraphKernelBppConfiguration bppInfos[38];
    uint8_t systemApiConfiguration[2809];
};

struct LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[27];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[20];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3489];
};

struct LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[22];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4139];
};

struct LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[54];
    uint8_t systemApiConfiguration[4819];
};

struct LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[22];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[19];
    StaticGraphKernelRes resolutionHistories[15];
    StaticGraphKernelBppConfiguration bppInfos[36];
    uint8_t systemApiConfiguration[2584];
};

struct LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[19];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[16];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[32];
    uint8_t systemApiConfiguration[2124];
};

struct LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[26];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[23];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[40];
    uint8_t systemApiConfiguration[3264];
};

struct LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[48];
    uint8_t systemApiConfiguration[3914];
};

struct LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[30];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[44];
    uint8_t systemApiConfiguration[3454];
};

struct LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[25];
    StaticGraphKernelBppConfiguration bppInfos[52];
    uint8_t systemApiConfiguration[4594];
};

struct LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[19];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[16];
    StaticGraphKernelRes resolutionHistories[13];
    StaticGraphKernelBppConfiguration bppInfos[33];
    uint8_t systemApiConfiguration[2144];
};

struct LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[23];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[20];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[37];
    uint8_t systemApiConfiguration[2824];
};

struct LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[30];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[19];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3474];
};

struct LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[4154];
};

struct LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[25];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[22];
    StaticGraphKernelRes resolutionHistories[17];
    StaticGraphKernelBppConfiguration bppInfos[39];
    uint8_t systemApiConfiguration[2836];
};

struct LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[29];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[26];
    StaticGraphKernelRes resolutionHistories[21];
    StaticGraphKernelBppConfiguration bppInfos[43];
    uint8_t systemApiConfiguration[3516];
};

struct LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[36];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[30];
    StaticGraphKernelRes resolutionHistories[23];
    StaticGraphKernelBppConfiguration bppInfos[51];
    uint8_t systemApiConfiguration[4166];
};

struct LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[40];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[34];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[55];
    uint8_t systemApiConfiguration[4846];
};

struct IsysWithCvOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[4];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[3];
    StaticGraphKernelRes resolutionHistories[4];
    StaticGraphKernelBppConfiguration bppInfos[4];
    uint8_t systemApiConfiguration[54];
};

struct SwSegnetOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
};

struct LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[31];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[25];
    StaticGraphKernelRes resolutionHistories[24];
    StaticGraphKernelBppConfiguration bppInfos[42];
    uint8_t systemApiConfiguration[3024];
};

struct LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3704];
};

struct LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[47];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[37];
    StaticGraphKernelRes resolutionHistories[35];
    StaticGraphKernelBppConfiguration bppInfos[58];
    uint8_t systemApiConfiguration[4794];
};

struct LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[51];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[41];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5474];
};

struct IsysPdaf2WithCvOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[5];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[5];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[54];
};

struct LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[35];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[29];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3496];
};

struct LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[39];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[33];
    StaticGraphKernelRes resolutionHistories[32];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4176];
};

struct LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[51];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[41];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5266];
};

struct LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[55];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[45];
    StaticGraphKernelRes resolutionHistories[43];
    StaticGraphKernelBppConfiguration bppInfos[66];
    uint8_t systemApiConfiguration[5946];
};

struct LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3276];
};

struct LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[3956];
};

struct LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[50];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[40];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[61];
    uint8_t systemApiConfiguration[5046];
};

struct LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[54];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[44];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[65];
    uint8_t systemApiConfiguration[5726];
};

struct IsysDolWithCvOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[5];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[4];
    StaticGraphKernelRes resolutionHistories[5];
    StaticGraphKernelBppConfiguration bppInfos[5];
    uint8_t systemApiConfiguration[54];
};

struct LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3469];
};

struct LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[49];
    uint8_t systemApiConfiguration[4149];
};

struct LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[49];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[39];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[61];
    uint8_t systemApiConfiguration[5239];
};

struct LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[53];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[43];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[65];
    uint8_t systemApiConfiguration[5919];
};

struct LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[27];
    StaticGraphKernelBppConfiguration bppInfos[46];
    uint8_t systemApiConfiguration[3689];
};

struct LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[38];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[32];
    StaticGraphKernelRes resolutionHistories[31];
    StaticGraphKernelBppConfiguration bppInfos[50];
    uint8_t systemApiConfiguration[4369];
};

struct LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[50];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[40];
    StaticGraphKernelRes resolutionHistories[38];
    StaticGraphKernelBppConfiguration bppInfos[62];
    uint8_t systemApiConfiguration[5459];
};

struct LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[54];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[44];
    StaticGraphKernelRes resolutionHistories[42];
    StaticGraphKernelBppConfiguration bppInfos[66];
    uint8_t systemApiConfiguration[6139];
};

struct LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[33];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[27];
    StaticGraphKernelRes resolutionHistories[26];
    StaticGraphKernelBppConfiguration bppInfos[44];
    uint8_t systemApiConfiguration[3464];
};

struct LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[37];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[31];
    StaticGraphKernelRes resolutionHistories[30];
    StaticGraphKernelBppConfiguration bppInfos[48];
    uint8_t systemApiConfiguration[4144];
};

struct LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[49];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[39];
    StaticGraphKernelRes resolutionHistories[37];
    StaticGraphKernelBppConfiguration bppInfos[60];
    uint8_t systemApiConfiguration[5234];
};

struct LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[53];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[43];
    StaticGraphKernelRes resolutionHistories[41];
    StaticGraphKernelBppConfiguration bppInfos[64];
    uint8_t systemApiConfiguration[5914];
};

struct LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[30];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[24];
    StaticGraphKernelRes resolutionHistories[24];
    StaticGraphKernelBppConfiguration bppInfos[41];
    uint8_t systemApiConfiguration[3024];
};

struct LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[34];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[28];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[45];
    uint8_t systemApiConfiguration[3704];
};

struct LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[46];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[36];
    StaticGraphKernelRes resolutionHistories[35];
    StaticGraphKernelBppConfiguration bppInfos[57];
    uint8_t systemApiConfiguration[4794];
};

struct LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[50];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[40];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[61];
    uint8_t systemApiConfiguration[5474];
};

struct LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[36];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[30];
    StaticGraphKernelRes resolutionHistories[28];
    StaticGraphKernelBppConfiguration bppInfos[47];
    uint8_t systemApiConfiguration[3716];
};

struct LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[40];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[34];
    StaticGraphKernelRes resolutionHistories[32];
    StaticGraphKernelBppConfiguration bppInfos[51];
    uint8_t systemApiConfiguration[4396];
};

struct LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[52];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[42];
    StaticGraphKernelRes resolutionHistories[39];
    StaticGraphKernelBppConfiguration bppInfos[63];
    uint8_t systemApiConfiguration[5486];
};

struct LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration {
    uint8_t numberOfFragments = 0;
    KernelFragments fragmentConfigurations[56];
    uint32_t streamId = 0;
    uint8_t tuningMode = 0;
    StaticGraphKernelRes resolutionInfos[46];
    StaticGraphKernelRes resolutionHistories[43];
    StaticGraphKernelBppConfiguration bppInfos[67];
    uint8_t systemApiConfiguration[6166];
};

struct GraphConfiguration100000 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100001 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100002 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100003 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100079 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100080 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100081 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100004 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100005 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100006 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100007 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2OuterNodeConfiguration isysPdaf2OuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100008 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[10];
};

struct GraphConfiguration100009 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100010 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100011 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100045 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100012 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100013 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration
        lbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100014 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100015 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100016 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100017 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration
        lbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100018 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration;
    SwNntmOuterNodeConfiguration swNntmOuterNodeConfiguration;
    SwScalerOuterNodeConfiguration swScalerOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100019 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolOuterNodeConfiguration isysDolOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100020 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100021 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration
        lbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100022 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100023 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100024 {
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration lbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[8];
};

struct GraphConfiguration100040 {
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[13];
};

struct GraphConfiguration100041 {
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100042 {
    VirtualSinkMapping sinkMappingConfiguration;
    LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100027 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration lbffIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[9];
};

struct GraphConfiguration100028 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration lbffIrWithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100029 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration lbffIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[12];
};

struct GraphConfiguration100030 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration lbffIrWithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100031 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[11];
};

struct GraphConfiguration100032 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100033 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[14];
};

struct GraphConfiguration100034 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100100 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100101 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100102 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100103 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100104 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[18];
};

struct GraphConfiguration100105 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100106 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100107 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100108 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[16];
};

struct GraphConfiguration100109 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100110 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100111 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[26];
};

struct GraphConfiguration100112 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration
        lbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[19];
};

struct GraphConfiguration100113 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration
        lbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100114 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration
        lbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[24];
};

struct GraphConfiguration100115 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration
        lbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[29];
};

struct GraphConfiguration100116 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration
        lbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[21];
};

struct GraphConfiguration100117 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration
        lbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[26];
};

struct GraphConfiguration100118 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration
        lbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[26];
};

struct GraphConfiguration100119 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    LbffDolSmoothOuterNodeConfiguration lbffDolSmoothOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration
        lbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100120 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration
        lbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[23];
};

struct GraphConfiguration100121 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration
        lbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[28];
};

struct GraphConfiguration100122 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration
        lbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[31];
};

struct GraphConfiguration100123 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration
        lbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration
        lbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[36];
};

struct GraphConfiguration100127 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration lbffIrNoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[15];
};

struct GraphConfiguration100128 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration lbffIrWithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100129 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration lbffIrNoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[20];
};

struct GraphConfiguration100130 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration
        lbffIrWithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[25];
};

struct GraphConfiguration100131 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[17];
};

struct GraphConfiguration100132 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100133 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[22];
};

struct GraphConfiguration100134 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    SwSegnetOuterNodeConfiguration swSegnetOuterNodeConfiguration;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration
        lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration;
    SwGdcOuterNodeConfiguration swGdcOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[27];
};

struct GraphConfiguration100026 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysOuterNodeConfiguration isysOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[2];
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
    IsysWithCvOuterNodeConfiguration isysWithCvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[4];
};

struct GraphConfiguration100038 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysDolWithCvOuterNodeConfiguration isysDolWithCvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[6];
};

struct GraphConfiguration100039 {
    VirtualSinkMapping sinkMappingConfiguration;
    IsysPdaf2WithCvOuterNodeConfiguration isysPdaf2WithCvOuterNodeConfiguration;
    StaticGraphLinkConfiguration linkConfigurations[6];
};
#pragma pack(pop)

class IsysOuterNode : public OuterNode {
 public:
    IsysOuterNode() : OuterNode() {}
    void Init(IsysOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerWithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerWithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwGdcOuterNode : public OuterNode {
 public:
    SwGdcOuterNode() : OuterNode() {}
    void Init(SwGdcOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerWithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerWithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwNntmOuterNode : public OuterNode {
 public:
    SwNntmOuterNode() : OuterNode() {}
    void Init(SwNntmOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwScalerOuterNode : public OuterNode {
 public:
    SwScalerOuterNode() : OuterNode() {}
    void Init(SwScalerOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysPdaf2OuterNode : public OuterNode {
 public:
    IsysPdaf2OuterNode() : OuterNode() {}
    void Init(IsysPdaf2OuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysDolOuterNode : public OuterNode {
 public:
    IsysDolOuterNode() : OuterNode() {}
    void Init(IsysDolOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsWithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsWithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDolSmoothOuterNode : public OuterNode {
 public:
    LbffDolSmoothOuterNode() : OuterNode() {}
    void Init(LbffDolSmoothOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsWithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsWithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrIrNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrWithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrWithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrIrNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrWithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffRgbIrWithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffIrNoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrWithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffIrWithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffIrWithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffIrNoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrWithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffIrWithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(LbffIrWithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysWithCvOuterNode : public OuterNode {
 public:
    IsysWithCvOuterNode() : OuterNode() {}
    void Init(IsysWithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class SwSegnetOuterNode : public OuterNode {
 public:
    SwSegnetOuterNode() : OuterNode() {}
    void Init(SwSegnetOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerNoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerNoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerWithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerWithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerNoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerNoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerWithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerWithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysPdaf2WithCvOuterNode : public OuterNode {
 public:
    IsysPdaf2WithCvOuterNode() : OuterNode() {}
    void Init(IsysPdaf2WithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3WithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class IsysDolWithCvOuterNode : public OuterNode {
 public:
    IsysDolWithCvOuterNode() : OuterNode() {}
    void Init(IsysDolWithCvOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsNoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsWithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsNoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol2InputsNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol2InputsWithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffDol2InputsWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsNoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsWithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsNoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffDol3InputsNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffDol3InputsWithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffDol3InputsWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrNoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffRgbIrNoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrWithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffRgbIrWithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrNoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffRgbIrNoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffRgbIrWithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffRgbIrWithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffRgbIrWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffIrNoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrWithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffIrWithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffIrWithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrNoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffIrNoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffIrNoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffIrWithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffIrWithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffIrWithGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(
        LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNodeConfiguration* selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};
class LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode : public OuterNode {
 public:
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode() : OuterNode() {}
    void Init(LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNodeConfiguration*
                  selectedGraphConfiguration);

    void setInnerNode(InnerNodeOptionsFlags nodeInnerOptions);
};

class imageSubGraphTopology100000 : public GraphTopology {
 public:
    imageSubGraphTopology100000(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[9];
};

class StaticGraph100000 : public IStaticGraphConfig {
 public:
    StaticGraph100000(GraphConfiguration100000* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100000();
    static const uint32_t hashCode = 2914817427;  // autogenerated

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
    imageSubGraphTopology100001(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[14];
};

class StaticGraph100001 : public IStaticGraphConfig {
 public:
    StaticGraph100001(GraphConfiguration100001* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100001();
    static const uint32_t hashCode = 2722821038;  // autogenerated

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
    imageSubGraphTopology100002(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];
};

class StaticGraph100002 : public IStaticGraphConfig {
 public:
    StaticGraph100002(GraphConfiguration100002* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100002();
    static const uint32_t hashCode = 3480542691;  // autogenerated

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
    imageSubGraphTopology100003(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100003 : public IStaticGraphConfig {
 public:
    StaticGraph100003(GraphConfiguration100003* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100003();
    static const uint32_t hashCode = 552482330;  // autogenerated

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

class imageSubGraphTopology100079 : public GraphTopology {
 public:
    imageSubGraphTopology100079(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100079 : public IStaticGraphConfig {
 public:
    StaticGraph100079(GraphConfiguration100079* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100079();
    static const uint32_t hashCode = 4082826981;  // autogenerated

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
    imageSubGraphTopology100080(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100080 : public IStaticGraphConfig {
 public:
    StaticGraph100080(GraphConfiguration100080* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100080();
    static const uint32_t hashCode = 4275052487;  // autogenerated

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

class imageSubGraphTopology100081 : public GraphTopology {
 public:
    imageSubGraphTopology100081(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class StaticGraph100081 : public IStaticGraphConfig {
 public:
    StaticGraph100081(GraphConfiguration100081* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100081();
    static const uint32_t hashCode = 6885079;  // autogenerated

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
    imageSubGraphTopology100004(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf2NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];
};

class StaticGraph100004 : public IStaticGraphConfig {
 public:
    StaticGraph100004(GraphConfiguration100004* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100004();
    static const uint32_t hashCode = 678910205;  // autogenerated

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
    imageSubGraphTopology100005(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100005 : public IStaticGraphConfig {
 public:
    StaticGraph100005(GraphConfiguration100005* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100005();
    static const uint32_t hashCode = 1129599756;  // autogenerated

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
    imageSubGraphTopology100006(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];
};

class StaticGraph100006 : public IStaticGraphConfig {
 public:
    StaticGraph100006(GraphConfiguration100006* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100006();
    static const uint32_t hashCode = 1649128389;  // autogenerated

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

class imageSubGraphTopology100007 : public GraphTopology {
 public:
    imageSubGraphTopology100007(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf2WithGmvWithTnrNoSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100007 : public IStaticGraphConfig {
 public:
    StaticGraph100007(GraphConfiguration100007* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100007();
    static const uint32_t hashCode = 3800731584;  // autogenerated

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

class imageSubGraphTopology100008 : public GraphTopology {
 public:
    imageSubGraphTopology100008(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3NoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[10];
};

class StaticGraph100008 : public IStaticGraphConfig {
 public:
    StaticGraph100008(GraphConfiguration100008* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100008();
    static const uint32_t hashCode = 4109353079;  // autogenerated

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
    imageSubGraphTopology100009(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrNoSapOuterNode* lbffBayerPdaf3WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[15];
};

class StaticGraph100009 : public IStaticGraphConfig {
 public:
    StaticGraph100009(GraphConfiguration100009* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100009();
    static const uint32_t hashCode = 3119139422;  // autogenerated

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
    imageSubGraphTopology100010(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[13];
};

class StaticGraph100010 : public IStaticGraphConfig {
 public:
    StaticGraph100010(GraphConfiguration100010* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100010();
    static const uint32_t hashCode = 3783435687;  // autogenerated

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
    imageSubGraphTopology100011(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrNoSapOuterNode* lbffBayerPdaf3WithGmvWithTnrNoSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[18];
};

class StaticGraph100011 : public IStaticGraphConfig {
 public:
    StaticGraph100011(GraphConfiguration100011* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100011();
    static const uint32_t hashCode = 3398140634;  // autogenerated

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

class imageSubGraphTopology100045 : public GraphTopology {
 public:
    imageSubGraphTopology100045(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrNoSapOuterNode* lbffBayerPdaf3NoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[18];
};

class StaticGraph100045 : public IStaticGraphConfig {
 public:
    StaticGraph100045(GraphConfiguration100045* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100045();
    static const uint32_t hashCode = 176907841;  // autogenerated

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
    imageSubGraphTopology100012(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrNoSapOuterNode* lbffDol2InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100012 : public IStaticGraphConfig {
 public:
    StaticGraph100012(GraphConfiguration100012* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100012();
    static const uint32_t hashCode = 144014565;  // autogenerated

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
    imageSubGraphTopology100013(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrNoSapOuterNode* lbffDol2InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100013 : public IStaticGraphConfig {
 public:
    StaticGraph100013(GraphConfiguration100013* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100013();
    static const uint32_t hashCode = 1202051034;  // autogenerated

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
    imageSubGraphTopology100014(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrNoSapOuterNode* lbffDol2InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100014 : public IStaticGraphConfig {
 public:
    StaticGraph100014(GraphConfiguration100014* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100014();
    static const uint32_t hashCode = 3377348061;  // autogenerated

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
    imageSubGraphTopology100015(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrNoSapOuterNode* lbffDol2InputsWithGmvWithTnrNoSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];
};

class StaticGraph100015 : public IStaticGraphConfig {
 public:
    StaticGraph100015(GraphConfiguration100015* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100015();
    static const uint32_t hashCode = 2846893190;  // autogenerated

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
    imageSubGraphTopology100016(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrNoSapOuterNode* lbffDol3InputsNoGmvNoTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class StaticGraph100016 : public IStaticGraphConfig {
 public:
    StaticGraph100016(GraphConfiguration100016* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100016();
    static const uint32_t hashCode = 2137187788;  // autogenerated

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
    imageSubGraphTopology100017(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrNoSapOuterNode* lbffDol3InputsWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class StaticGraph100017 : public IStaticGraphConfig {
 public:
    StaticGraph100017(GraphConfiguration100017* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100017();
    static const uint32_t hashCode = 3238724207;  // autogenerated

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
    imageSubGraphTopology100018(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrNoSapOuterNode* lbffDol3InputsNoGmvWithTnrNoSapOuterNode = nullptr;
    SwNntmOuterNode* swNntmOuterNode = nullptr;
    SwScalerOuterNode* swScalerOuterNode = nullptr;
    GraphLink* subGraphLinks[22];
};

class StaticGraph100018 : public IStaticGraphConfig {
 public:
    StaticGraph100018(GraphConfiguration100018* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100018();
    static const uint32_t hashCode = 3471629776;  // autogenerated

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
    imageSubGraphTopology100019(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrNoSapOuterNode* lbffDol3InputsWithGmvWithTnrNoSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];
};

class StaticGraph100019 : public IStaticGraphConfig {
 public:
    StaticGraph100019(GraphConfiguration100019* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100019();
    static const uint32_t hashCode = 349008703;  // autogenerated

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
    imageSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 10, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[10];
};

class irSubGraphTopology100020 : public GraphTopology {
 public:
    irSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class image_irSubGraphTopology100020 : public GraphTopology {
 public:
    image_irSubGraphTopology100020(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrNoGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100020 : public IStaticGraphConfig {
 public:
    StaticGraph100020(GraphConfiguration100020* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100020();
    static const uint32_t hashCode = 3438564774;  // autogenerated

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
    imageSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[15];
};

class irSubGraphTopology100021 : public GraphTopology {
 public:
    irSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class image_irSubGraphTopology100021 : public GraphTopology {
 public:
    image_irSubGraphTopology100021(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrNoSapOuterNode* lbffRgbIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[22];
};

class StaticGraph100021 : public IStaticGraphConfig {
 public:
    StaticGraph100021(GraphConfiguration100021* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100021();
    static const uint32_t hashCode = 4276022635;  // autogenerated

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
    imageSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[13];
};

class irSubGraphTopology100022 : public GraphTopology {
 public:
    irSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class image_irSubGraphTopology100022 : public GraphTopology {
 public:
    image_irSubGraphTopology100022(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrNoGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];
};

class StaticGraph100022 : public IStaticGraphConfig {
 public:
    StaticGraph100022(GraphConfiguration100022* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100022();
    static const uint32_t hashCode = 2177181214;  // autogenerated

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
    imageSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[18];
};

class irSubGraphTopology100023 : public GraphTopology {
 public:
    irSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];
};

class image_irSubGraphTopology100023 : public GraphTopology {
 public:
    image_irSubGraphTopology100023(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrNoSapOuterNode* lbffRgbIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[28];
};

class StaticGraph100023 : public IStaticGraphConfig {
 public:
    StaticGraph100023(GraphConfiguration100023* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100023();
    static const uint32_t hashCode = 4112854315;  // autogenerated

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
    imageSubGraphTopology100024(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 8, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvNoTnrNoSapOuterNode* lbffBayerNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[8];
};

class StaticGraph100024 : public IStaticGraphConfig {
 public:
    StaticGraph100024(GraphConfiguration100024* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100024();
    static const uint32_t hashCode = 844284306;  // autogenerated

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
    imageSubGraphTopology100040(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 13, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvNoTnrNoSapOuterNode* lbffBayerWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[13];
};

class StaticGraph100040 : public IStaticGraphConfig {
 public:
    StaticGraph100040(GraphConfiguration100040* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100040();
    static const uint32_t hashCode = 874730067;  // autogenerated

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
    imageSubGraphTopology100041(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerWithGmvWithTnrNoSapOuterNode* lbffBayerWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];
};

class StaticGraph100041 : public IStaticGraphConfig {
 public:
    StaticGraph100041(GraphConfiguration100041* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100041();
    static const uint32_t hashCode = 2147300611;  // autogenerated

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
    imageSubGraphTopology100042(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    LbffBayerNoGmvWithTnrNoSapOuterNode* lbffBayerNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[11];
};

class StaticGraph100042 : public IStaticGraphConfig {
 public:
    StaticGraph100042(GraphConfiguration100042* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100042();
    static const uint32_t hashCode = 3491144622;  // autogenerated

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
    imageSubGraphTopology100027(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 9, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvNoTnrNoSapOuterNode* lbffIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[9];
};

class StaticGraph100027 : public IStaticGraphConfig {
 public:
    StaticGraph100027(GraphConfiguration100027* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100027();
    static const uint32_t hashCode = 189755735;  // autogenerated

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
    imageSubGraphTopology100028(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrWithGmvNoTnrNoSapOuterNode* lbffIrWithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[14];
};

class StaticGraph100028 : public IStaticGraphConfig {
 public:
    StaticGraph100028(GraphConfiguration100028* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100028();
    static const uint32_t hashCode = 616830362;  // autogenerated

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
    imageSubGraphTopology100029(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 12, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrNoGmvWithTnrNoSapOuterNode* lbffIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[12];
};

class StaticGraph100029 : public IStaticGraphConfig {
 public:
    StaticGraph100029(GraphConfiguration100029* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100029();
    static const uint32_t hashCode = 3970859463;  // autogenerated

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
    imageSubGraphTopology100030(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffIrWithGmvWithTnrNoSapOuterNode* lbffIrWithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[17];
};

class StaticGraph100030 : public IStaticGraphConfig {
 public:
    StaticGraph100030(GraphConfiguration100030* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100030();
    static const uint32_t hashCode = 3349357766;  // autogenerated

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
    imageSubGraphTopology100031(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 11, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode* lbffBayerPdaf3asPdaf2NoGmvNoTnrNoSapOuterNode =
        nullptr;
    GraphLink* subGraphLinks[11];
};

class StaticGraph100031 : public IStaticGraphConfig {
 public:
    StaticGraph100031(GraphConfiguration100031* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100031();
    static const uint32_t hashCode = 2685553439;  // autogenerated

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
    imageSubGraphTopology100032(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode*
        lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[16];
};

class StaticGraph100032 : public IStaticGraphConfig {
 public:
    StaticGraph100032(GraphConfiguration100032* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100032();
    static const uint32_t hashCode = 637249946;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100032 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode
        _lbffBayerPdaf3asPdaf2WithGmvNoTnrNoSapOuterNode;
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
    imageSubGraphTopology100033(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 14, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode*
        lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[14];
};

class StaticGraph100033 : public IStaticGraphConfig {
 public:
    StaticGraph100033(GraphConfiguration100033* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100033();
    static const uint32_t hashCode = 3946150383;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100033 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode
        _lbffBayerPdaf3asPdaf2NoGmvWithTnrNoSapOuterNode;

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
    imageSubGraphTopology100034(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysOuterNode* isysOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode*
        lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[19];
};

class StaticGraph100034 : public IStaticGraphConfig {
 public:
    StaticGraph100034(GraphConfiguration100034* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100034();
    static const uint32_t hashCode = 4100007686;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100034 _graphConfiguration;

    /* Outer Nodes */
    IsysOuterNode _isysOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode
        _lbffBayerPdaf3asPdaf2WithGmvWithTnrNoSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100034 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100100 : public GraphTopology {
 public:
    imageSubGraphTopology100100(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvNoTnrWithSapOuterNode* lbffBayerNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100100 : public IStaticGraphConfig {
 public:
    StaticGraph100100(GraphConfiguration100100* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100100();
    static const uint32_t hashCode = 3814212434;  // autogenerated

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
    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100101 : public GraphTopology {
 public:
    imageSubGraphTopology100101(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvNoTnrWithSapOuterNode* lbffBayerWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100101 : public IStaticGraphConfig {
 public:
    StaticGraph100101(GraphConfiguration100101* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100101();
    static const uint32_t hashCode = 3190898911;  // autogenerated

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
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100102 : public GraphTopology {
 public:
    imageSubGraphTopology100102(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerNoGmvWithTnrWithSapOuterNode* lbffBayerNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100102 : public IStaticGraphConfig {
 public:
    StaticGraph100102(GraphConfiguration100102* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100102();
    static const uint32_t hashCode = 3712859908;  // autogenerated

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
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100103 : public GraphTopology {
 public:
    imageSubGraphTopology100103(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerWithGmvWithTnrWithSapOuterNode* lbffBayerWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100103 : public IStaticGraphConfig {
 public:
    StaticGraph100103(GraphConfiguration100103* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100103();
    static const uint32_t hashCode = 619377997;  // autogenerated

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
    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100104 : public GraphTopology {
 public:
    imageSubGraphTopology100104(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 18, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[18];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100104 : public IStaticGraphConfig {
 public:
    StaticGraph100104(GraphConfiguration100104* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100104();
    static const uint32_t hashCode = 2770355904;  // autogenerated

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
    GraphLink _graphLinks[18];
};

class imageSubGraphTopology100105 : public GraphTopology {
 public:
    imageSubGraphTopology100105(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf2WithGmvNoTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100105 : public IStaticGraphConfig {
 public:
    StaticGraph100105(GraphConfiguration100105* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100105();
    static const uint32_t hashCode = 1951006425;  // autogenerated

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
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100106 : public GraphTopology {
 public:
    imageSubGraphTopology100106(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf2NoGmvWithTnrWithSapOuterNode =
        nullptr;
    GraphLink* subGraphLinks[23];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100106 : public IStaticGraphConfig {
 public:
    StaticGraph100106(GraphConfiguration100106* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100106();
    static const uint32_t hashCode = 290732550;  // autogenerated

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
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100107 : public GraphTopology {
 public:
    imageSubGraphTopology100107(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf2WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf2WithGmvWithTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100107 : public IStaticGraphConfig {
 public:
    StaticGraph100107(GraphConfiguration100107* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100107();
    static const uint32_t hashCode = 1727023371;  // autogenerated

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
    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100108 : public GraphTopology {
 public:
    imageSubGraphTopology100108(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvNoTnrWithSapOuterNode* lbffBayerPdaf3NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100108 : public IStaticGraphConfig {
 public:
    StaticGraph100108(GraphConfiguration100108* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100108();
    static const uint32_t hashCode = 173440394;  // autogenerated

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
    GraphLink _graphLinks[16];
};

class imageSubGraphTopology100109 : public GraphTopology {
 public:
    imageSubGraphTopology100109(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvNoTnrWithSapOuterNode* lbffBayerPdaf3WithGmvNoTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100109 : public IStaticGraphConfig {
 public:
    StaticGraph100109(GraphConfiguration100109* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100109();
    static const uint32_t hashCode = 1543340907;  // autogenerated

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
    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100110 : public GraphTopology {
 public:
    imageSubGraphTopology100110(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3NoGmvWithTnrWithSapOuterNode* lbffBayerPdaf3NoGmvWithTnrWithSapOuterNode =
        nullptr;
    GraphLink* subGraphLinks[21];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100110 : public IStaticGraphConfig {
 public:
    StaticGraph100110(GraphConfiguration100110* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100110();
    static const uint32_t hashCode = 3102705644;  // autogenerated

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
    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100111 : public GraphTopology {
 public:
    imageSubGraphTopology100111(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3WithGmvWithTnrWithSapOuterNode* lbffBayerPdaf3WithGmvWithTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100111 : public IStaticGraphConfig {
 public:
    StaticGraph100111(GraphConfiguration100111* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100111();
    static const uint32_t hashCode = 66338681;  // autogenerated

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
    GraphLink _graphLinks[26];
};

class imageSubGraphTopology100112 : public GraphTopology {
 public:
    imageSubGraphTopology100112(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 19, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvNoTnrWithSapOuterNode* lbffDol2InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[19];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100112 : public IStaticGraphConfig {
 public:
    StaticGraph100112(GraphConfiguration100112* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100112();
    static const uint32_t hashCode = 1108287162;  // autogenerated

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
    GraphLink _graphLinks[19];
};

class imageSubGraphTopology100113 : public GraphTopology {
 public:
    imageSubGraphTopology100113(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvNoTnrWithSapOuterNode* lbffDol2InputsWithGmvNoTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[24];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100113 : public IStaticGraphConfig {
 public:
    StaticGraph100113(GraphConfiguration100113* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100113();
    static const uint32_t hashCode = 1560693719;  // autogenerated

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
    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100114 : public GraphTopology {
 public:
    imageSubGraphTopology100114(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 24, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode* lbffDol2InputsNoGmvWithTnrWithSapOuterNode =
        nullptr;
    GraphLink* subGraphLinks[24];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100114 : public IStaticGraphConfig {
 public:
    StaticGraph100114(GraphConfiguration100114* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100114();
    static const uint32_t hashCode = 62382684;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100114 _graphConfiguration;

    /* Outer Nodes */
    IsysDolWithCvOuterNode _isysDolWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffDol2InputsNoGmvWithTnrWithSapOuterNode _lbffDol2InputsNoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100114 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[24];
};

class imageSubGraphTopology100115 : public GraphTopology {
 public:
    imageSubGraphTopology100115(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 29, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDol2InputsWithGmvWithTnrWithSapOuterNode* lbffDol2InputsWithGmvWithTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[29];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100115 : public IStaticGraphConfig {
 public:
    StaticGraph100115(GraphConfiguration100115* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100115();
    static const uint32_t hashCode = 2994847221;  // autogenerated

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
    GraphLink _graphLinks[29];
};

class imageSubGraphTopology100116 : public GraphTopology {
 public:
    imageSubGraphTopology100116(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvNoTnrWithSapOuterNode* lbffDol3InputsNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100116 : public IStaticGraphConfig {
 public:
    StaticGraph100116(GraphConfiguration100116* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100116();
    static const uint32_t hashCode = 141943583;  // autogenerated

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
    GraphLink _graphLinks[21];
};

class imageSubGraphTopology100117 : public GraphTopology {
 public:
    imageSubGraphTopology100117(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvNoTnrWithSapOuterNode* lbffDol3InputsWithGmvNoTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100117 : public IStaticGraphConfig {
 public:
    StaticGraph100117(GraphConfiguration100117* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100117();
    static const uint32_t hashCode = 235579058;  // autogenerated

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
    GraphLink _graphLinks[26];
};

class imageSubGraphTopology100118 : public GraphTopology {
 public:
    imageSubGraphTopology100118(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsNoGmvWithTnrWithSapOuterNode* lbffDol3InputsNoGmvWithTnrWithSapOuterNode =
        nullptr;
    GraphLink* subGraphLinks[26];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100118 : public IStaticGraphConfig {
 public:
    StaticGraph100118(GraphConfiguration100118* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100118();
    static const uint32_t hashCode = 3835533685;  // autogenerated

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
    GraphLink _graphLinks[26];
};

class imageSubGraphTopology100119 : public GraphTopology {
 public:
    imageSubGraphTopology100119(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffDolSmoothOuterNode* lbffDolSmoothOuterNode = nullptr;
    LbffDol3InputsWithGmvWithTnrWithSapOuterNode* lbffDol3InputsWithGmvWithTnrWithSapOuterNode =
        nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100119 : public IStaticGraphConfig {
 public:
    StaticGraph100119(GraphConfiguration100119* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100119();
    static const uint32_t hashCode = 2100145004;  // autogenerated

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
    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100120 : public GraphTopology {
 public:
    imageSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 16, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[16];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100120 : public GraphTopology {
 public:
    irSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100120 : public GraphTopology {
 public:
    image_irSubGraphTopology100120(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 23, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvNoTnrWithSapOuterNode* lbffRgbIrNoGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[23];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100120 : public IStaticGraphConfig {
 public:
    StaticGraph100120(GraphConfiguration100120* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100120();
    static const uint32_t hashCode = 971823595;  // autogenerated

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
    GraphLink _graphLinks[23];
};

class imageSubGraphTopology100121 : public GraphTopology {
 public:
    imageSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100121 : public GraphTopology {
 public:
    irSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100121 : public GraphTopology {
 public:
    image_irSubGraphTopology100121(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 28, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvNoTnrWithSapOuterNode* lbffRgbIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvNoTnrNoSapOuterNode* lbffRgbIrIrNoGmvNoTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[28];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100121 : public IStaticGraphConfig {
 public:
    StaticGraph100121(GraphConfiguration100121* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100121();
    static const uint32_t hashCode = 760104734;  // autogenerated

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
    GraphLink _graphLinks[28];
};

class imageSubGraphTopology100122 : public GraphTopology {
 public:
    imageSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 21, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[21];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100122 : public GraphTopology {
 public:
    irSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100122 : public GraphTopology {
 public:
    image_irSubGraphTopology100122(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 31, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrNoGmvWithTnrWithSapOuterNode* lbffRgbIrNoGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[31];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100122 : public IStaticGraphConfig {
 public:
    StaticGraph100122(GraphConfiguration100122* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100122();
    static const uint32_t hashCode = 2835173097;  // autogenerated

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
    GraphLink _graphLinks[31];
};

class imageSubGraphTopology100123 : public GraphTopology {
 public:
    imageSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 26, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[26];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class irSubGraphTopology100123 : public GraphTopology {
 public:
    irSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 33, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[33];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class image_irSubGraphTopology100123 : public GraphTopology {
 public:
    image_irSubGraphTopology100123(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 36, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffRgbIrWithGmvWithTnrWithSapOuterNode* lbffRgbIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    LbffRgbIrIrNoGmvWithTnrNoSapOuterNode* lbffRgbIrIrNoGmvWithTnrNoSapOuterNode = nullptr;
    GraphLink* subGraphLinks[36];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100123 : public IStaticGraphConfig {
 public:
    StaticGraph100123(GraphConfiguration100123* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100123();
    static const uint32_t hashCode = 1906732972;  // autogenerated

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
    GraphLink _graphLinks[36];
};

class imageSubGraphTopology100127 : public GraphTopology {
 public:
    imageSubGraphTopology100127(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 15, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrNoGmvNoTnrWithSapOuterNode* lbffIrNoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[15];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100127 : public IStaticGraphConfig {
 public:
    StaticGraph100127(GraphConfiguration100127* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100127();
    static const uint32_t hashCode = 1889144206;  // autogenerated

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
    GraphLink _graphLinks[15];
};

class imageSubGraphTopology100128 : public GraphTopology {
 public:
    imageSubGraphTopology100128(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrWithGmvNoTnrWithSapOuterNode* lbffIrWithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100128 : public IStaticGraphConfig {
 public:
    StaticGraph100128(GraphConfiguration100128* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100128();
    static const uint32_t hashCode = 2596417523;  // autogenerated

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
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100129 : public GraphTopology {
 public:
    imageSubGraphTopology100129(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 20, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrNoGmvWithTnrWithSapOuterNode* lbffIrNoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[20];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100129 : public IStaticGraphConfig {
 public:
    StaticGraph100129(GraphConfiguration100129* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100129();
    static const uint32_t hashCode = 3199590544;  // autogenerated

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
    GraphLink _graphLinks[20];
};

class imageSubGraphTopology100130 : public GraphTopology {
 public:
    imageSubGraphTopology100130(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 25, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffIrWithGmvWithTnrWithSapOuterNode* lbffIrWithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[25];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100130 : public IStaticGraphConfig {
 public:
    StaticGraph100130(GraphConfiguration100130* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100130();
    static const uint32_t hashCode = 2452021393;  // autogenerated

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
    GraphLink _graphLinks[25];
};

class imageSubGraphTopology100131 : public GraphTopology {
 public:
    imageSubGraphTopology100131(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 17, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode*
        lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[17];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100131 : public IStaticGraphConfig {
 public:
    StaticGraph100131(GraphConfiguration100131* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100131();
    static const uint32_t hashCode = 150427038;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100131 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode
        _lbffBayerPdaf3asPdaf2NoGmvNoTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100131 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[17];
};

class imageSubGraphTopology100132 : public GraphTopology {
 public:
    imageSubGraphTopology100132(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode*
        lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100132 : public IStaticGraphConfig {
 public:
    StaticGraph100132(GraphConfiguration100132* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100132();
    static const uint32_t hashCode = 2229860427;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100132 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode
        _lbffBayerPdaf3asPdaf2WithGmvNoTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100132 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100133 : public GraphTopology {
 public:
    imageSubGraphTopology100133(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 22, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode*
        lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode = nullptr;
    GraphLink* subGraphLinks[22];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100133 : public IStaticGraphConfig {
 public:
    StaticGraph100133(GraphConfiguration100133* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100133();
    static const uint32_t hashCode = 3332109776;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100133 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode
        _lbffBayerPdaf3asPdaf2NoGmvWithTnrWithSapOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100133 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[22];
};

class imageSubGraphTopology100134 : public GraphTopology {
 public:
    imageSubGraphTopology100134(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 27, sinkMappingConfiguration) {}
    StaticGraphStatus configInnerNodes(
        SubGraphInnerNodeConfiguration& subGraphInnerNodeConfiguration) override;

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    SwSegnetOuterNode* swSegnetOuterNode = nullptr;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode*
        lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode = nullptr;
    SwGdcOuterNode* swGdcOuterNode = nullptr;
    GraphLink* subGraphLinks[27];

 private:
    StaticGraphStatus updateSegmentAwareKernels();
};

class StaticGraph100134 : public IStaticGraphConfig {
 public:
    StaticGraph100134(GraphConfiguration100134* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100134();
    static const uint32_t hashCode = 2469377657;  // autogenerated

 private:
    // Configuration
    GraphConfiguration100134 _graphConfiguration;

    /* Outer Nodes */
    IsysWithCvOuterNode _isysWithCvOuterNode;
    SwSegnetOuterNode _swSegnetOuterNode;
    LbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode
        _lbffBayerPdaf3asPdaf2WithGmvWithTnrWithSapOuterNode;
    SwGdcOuterNode _swGdcOuterNode;

    /*
        Topology
    */
    // Sub Graphs definition
    imageSubGraphTopology100134 _imageSubGraph;

    // All graph links
    GraphLink _graphLinks[27];
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
    StaticGraph100026(GraphConfiguration100026* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100026();
    static const uint32_t hashCode = 1006964276;  // autogenerated

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

class rawSubGraphTopology100035 : public GraphTopology {
 public:
    rawSubGraphTopology100035(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}

    IsysDolOuterNode* isysDolOuterNode = nullptr;
    GraphLink* subGraphLinks[4];
};

class StaticGraph100035 : public IStaticGraphConfig {
 public:
    StaticGraph100035(GraphConfiguration100035* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100035();
    static const uint32_t hashCode = 1685721370;  // autogenerated

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
    rawSubGraphTopology100036(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}

    IsysPdaf2OuterNode* isysPdaf2OuterNode = nullptr;
    GraphLink* subGraphLinks[4];
};

class StaticGraph100036 : public IStaticGraphConfig {
 public:
    StaticGraph100036(GraphConfiguration100036* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100036();
    static const uint32_t hashCode = 1685721370;  // autogenerated

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
    rawSubGraphTopology100037(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 4, sinkMappingConfiguration) {}

    IsysWithCvOuterNode* isysWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[4];
};

class StaticGraph100037 : public IStaticGraphConfig {
 public:
    StaticGraph100037(GraphConfiguration100037* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100037();
    static const uint32_t hashCode = 3835365160;  // autogenerated

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

class rawSubGraphTopology100038 : public GraphTopology {
 public:
    rawSubGraphTopology100038(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 6, sinkMappingConfiguration) {}

    IsysDolWithCvOuterNode* isysDolWithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[6];
};

class StaticGraph100038 : public IStaticGraphConfig {
 public:
    StaticGraph100038(GraphConfiguration100038* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100038();
    static const uint32_t hashCode = 963983022;  // autogenerated

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

class rawSubGraphTopology100039 : public GraphTopology {
 public:
    rawSubGraphTopology100039(VirtualSinkMapping* sinkMappingConfiguration)
            : GraphTopology(subGraphLinks, 6, sinkMappingConfiguration) {}

    IsysPdaf2WithCvOuterNode* isysPdaf2WithCvOuterNode = nullptr;
    GraphLink* subGraphLinks[6];
};

class StaticGraph100039 : public IStaticGraphConfig {
 public:
    StaticGraph100039(GraphConfiguration100039* selectedGraphConfiguration,
                      VirtualSinkMapping* sinkMappingConfiguration, SensorMode* selectedSensorMode,
                      int32_t selectedSettingsId);
    ~StaticGraph100039();
    static const uint32_t hashCode = 963983022;  // autogenerated

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
