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

#include "FragmentsConfigurator.h"
#include <math.h>
#include <cmath>

Gen2FragmentsConfigurator::Gen2FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments) :
    _staticGraph(staticGraph), _node(node), _numberOfFragments(numberOfFragments)
{
}

StaticGraphStatus Gen2FragmentsConfigurator::configureFragments(std::vector<SmurfKernelInfo*>& smurfKernels)
{
    _hadStripeWidthViolation = false;

    if (_staticGraph == nullptr || _node == nullptr || _numberOfFragments < 1)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    // Reset status
    for (int32_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        _node->fragmentVanishStatus[stripe] = VanishOption::Full;
    }

    StaticGraphStatus res = StaticGraphStatus::SG_OK;

    const uint16_t* kenelConfigOrder = _node->getRunKernelConfigOrder();

    for (uint16_t i = 0; i < _node->nodeKernels.kernelCount; i++)
    {
        uint16_t j = kenelConfigOrder[i];
        StaticGraphRunKernel* runKernel = &_node->nodeKernels.kernelList[j].run_kernel;

        StaticGraphFragmentDesc* kernelFragments = _node->nodeKernels.kernelList[j].fragment_descs;
        // Take previous kernel as reference, unless we will change it below.
        StaticGraphFragmentDesc* prevKernelFragments = j == 0 ? nullptr : _node->nodeKernels.kernelList[j - 1].fragment_descs;
        uint32_t prevKernelUuid = j == 0 ? 0 : _node->nodeKernels.kernelList[j - 1].run_kernel.kernel_uuid;

        int32_t additionalFeaturesBit = 0;
        _staticGraph->getAdditionalFeaturesBit(&additionalFeaturesBit);
        uint32_t referenceKernel = GraphResolutionConfiguratorHelper::getReferenceKernel(runKernel->kernel_uuid, additionalFeaturesBit);

        if (referenceKernel != 0)
        {
            // Special reference kernel
            for (uint32_t k = 0; k < _node->nodeKernels.kernelCount; k++)
            {
                if (_node->nodeKernels.kernelList[k].run_kernel.kernel_uuid == referenceKernel)
                {
                    prevKernelFragments = _node->nodeKernels.kernelList[k].fragment_descs;
                    prevKernelUuid = referenceKernel;
                    break;
                }
            }
        }

        //  Find the handling function for this kernel
        GraphResolutionConfiguratorKernelRole kernelRole = GraphResolutionConfiguratorHelper::getKernelRole(runKernel->kernel_uuid);

        switch (kernelRole)
        {
            case GraphResolutionConfiguratorKernelRole::DownScaler:
            case GraphResolutionConfiguratorKernelRole::DownScalerSmall:
            {
                res = configFragmentsDownscaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, false);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::DownScalerOutput:
            {
                res = configFragmentsDownscaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, true);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::EspaCropper:
            case GraphResolutionConfiguratorKernelRole::EspaCropperSmall:
            {
                res = configFragmentsCropper(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, true);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::CasEspaCropper:
            {
                res = configFragmentsCropper(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, false);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::UpScaler:
            {
                res = configFragmentsUpscaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::Output:
            {
                res = configFragmentsOutput(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, false);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::TnrOutput:
            {
                res = configFragmentsOutput(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments, true);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::TnrScaler:
            {
                res = configFragmentsTnrScaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }
            case GraphResolutionConfiguratorKernelRole::McFull:
            case GraphResolutionConfiguratorKernelRole::McSmall:
                res = configFragmentsTnrMcFeeder(runKernel, kernelFragments, kernelRole);
                break;
            case GraphResolutionConfiguratorKernelRole::TnrFeederFull:
            case GraphResolutionConfiguratorKernelRole::TnrFeederSmall:
            case GraphResolutionConfiguratorKernelRole::McFeederFull:
            case GraphResolutionConfiguratorKernelRole::McFeederSmall:
            {
                res = configFragmentsTnrFeeder(runKernel, kernelFragments, kernelRole);
                break;
            }
            case GraphResolutionConfiguratorKernelRole::Smurf:
            {
                res = configFragmentsSmurf(runKernel, kernelFragments, prevKernelFragments, smurfKernels);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::SmurfFeeder:
            {
                res = configFragmentsSmurfFeeder(runKernel, kernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::NonRcb:
            {
                // Before zoom kernels - take prev kernel fragments as-is
                res = copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);
                break;
            }

            default:
            {
                // No action for other kernels
                break;
            }
        }

        if (res != StaticGraphStatus::SG_OK)
        {
            return res;
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsDownscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool isOutputScaler)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    // Since this code is for output scaler as well we may have vanish stripes
    int16_t leftNonVanishedStripe = 0;
    int16_t rightNonVanishedStripe = _numberOfFragments - 1;

    for (int16_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if (isOutputScaler && _node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            // Vanished
            continue;
        }

        // Not vanished
        leftNonVanishedStripe = stripe;
        break;
    }

    for (int16_t stripe = _numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if (isOutputScaler && _node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            // Vanished
            continue;
        }

        // Not vanished
        rightNonVanishedStripe = stripe;
        break;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactorW = static_cast<double>(resInfo->output_width) / (resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right);
    auto scaleFactorH = static_cast<double>(resInfo->output_height) / (resInfo->input_height - resInfo->input_crop.top - resInfo->input_crop.bottom);
    auto scaleFactor = std::max(scaleFactorW, scaleFactorH);

    for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        if (scaleFactor == 1.0)
        {
            kernelFragments[stripe].fragmentOutputWidth = kernelFragments[stripe].fragmentInputWidth;
            _outputStartX[runKernel->kernel_uuid][stripe] = kernelFragments[stripe].fragmentStartX;
            continue;
        }

        int rightCrop = stripe == static_cast<int32_t>(_numberOfFragments - 1) ? resInfo->input_crop.right : 0;

        double value = (static_cast<double>(kernelFragments[stripe].fragmentInputWidth - rightCrop) * scaleFactor) / 4;
        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(floor(value)) *  4;

        // Start of output is rounded up since this is what b2i_ds does (Creates pixels starting from the pixel after)
        value = (scaleFactor * kernelFragments[stripe].fragmentStartX) / 2;
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(ceil(value)) * 2;

        // Check if pixels are missing in the last stripe
        if (stripe == rightNonVanishedStripe)
        {
            if (_outputStartX[runKernel->kernel_uuid][stripe] + kernelFragments[stripe].fragmentOutputWidth < resInfo->output_width)
            {
                kernelFragments[stripe].fragmentOutputWidth += 4;
            }
        }
    }

    // Check if we need to adjust the scale factor a little in order to meet b2i_ds constraints
    const double ratio_prec = 1U << GraphResolutionConfigurator::SCALE_PREC;
    int scaling_ratio = (int)ceil((1 / scaleFactor) * (double)(1U << GraphResolutionConfigurator::SCALE_PREC));
    const double scaling_ratio_f = static_cast<double>(scaling_ratio) / ratio_prec;
    double adjusted_scaling_ratio_f = scaling_ratio_f;

    for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        const uint16_t fragment_start_x = kernelFragments[stripe].fragmentStartX;
        const int32_t fragment_input_width = kernelFragments[stripe].fragmentInputWidth;
        const int32_t fragment_output_width = kernelFragments[stripe].fragmentOutputWidth;

        int32_t offset_common = ((scaling_ratio - (static_cast<uint32_t>(1U) << GraphResolutionConfigurator::SCALE_PREC)) >> 1);
        uint32_t orig_horizontal_offset = (runKernel->resolution_info->input_crop.left << GraphResolutionConfigurator::SCALE_PREC) + offset_common;

        const double calc = ceil((static_cast<double>(fragment_start_x) / scaling_ratio_f / 2.0)) * 2.0; //2 * ceil(x/2) means round up to a closest even number
        double horizontal_offset = (static_cast<double>(orig_horizontal_offset) / ratio_prec) + static_cast<double>(scaling_ratio_f * calc - static_cast<double>(fragment_start_x));
        int32_t horizontal_offset_fxp = static_cast<int32_t>(floor(horizontal_offset * ratio_prec));

        int32_t horizontal_offset_max = fragment_input_width * (1 << GraphResolutionConfigurator::SCALE_PREC) + (int32_t)(scaling_ratio * (1 + 1.0 / 128) - fragment_output_width * scaling_ratio);

        if (horizontal_offset_fxp > horizontal_offset_max)
        {
            double s_factor_f = scaling_ratio_f;

            while (horizontal_offset_fxp > horizontal_offset_max)
            {
                s_factor_f = (fragment_input_width + 0.5 + static_cast<double>(fragment_start_x)) /
                    (0.5 + fragment_output_width + 2.0 * ceil((static_cast<double>(fragment_start_x) / s_factor_f / 2.0)) - (1.0 + 1.0 / 128));

                // floor the value of s_factor according to the precision of 2^16
                s_factor_f = floor(s_factor_f * ratio_prec) / ratio_prec;

                // Update horizontal_offset_fxp and horizontal_offset_max
                double horizontal_offset_f = (s_factor_f - 1.0) / 2.0 + (2.0 * s_factor_f * ceil((static_cast<double>(fragment_start_x) / s_factor_f / 2.0)) - fragment_start_x);
                double horizontal_offset_max_f = fragment_input_width - fragment_output_width * s_factor_f + s_factor_f * (1.0 + 1.0 / 128);

                horizontal_offset_fxp = static_cast<int32_t>(floor(horizontal_offset_f * ratio_prec));
                horizontal_offset_max = static_cast<int32_t>(floor(horizontal_offset_max_f * ratio_prec));
            }

            adjusted_scaling_ratio_f = std::min(adjusted_scaling_ratio_f, s_factor_f);
        }
    }

    if (runKernel->enable == 1)
    {
        for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
        {
            if (kernelFragments[stripe].fragmentOutputWidth < B2I_DS_MIN_STRIPE_OUTPUT_WIDTH)
            {
                _hadStripeWidthViolation = true;
                return StaticGraphStatus::SG_ERROR;
            }

            if (!validateDownscalerConstraints(&(kernelFragments[stripe]), stripe, (1 / adjusted_scaling_ratio_f), runKernel))
            {
                return StaticGraphStatus::SG_ERROR;
            }
        }
    }

    // Now update output start according to new scale factor
    for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        // Start of output is rounded up since this is what b2i_ds does (Creates pixels starting from the pixel after)
        double value = kernelFragments[stripe].fragmentStartX / adjusted_scaling_ratio_f / 2;
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(ceil(value)) * 2;
    }

    // Set the adjusted factor for PAL
	int32_t adjusted_scaling_ratio = static_cast<int32_t>(adjusted_scaling_ratio_f * ratio_prec);

    auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
    if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelDownscalerSystemApiUuid())
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }

    StaticGraphKernelSystemApiB2iDs* systemApi = reinterpret_cast<StaticGraphKernelSystemApiB2iDs*>
        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

    systemApi->scaling_ratio = adjusted_scaling_ratio;

    return StaticGraphStatus::SG_OK;
}

bool Gen2FragmentsConfigurator::validateDownscalerConstraints(StaticGraphFragmentDesc* stripe, int32_t stripeIndex, double scaleFactor, StaticGraphRunKernel* runKernel)
{
    int scaling_ratio = (int)ceil((1 / scaleFactor) * (double)(1U << GraphResolutionConfigurator::SCALE_PREC));

    int32_t offset_common = ((scaling_ratio - (static_cast<uint32_t>(1U) << GraphResolutionConfigurator::SCALE_PREC)) >> 1);

    uint32_t orig_horizontal_offset;
    if (stripeIndex == 0)
    {
        orig_horizontal_offset = (runKernel->resolution_info->input_crop.left << GraphResolutionConfigurator::SCALE_PREC) + offset_common;
    }
    else
    {
        orig_horizontal_offset = offset_common;
    }

    // uncomment these lines in order to check if constraints are actually met.
    //
    // const double ratio_prec = 1U << GraphResolutionConfigurator::SCALE_PREC;
    // const double scaling_ratio_f = static_cast<double>(scaling_ratio) / ratio_prec;
    // const uint16_t fragment_start_x = stripe->fragmentStartX;
    // const double calc = ceil((static_cast<double>(fragment_start_x) / scaling_ratio_f / 2.0)) * 2.0; //2 * ceil(x/2) means round up to a closest even number
    // double horizontal_offset = (static_cast<double>(orig_horizontal_offset) / ratio_prec) + static_cast<double>(scaling_ratio_f * calc - static_cast<double>(fragment_start_x));
    // int32_t horizontal_offset_fxp = static_cast<int32_t>(floor(horizontal_offset * ratio_prec));

    const int32_t fragment_input_width = stripe->fragmentInputWidth;
    const int32_t fragment_output_width = stripe->fragmentOutputWidth;

    //1
    int32_t horizontal_offset_min = 0;
    int32_t horizontal_offset_max = ((5 * scaling_ratio) - (1U << GraphResolutionConfigurator::SCALE_PREC)) / 2;

    //2
    horizontal_offset_min = std::max((58982 * fragment_input_width) - (fragment_output_width * scaling_ratio), horizontal_offset_min);
    horizontal_offset_max = std::min((72089 * fragment_input_width) - (fragment_output_width * scaling_ratio), horizontal_offset_max);

    //3
    horizontal_offset_max = std::min(fragment_input_width * (1 << GraphResolutionConfigurator::SCALE_PREC) + (int32_t)(scaling_ratio * (1 + 1.0 / 128) - fragment_output_width * scaling_ratio), horizontal_offset_max);

    //return (horizontal_offset_fxp >= horizontal_offset_min && horizontal_offset_fxp <= horizontal_offset_max);

    // return true if constraints can be met. (PAL will chomp)
    return (horizontal_offset_min <= horizontal_offset_max);
}

void Gen2FragmentsConfigurator::vanishStripe(uint8_t stripe, uint32_t runKerenlUuid, StaticGraphFragmentDesc* kernelFragments, VanishOption vanishOption)
{
    _node->fragmentVanishStatus[stripe] = vanishOption;
    kernelFragments[stripe] = {};
    _outputStartX[runKerenlUuid][stripe] = 0;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsCropper(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool beforeTnr)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    // prev kernel is the downscaler
    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    // No cropping in DS, cropping is done by ESPA cropper

    int32_t leftPixel = runKernel->resolution_info->input_crop.left;
    int32_t rightPixel = static_cast<uint16_t>(runKernel->resolution_info->input_width - runKernel->resolution_info->input_crop.right);

    int32_t leftNonVanishedStripe = 0;
    int32_t rightNonVanishedStripe = _numberOfFragments - 1;

    std::vector<uint32_t> xOffset(_numberOfFragments, 0);

    if (beforeTnr)
    {
        for (int8_t stripe = 0; stripe < _numberOfFragments; stripe++)
        {
            if (leftPixel + MIN_STRIPE_WIDTH_BEFORE_TNR >= kernelFragments[stripe].fragmentStartX + kernelFragments[stripe].fragmentInputWidth)
            {
                // This stripe is cropped out, vanish it!
                // Note that we set output width to 0 for ESPA cropper and forward. But Stripe vanishes much eairlier in pipe, and these infos are not updated.
                vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterStats);
                continue;
            }

            // Not vanished
            leftNonVanishedStripe = stripe;
            break;
        }

        for (uint8_t stripe = _numberOfFragments - 1; stripe >= 0; stripe--)
        {
            if (rightPixel <= kernelFragments[stripe].fragmentStartX + MIN_STRIPE_WIDTH_BEFORE_TNR)
            {
                // This stripe is cropped out, vanish it!
                // Note that we set output width to 0 for ESPA cropper and forward. But Stripe vanishes much eairlier in pipe, and these infos are not updated.
                vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterStats);
                continue;
            }

            // Not vanished
            rightNonVanishedStripe = stripe;
            break;
        }
    }

    // The CAS ESPA cropper (beforeTnr == false) acts like the ESPA cropper, but it never vanishes
    // stripes on its own, and its output must align to a 4-pixel (not 8-pixel) granularity.
    uint16_t granularityAfter = beforeTnr ? 8 : 4;

    for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        if (!beforeTnr && _node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            // Already vanished by an earlier stage; nothing to do for this stripe.
            continue;
        }

        int32_t leftCrop = runKernel->resolution_info->input_crop.left > kernelFragments[stripe].fragmentStartX ?
            runKernel->resolution_info->input_crop.left - kernelFragments[stripe].fragmentStartX : 0;
        int32_t rightCrop = runKernel->resolution_info->input_crop.right > (runKernel->resolution_info->input_width - kernelFragments[stripe].fragmentStartX - kernelFragments[stripe].fragmentInputWidth) ?
            runKernel->resolution_info->input_crop.right - (runKernel->resolution_info->input_width - kernelFragments[stripe].fragmentStartX - kernelFragments[stripe].fragmentInputWidth) : 0;

        // Save for sys api
        xOffset[stripe] = static_cast<uint32_t>(leftCrop);

        // ESPA crop is after the down scaling and it must output resolution that divides by 8 for tnr scalers.
        int32_t stripeZoomCrop = leftCrop + rightCrop;

        int outputWidth = (int)kernelFragments[stripe].fragmentOutputWidth - stripeZoomCrop;
        if (outputWidth < 0)
        {
            return StaticGraphStatus::SG_ERROR;
        }

        // For start point, we need to remove the left cropping only for stripes 1 and on
        uint16_t outputStartX = static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX > runKernel->resolution_info->input_crop.left) ?
            static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX - runKernel->resolution_info->input_crop.left) : 0;

        // Start X must be % granularity, so that 1:4 resolutions align exactly to 1:1
        if (outputStartX % granularityAfter != 0)
        {
            uint16_t pixelsToAdd = granularityAfter - outputStartX % granularityAfter;

            // Start a little later, affects also output width, will be fixed below
            outputStartX += pixelsToAdd;
            outputWidth -= pixelsToAdd;
            xOffset[stripe] += pixelsToAdd;
        }

        if (outputWidth % granularityAfter != 0)
        {
            uint16_t pixelsToCrop = outputWidth % granularityAfter;

            // Additional crop on the right, affects only output width
            outputWidth -= pixelsToCrop;

            if (stripe == rightNonVanishedStripe)
            {
                // Last stripe - crop from left
                outputStartX += pixelsToCrop;
                xOffset[stripe] += pixelsToCrop;
            }
        }

        _outputStartX[runKernel->kernel_uuid][stripe] = outputStartX;
        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(outputWidth);
    }

    // Update system API offsets
    // If actually running with 1 stripe - do not update the system API
    if (_node->GetNumberOfFragments() > 1)
    {
        if (runKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer))))
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
        if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        StaticGraphKernelSystemApiIoBuffer* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer*>
            (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

        for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
        {
            systemApi->x_output_offset_per_stripe[stripe] = xOffset[stripe];
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsUpscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
{
    if (kernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    if (runKernel->resolution_info->input_width == runKernel->resolution_info->output_width &&
        runKernel->resolution_info->input_height == runKernel->resolution_info->output_height &&
        runKernel->resolution_info->input_crop.left == 0 &&
        runKernel->resolution_info->input_crop.right == 0 &&
        runKernel->resolution_info->input_crop.top == 0 &&
        runKernel->resolution_info->input_crop.bottom == 0)
    {
        // Upscaler bypassed
        return StaticGraphStatus::SG_OK;
    }

    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactorW = static_cast<double>(resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right) / resInfo->output_width;
    auto scaleFactorH = static_cast<double>(resInfo->input_height - resInfo->input_crop.top - resInfo->input_crop.bottom) / resInfo->output_height;
    auto scaleFactor = std::min(scaleFactorW, scaleFactorH);
    auto scaleFactorFixed = static_cast<double>(static_cast<int32_t>(scaleFactor * static_cast<double>(1 << 16))) / static_cast<double>(1 << 16);

    uint32_t upscalerWidthGranularity = 2;
    uint16_t inputUnits = 1;

    if (enforceUpscalerAspectRatioConstraints())
    {
        int32_t croppedInputWidth = resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right;
        int32_t outputWidth = resInfo->output_width;

        // We would like to keep upscalerWidthGranularity as large as possible in order to minimize the number of pixels that cannot be used for upscaling
        // (upscalerWidthGranularity is divided to stripes, so the larger it is the more accurately we can divide)
        // Find the largest granularity that divides both
        upscalerWidthGranularity = calculateGcd(static_cast<uint32_t>(croppedInputWidth), static_cast<uint32_t>(outputWidth));
        inputUnits = static_cast<uint16_t>(croppedInputWidth / upscalerWidthGranularity);
    }

    int32_t leftPixel = runKernel->resolution_info->input_crop.left;
    int32_t rightPixel = static_cast<uint16_t>(runKernel->resolution_info->input_width - runKernel->resolution_info->input_crop.right);

    uint8_t leftNonVanishedStripe = 0;
    uint8_t rightNonVanishedStripe = _numberOfFragments - 1;

    for (int8_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            continue;
        }

        if (leftPixel + MIN_STRIPE_WIDTH_AFTER_TNR >= kernelFragments[stripe].fragmentStartX + kernelFragments[stripe].fragmentInputWidth)
        {
            // This stripe is cropped out, vanish it!
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterTnr);
            continue;
        }

        // Not vanished
        leftNonVanishedStripe = stripe;
        break;
    }

    for (uint8_t stripe = _numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if (_node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            continue;
        }

        if (rightPixel <= kernelFragments[stripe].fragmentStartX + MIN_STRIPE_WIDTH_AFTER_TNR)
        {
            // This stripe is cropped out, vanish it!
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterTnr);
            continue;
        }
        // Not vanished
        rightNonVanishedStripe = stripe;
        break;
    }

    for (uint8_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        int32_t leftCrop = resInfo->input_crop.left > kernelFragments[stripe].fragmentStartX ?
            resInfo->input_crop.left - kernelFragments[stripe].fragmentStartX : 0;
        int32_t rightCrop = resInfo->input_crop.right > (resInfo->input_width - kernelFragments[stripe].fragmentStartX - kernelFragments[stripe].fragmentInputWidth) ?
            resInfo->input_crop.right - (resInfo->input_width - kernelFragments[stripe].fragmentStartX - kernelFragments[stripe].fragmentInputWidth) : 0;

        int32_t stripeZoomCrop = leftCrop + rightCrop;

        // Calculate the step, proportional to the part of input to upscaler that this stripe is working on
        uint16_t inputWidthAfterZoomCrop = static_cast<uint16_t>(kernelFragments[stripe].fragmentInputWidth - stripeZoomCrop);

        uint16_t pixelsToCrop = 0;
        uint16_t maxInputWidth = static_cast<uint16_t>(UPSCALER_MAX_OUTPUT_WIDTH * scaleFactorFixed);
        if (inputWidthAfterZoomCrop > maxInputWidth)
        {
            pixelsToCrop = inputWidthAfterZoomCrop - maxInputWidth;
            inputWidthAfterZoomCrop = maxInputWidth;
        }

        uint16_t inputWidthAfterTotalCrop = inputWidthAfterZoomCrop;

        if (enforceUpscalerAspectRatioConstraints())
        {
            uint16_t stripeStepW = GRA_ROUND_DOWN(static_cast<uint16_t>(static_cast<double>(inputWidthAfterZoomCrop) / (resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right) * upscalerWidthGranularity), 2);
            inputWidthAfterTotalCrop = stripeStepW * inputUnits;
        }

        if (inputWidthAfterTotalCrop < 16)
        {
            // Too little left after cropping, vanish this stripe
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterTnr);
            continue;
        }

        pixelsToCrop += (inputWidthAfterZoomCrop - inputWidthAfterTotalCrop);

        // Estimated output width, used only to validate aspect-ratio consistency below; the actual
        // output width assigned to this stripe is derived below from the exact per-stripe output
        // start/end columns (see the "Upscaler_1_2 fix" block).
        uint16_t estimatedOutputWidth = static_cast<uint16_t>(2 * GRA_ROUND(static_cast<double>(inputWidthAfterTotalCrop) / scaleFactorFixed / 2.0));

        // Validate output width
        if (enforceUpscalerAspectRatioConstraints() &&
            static_cast<double>(inputWidthAfterTotalCrop) / estimatedOutputWidth !=
            static_cast<double>(resInfo->input_height - resInfo->input_crop.top - resInfo->input_crop.bottom) / resInfo->output_height)
        {
            // Output width is not valid, return error
            return StaticGraphStatus::SG_ERROR;
        }

        if (stripe == leftNonVanishedStripe && stripe != rightNonVanishedStripe)
        {
            // Crop on the right
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = 0;
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropRight = pixelsToCrop;
        }
        else if (stripe == rightNonVanishedStripe && stripe != leftNonVanishedStripe)
        {
            // Crop on the left
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = pixelsToCrop;
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropRight = 0;
        }
        else
        {
            // Crop both sides
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = GRA_ROUND_DOWN(static_cast<uint16_t>(pixelsToCrop / 2), 2);
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropRight = pixelsToCrop - kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft;
        }

        uint16_t stripeStart = static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX > resInfo->input_crop.left ?
            kernelFragments[stripe].fragmentStartX - resInfo->input_crop.left : 0);

        stripeStart += kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft;

        double widthIn = static_cast<double>(resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right);
        double widthOut = 2.0F * GRA_ROUND(static_cast<double>(widthIn) / scaleFactorFixed / 2.0F);

        double horizontalOffset = (static_cast<double>(widthIn) - static_cast<double>(scaleFactorFixed) * (static_cast<double>(widthOut) - 1.0F)) / 2.0F;

        // ---- Upscaler_1_2 fix ----
        // Find the stripe's exact output start column (outputStartCol): the smallest even output
        // column whose corresponding input column (horizontalOffset + outputStartCol * scaleFactorFixed,
        // floored) also lands on an even input column. If the natural start does not land on an even
        // input column, extend the left crop (stripeStart) until it does. The right edge
        // (stripeRightAfterCrop) is fixed by the crop already computed above and does not move.
        double stripeRightAfterCrop = static_cast<double>(stripeStart) + static_cast<double>(inputWidthAfterTotalCrop);

        double naturalOutputStartCol = std::ceil((static_cast<double>(stripeStart) - horizontalOffset) / scaleFactorFixed);
        int32_t outputStartCol = static_cast<int32_t>(2 * std::ceil(naturalOutputStartCol / 2.0));

        // Bounded: parity settles within a couple of iterations for realistic scale factors.
        for (int32_t guard = 0; guard < 4 &&
             static_cast<int32_t>(std::floor(horizontalOffset + outputStartCol * scaleFactorFixed)) % 2 != 0; guard++)
        {
            outputStartCol += 2;
        }

        int32_t inputStartAfterCrop = static_cast<int32_t>(std::floor(horizontalOffset + outputStartCol * scaleFactorFixed));
        int32_t leftExtraCrop = inputStartAfterCrop - static_cast<int32_t>(stripeStart);
        if (leftExtraCrop > 0)
        {
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = static_cast<uint16_t>(
                kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft + leftExtraCrop);
            stripeStart = static_cast<uint16_t>(inputStartAfterCrop);
        }

        int32_t lastOutputCol = static_cast<int32_t>(std::floor((stripeRightAfterCrop - horizontalOffset) / scaleFactorFixed));
        uint16_t outputWidth = static_cast<uint16_t>(GRA_ROUND_DOWN(lastOutputCol - outputStartCol + 1, 2));

        if (stripe == rightNonVanishedStripe)
        {
            // Do not create more pixels that required
            if (outputStartCol + outputWidth > resInfo->output_width)
            {
				outputWidth = static_cast<uint16_t>(resInfo->output_width - outputStartCol);
            }
        }

        kernelFragments[stripe].fragmentOutputWidth = outputWidth;
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(outputStartCol);
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsOutput(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments, bool isTnr)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    int16_t leftNonVanishedStripe = 0;
    int16_t rightNonVanishedStripe = _numberOfFragments - 1;

    for (int16_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if ((_node->fragmentVanishStatus[stripe] == VanishOption::Full) ||
            (isTnr && _node->fragmentVanishStatus[stripe] == VanishOption::AfterTnr))
        {
            // Not vanished
            leftNonVanishedStripe = stripe;
            break;
        }
    }

    for (int16_t stripe = _numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if ((_node->fragmentVanishStatus[stripe] == VanishOption::Full) ||
            (isTnr && _node->fragmentVanishStatus[stripe] == VanishOption::AfterTnr))
        {
            // Not vanished
            rightNonVanishedStripe = stripe;
            break;
        }
    }

    // Get the Sys Api structure
    if (runKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer))))
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }

    auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
    if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }

    StaticGraphKernelSystemApiIoBuffer* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer*>
        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

    // Get 8 / 10 bit info from sys api
    uint8_t precision = systemApi->component_precision == 0 ? 8 : 10;

    FormatType bufferFormat = GraphResolutionConfiguratorHelper::getFormatForDrainer(runKernel->kernel_uuid, precision);

    // Remove overlaps between stripes
    std::vector<uint16_t> newOutputStartX = std::vector<uint16_t>(_numberOfFragments, 0);

    for (int16_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        if (stripe == leftNonVanishedStripe) // first stripe
        {
            newOutputStartX[stripe] = 0;
        }
        else //middle or last stripe
        {
            newOutputStartX[stripe] =
                (_outputStartX[runKernel->kernel_uuid][stripe] + _outputStartX[runKernel->kernel_uuid][stripe-1] + kernelFragments[stripe-1].fragmentOutputWidth) / 4 * 2;

            // Align to format restrictions if TNR drainer & data is 10-bit packed
            newOutputStartX[stripe] = alignToFormatRestrictions(newOutputStartX[stripe], bufferFormat);
        }
    }

    _outputStartX[runKernel->kernel_uuid] = newOutputStartX;

    // Data Width is calculated according to data starts
    for (int16_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
        if (stripe == rightNonVanishedStripe) // last stripe
        {
            kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(runKernel->resolution_info->output_width - _outputStartX[runKernel->kernel_uuid][stripe]);
        }
        else // first or middle stripe
        {
            if (_outputStartX[runKernel->kernel_uuid][stripe + 1] <= _outputStartX[runKernel->kernel_uuid][stripe])
            {
                return StaticGraphStatus::SG_ERROR;
            }

            kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(_outputStartX[runKernel->kernel_uuid][stripe+1] - _outputStartX[runKernel->kernel_uuid][stripe]);
        }
    }

    // Update system API offsets
    // If actually running with 1 stripe - do not update the system API
    if (_node->GetNumberOfFragments() > 1)
    {
        for (int16_t stripe = 0; stripe < _numberOfFragments; stripe++)
        {
            systemApi->x_output_offset_per_stripe[stripe] = 0;

            for (uint8_t plane = 0; plane < 3; plane++)
            {
                systemApi->plane_start_address_per_stripe[stripe * 3 + plane] = 0;
            }
        }

        for (int16_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
        {
            uint32_t sumOfPrevWidths = 0;

            for (int16_t s = leftNonVanishedStripe; s < stripe; s++)
            {
                sumOfPrevWidths += kernelFragments[s].fragmentOutputWidth;
            }

            // OutputOffsetPerStripe: Sum(prev output widths) + input_crop.left - stripe.startX
            systemApi->x_output_offset_per_stripe[stripe] =
                sumOfPrevWidths + runKernel->resolution_info->input_crop.left - kernelFragments[stripe].fragmentStartX;

            // PlaneOffsetStartAddressPerStripe: Sum(prev output widths) * DataSize
            for (uint8_t plane = 0; plane < 2; plane++)
            {
                systemApi->plane_start_address_per_stripe[stripe * 3 + plane] = getPlaneStartAddress(sumOfPrevWidths, bufferFormat, plane);
            }
        }
    }

    return StaticGraphStatus::SG_OK;
}

