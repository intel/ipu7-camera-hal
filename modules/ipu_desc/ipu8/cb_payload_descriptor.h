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

typedef struct _cb_payload_descriptor_t
{
    // from <CB>_dev_ids.h
    uint32_t device_id;

    // Offset from Base address of the FF. Configuration should start from the HW register at this offset
    uint32_t offset_in_device;

    // Size of the section in bytes
    uint16_t payload_size;

    // Section start offset from location of terminal payload in DDR
    uint16_t offset_in_payload;
} cb_payload_descriptor_t;

typedef struct payload_descriptor_s
{
    uint32_t number_of_sections;
    cb_payload_descriptor_t sections[128];
} payload_descriptor_t;
