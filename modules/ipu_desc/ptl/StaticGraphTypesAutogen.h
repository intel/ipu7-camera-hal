/*
 * Copyright (C) 2023-2024 Intel Corporation.
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

#ifdef STATIC_GRAPH_USE_IA_AIC_TYPES
#define STATIC_GRAPH_USE_IA_LEGACY_TYPES
#include "ia_aic_types.h"
typedef aic::ImagingKernelGroup StaticGraphNodeKernels;
typedef aic::ia_pac_kernel_info StaticGraphPacRunKernel;
typedef aic::IaAicFragmentDesc StaticGraphFragmentDesc;
#endif

#ifdef STATIC_GRAPH_USE_IA_LEGACY_TYPES
#include "ia_isp_bxt_types.h"
#if (IA_RESOLUTION_STATIC_GRAPH_PACK != 4)
    #error "Static graph resulution structs packing alignment is defferent than defined in ia_types.h"
#endif
typedef ia_binary_data StaticGraphKernelSystemApi;
typedef ia_rectangle StaticGraphKernelResCrop;
typedef ia_isp_bxt_resolution_info StaticGraphKernelRes;
typedef ia_isp_bxt_bpp_info_t StaticGraphCompKernelBpp;
typedef ia_isp_bxt_run_kernels StaticGraphRunKernel;
#endif

#ifdef STATIC_GRAPH_LOG
#define SG_PRINTF(fmt, ...) do { printf(fmt, ##__VA_ARGS__); printf("\n"); } while ((void)0, 0)
#define STATIC_GRAPH_LOG(...) SG_PRINTF(__VA_ARGS__)
#else
#define STATIC_GRAPH_LOG(...) ((void)0)
#endif

enum class NodeResourceId : uint8_t {
    Lbff = 0,
    Bbps = 1,
    SwIsys = 2,
    SwGdc = 3,
};

enum class StaticGraphStatus : uint8_t
{
    SG_OK = 0,
    SG_ERROR = 1
};

enum class VirtualSink : uint8_t
{
    PreviewSink,
    VideoSink,
    StillsSink,
    ThumbnailSink,
    RawSink,
    RawPdafSink,
    RawDolLongSink,
    VideoIrSink,
    PreviewIrSink,
};

enum class HwSink : uint8_t
{
    Disconnected,
    AeOutSink,
    AfStdOutSink,
    AwbStdOutSink,
    AwbSatOutSink,
    ImageMpSink,
    ImageDpSink,
    GmvMatchOutSink,
    ProcessedMainSink,
    AwbSveOutSink,
    IrAeOutSink,
    IrAfStdOutSink,
    IrAwbStdOutSink,
    IrAwbSatOutSink,
    IrMpSink,
    RawIsysSink,
    PdafOutSink,
    RawIsysDolLongSink,
    RawIsysPdafSink,
};

#pragma pack(push, 4)

#ifndef STATIC_GRAPH_USE_IA_LEGACY_TYPES
// ia_isp_bxt_bpp_info_t
struct StaticGraphCompKernelBpp {
    uint8_t input_bpp = 0;
    uint8_t output_bpp = 0;
};

// ia_rectangle
struct StaticGraphKernelResCrop {
    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;
    int32_t bottom = 0;
};

// ia_isp_bxt_resolution_info
struct StaticGraphKernelRes {
    int32_t input_width = 0;
    int32_t input_height = 0;
    StaticGraphKernelResCrop input_crop;
    int32_t output_width = 0;
    int32_t output_height = 0;
    StaticGraphKernelResCrop output_crop;
};
#endif

struct StaticGraphKernelBppConfiguration {
    uint8_t input_bpp = 0;
    uint8_t output_bpp = 0;
};

struct StaticGraphPlaneCompressionConfiguration {
    uint8_t isEnabled = 0;
    uint8_t tsAlignInterval = 0;
    uint16_t rowsOfTiles = 0;
    uint32_t pixelsOffset = 0;
    uint32_t tsOffset = 0;
    uint32_t rowOfTilesStride = 0;
    uint8_t bpp = 0;
    uint8_t encoderPlaneId = 0;
    uint8_t decoderPlaneId = 0;
    uint8_t isLossy = 0;
    uint8_t isFootprint = 0;
    uint8_t footprintCompressionRatio = 0;
    uint8_t lossyRatioPlus = 0;
    uint8_t lossyRatioMins = 0;
    uint8_t lossyInstUpper = 0;
    uint8_t lossyInstLower = 0;
    uint8_t initHistory = 0;
    uint8_t initQp = 0;
    uint8_t maxQp = 0;
    uint8_t minQp = 0;
    uint8_t windowSize = 0;
    uint8_t maxQpInc = 0;
    uint8_t maxQpDec = 0;
    uint8_t qpIncReset = 0;
    uint8_t logFootprintGuardBand = 0;
};

struct StaticGraphLinkCompressionConfiguration {
    uint8_t isEnabled = 0;
    StaticGraphPlaneCompressionConfiguration lumaCompressionConfiguration;
    StaticGraphPlaneCompressionConfiguration chromaCompressionConfiguration;
};

struct StaticGraphLinkConfiguration {
    uint32_t bufferSize = 0;
    uint8_t streamingMode = 0;
};

struct VirtualSinkMapping {
    uint8_t preview = 0;
    uint8_t video = 0;
    uint8_t stills = 0;
    uint8_t thumbnail = 0;
    uint8_t raw = 0;
    uint8_t rawPdaf = 0;
    uint8_t rawDolLong = 0;
    uint8_t videoIr = 0;
    uint8_t previewIr = 0;
};

#pragma pack(pop)

#ifndef STATIC_GRAPH_USE_IA_LEGACY_TYPES

// ia_binary_data
struct StaticGraphKernelSystemApi {
    void *data;
    uint32_t size;
};

// ia_isp_bxt_run_kernels
struct StaticGraphRunKernel {
    uint32_t stream_id;
    uint32_t kernel_uuid;
    int32_t enable;
    StaticGraphKernelRes *resolution_info;
    StaticGraphKernelRes *resolution_history;
    uint32_t metadata[4];
    StaticGraphCompKernelBpp bpp_info;
    uint32_t output_count;
    StaticGraphKernelSystemApi system_api;
};
#endif

#ifndef STATIC_GRAPH_USE_IA_AIC_TYPES

struct StaticGraphFragmentDesc {
    uint16_t inputWidth = 0;
    uint16_t outputWidth = 0;
    uint16_t left = 0;
};

// ia_pac_kernel_info
struct StaticGraphPacRunKernel {
    StaticGraphRunKernel run_kernel;
    StaticGraphFragmentDesc* fragment_descs;
    bool fragments_defined;
};

// ImagingKernelGroup
struct StaticGraphNodeKernels
{
    uint32_t kernelCount;
    StaticGraphPacRunKernel *kernelList;
    uint32_t operationMode;
    uint32_t streamId;
};

#endif
struct HwBitmaps {
    uint32_t rbm[4] = {};
    uint32_t deb[4] = {};
    uint32_t teb[2] = {};
    uint32_t reb[4] = {};
};

enum class NodeTypes : uint8_t {
    Isys,
    Cb,
    Sw,
};

enum class GraphElementType : uint8_t {
    // Sources
    Sensor,
    LscBuffer,
    LscBufferIr,
    PdafBuffer,
    SensorDolLongExposure,
    // Sinks
    AeOut,
    AfStdOut,
    AwbStdOut,
    AwbSatOut,
    ImageMp,
    ImageDp,
    GmvMatchOut,
    ProcessedMain,
    AwbSveOut,
    IrAeOut,
    IrAfStdOut,
    IrAwbStdOut,
    IrAwbSatOut,
    IrMp,
    RawIsys,
    PdafOut,
    RawIsysDolLong,
    RawIsysPdaf,
    // Outer Nodes
    Isys,
    LbffBayer,
    BbpsNoTnr,
    BbpsWithTnr,
    LbffBayerWithGmv,
    SwGdc,
    LbffRgbIr,
    LbffIrNoGmvIrStream,
    BbpsIrWithTnr,
    LbffBayerBurstOutNo3A,
    BbpsIrNoTnr,
    LbffIrNoGmv,
    IsysPdaf2,
    LbffBayerPdaf2,
    LbffBayerPdaf3,
    IsysDol,
    LbffDol2Inputs,
    LbffDolSmooth,
    LbffDol3Inputs,
    LbffBayerPdaf2WithGmv,
    LbffBayerPdaf3WithGmv,
    LbffRgbIrWithGmv,
    LbffIrWithGmvIrStream,
    LbffDol2InputsWithGmv,
    LbffDol3InputsWithGmv,
};

enum class LinkType : uint8_t {
    Source2Node,
    Node2Node,
    Node2Sink,
    Node2Self,
};

enum class FormatType : uint8_t {
    SINGLE_PL_8_P,
    SINGLE_PL_8,
    SINGLE_PL_8_MSB,
    SINGLE_PL_10_P,
    SINGLE_PL_10,
    SINGLE_PL_10_MSB,
    SINGLE_PL_12_P,
    SINGLE_PL_12,
    SINGLE_PL_12_MSB,
    SINGLE_PL_16,
    YUV420_8_SP_P,
    YUV420_8_SP_P_REV,
    YUV420_8_SP_MSB,
    YUV420_8_SP_REV_MSB,
    YUV420_8_SP_LSB,
    YUV420_8_SP_REV_LSB,
    YUV420_10_SP_P,
    YUV420_10_SP_P_REV,
    YUV420_10_SP_MSB,
    YUV420_10_SP_REV_MSB,
    YUV420_10_SP_LSB,
    YUV420_10_SP_REV_LSB,
    YUV420_12_SP_P,
    YUV420_12_SP_P_REV,
    YUV420_12_SP_MSB,
    YUV420_12_SP_REV_MSB,
    YUV420_12_SP_LSB,
    YUV420_12_SP_REV_LSB,
    YUV420_16_SP,
    YUV420_16_SP_REV,
    YUV420_8_FP_P,
    YUV420_8_FP_P_REV,
    YUV420_8_FP_MSB,
    YUV420_8_FP_REV_MSB,
    YUV420_8_FP_LSB,
    YUV420_8_FP_REV_LSB,
    YUV420_10_FP_P,
    YUV420_10_FP_P_REV,
    YUV420_10_FP_MSB,
    YUV420_10_FP_REV_MSB,
    YUV420_10_FP_LSB,
    YUV420_10_FP_REV_LSB,
    YUV420_12_FP_P,
    YUV420_12_FP_P_REV,
    YUV420_12_FP_MSB,
    YUV420_12_FP_REV_MSB,
    YUV420_12_FP_LSB,
    YUV420_12_FP_REV_LSB,
    YUV420_16_FP,
    YUV420_16_FP_REV,
    META_16,
    YUV420_10_SP_MSB_T32,
    YUV420_12_SP_MSB_T32,
    YUV420_8_SP_P_T32,
    YUV420_8_1P_P,
    YUV422_8_SP_P,
    YUV422_8_SP_P_REV,
    YUV422_YUYV_8_1P_P,
    YUV420_8_SP_P_T16,
    META_8_T16,
};
