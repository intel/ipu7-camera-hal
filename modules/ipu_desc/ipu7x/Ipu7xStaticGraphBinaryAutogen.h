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

#ifndef STATIC_GRAPH_BINARY_H
#define STATIC_GRAPH_BINARY_H

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

enum class GraphConfigurationKeyAttributes : uint32_t
{
    None = 0x00000000,
    PdafType1 = 0x00000001,
    PdafType2 = 0x00000002,
    PdafType3 = 0x00000004,
    DvsActive = 0x00000008,
    Dol2Inputs = 0x00000010,
    PipelineLowLight = 0x00000040,
    PipelineNormalLight = 0x00000080,
    VaiActive = 0x00000100,
    StillsModeCpHdr = 0x00000200,
    ImvDewarpActive = 0x00000400,
};

struct GraphConfigurationKey {
    uint32_t fps = 0;
    uint32_t attributes = 0;
    StreamConfig preview;
    StreamConfig video;
    StreamConfig postProcessingVideo;
    StreamConfig stills;
    StreamConfig postProcessingStills;
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
    uint32_t sapAttributes = 0;
    uint32_t binaryCommonHashCode = 0;
    uint32_t numberOfResolutions = 0;
    uint32_t numberOfSensorModes = 0;
};

struct ZoomKeyResolution
{
    uint32_t width = 0;
    uint32_t height = 0;
};

struct ZoomKeyResolutions {
    uint32_t numberOfZoomKeyOptions;
    ZoomKeyResolution* zoomKeyResolutionOptions;
};

#pragma pack(pop)

#endif
