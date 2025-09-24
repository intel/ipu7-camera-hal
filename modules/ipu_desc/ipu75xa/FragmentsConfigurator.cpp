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

Ipu8FragmentsConfigurator::Ipu8FragmentsConfigurator(IStaticGraphConfig* staticGraph, OuterNode* node, uint32_t upscalerWidthGranularity)
    : _staticGraph(staticGraph), _node(node), _upscalerWidthGranularity(upscalerWidthGranularity)
{
}

StaticGraphStatus Ipu8FragmentsConfigurator::configureFragments()
{
    if (_staticGraph == nullptr || _node == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    // Reset status
    for (int32_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        _node->fragmentVanishStatus[stripe] = VanishOption::Full;
    }

    for (uint32_t j = 0; j < _node->nodeKernels.kernelCount; j++)
    {
        StaticGraphRunKernel* runKernel = &_node->nodeKernels.kernelList[j].run_kernel;
        StaticGraphFragmentDesc* kernelFragments = _node->nodeKernels.kernelList[j].fragment_descs;

        // Take previous kernel as reference, unless we will change it below.
        StaticGraphFragmentDesc* prevKernelFragments = j == 0 ? nullptr : _node->nodeKernels.kernelList[j - 1].fragment_descs;
        uint32_t prevKernelUuid = j == 0 ? 0 : _node->nodeKernels.kernelList[j - 1].run_kernel.kernel_uuid;

        uint32_t referenceKernel = GraphResolutionConfiguratorHelper::getReferenceKernel(runKernel->kernel_uuid);

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
            {
                configFragmentsDownscaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::EspaCropper:
            {
                configFragmentsCropper(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::UpScaler:
            {
                configFragmentsUpscaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::Output:
            {
                configFragmentsOutput(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::TnrScaler:
            {
                configFragmentsTnrScaler(runKernel, kernelFragments, prevKernelUuid, prevKernelFragments);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::TnrFeederFull:
            case GraphResolutionConfiguratorKernelRole::TnrFeederSmall:
            {
                configFragmentsTnrFeeder(runKernel, kernelFragments, kernelRole);
                break;
            }

            case GraphResolutionConfiguratorKernelRole::NonRcb:
            {
                // Before zoom kernels - take prev kernel fragments as-is
                copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);
            }

            default:
            {
                // No action for other kernels
                break;
            }
        }
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsDownscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactorW = static_cast<double>(resInfo->output_width) / (resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right);
    auto scaleFactorH = static_cast<double>(resInfo->output_height) / (resInfo->input_height - resInfo->input_crop.top - resInfo->input_crop.bottom);
    auto scaleFactor = std::max(scaleFactorW, scaleFactorH);

    for (int32_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        int rightCrop = stripe == static_cast<int32_t>(_node->numberOfFragments - 1) ? resInfo->input_crop.right : 0;

        double value = (static_cast<double>(kernelFragments[stripe].fragmentInputWidth - rightCrop) * scaleFactor) / 4;
        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(floor(value)) *  4;

        // Start of output is rounded up since this is what b2i_ds does (Creates pixels starting from the pixel after)
        value = (scaleFactor * kernelFragments[stripe].fragmentStartX) / 2;
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(ceil(value)) * 2;
    }

    return StaticGraphStatus::SG_OK;
}

void Ipu8FragmentsConfigurator::vanishStripe(uint8_t stripe, uint32_t runKerenlUuid, StaticGraphFragmentDesc* kernelFragments, VanishOption vanishOption)
{
    _node->fragmentVanishStatus[stripe] = vanishOption;
    kernelFragments[stripe] = {};
    _outputStartX[runKerenlUuid][stripe] = 0;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsCropper(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
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
    int32_t rightNonVanishedStripe = _node->numberOfFragments - 1;

    std::vector<uint32_t> xOffset(_node->numberOfFragments, 0);

    for (int8_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        if (leftPixel + VANISH_MIN >= kernelFragments[stripe].fragmentStartX + kernelFragments[stripe].fragmentInputWidth)
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

    for (uint8_t stripe = _node->numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if (rightPixel <= kernelFragments[stripe].fragmentStartX + VANISH_MIN)
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

    for (int32_t stripe = leftNonVanishedStripe; stripe <= rightNonVanishedStripe; stripe++)
    {
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

        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(outputWidth);

        // For start point, we need to remove the left cropping only for stripes 1 and on
        uint16_t outputStartX = static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX > runKernel->resolution_info->input_crop.left) ?
            static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX - runKernel->resolution_info->input_crop.left) : 0;

        _outputStartX[runKernel->kernel_uuid][stripe] = outputStartX;

        if (kernelFragments[stripe].fragmentOutputWidth % 8 != 0)
        {
            uint16_t pixelsToCrop = kernelFragments[stripe].fragmentOutputWidth % 8;

            // Additional crop on the right, affects only output width
            kernelFragments[stripe].fragmentOutputWidth -= pixelsToCrop;

            if (stripe == rightNonVanishedStripe)
            {
                // Last stripe - crop from left
                _outputStartX[runKernel->kernel_uuid][stripe] += pixelsToCrop;
                xOffset[stripe] += pixelsToCrop;
            }
        }
    }

    // Update system API offsets

#ifdef STATIC_GRAPH_USE_IA_LEGACY_TYPES
    if (runKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer1_4))))
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }
#endif

    auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
    if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }

    StaticGraphKernelSystemApiIoBuffer1_4* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer1_4*>
        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

    for (uint8_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        systemApi->x_output_offset_per_stripe[stripe] = xOffset[stripe];
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsUpscaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
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

    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_node->numberOfFragments, 0);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactorW = static_cast<double>(resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right) / resInfo->output_width;
    auto scaleFactorH = static_cast<double>(resInfo->input_height - resInfo->input_crop.top - resInfo->input_crop.bottom) / resInfo->output_height;
    auto scaleFactor = std::max(scaleFactorW, scaleFactorH);
    uint32_t upscalerWidthGranularity = _upscalerWidthGranularity;
    uint16_t inputUnits = static_cast<uint16_t>((resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right) / upscalerWidthGranularity);

    // We would like to keep upscalerWidthGranularity as large as possible in order to minimize the number of pixels that cannot be used for upscaling
    // (upscalerWidthGranularity is divided to stripes, so the larger it is the more accurately we can divide)
    while (inputUnits % 2 == 0)
    {
        inputUnits /= 2;
        upscalerWidthGranularity *= 2;
    }

    int32_t leftPixel = runKernel->resolution_info->input_crop.left;
    int32_t rightPixel = static_cast<uint16_t>(runKernel->resolution_info->input_width - runKernel->resolution_info->input_crop.right);

    uint8_t leftNonVanishedStripe = 0;
    uint8_t rightNonVanishedStripe = _node->numberOfFragments - 1;

    for (int8_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            continue;
        }

        if (leftPixel >= kernelFragments[stripe].fragmentStartX + kernelFragments[stripe].fragmentInputWidth)
        {
            // This stripe is cropped out, vanish it!
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterTnr);
            continue;
        }

        // Not vanished
        leftNonVanishedStripe = stripe;
        break;
    }

    for (uint8_t stripe = _node->numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if (_node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            continue;
        }

        if (rightPixel <= kernelFragments[stripe].fragmentStartX)
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
        uint16_t maxInputWidth = static_cast<uint16_t>(UPSCALER_MAX_OUTPUT_WIDTH * scaleFactor);
        if (inputWidthAfterZoomCrop > maxInputWidth)
        {
            pixelsToCrop = inputWidthAfterZoomCrop - maxInputWidth;
            inputWidthAfterZoomCrop = maxInputWidth;
        }

        uint16_t stripeStepW = GRA_ROUND_DOWN(static_cast<uint16_t>(static_cast<double>(inputWidthAfterZoomCrop) / (resInfo->input_width - resInfo->input_crop.left - resInfo->input_crop.right) * upscalerWidthGranularity), 2);
        uint16_t inputWidthAfterTotalCrop = stripeStepW * inputUnits;

        if (inputWidthAfterTotalCrop < 16)
        {
            // Too little left after cropping, vanish this stripe
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterTnr);
            continue;
        }

        pixelsToCrop += (inputWidthAfterZoomCrop - inputWidthAfterTotalCrop);

        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(GRA_ROUND(static_cast<double>(inputWidthAfterTotalCrop) / scaleFactor));

        // Validate output width
        if (static_cast<double>(inputWidthAfterTotalCrop) / kernelFragments[stripe].fragmentOutputWidth !=
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

        uint16_t outputStartX = static_cast<uint16_t>(kernelFragments[stripe].fragmentStartX > resInfo->input_crop.left ?
            kernelFragments[stripe].fragmentStartX - resInfo->input_crop.left : 0);

        outputStartX += kernelFragments[stripe].upscalerFragDesc.fragmentInputCropLeft;

        outputStartX = GRA_ROUND_UP(static_cast<uint16_t>(ceil(static_cast<double>(outputStartX) / scaleFactor)), 2);

        _outputStartX[runKernel->kernel_uuid][stripe] = outputStartX;
    }

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsOutput(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    int16_t leftNonVanishedStripe = 0;
    int16_t rightNonVanishedStripe = _node->numberOfFragments - 1;

    for (int16_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::Full)
        {
            // Not vanished
            leftNonVanishedStripe = stripe;
            break;
        }
    }

    for (int16_t stripe = _node->numberOfFragments - 1; stripe >= 0; stripe--)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::Full)
        {
            // Not vanished
            rightNonVanishedStripe = stripe;
            break;
        }
    }

    // Remove overlaps between stripes
    FormatType bufferFormat = GraphResolutionConfiguratorHelper::getFormatForDrainer(runKernel->kernel_uuid);
    std::vector<uint16_t> newOutputStartX = std::vector<uint16_t>(_node->numberOfFragments, 0);

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
            kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(runKernel->resolution_info->input_width - _outputStartX[runKernel->kernel_uuid][stripe]);
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
#ifdef STATIC_GRAPH_USE_IA_LEGACY_TYPES
    if (runKernel->system_api.size != ((GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4)) + (sizeof(StaticGraphKernelSystemApiIoBuffer1_4))))
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }
#endif

    auto systemApiHeader = static_cast<SystemApiRecordHeader*>(runKernel->system_api.data);
    if (systemApiHeader->systemApiUuid != GraphResolutionConfiguratorHelper::getRunKernelIoBufferSystemApiUuid())
    {
        // TODO log error
        return StaticGraphStatus::SG_ERROR;
    }

    StaticGraphKernelSystemApiIoBuffer1_4* systemApi = reinterpret_cast<StaticGraphKernelSystemApiIoBuffer1_4*>
        (static_cast<int8_t*>(runKernel->system_api.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

    for (int16_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
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

    return StaticGraphStatus::SG_OK;
}

uint32_t Ipu8FragmentsConfigurator::getPlaneStartAddress(uint32_t sumOfPrevWidths, FormatType formatType, uint8_t plane)
{
    // Calculate according to format BPP.
    uint32_t bitsPerElement = 8;
    uint32_t elementsPerCacheLine = 64;
    uint8_t numberOfPlanes = 3;

    if (formatType == FormatType::YUV420_8_SP_P)
    {
        // 8-bit packed (OFS output)
        bitsPerElement = 8;
        elementsPerCacheLine = 64;
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

    // Offset is calculated by taking whole cache lines and then adding the remaining pixles and translate to bytes.
    uint32_t wholeCacheLines = sumOfPrevWidths / elementsPerCacheLine;
    uint32_t remainingPixels = sumOfPrevWidths % elementsPerCacheLine;

    if ((remainingPixels * bitsPerElement) % 8 != 0)
    {
        // Log error
        return 0;
    }

    return wholeCacheLines * 64 + (remainingPixels * bitsPerElement) / 8;
}

uint16_t Ipu8FragmentsConfigurator::alignToFormatRestrictions(uint16_t size, FormatType bufferFormat)
{
    if (bufferFormat != FormatType::YUV420_10_SP_P)
    {
        return size;
    }

    uint16_t elementsPerCacheLine = 50;

    uint16_t remainingPixels = size % elementsPerCacheLine;
    uint16_t pixelsToRemove = remainingPixels % 4;

    return size - pixelsToRemove;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsTnrScaler(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments,
    uint32_t prevKernelUuid, StaticGraphFragmentDesc* prevKernelFragments)
{
    if (kernelFragments == nullptr || prevKernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }

    copyFragments(runKernel, prevKernelFragments, prevKernelUuid, kernelFragments);

    auto resInfo = runKernel->resolution_info;

    auto scaleFactor = static_cast<double>(resInfo->output_width) / (resInfo->input_width);

    for (int32_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] != VanishOption::Full)
        {
            continue;
        }

        kernelFragments[stripe].fragmentOutputWidth = static_cast<uint16_t>(kernelFragments[stripe].fragmentInputWidth * scaleFactor);

        // Start of output is rounded up since this is what b2i_ds does (Creates pixels starting from the pixel after)
        _outputStartX[runKernel->kernel_uuid][stripe] = static_cast<uint16_t>(ceil(scaleFactor * kernelFragments[stripe].fragmentStartX / 2)) * 2;
    }

    // Save stripes for feeder configuration
    _tnrScalerFragments = kernelFragments;
    _tnrScalerUuid = runKernel->kernel_uuid;

    return StaticGraphStatus::SG_OK;
}

StaticGraphStatus Ipu8FragmentsConfigurator::configFragmentsTnrFeeder(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* kernelFragments, GraphResolutionConfiguratorKernelRole kernelRole)
{
    if (kernelFragments == nullptr)
    {
        return StaticGraphStatus::SG_ERROR;
    }
    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_node->numberOfFragments, 0);

    for (uint8_t stripe = 0; stripe < _node->numberOfFragments; stripe++)
    {
        if (_node->fragmentVanishStatus[stripe] == VanishOption::AfterStats)
        {
            vanishStripe(stripe, runKernel->kernel_uuid, kernelFragments, VanishOption::AfterStats);
            continue;
        }

        if (kernelRole == GraphResolutionConfiguratorKernelRole::TnrFeederFull)
        {
            // TNR Full resolution
            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentInputWidth;
            kernelFragments[stripe].fragmentStartX = _tnrScalerFragments[stripe].fragmentStartX;
            _outputStartX[runKernel->kernel_uuid][stripe] = _tnrScalerFragments[stripe].fragmentStartX;
        }
        else // GraphResolutionConfiguratorKernelRole::TnrFeederSmall
        {
            // TNR Small resolution
            kernelFragments[stripe].fragmentOutputWidth = _tnrScalerFragments[stripe].fragmentOutputWidth;
            kernelFragments[stripe].fragmentStartX = _outputStartX[_tnrScalerUuid][stripe];
            _outputStartX[runKernel->kernel_uuid][stripe] = _outputStartX[_tnrScalerUuid][stripe];
        }
    }

    return StaticGraphStatus::SG_OK;
}

void Ipu8FragmentsConfigurator::copyFragments(StaticGraphRunKernel* runKernel, StaticGraphFragmentDesc* prevKernelFragments, uint32_t prevKernelUuid, StaticGraphFragmentDesc* kernelFragments)
{
    if (prevKernelFragments == nullptr || kernelFragments == nullptr)
    {
        return;
    }

    _outputStartX[runKernel->kernel_uuid] = std::vector<uint16_t>(_node->numberOfFragments, 0);

    if (_outputStartX.find(prevKernelUuid) == _outputStartX.end())
    {
        // This is the main DS, we start from it, no need to copy
        return;
    }

    for (uint32_t i = 0; i < _node->numberOfFragments; i++)
    {
        kernelFragments[i].fragmentInputWidth = prevKernelFragments[i].fragmentOutputWidth;
        kernelFragments[i].fragmentOutputWidth = prevKernelFragments[i].fragmentOutputWidth;
        kernelFragments[i].fragmentStartX = _outputStartX[prevKernelUuid][i];
        kernelFragments[i].upscalerFragDesc.fragmentInputCropLeft = 0;
        kernelFragments[i].upscalerFragDesc.fragmentInputCropRight = 0;

        _outputStartX[runKernel->kernel_uuid][i] = kernelFragments[i].fragmentStartX;
    }
}
