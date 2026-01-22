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

#include "StaticGraphReaderAutogen.h"

#include <cstring>

StaticGraphStatus StaticGraphReader::Init(StaticReaderBinaryData& binaryGraphSettings) {
    if (!binaryGraphSettings.data) {
        STATIC_GRAPH_LOG("Binary settings is empty.");
        return StaticGraphStatus::SG_ERROR;
    }

    int8_t* currOffset = static_cast<int8_t*>(binaryGraphSettings.data);
    _binaryHeader = *reinterpret_cast<BinaryHeader*>(currOffset);

    if (_binaryHeader.binaryCommonHashCode != staticGraphCommonHashCode) {
        STATIC_GRAPH_LOG(
            "Binary hash code is not matching the static graph structure hash code. Binary should "
            "be re-created.");
        return StaticGraphStatus::SG_ERROR;
    }

    // Skipping BinaryHeader

    currOffset += sizeof(BinaryHeader);

    uint32_t numOfAvailablePins = 0;
    DataRangeHeader dataRangeHeader = *(DataRangeHeader*)currOffset;

    for (int j = 0; j < enNumOfOutPins; j++)
        numOfAvailablePins += dataRangeHeader.NumberOfPinResolutions[j];

    currOffset += sizeof(DataRangeHeader) + sizeof(DriverDesc) * numOfAvailablePins;

    uint32_t numOfGraphs = *(uint32_t*)currOffset;
    currOffset += sizeof(numOfGraphs) + numOfGraphs * sizeof(GraphHashCode);

    _graphConfigurationHeaders = reinterpret_cast<GraphConfigurationHeader*>(currOffset);
    currOffset += sizeof(GraphConfigurationHeader) * _binaryHeader.numberOfResolutions;
    _sensorModes = reinterpret_cast<SensorMode*>(currOffset);
    currOffset += sizeof(SensorMode) * _binaryHeader.numberOfSensorModes;
    _configurationData = currOffset;

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus StaticGraphReader::GetStaticGraphConfig(GraphConfigurationKey& settingsKey,
                                                          IStaticGraphConfig** graph) {
    if (!_graphConfigurationHeaders || !_sensorModes || !_configurationData) {
        STATIC_GRAPH_LOG("Static graph reader was not initialized properly.");
        return StaticGraphStatus::SG_ERROR;
    }

    if (!graph) {
        STATIC_GRAPH_LOG("Cannot get graph configuration into null parameter");
        return StaticGraphStatus::SG_ERROR;
    }

    GraphConfigurationHeader* selectedGraphConfigurationHeader = nullptr;

    for (uint32_t i = 0; i < _binaryHeader.numberOfResolutions; i++) {
        if (memcmp(&_graphConfigurationHeaders[i].settingsKey, &settingsKey,
                   sizeof(GraphConfigurationKey)) == 0) {
            selectedGraphConfigurationHeader = &_graphConfigurationHeaders[i];
            STATIC_GRAPH_LOG("Static graph selected setting id - %d",
                             selectedGraphConfigurationHeader->settingId);

            break;
        }
    }

    if (!selectedGraphConfigurationHeader) {
        STATIC_GRAPH_LOG("Resolution settings was not found for the given key.");
        return StaticGraphStatus::SG_ERROR;
    }

    int8_t* selectedConfigurationData =
        _configurationData + selectedGraphConfigurationHeader->resConfigDataOffset;

    GraphConfigurationHeader* baseGraphConfigurationHeader = nullptr;

    for (uint32_t i = 0; i < _binaryHeader.numberOfResolutions; i++) {
        if (_graphConfigurationHeaders[i].resConfigDataOffset ==
            selectedGraphConfigurationHeader->resConfigDataOffset) {
            if (selectedGraphConfigurationHeader != &_graphConfigurationHeaders[i]) {
                baseGraphConfigurationHeader = &_graphConfigurationHeaders[i];
            }
            break;
        }
    }

    VirtualSinkMapping* baseSinkMappingConfiguration =
        reinterpret_cast<VirtualSinkMapping*>(selectedConfigurationData);

    VirtualSinkMapping selectedSinkMappingConfiguration;
    GetSinkMappingConfiguration(baseGraphConfigurationHeader, baseSinkMappingConfiguration,
                                selectedGraphConfigurationHeader,
                                &selectedSinkMappingConfiguration);

    // fetching the graph
    switch (selectedGraphConfigurationHeader->graphId) {
        case 100000:
            if (StaticGraph100000::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100000(
                reinterpret_cast<GraphConfiguration100000*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100001:
            if (StaticGraph100001::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100001(
                reinterpret_cast<GraphConfiguration100001*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100002:
            if (StaticGraph100002::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100002(
                reinterpret_cast<GraphConfiguration100002*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100003:
            if (StaticGraph100003::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100003(
                reinterpret_cast<GraphConfiguration100003*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100079:
            if (StaticGraph100079::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100079(
                reinterpret_cast<GraphConfiguration100079*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100080:
            if (StaticGraph100080::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100080(
                reinterpret_cast<GraphConfiguration100080*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100081:
            if (StaticGraph100081::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100081(
                reinterpret_cast<GraphConfiguration100081*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100004:
            if (StaticGraph100004::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100004(
                reinterpret_cast<GraphConfiguration100004*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100005:
            if (StaticGraph100005::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100005(
                reinterpret_cast<GraphConfiguration100005*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100006:
            if (StaticGraph100006::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100006(
                reinterpret_cast<GraphConfiguration100006*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100007:
            if (StaticGraph100007::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100007(
                reinterpret_cast<GraphConfiguration100007*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100008:
            if (StaticGraph100008::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100008(
                reinterpret_cast<GraphConfiguration100008*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100009:
            if (StaticGraph100009::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100009(
                reinterpret_cast<GraphConfiguration100009*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100010:
            if (StaticGraph100010::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100010(
                reinterpret_cast<GraphConfiguration100010*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100011:
            if (StaticGraph100011::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100011(
                reinterpret_cast<GraphConfiguration100011*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100045:
            if (StaticGraph100045::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100045(
                reinterpret_cast<GraphConfiguration100045*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100012:
            if (StaticGraph100012::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100012(
                reinterpret_cast<GraphConfiguration100012*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100013:
            if (StaticGraph100013::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100013(
                reinterpret_cast<GraphConfiguration100013*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100014:
            if (StaticGraph100014::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100014(
                reinterpret_cast<GraphConfiguration100014*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100015:
            if (StaticGraph100015::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100015(
                reinterpret_cast<GraphConfiguration100015*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100016:
            if (StaticGraph100016::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100016(
                reinterpret_cast<GraphConfiguration100016*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100017:
            if (StaticGraph100017::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100017(
                reinterpret_cast<GraphConfiguration100017*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100018:
            if (StaticGraph100018::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100018(
                reinterpret_cast<GraphConfiguration100018*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100019:
            if (StaticGraph100019::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100019(
                reinterpret_cast<GraphConfiguration100019*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100020:
            if (StaticGraph100020::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100020(
                reinterpret_cast<GraphConfiguration100020*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100021:
            if (StaticGraph100021::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100021(
                reinterpret_cast<GraphConfiguration100021*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100022:
            if (StaticGraph100022::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100022(
                reinterpret_cast<GraphConfiguration100022*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100023:
            if (StaticGraph100023::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100023(
                reinterpret_cast<GraphConfiguration100023*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100024:
            if (StaticGraph100024::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100024(
                reinterpret_cast<GraphConfiguration100024*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100040:
            if (StaticGraph100040::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100040(
                reinterpret_cast<GraphConfiguration100040*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100041:
            if (StaticGraph100041::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100041(
                reinterpret_cast<GraphConfiguration100041*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100042:
            if (StaticGraph100042::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100042(
                reinterpret_cast<GraphConfiguration100042*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100027:
            if (StaticGraph100027::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100027(
                reinterpret_cast<GraphConfiguration100027*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100028:
            if (StaticGraph100028::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100028(
                reinterpret_cast<GraphConfiguration100028*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100029:
            if (StaticGraph100029::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100029(
                reinterpret_cast<GraphConfiguration100029*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100030:
            if (StaticGraph100030::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100030(
                reinterpret_cast<GraphConfiguration100030*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100031:
            if (StaticGraph100031::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100031(
                reinterpret_cast<GraphConfiguration100031*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100032:
            if (StaticGraph100032::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100032(
                reinterpret_cast<GraphConfiguration100032*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100033:
            if (StaticGraph100033::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100033(
                reinterpret_cast<GraphConfiguration100033*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100034:
            if (StaticGraph100034::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100034(
                reinterpret_cast<GraphConfiguration100034*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100100:
            if (StaticGraph100100::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100100(
                reinterpret_cast<GraphConfiguration100100*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100101:
            if (StaticGraph100101::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100101(
                reinterpret_cast<GraphConfiguration100101*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100102:
            if (StaticGraph100102::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100102(
                reinterpret_cast<GraphConfiguration100102*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100103:
            if (StaticGraph100103::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100103(
                reinterpret_cast<GraphConfiguration100103*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100104:
            if (StaticGraph100104::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100104(
                reinterpret_cast<GraphConfiguration100104*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100105:
            if (StaticGraph100105::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100105(
                reinterpret_cast<GraphConfiguration100105*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100106:
            if (StaticGraph100106::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100106(
                reinterpret_cast<GraphConfiguration100106*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100107:
            if (StaticGraph100107::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100107(
                reinterpret_cast<GraphConfiguration100107*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100108:
            if (StaticGraph100108::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100108(
                reinterpret_cast<GraphConfiguration100108*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100109:
            if (StaticGraph100109::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100109(
                reinterpret_cast<GraphConfiguration100109*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100110:
            if (StaticGraph100110::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100110(
                reinterpret_cast<GraphConfiguration100110*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100111:
            if (StaticGraph100111::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100111(
                reinterpret_cast<GraphConfiguration100111*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100112:
            if (StaticGraph100112::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100112(
                reinterpret_cast<GraphConfiguration100112*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100113:
            if (StaticGraph100113::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100113(
                reinterpret_cast<GraphConfiguration100113*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100114:
            if (StaticGraph100114::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100114(
                reinterpret_cast<GraphConfiguration100114*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100115:
            if (StaticGraph100115::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100115(
                reinterpret_cast<GraphConfiguration100115*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100116:
            if (StaticGraph100116::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100116(
                reinterpret_cast<GraphConfiguration100116*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100117:
            if (StaticGraph100117::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100117(
                reinterpret_cast<GraphConfiguration100117*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100118:
            if (StaticGraph100118::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100118(
                reinterpret_cast<GraphConfiguration100118*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100119:
            if (StaticGraph100119::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100119(
                reinterpret_cast<GraphConfiguration100119*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100120:
            if (StaticGraph100120::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100120(
                reinterpret_cast<GraphConfiguration100120*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100121:
            if (StaticGraph100121::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100121(
                reinterpret_cast<GraphConfiguration100121*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100122:
            if (StaticGraph100122::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100122(
                reinterpret_cast<GraphConfiguration100122*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100123:
            if (StaticGraph100123::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100123(
                reinterpret_cast<GraphConfiguration100123*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100127:
            if (StaticGraph100127::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100127(
                reinterpret_cast<GraphConfiguration100127*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100128:
            if (StaticGraph100128::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100128(
                reinterpret_cast<GraphConfiguration100128*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100129:
            if (StaticGraph100129::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100129(
                reinterpret_cast<GraphConfiguration100129*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100130:
            if (StaticGraph100130::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100130(
                reinterpret_cast<GraphConfiguration100130*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100131:
            if (StaticGraph100131::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100131(
                reinterpret_cast<GraphConfiguration100131*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100132:
            if (StaticGraph100132::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100132(
                reinterpret_cast<GraphConfiguration100132*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100133:
            if (StaticGraph100133::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100133(
                reinterpret_cast<GraphConfiguration100133*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100134:
            if (StaticGraph100134::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100134(
                reinterpret_cast<GraphConfiguration100134*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100026:
            if (StaticGraph100026::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100026(
                reinterpret_cast<GraphConfiguration100026*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100035:
            if (StaticGraph100035::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100035(
                reinterpret_cast<GraphConfiguration100035*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100036:
            if (StaticGraph100036::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100036(
                reinterpret_cast<GraphConfiguration100036*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100037:
            if (StaticGraph100037::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100037(
                reinterpret_cast<GraphConfiguration100037*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100038:
            if (StaticGraph100038::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100038(
                reinterpret_cast<GraphConfiguration100038*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        case 100039:
            if (StaticGraph100039::hashCode != selectedGraphConfigurationHeader->graphHashCode) {
                STATIC_GRAPH_LOG(
                    "Graph %d hash code is not matching the settings. Binary should be re-created.",
                    selectedGraphConfigurationHeader->graphId);
                return StaticGraphStatus::SG_ERROR;
            }
            *graph = new StaticGraph100039(
                reinterpret_cast<GraphConfiguration100039*>(selectedConfigurationData),
                &selectedSinkMappingConfiguration,
                &_sensorModes[selectedGraphConfigurationHeader->sensorModeIndex],
                selectedGraphConfigurationHeader->settingId);
            break;
        default:
            STATIC_GRAPH_LOG("Graph %d was not found", selectedGraphConfigurationHeader->graphId);
            return StaticGraphStatus::SG_ERROR;
    }

    return StaticGraphStatus::SG_OK;
}

void StaticGraphReader::GetSinkMappingConfiguration(
    GraphConfigurationHeader* baseGraphConfigurationHeader,
    VirtualSinkMapping* baseSinkMappingConfiguration,
    GraphConfigurationHeader* selectedGraphConfigurationHeader,
    VirtualSinkMapping* selectedSinkMappingConfiguration) {
    if (baseGraphConfigurationHeader == nullptr) {
        memcpy(selectedSinkMappingConfiguration, baseSinkMappingConfiguration,
               sizeof(VirtualSinkMapping));
    } else {
        if (selectedGraphConfigurationHeader->settingsKey.preview.bpp ==
                baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.preview.width ==
                baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.preview.height ==
                baseGraphConfigurationHeader->settingsKey.preview.height) {
            selectedSinkMappingConfiguration->preview = baseSinkMappingConfiguration->preview;
        } else if (selectedGraphConfigurationHeader->settingsKey.preview.bpp ==
                       baseGraphConfigurationHeader->settingsKey.video.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.preview.width ==
                       baseGraphConfigurationHeader->settingsKey.video.width &&
                   selectedGraphConfigurationHeader->settingsKey.preview.height ==
                       baseGraphConfigurationHeader->settingsKey.video.height) {
            selectedSinkMappingConfiguration->preview = baseSinkMappingConfiguration->video;
        } else if (selectedGraphConfigurationHeader->settingsKey.preview.bpp ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.preview.width ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
                   selectedGraphConfigurationHeader->settingsKey.preview.height ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height) {
            selectedSinkMappingConfiguration->preview =
                baseSinkMappingConfiguration->postProcessingVideo;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for preview sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.video.bpp ==
                baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.video.width ==
                baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.video.height ==
                baseGraphConfigurationHeader->settingsKey.preview.height &&
            selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->preview) {
            selectedSinkMappingConfiguration->video = baseSinkMappingConfiguration->preview;
        } else if (selectedGraphConfigurationHeader->settingsKey.video.bpp ==
                       baseGraphConfigurationHeader->settingsKey.video.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.video.width ==
                       baseGraphConfigurationHeader->settingsKey.video.width &&
                   selectedGraphConfigurationHeader->settingsKey.video.height ==
                       baseGraphConfigurationHeader->settingsKey.video.height &&
                   selectedSinkMappingConfiguration->preview !=
                       baseSinkMappingConfiguration->video) {
            selectedSinkMappingConfiguration->video = baseSinkMappingConfiguration->video;
        } else if (selectedGraphConfigurationHeader->settingsKey.video.bpp ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.video.width ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
                   selectedGraphConfigurationHeader->settingsKey.video.height ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height &&
                   selectedSinkMappingConfiguration->preview !=
                       baseSinkMappingConfiguration->postProcessingVideo) {
            selectedSinkMappingConfiguration->video =
                baseSinkMappingConfiguration->postProcessingVideo;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for video sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp ==
                baseGraphConfigurationHeader->settingsKey.preview.bpp &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width ==
                baseGraphConfigurationHeader->settingsKey.preview.width &&
            selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height ==
                baseGraphConfigurationHeader->settingsKey.preview.height &&
            selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->preview &&
            selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->preview) {
            selectedSinkMappingConfiguration->postProcessingVideo =
                baseSinkMappingConfiguration->preview;
        } else if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp ==
                       baseGraphConfigurationHeader->settingsKey.video.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width ==
                       baseGraphConfigurationHeader->settingsKey.video.width &&
                   selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height ==
                       baseGraphConfigurationHeader->settingsKey.video.height &&
                   selectedSinkMappingConfiguration->preview !=
                       baseSinkMappingConfiguration->video &&
                   selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->video) {
            selectedSinkMappingConfiguration->postProcessingVideo =
                baseSinkMappingConfiguration->video;
        } else if (selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.width ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.width &&
                   selectedGraphConfigurationHeader->settingsKey.postProcessingVideo.height ==
                       baseGraphConfigurationHeader->settingsKey.postProcessingVideo.height &&
                   selectedSinkMappingConfiguration->preview !=
                       baseSinkMappingConfiguration->postProcessingVideo &&
                   selectedSinkMappingConfiguration->video !=
                       baseSinkMappingConfiguration->postProcessingVideo) {
            selectedSinkMappingConfiguration->postProcessingVideo =
                baseSinkMappingConfiguration->postProcessingVideo;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for postProcessingVideo sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.stills.bpp ==
                baseGraphConfigurationHeader->settingsKey.stills.bpp &&
            selectedGraphConfigurationHeader->settingsKey.stills.width ==
                baseGraphConfigurationHeader->settingsKey.stills.width &&
            selectedGraphConfigurationHeader->settingsKey.stills.height ==
                baseGraphConfigurationHeader->settingsKey.stills.height &&
            selectedSinkMappingConfiguration->preview != baseSinkMappingConfiguration->stills &&
            selectedSinkMappingConfiguration->video != baseSinkMappingConfiguration->stills &&
            selectedSinkMappingConfiguration->postProcessingVideo !=
                baseSinkMappingConfiguration->stills) {
            selectedSinkMappingConfiguration->stills = baseSinkMappingConfiguration->stills;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for stills sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.videoIr.bpp ==
                baseGraphConfigurationHeader->settingsKey.videoIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.width ==
                baseGraphConfigurationHeader->settingsKey.videoIr.width &&
            selectedGraphConfigurationHeader->settingsKey.videoIr.height ==
                baseGraphConfigurationHeader->settingsKey.videoIr.height) {
            selectedSinkMappingConfiguration->videoIr = baseSinkMappingConfiguration->videoIr;
        } else if (selectedGraphConfigurationHeader->settingsKey.videoIr.bpp ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.videoIr.width ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.width &&
                   selectedGraphConfigurationHeader->settingsKey.videoIr.height ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.height) {
            selectedSinkMappingConfiguration->videoIr = baseSinkMappingConfiguration->previewIr;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for videoIr sink.");
        }

        if (selectedGraphConfigurationHeader->settingsKey.previewIr.bpp ==
                baseGraphConfigurationHeader->settingsKey.videoIr.bpp &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.width ==
                baseGraphConfigurationHeader->settingsKey.videoIr.width &&
            selectedGraphConfigurationHeader->settingsKey.previewIr.height ==
                baseGraphConfigurationHeader->settingsKey.videoIr.height &&
            selectedSinkMappingConfiguration->videoIr != baseSinkMappingConfiguration->videoIr) {
            selectedSinkMappingConfiguration->previewIr = baseSinkMappingConfiguration->videoIr;
        } else if (selectedGraphConfigurationHeader->settingsKey.previewIr.bpp ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.bpp &&
                   selectedGraphConfigurationHeader->settingsKey.previewIr.width ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.width &&
                   selectedGraphConfigurationHeader->settingsKey.previewIr.height ==
                       baseGraphConfigurationHeader->settingsKey.previewIr.height &&
                   selectedSinkMappingConfiguration->videoIr !=
                       baseSinkMappingConfiguration->previewIr) {
            selectedSinkMappingConfiguration->previewIr = baseSinkMappingConfiguration->previewIr;
        } else {
            STATIC_GRAPH_LOG("Did not find correct mapping for previewIr sink.");
        }
    }
}