uint32_t Gen2FragmentsConfigurator::getPlaneStartAddress(uint32_t sumOfPrevWidths, FormatType formatType, uint8_t plane)
{
    constexpr uint32_t cacheLineSizeInBytes = 64;
    constexpr uint32_t bitsPerByte = 8;

    // Calculate according to format BPP.
    uint32_t bitsPerElement = 8;
    uint32_t elementsPerCacheLine = 64;
    uint8_t numberOfPlanes = 3;
    bool isQuad = false;
    uint32_t elementsInQuad = 0;
    uint8_t planeHorizSubSample = 0;

    if (formatType == FormatType::YUV420_8_SP_P)
    {
        // NV12 8-bit packed (OFS output)
        bitsPerElement = 8;
        elementsPerCacheLine = 64;
        numberOfPlanes = 2;
    }
    else if (formatType == FormatType::YUV420_10_SP_MSB)
    {
        // P010 10-bit (OFS output)
        bitsPerElement = 16;
        elementsPerCacheLine = 32;
        numberOfPlanes = 2;
    }
    else if (formatType == FormatType::YUV420_10_SP_P)
    {
        // 10-bit packed (TNR ref)
        bitsPerElement = 10;
        elementsPerCacheLine = 50;
        numberOfPlanes = 2;
    }
    else if (formatType == FormatType::META_8)
    {
        // 8-bit meta data (TNR recursice similarity)
        bitsPerElement = 8;
        elementsPerCacheLine = 64;
        numberOfPlanes = 1;
    }
    else
    {
        // Format not supported
        // Log error
        return 0;
    }

    if (plane >= numberOfPlanes)
    {
        // Plane does not exist
        return 0;
    }

    if (isQuad)
    {
        uint32_t bitsPerCacheLine = cacheLineSizeInBytes * bitsPerByte;
        elementsPerCacheLine = bitsPerCacheLine / bitsPerElement;
        elementsPerCacheLine = elementsPerCacheLine / elementsInQuad * elementsInQuad;
    }

    uint32_t pixelsPerCacheLine = elementsPerCacheLine;
    if (isQuad)
    {
        // pixelsPerCacheLine counts QUAD columns, not luma pixels.
        pixelsPerCacheLine /= elementsInQuad;
    }

    sumOfPrevWidths >>= planeHorizSubSample;

    // Offset is calculated by taking whole cache lines and then adding the remaining pixles and translate to bytes.
    uint32_t wholeCacheLines = sumOfPrevWidths / pixelsPerCacheLine;
    uint32_t remainingPixels = sumOfPrevWidths % pixelsPerCacheLine;

    uint32_t elementsPerPixel = 1;
    if (isQuad)
    {
        elementsPerPixel = elementsInQuad;
    }

    if ((remainingPixels * elementsPerPixel * bitsPerElement) % bitsPerByte != 0)
    {
        // Log error
        return 0;
    }

    return wholeCacheLines * cacheLineSizeInBytes +
        (remainingPixels * elementsPerPixel * bitsPerElement) / bitsPerByte;
}

