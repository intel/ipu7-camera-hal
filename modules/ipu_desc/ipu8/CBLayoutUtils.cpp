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

#define LOG_TAG CBLayoutUtils

#include "CBLayoutUtils.h"

#include <ia_pal_types_isp_ids_autogen.h>
#include <set>

#include "lbff_ids_array.h"
#include "ipu_manifest_db_ipu8_psys_cb_lbff_dev_ids.h"
#include "ipu_manifest_db_ipu8_psys_cb_lbff_descriptors.h"

namespace icamera {
namespace CBLayoutUtils {

static const std::set<std::pair<uint8_t, uint8_t>> s3AStatsTerminalSet = {
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AE_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AF_STD_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_STD_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_SAT_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_PDAF_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_SVE_OUTPUT),
};

static const std::set<std::pair<uint8_t, uint8_t>> sMetaDataTerminalSet = {
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_LSC_INPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AE_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AF_STD_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_STD_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_SAT_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_PDAF_OUTPUT),
    std::make_pair<uint8_t, uint8_t>(NODE_RESOURCE_ID_LBFF, LBFF_TERMINAL_CONNECT_AWB_SVE_OUTPUT),
};

// See ia_statistics_buffer_type in imaging-control/include/ia_types.h
#define NUM_STATISTICS_BUFFER_TYPES 8

static const int32_t kStatsBufToTermIds[NUM_STATISTICS_BUFFER_TYPES] =
{
    LBFF_TERMINAL_CONNECT_AWB_STD_OUTPUT,   // rgbs_std
    LBFF_TERMINAL_CONNECT_AWB_SVE_OUTPUT,   // rgbs_sve
    LBFF_TERMINAL_CONNECT_AWB_SAT_OUTPUT,   // rgbs_sat
    LBFF_TERMINAL_CONNECT_AE_OUTPUT,        // ae_histogram
    LBFF_TERMINAL_CONNECT_AF_STD_OUTPUT,    // af_std
    -1,                                     // af_sve (not supported)
    LBFF_TERMINAL_CONNECT_PDAF_OUTPUT,      // pdaf_out
    LBFF_TERMINAL_CONNECT_GMV_MATCH_OUTPUT  // gmv_match_out
};

bool isMetaDataTerminal(uint8_t resourceId, uint8_t terminalId) {
    if (sMetaDataTerminalSet.find(std::make_pair(resourceId, terminalId)) !=
        sMetaDataTerminalSet.end()) {
        return true;
    }

    return false;
}

bool is3AStatsTerminal(uint8_t resourceId, uint8_t terminalId) {
    if (s3AStatsTerminalSet.find(std::make_pair(resourceId, terminalId)) !=
        s3AStatsTerminalSet.end()) {
        return true;
    }

    return false;
}

bool isFrameTerminal(uint8_t resourceId, uint8_t terminalId) {
    const TerminalDescriptor* table = nullptr;
    uint32_t count = 0;
    getCbTerminalDescriptors(resourceId, table, count);
    if (!table) return false;
    for (uint32_t i = 0; i < count; i++) {
        if (terminalId == table[i].TerminalId) {
            return table[i].TerminalBufferType == TERMINAL_BUFFER_TYPE_DATA;
        }
    }

    return false;
}

int32_t getKernelForDataTerminal(uint8_t resourceId, uint8_t terminalId) {
    const TerminalDescriptor* table = nullptr;
    uint32_t count = 0;
    getCbTerminalDescriptors(resourceId, table, count);
    if (!table) return 0;
    for (uint32_t i = 0; i < count; i++) {
        if (terminalId == table[i].TerminalId) {
            return table[i].TerminalLinkedKernel;
        }
    }
    return 0;
}

status_t getCbTerminalDescriptors(uint8_t resourceId,
                                  const TerminalDescriptor *&descriptors, uint32_t &count) {
    status_t ret = OK;

    switch (resourceId) {
        case NODE_RESOURCE_ID_LBFF:
            descriptors = LBFFTerminalDesc;
            count = CountOfLBFFTerminalDesc;
            break;
        default:
            ret = INVALID_OPERATION;
            descriptors = nullptr;
            count = 0;
            break;
    }

    return ret;
}

const TerminalDescriptor *getTerminalDescriptor(uint8_t resourceId, uint8_t terminalId) {
    status_t ret = OK;

    const TerminalDescriptor *descriptors = nullptr;
    const TerminalDescriptor *descriptor = nullptr;
    uint32_t count = 0;

    ret = getCbTerminalDescriptors(resourceId, descriptors, count);
    if (ret != OK) return nullptr;

    if (terminalId < count) {
        descriptor = &descriptors[terminalId];
    }

    return descriptor;
}

PacBufferType getTerminalPacBufferType(uint8_t resourceId, uint8_t terminalId) {
    const TerminalDescriptor *descriptor = getTerminalDescriptor(resourceId, terminalId);
    if (descriptor) {
        return descriptor->PacBufferType;
    }

    return PAC_BUFFER_TYPE_NONE;
}

static const payload_descriptor_t *sLBCBPayloadDescriptors[] = {
    &lbff_0_descriptors,    // TERMINAL_LOAD_ALGO_CACHED
    &lbff_1_descriptors,    // TERMINAL_LOAD_ALGO_FRAG_SEQ
    &lbff_2_descriptors,    // TERMINAL_LOAD_SYSTEM
    &lbff_3_descriptors,    // DOL, TNR and CAS
};

status_t getCbPayloadDescriptor(uint8_t resourceId,
                                const payload_descriptor_t ***cbPayloadDescriptor,
                                uint32_t &count) {
    status_t ret = OK;

    switch (resourceId) {
        case NODE_RESOURCE_ID_LBFF:
            *cbPayloadDescriptor = sLBCBPayloadDescriptors;
            count = sizeof(sLBCBPayloadDescriptors) / sizeof(sLBCBPayloadDescriptors[0]);
            break;
        default:
            *cbPayloadDescriptor = nullptr;
            count = 0;
            ret = INVALID_OPERATION;
            break;
    }

    return ret;
}

int cbDeviceId2Uuid(uint8_t resourceId, uint32_t deviceId) {
    int uuid = -1;
    switch (resourceId) {
        case NODE_RESOURCE_ID_LBFF:
            uuid = lbff_id_to_uuid_tag[deviceId];
            break;
        default:
            break;
    }

    return uuid;
}

const int32_t* getStatsBufToTermIds() {
    return kStatsBufToTermIds;
}

uint32_t getIspIfdKernelId() {
    return ia_pal_uuid_isp_ifd_pipe_1_4;
}

}  // namespace CBLayoutUtils
}  // namespace icamera
