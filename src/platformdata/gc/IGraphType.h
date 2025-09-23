/*
 * Copyright (C) 2018-2024 Intel Corporation
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

#include <string>
#include "HalStream.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

#if defined(GRC_IPU7X)
#include "Ipu7xStaticGraphAutogen.h"
#include "Ipu7xStaticGraphReaderAutogen.h"
#elif defined(GRC_IPU75XA)
#include "Ipu75xaStaticGraphAutogen.h"
#include "Ipu75xaStaticGraphReaderAutogen.h"
#elif defined(GRC_IPU8)
#include "Ipu8StaticGraphAutogen.h"
#include "Ipu8StaticGraphReaderAutogen.h"
#else
#include "StaticGraphAutogen.h"
#include "StaticGraphReaderAutogen.h"
#endif

#include "ia_aic_types.h"

namespace icamera {

#ifdef TNR7_CM
// Stream id associated with video stream.
static const int32_t VIDEO_STREAM_ID = 60006;
#elif defined(DOL_FEATURE)
static const int32_t VIDEO_STREAM_ID = 60014;
#else
static const int32_t VIDEO_STREAM_ID = 60001;
#endif
// Stream id associated with still capture with gpu tnr.
static const int32_t STILL_TNR_STREAM_ID = 60009;

#ifdef DOL_FEATURE
static const int32_t STILL_STREAM_ID = 60015;
#else
// Stream id associated with still capture.
static const int32_t STILL_STREAM_ID = 60000;
#endif

#define MAX_RBM_STR_SIZE    128

enum StageType {
    STAGE_IPU = 0,
    STAGE_SW_POST,
    STAGE_GPU_TNR
};

struct PtzInfo {
    // The pos (percentage) of start point of crop region (located in sensor active array pixels)
    float x;
    float y;
    // The size of crop region, xSize + x <= 1.0, ySize + y <= 1.0
    float xSize;
    float ySize;

    float zoomRatio;
    bool zoomCentered;

    PtzInfo() : x(0.0), y(0.0), xSize(1.0), ySize(1.0), zoomRatio(1.0), zoomCentered(true) {}
};

namespace IGraphType {
class ConnectionConfig {
 public:
    ConnectionConfig(): mSourceStage(INVALID_PORT),
                        mSourceTerminal(INVALID_PORT),
                        mSourceIteration(0),
                        mSinkStage(INVALID_PORT),
                        mSinkTerminal(INVALID_PORT),
                        mSinkIteration(0),
                        mConnectionType(0) {}

    ConnectionConfig(uuid sourceStage,
                     uuid sourceTerminal,
                     uuid sourceIteration,
                     uuid sinkStage,
                     uuid sinkTerminal,
                     uuid sinkIteration,
                     int connectionType):
                         mSourceStage(sourceStage),
                         mSourceTerminal(sourceTerminal),
                         mSourceIteration(sourceIteration),
                         mSinkStage(sinkStage),
                         mSinkTerminal(sinkTerminal),
                         mSinkIteration(sinkIteration),
                         mConnectionType(connectionType) {}

    uuid mSourceStage;
    uuid mSourceTerminal;
    uuid mSourceIteration;
    uuid mSinkStage;
    uuid mSinkTerminal;
    uuid mSinkIteration;
    int mConnectionType;
};

/**
* \struct PortFormatSettings
* Format settings for a port in the graph
*/
struct PortFormatSettings {
    int32_t      enabled;
    uint32_t     terminalId; /**< Unique terminal id (is a fourcc code) */
    int32_t      width;    /**< Width of the frame in pixels */
    int32_t      height;   /**< Height of the frame in lines */
    int32_t      fourcc;   /**< Frame format */
    int32_t      format;   /**< v4l2 format */
    int32_t      bpl;      /**< Bytes per line*/
    int32_t      bpp;      /**< Bits per pixel */
};

/**
 * \struct PipelineConnection
 * Group port format, connection, stream, edge port for
 * pipeline configuration
 */
struct PipelineConnection {
    PipelineConnection() : stream(nullptr), hasEdgePort(false) { CLEAR(portFormatSettings); }
    PortFormatSettings portFormatSettings;
    ConnectionConfig connectionConfig;
    const HalStream *stream;
    bool hasEdgePort;
};

enum terminal_connection_type {
    connection_type_push, /* data is pushed by source stage execute */
    connection_type_pull  /* data is pulled by sink stage execute */
};

// Not support the below API in ipu7, keep them only for ipu7 build
struct StageAttr{
    char rbm[MAX_RBM_STR_SIZE];
    uint32_t rbm_bytes;
    StageAttr() : rbm_bytes(0) {}
};

struct ScalerInfo {
    int32_t streamId;
    float scalerWidth;
    float scalerHeight;
};

struct PrivPortFormat {
    int32_t streamId;
    PortFormatSettings formatSetting;
};
}  // namespace IGraphType

}  // icamera
