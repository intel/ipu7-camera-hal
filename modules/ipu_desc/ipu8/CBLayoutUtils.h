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

#include "iutils/Errors.h"
#include "cb_payload_descriptor.h"
#if defined(GRC_IPU7X)
#include "Ipu7xTerminalDescriptorAutogen.h"
#elif defined(GRC_IPU75XA)
#include "Ipu75xaTerminalDescriptorAutogen.h"
#elif defined(GRC_IPU8)
#include "Ipu8TerminalDescriptorAutogen.h"
#else
#include "TerminalDescriptorAutogen.h"
#endif

namespace icamera {

typedef enum _NodeResourceIdType
{
    NODE_RESOURCE_ID_LBFF = 0,
    NODE_RESOURCE_ID_ISYS = 1
} NodeResourceIdType;

namespace CBLayoutUtils {

status_t getCbTerminalDescriptors(uint8_t resourceId,
                                  const TerminalDescriptor *&descriptors, uint32_t &count);
const TerminalDescriptor *getTerminalDescriptor(uint8_t resourceId, uint8_t terminalId);
PacBufferType getTerminalPacBufferType(uint8_t resourceId, uint8_t terminalId);
bool isMetaDataTerminal(uint8_t resourceId, uint8_t terminalId);
bool is3AStatsTerminal(uint8_t resourceId, uint8_t terminalId);
bool isFrameTerminal(uint8_t resourceId, uint8_t terminalId);
int32_t getKernelForDataTerminal(uint8_t resourceId, uint8_t terminalId);

status_t getCbPayloadDescriptor(uint8_t resourceId,
                                const payload_descriptor_t ***cbPayloadDescriptor,
                                uint32_t &count);
int cbDeviceId2Uuid(uint8_t resourceId, uint32_t deviceId);

const int32_t* getStatsBufToTermIds();

uint32_t getIspIfdKernelId();
}  // namespace CBLayoutUtils
}  // namespace icamera
