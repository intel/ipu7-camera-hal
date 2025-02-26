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
#pragma once
#include <vector>
#include "Ipu7xStaticGraphAutogen.h"
#include "Ipu7xStaticGraphTypesAutogen.h"
#include "GraphResolutionConfigurator.h"

class GraphResolutionConfiguratorHelper
{
public:
    GraphResolutionConfiguratorHelper();

    static uint32_t getRunKernelUuid(GraphResolutionConfiguratorKernelRole role);
    static uint32_t getRunKernelUuidOfOutput(HwSink hwSink, int32_t graphId, GraphLink** links);
    static StaticGraphStatus getRunKernelUuidForResHistoryUpdate(std::vector<uint32_t>& kernelUuids);
    static uint32_t getRunKernelIoBufferSystemApiUuid();
};
