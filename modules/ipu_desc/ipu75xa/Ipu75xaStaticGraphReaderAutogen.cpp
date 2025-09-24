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

#include "Ipu75xaStaticGraphReaderAutogen.h"
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

    _zoomKeyResolutions = *(ZoomKeyResolutions*)currOffset;
    currOffset += sizeof(_zoomKeyResolutions.numberOfZoomKeyOptions);
    if (_zoomKeyResolutions.numberOfZoomKeyOptions > 0)
    {
        _zoomKeyResolutions.zoomKeyResolutionOptions = reinterpret_cast<ZoomKeyResolution*>(currOffset);
        currOffset += _zoomKeyResolutions.numberOfZoomKeyOptions*sizeof(ZoomKeyResolution);
    }
    else
    {
        _zoomKeyResolutions.zoomKeyResolutionOptions = nullptr;
    }

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
    GraphConfigurationHeader** selectedGraphConfigurationHeaders = new GraphConfigurationHeader*[_zoomKeyResolutions.numberOfZoomKeyOptions+1];
    uint32_t selectedConfigurationsCount = 0;

    for (uint32_t i=0; i < _binaryHeader.numberOfResolutions; i++)
    {
        if (memcmp ( &_graphConfigurationHeaders[i].settingsKey,
            &settingsKey,
            sizeof(GraphConfigurationKey)) == 0)
        {
            selectedGraphConfigurationHeader = &_graphConfigurationHeaders[i];
            STATIC_GRAPH_LOG("Static graph selected setting id - %d", selectedGraphConfigurationHeader->settingId);

            selectedConfigurationsCount++;
            if (selectedConfigurationsCount > _zoomKeyResolutions.numberOfZoomKeyOptions+1)
            {
                STATIC_GRAPH_LOG("Too many resolution settings were found for the given key.");
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }

            selectedGraphConfigurationHeaders[selectedConfigurationsCount-1] = selectedGraphConfigurationHeader;
            if (_zoomKeyResolutions.numberOfZoomKeyOptions == 0)
            {
                break;
            }

        }
    }

    if (selectedConfigurationsCount > 1)
    {
        selectedGraphConfigurationHeader = selectedGraphConfigurationHeaders[0];
    }

    if (!selectedGraphConfigurationHeader || selectedConfigurationsCount == 0)
    {
        STATIC_GRAPH_LOG("Resolution settings was not found for the given key.");
        delete[] selectedGraphConfigurationHeaders;
        return StaticGraphStatus::SG_ERROR;
    }

    for (uint32_t i = 0; i < selectedConfigurationsCount; ++i)
    {
        if (selectedGraphConfigurationHeaders[i]->graphId != selectedGraphConfigurationHeader->graphId ||
        selectedGraphConfigurationHeaders[i]->sensorModeIndex != selectedGraphConfigurationHeader->sensorModeIndex)
        {
             if (!selectedGraphConfigurationHeader)
             {
                 STATIC_GRAPH_LOG("One or more configurations with same key have differnt graph id or sensor mdoe.");
                 delete[] selectedGraphConfigurationHeaders;
                 return StaticGraphStatus::SG_ERROR;
             }
        }
    }

    int8_t** selectedConfigurationData = new int8_t*[selectedConfigurationsCount];
    for (uint32_t i = 0; i <selectedConfigurationsCount; ++i)
    {
        selectedConfigurationData[i] = _configurationData + selectedGraphConfigurationHeaders[i]->
                resConfigDataOffset;
    }

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

    VirtualSinkMapping* baseSinkMappingConfiguration = reinterpret_cast<VirtualSinkMapping*>(selectedConfigurationData[0]);

    VirtualSinkMapping selectedSinkMappingConfiguration;
    GetSinkMappingConfiguration(baseGraphConfigurationHeader, baseSinkMappingConfiguration, selectedGraphConfigurationHeader, &selectedSinkMappingConfiguration);

    // fetching the graph
    switch (selectedGraphConfigurationHeader->graphId)
    {
        case 100000:
            if (StaticGraph100000::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100000(
                reinterpret_cast<GraphConfiguration100000**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100001:
            if (StaticGraph100001::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100001(
                reinterpret_cast<GraphConfiguration100001**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100002:
            if (StaticGraph100002::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100002(
                reinterpret_cast<GraphConfiguration100002**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100003:
            if (StaticGraph100003::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100003(
                reinterpret_cast<GraphConfiguration100003**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100005:
            if (StaticGraph100005::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100005(
                reinterpret_cast<GraphConfiguration100005**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100006:
            if (StaticGraph100006::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100006(
                reinterpret_cast<GraphConfiguration100006**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100007:
            if (StaticGraph100007::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100007(
                reinterpret_cast<GraphConfiguration100007**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100008:
            if (StaticGraph100008::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100008(
                reinterpret_cast<GraphConfiguration100008**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100015:
            if (StaticGraph100015::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100015(
                reinterpret_cast<GraphConfiguration100015**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100016:
            if (StaticGraph100016::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100016(
                reinterpret_cast<GraphConfiguration100016**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100025:
            if (StaticGraph100025::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100025(
                reinterpret_cast<GraphConfiguration100025**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100026:
            if (StaticGraph100026::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100026(
                reinterpret_cast<GraphConfiguration100026**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100027:
            if (StaticGraph100027::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100027(
                reinterpret_cast<GraphConfiguration100027**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100028:
            if (StaticGraph100028::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100028(
                reinterpret_cast<GraphConfiguration100028**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100029:
            if (StaticGraph100029::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100029(
                reinterpret_cast<GraphConfiguration100029**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100030:
            if (StaticGraph100030::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100030(
                reinterpret_cast<GraphConfiguration100030**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100031:
            if (StaticGraph100031::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100031(
                reinterpret_cast<GraphConfiguration100031**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100032:
            if (StaticGraph100032::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100032(
                reinterpret_cast<GraphConfiguration100032**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100033:
            if (StaticGraph100033::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100033(
                reinterpret_cast<GraphConfiguration100033**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100034:
            if (StaticGraph100034::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100034(
                reinterpret_cast<GraphConfiguration100034**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100035:
            if (StaticGraph100035::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100035(
                reinterpret_cast<GraphConfiguration100035**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100036:
            if (StaticGraph100036::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100036(
                reinterpret_cast<GraphConfiguration100036**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100037:
            if (StaticGraph100037::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100037(
                reinterpret_cast<GraphConfiguration100037**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100038:
            if (StaticGraph100038::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100038(
                reinterpret_cast<GraphConfiguration100038**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100039:
            if (StaticGraph100039::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100039(
                reinterpret_cast<GraphConfiguration100039**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100040:
            if (StaticGraph100040::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100040(
                reinterpret_cast<GraphConfiguration100040**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100041:
            if (StaticGraph100041::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100041(
                reinterpret_cast<GraphConfiguration100041**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100042:
            if (StaticGraph100042::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100042(
                reinterpret_cast<GraphConfiguration100042**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100044:
            if (StaticGraph100044::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100044(
                reinterpret_cast<GraphConfiguration100044**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100050:
            if (StaticGraph100050::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100050(
                reinterpret_cast<GraphConfiguration100050**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100051:
            if (StaticGraph100051::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100051(
                reinterpret_cast<GraphConfiguration100051**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100052:
            if (StaticGraph100052::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100052(
                reinterpret_cast<GraphConfiguration100052**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100053:
            if (StaticGraph100053::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100053(
                reinterpret_cast<GraphConfiguration100053**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100054:
            if (StaticGraph100054::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100054(
                reinterpret_cast<GraphConfiguration100054**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100055:
            if (StaticGraph100055::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100055(
                reinterpret_cast<GraphConfiguration100055**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100056:
            if (StaticGraph100056::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100056(
                reinterpret_cast<GraphConfiguration100056**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        case 100057:
            if (StaticGraph100057::hashCode != selectedGraphConfigurationHeader->graphHashCode)
            {
                STATIC_GRAPH_LOG("Graph %d hash code is not matching the settings. Binary should be re-created.", selectedGraphConfigurationHeader->graphId);
                delete[] selectedConfigurationData;
                delete[] selectedGraphConfigurationHeaders;
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100057(
                reinterpret_cast<GraphConfiguration100057**>(selectedConfigurationData), selectedConfigurationsCount, &_zoomKeyResolutions, &selectedSinkMappingConfiguration, &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex], selectedGraphConfigurationHeader->settingId);
            break;
        default:
            delete[] selectedConfigurationData;
            delete[] selectedGraphConfigurationHeaders;
            STATIC_GRAPH_LOG("Graph %d was not found", selectedGraphConfigurationHeader->graphId);
            return StaticGraphStatus::SG_ERROR;
    }

    delete[] selectedConfigurationData;
    delete[] selectedGraphConfigurationHeaders;

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
