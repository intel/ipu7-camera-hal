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

#pragma pack(push, 4)
struct SensorMode {
    uint16_t horizontalCropOffset = 0;
    uint16_t verticalCropOffset = 0;
    uint16_t croppedImageWidth = 0;
    uint16_t croppedImageHeight = 0;
    uint16_t horizontalScalingNumerator = 0;
    uint16_t horizontalScalingDenominator = 0;
    uint16_t verticalScalingNumerator = 0;
    uint16_t verticalScalingDenominator = 0;
};

struct StreamConfig {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bpp = 0;
};

struct AutoCalibrationStreamConfig {
    uint32_t depthOutputWidth = 0;
    uint32_t depthOutputHeight = 0;
    uint32_t sliceNumber = 0;
};

enum class GraphConfigurationKeyAttributes : uint32_t {
    None = 0x00000000,
    PdafType1 = 0x00000001,
    PdafType2 = 0x00000002,
    PdafType3 = 0x00000004,
    Dol2Inputs = 0x00000008,
    Dol3Inputs = 0x00000010,
    DvsActive = 0x00000020,
    PipelineLowLight = 0x00000040,
    PipelineNormalLight = 0x00000080,
    LowPowerActive = 0x00000100,
};

struct GraphConfigurationKey {
    uint32_t fps = 0;
    uint32_t attributes = 0;
    StreamConfig preview;
    StreamConfig video;
    StreamConfig postProcessingVideo;
    StreamConfig stills;
    StreamConfig raw;
    StreamConfig videoIr;
    StreamConfig previewIr;
};

struct GraphConfigurationHeader {
    GraphConfigurationKey settingsKey;
    uint16_t settingId = 0;
    int32_t graphId = 0;
    uint8_t sensorModeIndex = 0;
    int32_t resConfigDataOffset = 0;
    uint32_t graphHashCode = 0;
};

struct BinaryHeader {
    uint32_t isSapEnable = 0;
    uint32_t binaryCommonHashCode = 0;
    uint32_t numberOfResolutions = 0;
    uint32_t numberOfSensorModes = 0;
};

#pragma pack(pop)
