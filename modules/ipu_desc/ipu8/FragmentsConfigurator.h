/*
* INTEL CONFIDENTIAL
* Copyright (c) 2022 Intel Corporation
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
#include "GraphResolutionConfiguratorInclude.h"
#include <map>

class Ipu8FragmentsConfigurator
{
public:
    static const int32_t MIN_STRIPE_WIDTH_BEFORE_TNR = 128;
    static const int32_t MIN_STRIPE_WIDTH_AFTER_TNR = 64;
    static const int32_t UPSCALER_MAX_OUTPUT_WIDTH = 4672;
    Ipu8FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node);

    StaticGraphStatus configureFragments(std::vector<SmurfKernelInfo*>& smurfKernels);

private:
    // Stripe Actions - each filter will perform one action according to its role
    StaticGraphStatus configFragmentsDownscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsCropper(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsUpscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsOutput(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool isTnr);
    StaticGraphStatus configFragmentsTnrScaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsTnrFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole);
    StaticGraphStatus configFragmentsSmurf(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, StaticGraphFragmentDesc* prevKernelFragments,
        std::vector<SmurfKernelInfo*>& smurfKernels);
    StaticGraphStatus configFragmentsSmurfFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments);

    StaticGraphStatus copyFragments(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* prevKernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* kernelFragments);
    void vanishStripe(uint8_t stripe, uint32_t runKerenlUuid, StaticGraphFragmentDesc* kernelFragments, VanishOption vanishOption);
    uint32_t getPlaneStartAddress(uint32_t sumOfPrevWidths, FormatType formatType, uint8_t plane);
    uint16_t alignToFormatRestrictions(uint16_t size, FormatType bufferFormat);
    bool validateDownscalerOutputWidth(StaticGraphFragmentDesc* stripe, uint16_t addition, int32_t stripeIndex, double scaleFactor, StaticGraphRunKernel* runKernel);
    uint32_t calculateGcd(uint32_t a, uint32_t b);

    OuterNode* _node = nullptr;
    IStaticGraphConfig* _staticGraph = nullptr;

    // Fragments binaries do not contain output start x, so we keep them here
    std::map<uint32_t, std::vector<uint16_t>> _outputStartX;

    // Save TNR resolutions for feeder configurations
    StaticGraphFragmentDesc* _tnrScalerFragments = nullptr;
    StaticGraphRunKernel* _tnrScalerRunKernel = nullptr;
};
