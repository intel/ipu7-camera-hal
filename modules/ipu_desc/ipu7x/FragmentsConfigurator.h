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

class Gen2FragmentsConfigurator
{
public:
    static const int32_t MIN_STRIPE_WIDTH_BEFORE_TNR = 128;
    static const int32_t MIN_STRIPE_WIDTH_AFTER_TNR = 64;
    // b2i_ds HW/PAL minimum non-vanished stripe output width (matches Resolutionary validation).
    static const int32_t B2I_DS_MIN_STRIPE_OUTPUT_WIDTH = 64;
    static const int32_t UPSCALER_MAX_OUTPUT_WIDTH = 4672;

    Gen2FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments);
    virtual ~Gen2FragmentsConfigurator() = default;

    StaticGraphStatus configureFragments(std::vector<SmurfKernelInfo*>& smurfKernels);

    // True if the last configureFragments() failed due to an illegal (< 64px) b2i_ds stripe
    // output width, as opposed to some other failure.
    bool hadStripeWidthViolation() const { return _hadStripeWidthViolation; }

protected:
    virtual bool enforceUpscalerAspectRatioConstraints() const { return true; }

    StaticGraphStatus configFragmentsDownscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool isOutputScaler);
    StaticGraphStatus configFragmentsCropper(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool beforeTnr);
    StaticGraphStatus configFragmentsUpscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsOutput(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool isTnr);
    StaticGraphStatus configFragmentsTnrScaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments);
    StaticGraphStatus configFragmentsTnrFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole);
    virtual StaticGraphStatus configFragmentsTnrMcFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole) {
        (void)runKernel;
        (void)kernelFragments;
        (void)kernelRole;
        return StaticGraphStatus::SG_OK;
    }
    StaticGraphStatus configFragmentsSmurf(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, StaticGraphFragmentDesc* prevKernelFragments,
        std::vector<SmurfKernelInfo*>& smurfKernels);
    StaticGraphStatus configFragmentsSmurfFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments);

    StaticGraphStatus copyFragments(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* prevKernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* kernelFragments);
    void vanishStripe(uint8_t stripe, uint32_t runKerenlUuid, StaticGraphFragmentDesc* kernelFragments, VanishOption vanishOption);
    uint32_t getPlaneStartAddress(uint32_t sumOfPrevWidths, FormatType formatType, uint8_t plane);
    uint16_t alignToFormatRestrictions(uint16_t size, FormatType bufferFormat);
    bool validateDownscalerConstraints(StaticGraphFragmentDesc* stripe, int32_t stripeIndex, double scaleFactor, StaticGraphRunKernel* runKernel);
    uint32_t calculateGcd(uint32_t a, uint32_t b);

    OuterNode* _node = nullptr;
    IStaticGraphConfig* _staticGraph = nullptr;
    uint8_t _numberOfFragments = 0;
    bool _hadStripeWidthViolation = false;

    std::map<uint32_t, std::vector<uint16_t>> _outputStartX;

    StaticGraphFragmentDesc* _tnrScalerFragments = nullptr;
    StaticGraphRunKernel* _tnrScalerRunKernel = nullptr;
    StaticGraphRunKernel* _mcFullRefRunKernel = nullptr;
    StaticGraphFragmentDesc* _mcFullRefFragments = nullptr;
    StaticGraphRunKernel* _mcSmallRefRunKernel = nullptr;
    StaticGraphFragmentDesc* _mcSmallRefFragments = nullptr;
    StaticGraphRunKernel* _mcFullRunKernel = nullptr;
};

class Ipu8FragmentsConfigurator : public Gen2FragmentsConfigurator
{
public:
    Ipu8FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments);
};

class Ipu9FragmentsConfigurator : public Gen2FragmentsConfigurator
{
public:
    Ipu9FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments);

protected:
    bool enforceUpscalerAspectRatioConstraints() const override { return false; }
    StaticGraphStatus configFragmentsTnrMcFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole);
};

