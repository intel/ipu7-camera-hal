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

#include "Ipu8StaticGraphReaderAutogen.h"
#include <cstring>

StaticGraphStatus StaticGraphReader::Init(StaticReaderBinaryData& binaryGraphSettings) {
    if (!binaryGraphSettings.data)
    {
        STATIC_GRAPH_LOG("Binary settings is empty.");
        return StaticGraphStatus::SG_ERROR;
    }

    int8_t* currOffset = static_cast<int8_t*>(binaryGraphSettings.data);
    _binaryHeader =  *reinterpret_cast<BinaryHeader*>(currOffset);

    if (_binaryHeader.binaryCommonHashCode != staticGraphCommonHashCode)
    {
        STATIC_GRAPH_LOG("Binary hash code is not matching the static graph structure hash code. Binary should be re-created.");
        return StaticGraphStatus::SG_ERROR;
    }

    //Skipping BinaryHeader

    currOffset += sizeof(BinaryHeader);

    uint32_t numOfAvailablePins = 0;
    DataRangeHeader dataRangeHeader = *(DataRangeHeader*)currOffset;

    for (int j = 0; j < enNumOfOutPins; j++)
         numOfAvailablePins+= dataRangeHeader.NumberOfPinResolutions[j];

    currOffset += sizeof(DataRangeHeader) + sizeof(DriverDesc) * numOfAvailablePins;

    uint32_t numOfGraphs = *(uint32_t*)currOffset;
    currOffset += sizeof(numOfGraphs) + numOfGraphs * sizeof(GraphHashCode);

    _graphConfigurationHeaders = reinterpret_cast<GraphConfigurationHeader*>(currOffset);
    currOffset += sizeof(GraphConfigurationHeader)*_binaryHeader.numberOfResolutions;
    _sensorModes = reinterpret_cast<SensorMode*>(currOffset);
    currOffset += sizeof(SensorMode)*_binaryHeader.numberOfSensorModes;
    _configurationData = currOffset;

    return StaticGraphStatus::SG_OK;
}

std::pair<int, const GraphConfigurationHeader*> StaticGraphReader::GetGraphConfigurationHeaders() const 
{
    return std::make_pair(_binaryHeader.numberOfResolutions, _graphConfigurationHeaders);
}

GraphConfigurationKey* StaticGraphReader::GetFdGraphConfigurationKey(GraphConfigurationKey& settingsKey) const
{
    for (uint32_t i = 0; i < _binaryHeader.numberOfResolutions; i++)
    {
        if (settingsKey.attributes == _graphConfigurationHeaders[i].settingsKey.attributes && 
            (((settingsKey.preview.width != 0 && _graphConfigurationHeaders[i].settingsKey.preview.width == settingsKey.preview.width && _graphConfigurationHeaders[i].settingsKey.preview.height == settingsKey.preview.height) ||
            (settingsKey.video.width != 0 && _graphConfigurationHeaders[i].settingsKey.video.width == settingsKey.video.width && _graphConfigurationHeaders[i].settingsKey.video.height == settingsKey.video.height)) && 
            _graphConfigurationHeaders[i].settingsKey.postProcessingVideo.width != 0))
        {
            return &_graphConfigurationHeaders[i].settingsKey;
        }
    }
    return NULL;
}