uint16_t Gen2FragmentsConfigurator::alignToFormatRestrictions(uint16_t size, FormatType bufferFormat)
{
    if (bufferFormat == FormatType::YUV420_10_SP_P)
    {
        uint16_t elementsPerCacheLine = 50;

        uint16_t remainingPixels = size % elementsPerCacheLine;
        uint16_t pixelsToRemove = remainingPixels % 4;

        return size - pixelsToRemove;
    }

    return size;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsTnrScaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactor = static_cast<double>(resInfo->output_width) / (resInfo->input_width);

    for (int32_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::AfterStats)
        {
            continue;
        }

        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(kernelFragments[stripe].fragmentInputWidth * scaleFactor);

        // Start of output is rounded up since this is what b2i_ds does (Creates pixels starting from the pixel after)
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(ceil(scaleFactor * kernelFragments[stripe].fragmentStartX / 2)) * 2;
    }

    // Save stripes for feeder configuration
    _tnrScalerFragments = kernelFragments;
    _tnrScalerRunKernel = runKernel;

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsTnrFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole)
{
    if (kernelRole == GraphResolutionConfiguratorKernelRole::McFeederFull)
    {
        _mcFullRefRunKernel = runKernel;
        _mcFullRefFragments = kernelFragments;

        return StaticGraphStatus::SG_OK;
    }
    if (kernelRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
    {
        _mcSmallRefRunKernel = runKernel;
        _mcSmallRefFragments = kernelFragments;

        return StaticGraphStatus::SG_OK;
    }

    if (kernelFragments == nullptr || _tnrScalerFragments == nullptr || _tnrScalerRunKernel == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }
    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);

    for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::AfterStats)
        {
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterStats);
            continue;
        }

        if (kernelRole == GraphResolutionConfiguratorKernelRole::TnrFeederFull)
        {
            // TNR Full resolution
            kernelFragments[stripe].fragmentInputWidth = static_cast<uint16_t>(_tnrScalerRunKernel->resolution_info->input_width);
            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentInputWidth;
            kernelFragments[stripe].fragmentStartX = _tnrScalerFragments[stripe].fragmentStartX;
            _outputStartX[runKernel->kernel_uuid][stripe] = _tnrScalerFragments[stripe].fragmentStartX;
        }
        else if (kernelRole == GraphResolutionConfiguratorKernelRole::TnrFeederSmall || kernelRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
        {
            // TNR Small resolution
            kernelFragments[stripe].fragmentInputWidth = static_cast<uint16_t>(_tnrScalerRunKernel->resolution_info->output_width);
            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentOutputWidth;
            kernelFragments[stripe].fragmentStartX = _outputStartX[_tnrScalerRunKernel->kernel_uuid][stripe];
            _outputStartX[runKernel->kernel_uuid][stripe] = _outputStartX[_tnrScalerRunKernel->kernel_uuid][stripe];
        }
    }

    // Update system API offsets
    // If actually running with 1 stripe - do not update the system API
    if (_node->GetNumberOfFragments() > 1)
    {
        if (runKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer))))
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
        if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        StaticGraphKernelSystemApiIoBuffer* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer*>
            (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

        for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
        {
            systemApi->x_output_offset_per_stripe[stripe] = _outputStartX[runKernel->kernel_uuid][stripe];
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsSmurf(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    StaticGraphFragmentDesc* prevKernelFragments, std::vector<SmurfKernelInfo*>& smurfKernels)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    auto resInfo = runKernel->resolution_info;
    if (resInfo->input_width == 0 || resInfo->input_height == 0 ||
        resInfo->output_width == 0 || resInfo->output_height == 0)
    {
        // Smurf not in use
        return StaticGraphStatus::SG_OK;
    }

    // Find the device that is fed by this smurf (the second in the pair)
    StaticGraphRunKernel* deviceRunKernel = nullptr;
    StaticGraphRunKernel* feederRunKernel = nullptr;
    for (auto& smurfInfo : smurfKernels)
    {
        if (smurfInfo->_smurfRunKernel->kernel_uuid == runKernel->kernel_uuid)
        {
            deviceRunKernel = smurfInfo->_deviceRunKernel;
            feederRunKernel = smurfInfo->_feederRunKernel;
            break;
        }
    }

    if (deviceRunKernel == nullptr || feederRunKernel == nullptr)
    {
        // Smurf does not have a device?
        return StaticGraphStatus::SG_ERROR;
    }

    // Find the index of the device in the node kernels
    StaticGraphFragmentDesc* deviceFragments = nullptr;
    StaticGraphFragmentDesc* feederFragments = nullptr;
    for (uint32_t j = 0; j < _node->nodeKernels.kernelCount; j++)
    {
        if (_node->nodeKernels.kernelList[j].run_kernel.kernel_uuid == deviceRunKernel->kernel_uuid)
        {
            deviceFragments = _node->nodeKernels.kernelList[j].fragment_descs;
        }
        if (_node->nodeKernels.kernelList[j].run_kernel.kernel_uuid == feederRunKernel->kernel_uuid)
        {
            feederFragments = _node->nodeKernels.kernelList[j].fragment_descs;
        }

        if (deviceFragments != nullptr && feederRunKernel != nullptr)
        {
            break;
        }
    }

    if (deviceFragments == nullptr || feederFragments == nullptr)
    {
        // Smurf does not have a device?
        return StaticGraphStatus::SG_ERROR;
    }

    uint32_t newScaleFactorH = (resInfo->input_width << GraphResolutionConfigurator::SMURF_SCALE_PREC) / (resInfo->output_width + resInfo->output_crop.left + resInfo->output_crop.right);
    uint32_t newScaleFactorV = (resInfo->input_height << GraphResolutionConfigurator::SMURF_SCALE_PREC) / (resInfo->output_height + resInfo->output_crop.top + resInfo->output_crop.bottom);
    double newScaleFactor = (double)(1 << GraphResolutionConfigurator::SMURF_SCALE_PREC) / std::min(newScaleFactorH, newScaleFactorV);

    // Smurf input is the same as the feeder output
    copyFragments(runKernel, feederFragments, feederRunKernel->kernel_uuid, kernelFragments);

    for (int8_t stripe = 0; stripe < _node->GetNumberOfFragments(); stripe++)
    {
        // If device is vanished, vanish the smurf too
        if (deviceFragments[stripe].fragmentOutputWidth == 0 &&
            deviceFragments[stripe].fragmentInputWidth == 0)
        {
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, _node->fragmentVanishStatus[stripe]);
            continue;
        }

        // Smurf output is the same as the device input
        kernelFragments[stripe].fragmentOutputWidth = deviceFragments[stripe].fragmentInputWidth;

        // Get the start X that will we actually have (since feeder can only crop even numbers) prevKernelFragments is the feeder
        uint32_t requiredOutputStartX = deviceFragments[stripe].fragmentStartX + resInfo->output_crop.left;
        uint32_t actualOutputStartX = static_cast<uint32_t>(ceil(newScaleFactor * prevKernelFragments[stripe].fragmentStartX / 2)) * 2;

        if (requiredOutputStartX < actualOutputStartX)
        {
            // We did not provide enough pixels from segmap feeder
            return StaticGraphStatus::SG_ERROR;
        }

        kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = 0;
        if (actualOutputStartX > (uint32_t)resInfo->output_crop.left)
        {
            // This is actually output crop (PAL knows :)
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = static_cast<uint16_t>(requiredOutputStartX - actualOutputStartX);
        }
        else
        {
            // PAL knows to crop from actual start to the output crop (zoom crop).
            // Tell PAL to crop to the beginning of stripe in addition
            kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft = deviceFragments[stripe].fragmentStartX;
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::configFragmentsSmurfFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments)
{
    if (runKernel == nullptr || kernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);

    for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        _outputStartX[runKernel->kernel_uuid][stripe] = kernelFragments[stripe].fragmentStartX;
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Gen2FragmentsConfigurator::copyFragments(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* prevKernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* kernelFragments)
{
    if (prevKernelFragments == nullptr || kernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);

    if (_outputStartX.find(prevKernelUuid) == _outputStartX.end())
    {
        // This is the main DS, we start from it, no need to copy
        return StaticGraphStatus::SG_OK;
    }

    for (uint32_t i = 0; i < _numberOfFragments; i++)
    {
        kernelFragments[i].fragmentInputWidth = prevKernelFragments[i].fragmentOutputWidth;
        kernelFragments[i].fragmentOutputWidth = prevKernelFragments[i].fragmentOutputWidth;
        kernelFragments[i].fragmentStartX = _outputStartX[prevKernelUuid][i];
        kernelFragments[i].upscalerFragDesc.fragmentInputCropLeft = 0;
        kernelFragments[i].upscalerFragDesc.fragmentInputCropRight = 0;

        _outputStartX[runKernel->kernel_uuid][i] = kernelFragments[i].fragmentStartX;
    }

    return StaticGraphStatus::SG_OK;
}

// Find the greatest common divisor, curtesy of CoPilot
uint32_t Gen2FragmentsConfigurator::calculateGcd(uint32_t a, uint32_t b)
{
    while (b != 0)
    {
        uint32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

#if SUPPORT_HISTORY_CHANGE_BASED_ON_GRAPH == 0

Ipu8FragmentsConfigurator::Ipu8FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments) :
    Gen2FragmentsConfigurator(staticGraph, node, numberOfFragments)
{
}

#else

#include "LbmcConfigurator.h"
Ipu9FragmentsConfigurator::Ipu9FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint8_t numberOfFragments) :
    Gen2FragmentsConfigurator(staticGraph, node, numberOfFragments)
{
}

StaticGraphStatus Ipu9FragmentsConfigurator::configFragmentsTnrMcFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole)
{
    if (kernelRole == GraphResolutionConfiguratorKernelRole::McFull)
    {
        _mcFullRunKernel = runKernel;
    }

    StaticGraphRunKernel* refRunKernel = kernelRole == GraphResolutionConfiguratorKernelRole::McFull ? _mcFullRefRunKernel : _mcSmallRefRunKernel;
    StaticGraphFragmentDesc* refkernelFragments = kernelRole == GraphResolutionConfiguratorKernelRole::McFull ? _mcFullRefFragments : _mcSmallRefFragments;
    GraphResolutionConfiguratorKernelRole refKernelRole = kernelRole == GraphResolutionConfiguratorKernelRole::McFull ? GraphResolutionConfiguratorKernelRole::McFeederFull : GraphResolutionConfiguratorKernelRole::McFeederSmall;

    if (refkernelFragments == nullptr || _tnrScalerFragments == nullptr || _tnrScalerRunKernel == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }
    _outputStartX[refRunKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);
    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_numberOfFragments, 0);

    for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::AfterStats)
        {
            vanishStripe(stripe, refRunKernel->kernel_uuid, refkernelFragments, VanishOption::AfterStats);
            continue;
        }

        if (refKernelRole == GraphResolutionConfiguratorKernelRole::McFeederFull)
        {
            // TNR Full resolution
            refkernelFragments[stripe].fragmentInputWidth = static_cast<uint16_t>(_tnrScalerRunKernel->resolution_info->input_width);
            refkernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentInputWidth;
            refkernelFragments[stripe].fragmentStartX = _tnrScalerFragments[stripe].fragmentStartX;
            _outputStartX[refRunKernel->kernel_uuid][stripe] = _tnrScalerFragments[stripe].fragmentStartX;

            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentInputWidth;
            _outputStartX[runKernel->kernel_uuid][stripe] = _tnrScalerFragments[stripe].fragmentStartX;
        }
        else
        if (refKernelRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
        {
            // TNR Small resolution
            refkernelFragments[stripe].fragmentInputWidth = static_cast<uint16_t>(_tnrScalerRunKernel->resolution_info->output_width);
            refkernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentOutputWidth;
            refkernelFragments[stripe].fragmentStartX = _outputStartX[_tnrScalerRunKernel->kernel_uuid][stripe];
            _outputStartX[refRunKernel->kernel_uuid][stripe] = _outputStartX[_tnrScalerRunKernel->kernel_uuid][stripe];

            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentOutputWidth;
            _outputStartX[runKernel->kernel_uuid][stripe] = _outputStartX[_tnrScalerRunKernel->kernel_uuid][stripe];
        }

        if (refKernelRole == GraphResolutionConfiguratorKernelRole::McFeederFull || refKernelRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
        {
            LbmcConfig mcConfig(refKernelRole);
            mcConfig.ConfigureReferenceFeeder(_mcFullRunKernel->system_api, refRunKernel->bpp_info.output_bpp, refRunKernel->resolution_history->output_width, &refkernelFragments[stripe], _outputStartX[refRunKernel->kernel_uuid][stripe]);

            kernelFragments[stripe].fragmentInputWidth = static_cast<uint16_t>(refkernelFragments[stripe].fragmentOutputWidth);
            kernelFragments[stripe].fragmentStartX = _outputStartX[refRunKernel->kernel_uuid][stripe];

            if (_node->GetNumberOfFragments() > 1)
            {
                if (kernelRole == GraphResolutionConfiguratorKernelRole::McFull)
                {
                    StaticGraphKernelSystemApiMcFull* systemApiMcFull = reinterpret_cast<StaticGraphKernelSystemApiMcFull*>
                        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

                     systemApiMcFull->mu_stripe_lookahead = mcConfig.mu_stripe_lookahead;
                     systemApiMcFull->mu_stripe_bleed_left[stripe] = mcConfig.mu_stripe_bleed_left;
                     systemApiMcFull->read_horz_offset[stripe] = mcConfig.read_horz_offset;
                }
                else
                if (kernelRole == GraphResolutionConfiguratorKernelRole::McSmall)
                {
                    StaticGraphKernelSystemApiMcSmall* systemApiSmall = reinterpret_cast<StaticGraphKernelSystemApiMcSmall*>
                        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

                     systemApiSmall->mu_stripe_lookahead = mcConfig.mu_stripe_lookahead;
                     systemApiSmall->mu_stripe_bleed_left[stripe] = mcConfig.mu_stripe_bleed_left;
                     systemApiSmall->read_horz_offset[stripe] = mcConfig.read_horz_offset;
                }
            }
        }
    }
    // Update system API offsets
    // If actually running with 1 stripe - do not update the system API
    if (_node->GetNumberOfFragments() > 1)
    {
        if (refRunKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer))))
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        auto systemApiHeader = static_cast<SystemApiRecordHeader*>(refRunKernel->system_api.data);
        if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
        {
            // TODO log error
            return StaticGraphStatus::SG_ERROR;
        }

        StaticGraphKernelSystemApiIoBuffer* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer*>
            (static_cast<int8_t*>(refRunKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

        for (uint8_t stripe = 0; stripe < _numberOfFragments; stripe++)
        {
            systemApi->x_output_offset_per_stripe[stripe] = _outputStartX[refRunKernel->kernel_uuid][stripe];
        }
    }
    return StaticGraphStatus::SG_OK;

}
#endif