StaticGraphStatus StaticGraphReader::GetStaticGraphConfig(GraphConfigurationKey& settingsKey, IStaticGraphConfig** graph)
{
    if (!_graphConfigurationHeaders || !_sensorModes || !_configurationData)
    {
        STATIC_GRAPH_LOG("Static graph reader was not initialized properly.");
        return StaticGraphStatus::SG_ERROR;
    }

    if (!graph)
    {
        STATIC_GRAPH_LOG("Cannot get graph configuration into null parameter");
        return StaticGraphStatus::SG_ERROR;
    }

    GraphConfigurationHeader* selectedGraphConfigurationHeader = nullptr;

    for (uint32_t i=0; i < _binaryHeader.numberOfResolutions; i++)
    {
        if (memcmp ( &_graphConfigurationHeaders[i].settingsKey,
            &settingsKey,
            sizeof(GraphConfigurationKey)) == 0)
        {
            selectedGraphConfigurationHeader = &_graphConfigurationHeaders[i];
            STATIC_GRAPH_LOG("Static graph selected setting id - %d", selectedGraphConfigurationHeader->settingId);

            break;

        }
    }

    if (!selectedGraphConfigurationHeader )
    {
        STATIC_GRAPH_LOG("Resolution settings was not found for the given key.");
        return StaticGraphStatus::SG_ERROR;
    }

    int8_t* selectedConfigurationData = _configurationData + selectedGraphConfigurationHeader->resConfigDataOffset;

    GraphConfigurationHeader* baseGraphConfigurationHeader = nullptr;

    for (uint32_t i = 0; i < _binaryHeader.numberOfResolutions; i++)
    {
        if (_graphConfigurationHeaders[i].resConfigDataOffset == selectedGraphConfigurationHeader->resConfigDataOffset)
        {
            if (selectedGraphConfigurationHeader != &_graphConfigurationHeaders[i])
            {
                baseGraphConfigurationHeader = &_graphConfigurationHeaders[i];
            }
            break;
        }
    }

    VirtualSinkMapping* baseSinkMappingConfiguration = reinterpret_cast<VirtualSinkMapping*>(selectedConfigurationData);

    VirtualSinkMapping selectedSinkMappingConfiguration;
    GetSinkMappingConfiguration(baseGraphConfigurationHeader, baseSinkMappingConfiguration, selectedGraphConfigurationHeader, &selectedSinkMappingConfiguration);

    // fetching the graph
    switch (selectedGraphConfigurationHeader->graphId)
    {
        case 100000:
            if (StaticGraph100000::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100000(
                reinterpret_cast<GraphConfiguration100000*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100001:
            if (StaticGraph100001::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100001(
                reinterpret_cast<GraphConfiguration100001*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100002:
            if (StaticGraph100002::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100002(
                reinterpret_cast<GraphConfiguration100002*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100003:
            if (StaticGraph100003::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100003(
                reinterpret_cast<GraphConfiguration100003*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100137:
            if (StaticGraph100137::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100137(
                reinterpret_cast<GraphConfiguration100137*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100079:
            if (StaticGraph100079::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100079(
                reinterpret_cast<GraphConfiguration100079*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100080:
            if (StaticGraph100080::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100080(
                reinterpret_cast<GraphConfiguration100080*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100138:
            if (StaticGraph100138::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100138(
                reinterpret_cast<GraphConfiguration100138*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100142:
            if (StaticGraph100142::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100142(
                reinterpret_cast<GraphConfiguration100142*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100162:
            if (StaticGraph100162::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100162(
                reinterpret_cast<GraphConfiguration100162*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100143:
            if (StaticGraph100143::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100143(
                reinterpret_cast<GraphConfiguration100143*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100144:
            if (StaticGraph100144::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100144(
                reinterpret_cast<GraphConfiguration100144*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100081:
            if (StaticGraph100081::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100081(
                reinterpret_cast<GraphConfiguration100081*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100004:
            if (StaticGraph100004::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100004(
                reinterpret_cast<GraphConfiguration100004*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100005:
            if (StaticGraph100005::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100005(
                reinterpret_cast<GraphConfiguration100005*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100006:
            if (StaticGraph100006::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100006(
                reinterpret_cast<GraphConfiguration100006*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100066:
            if (StaticGraph100066::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100066(
                reinterpret_cast<GraphConfiguration100066*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100007:
            if (StaticGraph100007::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100007(
                reinterpret_cast<GraphConfiguration100007*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100067:
            if (StaticGraph100067::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100067(
                reinterpret_cast<GraphConfiguration100067*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100139:
            if (StaticGraph100139::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100139(
                reinterpret_cast<GraphConfiguration100139*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100169:
            if (StaticGraph100169::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100169(
                reinterpret_cast<GraphConfiguration100169*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100008:
            if (StaticGraph100008::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100008(
                reinterpret_cast<GraphConfiguration100008*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100009:
            if (StaticGraph100009::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100009(
                reinterpret_cast<GraphConfiguration100009*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100010:
            if (StaticGraph100010::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100010(
                reinterpret_cast<GraphConfiguration100010*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100011:
            if (StaticGraph100011::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100011(
                reinterpret_cast<GraphConfiguration100011*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100140:
            if (StaticGraph100140::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100140(
                reinterpret_cast<GraphConfiguration100140*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100045:
            if (StaticGraph100045::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100045(
                reinterpret_cast<GraphConfiguration100045*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100012:
            if (StaticGraph100012::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100012(
                reinterpret_cast<GraphConfiguration100012*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100013:
            if (StaticGraph100013::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100013(
                reinterpret_cast<GraphConfiguration100013*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100014:
            if (StaticGraph100014::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100014(
                reinterpret_cast<GraphConfiguration100014*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100015:
            if (StaticGraph100015::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100015(
                reinterpret_cast<GraphConfiguration100015*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100016:
            if (StaticGraph100016::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100016(
                reinterpret_cast<GraphConfiguration100016*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100017:
            if (StaticGraph100017::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100017(
                reinterpret_cast<GraphConfiguration100017*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100018:
            if (StaticGraph100018::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100018(
                reinterpret_cast<GraphConfiguration100018*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100019:
            if (StaticGraph100019::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100019(
                reinterpret_cast<GraphConfiguration100019*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100020:
            if (StaticGraph100020::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100020(
                reinterpret_cast<GraphConfiguration100020*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100021:
            if (StaticGraph100021::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100021(
                reinterpret_cast<GraphConfiguration100021*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100022:
            if (StaticGraph100022::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100022(
                reinterpret_cast<GraphConfiguration100022*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100023:
            if (StaticGraph100023::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100023(
                reinterpret_cast<GraphConfiguration100023*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100024:
            if (StaticGraph100024::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100024(
                reinterpret_cast<GraphConfiguration100024*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100040:
            if (StaticGraph100040::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100040(
                reinterpret_cast<GraphConfiguration100040*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100041:
            if (StaticGraph100041::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100041(
                reinterpret_cast<GraphConfiguration100041*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100042:
            if (StaticGraph100042::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100042(
                reinterpret_cast<GraphConfiguration100042*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100027:
            if (StaticGraph100027::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100027(
                reinterpret_cast<GraphConfiguration100027*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100028:
            if (StaticGraph100028::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100028(
                reinterpret_cast<GraphConfiguration100028*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100029:
            if (StaticGraph100029::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100029(
                reinterpret_cast<GraphConfiguration100029*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100030:
            if (StaticGraph100030::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100030(
                reinterpret_cast<GraphConfiguration100030*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100031:
            if (StaticGraph100031::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100031(
                reinterpret_cast<GraphConfiguration100031*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100032:
            if (StaticGraph100032::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100032(
                reinterpret_cast<GraphConfiguration100032*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100033:
            if (StaticGraph100033::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100033(
                reinterpret_cast<GraphConfiguration100033*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100034:
            if (StaticGraph100034::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100034(
                reinterpret_cast<GraphConfiguration100034*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100141:
            if (StaticGraph100141::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100141(
                reinterpret_cast<GraphConfiguration100141*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100100:
            if (StaticGraph100100::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100100(
                reinterpret_cast<GraphConfiguration100100*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100101:
            if (StaticGraph100101::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100101(
                reinterpret_cast<GraphConfiguration100101*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100102:
            if (StaticGraph100102::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100102(
                reinterpret_cast<GraphConfiguration100102*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100157:
            if (StaticGraph100157::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100157(
                reinterpret_cast<GraphConfiguration100157*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100103:
            if (StaticGraph100103::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100103(
                reinterpret_cast<GraphConfiguration100103*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100135:
            if (StaticGraph100135::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100135(
                reinterpret_cast<GraphConfiguration100135*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100104:
            if (StaticGraph100104::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100104(
                reinterpret_cast<GraphConfiguration100104*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100105:
            if (StaticGraph100105::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100105(
                reinterpret_cast<GraphConfiguration100105*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100106:
            if (StaticGraph100106::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100106(
                reinterpret_cast<GraphConfiguration100106*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100166:
            if (StaticGraph100166::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100166(
                reinterpret_cast<GraphConfiguration100166*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100107:
            if (StaticGraph100107::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100107(
                reinterpret_cast<GraphConfiguration100107*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100145:
            if (StaticGraph100145::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100145(
                reinterpret_cast<GraphConfiguration100145*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100108:
            if (StaticGraph100108::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100108(
                reinterpret_cast<GraphConfiguration100108*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100109:
            if (StaticGraph100109::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100109(
                reinterpret_cast<GraphConfiguration100109*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100110:
            if (StaticGraph100110::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100110(
                reinterpret_cast<GraphConfiguration100110*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100111:
            if (StaticGraph100111::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100111(
                reinterpret_cast<GraphConfiguration100111*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100136:
            if (StaticGraph100136::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100136(
                reinterpret_cast<GraphConfiguration100136*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100200:
            if (StaticGraph100200::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100200(
                reinterpret_cast<GraphConfiguration100200*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100201:
            if (StaticGraph100201::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100201(
                reinterpret_cast<GraphConfiguration100201*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100112:
            if (StaticGraph100112::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100112(
                reinterpret_cast<GraphConfiguration100112*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100113:
            if (StaticGraph100113::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100113(
                reinterpret_cast<GraphConfiguration100113*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100114:
            if (StaticGraph100114::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100114(
                reinterpret_cast<GraphConfiguration100114*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100146:
            if (StaticGraph100146::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100146(
                reinterpret_cast<GraphConfiguration100146*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100115:
            if (StaticGraph100115::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100115(
                reinterpret_cast<GraphConfiguration100115*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100116:
            if (StaticGraph100116::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100116(
                reinterpret_cast<GraphConfiguration100116*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100117:
            if (StaticGraph100117::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100117(
                reinterpret_cast<GraphConfiguration100117*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100118:
            if (StaticGraph100118::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100118(
                reinterpret_cast<GraphConfiguration100118*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100119:
            if (StaticGraph100119::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100119(
                reinterpret_cast<GraphConfiguration100119*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100120:
            if (StaticGraph100120::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100120(
                reinterpret_cast<GraphConfiguration100120*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100121:
            if (StaticGraph100121::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100121(
                reinterpret_cast<GraphConfiguration100121*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100122:
            if (StaticGraph100122::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100122(
                reinterpret_cast<GraphConfiguration100122*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100123:
            if (StaticGraph100123::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100123(
                reinterpret_cast<GraphConfiguration100123*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100127:
            if (StaticGraph100127::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100127(
                reinterpret_cast<GraphConfiguration100127*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100128:
            if (StaticGraph100128::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100128(
                reinterpret_cast<GraphConfiguration100128*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100129:
            if (StaticGraph100129::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100129(
                reinterpret_cast<GraphConfiguration100129*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100130:
            if (StaticGraph100130::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100130(
                reinterpret_cast<GraphConfiguration100130*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100131:
            if (StaticGraph100131::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100131(
                reinterpret_cast<GraphConfiguration100131*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100132:
            if (StaticGraph100132::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100132(
                reinterpret_cast<GraphConfiguration100132*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100133:
            if (StaticGraph100133::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100133(
                reinterpret_cast<GraphConfiguration100133*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100134:
            if (StaticGraph100134::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100134(
                reinterpret_cast<GraphConfiguration100134*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100235:
            if (StaticGraph100235::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100235(
                reinterpret_cast<GraphConfiguration100235*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100236:
            if (StaticGraph100236::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100236(
                reinterpret_cast<GraphConfiguration100236*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100202:
            if (StaticGraph100202::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100202(
                reinterpret_cast<GraphConfiguration100202*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100203:
            if (StaticGraph100203::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100203(
                reinterpret_cast<GraphConfiguration100203*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100279:
            if (StaticGraph100279::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100279(
                reinterpret_cast<GraphConfiguration100279*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100280:
            if (StaticGraph100280::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100280(
                reinterpret_cast<GraphConfiguration100280*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100281:
            if (StaticGraph100281::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100281(
                reinterpret_cast<GraphConfiguration100281*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100204:
            if (StaticGraph100204::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100204(
                reinterpret_cast<GraphConfiguration100204*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100205:
            if (StaticGraph100205::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100205(
                reinterpret_cast<GraphConfiguration100205*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100206:
            if (StaticGraph100206::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100206(
                reinterpret_cast<GraphConfiguration100206*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100266:
            if (StaticGraph100266::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100266(
                reinterpret_cast<GraphConfiguration100266*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100207:
            if (StaticGraph100207::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100207(
                reinterpret_cast<GraphConfiguration100207*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100267:
            if (StaticGraph100267::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100267(
                reinterpret_cast<GraphConfiguration100267*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100208:
            if (StaticGraph100208::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100208(
                reinterpret_cast<GraphConfiguration100208*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100209:
            if (StaticGraph100209::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100209(
                reinterpret_cast<GraphConfiguration100209*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100210:
            if (StaticGraph100210::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100210(
                reinterpret_cast<GraphConfiguration100210*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100211:
            if (StaticGraph100211::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100211(
                reinterpret_cast<GraphConfiguration100211*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100245:
            if (StaticGraph100245::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100245(
                reinterpret_cast<GraphConfiguration100245*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100212:
            if (StaticGraph100212::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100212(
                reinterpret_cast<GraphConfiguration100212*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100213:
            if (StaticGraph100213::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100213(
                reinterpret_cast<GraphConfiguration100213*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100214:
            if (StaticGraph100214::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100214(
                reinterpret_cast<GraphConfiguration100214*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100215:
            if (StaticGraph100215::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100215(
                reinterpret_cast<GraphConfiguration100215*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100216:
            if (StaticGraph100216::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100216(
                reinterpret_cast<GraphConfiguration100216*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100217:
            if (StaticGraph100217::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100217(
                reinterpret_cast<GraphConfiguration100217*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100218:
            if (StaticGraph100218::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100218(
                reinterpret_cast<GraphConfiguration100218*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100219:
            if (StaticGraph100219::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100219(
                reinterpret_cast<GraphConfiguration100219*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100220:
            if (StaticGraph100220::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100220(
                reinterpret_cast<GraphConfiguration100220*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100221:
            if (StaticGraph100221::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100221(
                reinterpret_cast<GraphConfiguration100221*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100222:
            if (StaticGraph100222::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100222(
                reinterpret_cast<GraphConfiguration100222*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100223:
            if (StaticGraph100223::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100223(
                reinterpret_cast<GraphConfiguration100223*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100224:
            if (StaticGraph100224::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100224(
                reinterpret_cast<GraphConfiguration100224*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100240:
            if (StaticGraph100240::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100240(
                reinterpret_cast<GraphConfiguration100240*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100241:
            if (StaticGraph100241::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100241(
                reinterpret_cast<GraphConfiguration100241*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100242:
            if (StaticGraph100242::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100242(
                reinterpret_cast<GraphConfiguration100242*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100227:
            if (StaticGraph100227::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100227(
                reinterpret_cast<GraphConfiguration100227*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100228:
            if (StaticGraph100228::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100228(
                reinterpret_cast<GraphConfiguration100228*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100229:
            if (StaticGraph100229::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100229(
                reinterpret_cast<GraphConfiguration100229*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100230:
            if (StaticGraph100230::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100230(
                reinterpret_cast<GraphConfiguration100230*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100231:
            if (StaticGraph100231::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100231(
                reinterpret_cast<GraphConfiguration100231*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100232:
            if (StaticGraph100232::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100232(
                reinterpret_cast<GraphConfiguration100232*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100233:
            if (StaticGraph100233::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100233(
                reinterpret_cast<GraphConfiguration100233*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100234:
            if (StaticGraph100234::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100234(
                reinterpret_cast<GraphConfiguration100234*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100026:
            if (StaticGraph100026::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100026(
                reinterpret_cast<GraphConfiguration100026*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100059:
            if (StaticGraph100059::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100059(
                reinterpret_cast<GraphConfiguration100059*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100035:
            if (StaticGraph100035::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100035(
                reinterpret_cast<GraphConfiguration100035*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100036:
            if (StaticGraph100036::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100036(
                reinterpret_cast<GraphConfiguration100036*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100037:
            if (StaticGraph100037::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100037(
                reinterpret_cast<GraphConfiguration100037*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100058:
            if (StaticGraph100058::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100058(
                reinterpret_cast<GraphConfiguration100058*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100038:
            if (StaticGraph100038::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100038(
                reinterpret_cast<GraphConfiguration100038*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100039:
            if (StaticGraph100039::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100039(
                reinterpret_cast<GraphConfiguration100039*>(selectedConfigurationData), &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        default:
            STATIC_GRAPH_LOG("Graph %d was not found", selectedGraphConfigurationHeader->graphId);
            return StaticGraphStatus::SG_ERROR;
    }

    return StaticGraphStatus::SG_OK;
}

void StaticGraphReader::GetSinkMappingConfiguration(GraphConfigurationHeader* baseGraphConfigurationHeader, VirtualSinkMapping* baseSinkMappingConfiguration, GraphConfigurationHeader* selectedGraphConfigurationHeader, VirtualSinkMapping* selectedSinkMappingConfiguration) {
    if (baseGraphConfigurationHeader == nullptr)
    {
        memcpy(selectedSinkMappingConfiguration, baseSinkMappingConfiguration, sizeof(VirtualSinkMapping));
    }
    else
    {
        if (selectedGraphConfigurationHeader->settingsKey.preview.bpp == baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.preview.width == baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.preview.height == baseGraphConfigurationHeader->settingsKey.preview.height
            )
        {
            selectedSinkMappingConfiguration->preview = baseSinkMappingConfiguration->preview;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.preview.bpp == baseGraphConfigurationHeader->settingsKey.video.bpp &&
            selectedGraphConfigurationHeader->settingsKey.preview.width == baseGraphConfigurationHeader->settingsKey.video.width &&
            selectedGraphConfigurationHeader->settingsKey.preview.height == baseGraphConfigurationHeader->settingsKey.video.height
            )
        {
            selectedSinkMappingConfiguration->preview = baseSinkMappingConfiguration->video;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.preview.bpp == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
            selectedGraphConfigurationHeader->settingsKey.preview.width == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
            selectedGraphConfigurationHeader->settingsKey.preview.height == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height
            )
        {
            selectedSinkMappingConfiguration->preview = baseSinkMappingConfiguration->postProcessingVideo;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for preview sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.video.bpp == baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.video.width == baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.video.height == baseGraphConfigurationHeader->settingsKey.preview.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->preview
            )
        {
            selectedSinkMappingConfiguration->video = baseSinkMappingConfiguration->preview;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.video.bpp == baseGraphConfigurationHeader->settingsKey.video.bpp &&
            selectedGraphConfigurationHeader->settingsKey.video.width == baseGraphConfigurationHeader->settingsKey.video.width &&
            selectedGraphConfigurationHeader->settingsKey.video.height == baseGraphConfigurationHeader->settingsKey.video.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->video
            )
        {
            selectedSinkMappingConfiguration->video = baseSinkMappingConfiguration->video;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.video.bpp == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
            selectedGraphConfigurationHeader->settingsKey.video.width == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
            selectedGraphConfigurationHeader->settingsKey.video.height == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->postProcessingVideo
            )
        {
            selectedSinkMappingConfiguration->video = baseSinkMappingConfiguration->postProcessingVideo;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for video sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp == baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width == baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height == baseGraphConfigurationHeader->settingsKey.preview.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->preview
            && selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->preview
            )
        {
            selectedSinkMappingConfiguration->postProcessingVideo = baseSinkMappingConfiguration->preview;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp == baseGraphConfigurationHeader->settingsKey.video.bpp &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width == baseGraphConfigurationHeader->settingsKey.video.width &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height == baseGraphConfigurationHeader->settingsKey.video.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->video
            && selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->video
            )
        {
            selectedSinkMappingConfiguration->postProcessingVideo = baseSinkMappingConfiguration->video;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height == baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->postProcessingVideo
            && selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->postProcessingVideo
            )
        {
            selectedSinkMappingConfiguration->postProcessingVideo = baseSinkMappingConfiguration->postProcessingVideo;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for postProcessingVideo sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.stills.bpp == baseGraphConfigurationHeader->settingsKey.stills.bpp &&
            selectedGraphConfigurationHeader->settingsKey.stills.width == baseGraphConfigurationHeader->settingsKey.stills.width &&
            selectedGraphConfigurationHeader->settingsKey.stills.height == baseGraphConfigurationHeader->settingsKey.stills.height
            && selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->stills
            && selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->stills
            && selectedSinkMappingConfiguration->postProcessingVideo != baseSinkMappingConfiguration->stills
            )
        {
            selectedSinkMappingConfiguration->stills = baseSinkMappingConfiguration->stills;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for stills sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.videoIr.bpp == baseGraphConfigurationHeader->settingsKey.videoIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.width == baseGraphConfigurationHeader->settingsKey.videoIr.width &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.height == baseGraphConfigurationHeader->settingsKey.videoIr.height
            )
        {
            selectedSinkMappingConfiguration->videoIr = baseSinkMappingConfiguration->videoIr;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.videoIr.bpp == baseGraphConfigurationHeader->settingsKey.previewIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.width == baseGraphConfigurationHeader->settingsKey.previewIr.width &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.height == baseGraphConfigurationHeader->settingsKey.previewIr.height
            )
        {
            selectedSinkMappingConfiguration->videoIr = baseSinkMappingConfiguration->previewIr;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for videoIr sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.previewIr.bpp == baseGraphConfigurationHeader->settingsKey.videoIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.width == baseGraphConfigurationHeader->settingsKey.videoIr.width &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.height == baseGraphConfigurationHeader->settingsKey.videoIr.height
            && selectedSinkMappingConfiguration->videoIr != baseSinkMappingConfiguration->videoIr
            )
        {
            selectedSinkMappingConfiguration->previewIr = baseSinkMappingConfiguration->videoIr;
        }
        else
        if (selectedGraphConfigurationHeader->settingsKey.previewIr.bpp == baseGraphConfigurationHeader->settingsKey.previewIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.width == baseGraphConfigurationHeader->settingsKey.previewIr.width &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.height == baseGraphConfigurationHeader->settingsKey.previewIr.height
            && selectedSinkMappingConfiguration->videoIr != baseSinkMappingConfiguration->previewIr
            )
        {
            selectedSinkMappingConfiguration->previewIr = baseSinkMappingConfiguration->previewIr;
        }
        else
        {
            STATIC_GRAPH_LOG("Did not find correct mapping for previewIr sink.");
        }

    }
}
