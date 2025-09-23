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
#include <vector>
#include <map>
#include <stdint.h>

#define _USE_MATH_DEFINES
#define GRA_ROUND_UP(a,b)  (((a) + ((b)-1)) / (b) * (b))
#define GRA_ROUND_DOWN(a,b)  ((a) / (b) * (b))
#define GRA_ROUND(a)  (((double)(a) > 0.0) ? floor((double)(a) + 0.5) : ceil((double)(a) - 0.5))

// ROI in user level
class RegionOfInterest
{
public:
    double zoomFactor;
    double panFactor;
    double tiltFactor;

    // If true, take factors relative to sensor image
    // (needed for WFOV face tracking for example)
    bool fromInput;
};

// ROI translated to sensor resolution
// sensor width = crop.left + width + crop.right
// sensor height = crop.top + height + crop.bottom
class SensorRoi
{
public:
    uint32_t width;         // ROI width
    uint32_t height;        // ROI height
    uint32_t cropLeft;      // Crop from sensor width to ROI left
    uint32_t cropRight;     // Crop from sensor width from ROI right
    uint32_t cropTop;       // Crop from sensor height to ROI top
    uint32_t cropBottom;    // Crop from sensor height from ROI bottom
};

class ResolutionRoi
{
public:
    uint32_t width;         // ROI width
    uint32_t height;        // ROI height
    uint32_t left;          // ROI left point
    uint32_t right;         // ROI right point
    uint32_t top;           // ROI top point
    uint32_t bottom;        // ROI bottom point
};

enum class GraphResolutionConfiguratorKernelRole : uint8_t
{
    UpScaler,
    DownScaler,
    EspaCropper,
    NonRcb,
    Output,
    TnrScaler,
    TnrFeederFull,
    TnrFeederSmall,
    None
};

class RunKernelCoords
{
public:
    RunKernelCoords()
    {
        nodeInd = 0;
        kernelInd = 0;
    }
    uint32_t nodeInd;
    uint32_t kernelInd;
};

class GraphResolutionConfigurator
{
public:
    GraphResolutionConfigurator(IStaticGraphConfig* staticGraph);
    ~GraphResolutionConfigurator()
    {
        _kernelsForUpdate.clear();
    }

    StaticGraphStatus updateStaticGraphConfig(const RegionOfInterest& roi,
                                              const RegionOfInterest& prevRoi,
                                              bool isCenteredZoom,
                                              bool prevIsCenteredZoom,
                                              bool& isKeyResolutionChanged);
    // Calculate ROI in sensor dimensions. User ROI is given relative to *full* output ROI
    StaticGraphStatus getSensorRoi(const RegionOfInterest& userRoi, SensorRoi& sensorRoi);
    // Calculate ROI in sensor dimensions. Resolution ROI is given relative to *final* (zoomed) output ROI
    StaticGraphStatus getInputRoiForOutput(const ResolutionRoi& roi, const HwSink hwSink, SensorRoi& sensorRoi);

    // Calculate ROI in sensor dimensions. Resolution ROI is given relative to *final* (zoomed) output ROI
    // This function is used for statistics output only
    StaticGraphStatus getStatsRoiFromSensorRoi(const SensorRoi& sensorRoi, ResolutionRoi& statsRoi);
    StaticGraphStatus undoSensorCropandScale(SensorRoi& sensor_roi);
    StaticGraphStatus sensorCropOrScaleExist(bool& sensor_crop_or_scale_exist);
    StaticGraphStatus undoSensorScaleRipAngle(int32_t& rip_angle);
protected:
    StaticGraphStatus updateRunKernelPassThrough(StaticGraphRunKernel* runKernel, uint32_t width, uint32_t height);
    StaticGraphStatus updateRunKernelResolutionHistory(StaticGraphRunKernel* runKernel, StaticGraphRunKernel* prevRunKernel, bool updateResolution = true);

    IStaticGraphConfig* _staticGraph;
    double _widthIn2OutScale = 1;
    double _heightIn2OutScale = 1;

    double _sensorHorizontalScaling = 1.0;
    double _sensorVerticalScaling = 1.0;
    size_t _sensorHorizontalCropLeft;
    size_t _sensorHorizontalCropRight;
    size_t _sensorVerticalCropTop;
    size_t _sensorVerticalCropBottom;

private:
    StaticGraphStatus initRunKernelCoord(GraphResolutionConfiguratorKernelRole role, RunKernelCoords& coord);
    StaticGraphStatus initOutputRunKernelCoord(RunKernelCoords& coord);
    StaticGraphStatus initKernelCoordsForUpdate();
    StaticGraphStatus findRunKernel(uint32_t kernelUuid, RunKernelCoords& coord);

    StaticGraphRunKernel* getRunKernel(RunKernelCoords& coord);
#if SUPPORT_KEY_RESOLUTIONS == 1
    StaticGraphStatus getZoomKeyResolutionIndex(ZoomKeyResolutions* zoomKeyResolutions, SensorRoi sensorRoi, uint32_t& selectedIndex);
#endif
    StaticGraphStatus updateRunKernelOfScalers(bool fromInput, SensorRoi& roi);

    StaticGraphStatus updateRunKernelDownScaler(StaticGraphRunKernel* runKernel, SensorRoi& roi, uint32_t inputWidth, uint32_t inputHeight,
        uint32_t outputWidth, uint32_t outputHeight, StaticGraphKernelResCrop* originalScalerCrop);
    StaticGraphStatus adjustDownscalerCrop(StaticGraphKernelRes* scalerResInfo);
    StaticGraphStatus updateRunKernelUpScaler(StaticGraphRunKernel* runKernel, uint32_t inputWidth, uint32_t inputHeight,
        uint32_t outputWidth, uint32_t outputHeight, uint32_t& upscalerActualInputWidth, uint32_t& upscalerActualInputHeight,
        uint32_t& upscalerActualOutputWidth, uint32_t& upscalerActualOutputHeight);
    StaticGraphStatus updateRunKernelFinalCropper(StaticGraphRunKernel* runKernel, uint32_t inputWidth, uint32_t inputHeight,
        uint32_t outputWidth, uint32_t outputHeight);
    StaticGraphStatus updateCroppingScaler(StaticGraphRunKernel* downscalerRunKernel, StaticGraphRunKernel* upscalerRunKernel);

    RunKernelCoords _downscalerRunKernelCoord;
    RunKernelCoords _upscalerRunKernelCoord;
    RunKernelCoords _cropperRunKernelCoord;
    RunKernelCoords _outputRunKernelCoord;
    std::vector<RunKernelCoords> _kernelsForUpdate;

    StaticGraphKernelResCrop _originalCropOfFinalCropper = { 0,0,0,0 };
    StaticGraphKernelResCrop _originalCropInputToScaler = {0,0,0,0};
    StaticGraphKernelResCrop _originalCropScalerToOutput = { 0,0,0,0 };
};

class Ipu8FragmentsConfigurator;

class Ipu8GraphResolutionConfigurator : public GraphResolutionConfigurator
{
public:
    Ipu8GraphResolutionConfigurator(IStaticGraphConfig* staticGraph);
    ~Ipu8GraphResolutionConfigurator();

    StaticGraphStatus updateStaticGraphConfig(const RegionOfInterest& roi, bool isCenteredZoom);

    virtual StaticGraphStatus getInputRoiForOutput(const ResolutionRoi& roi, const HwSink hwSink, SensorRoi& sensorRoi);

    // Calculate ROI in sensor dimensions. Resolution ROI is given relative to *final* (zoomed) output ROI
    // This function is used for statistics output only
    virtual StaticGraphStatus getStatsRoiFromSensorRoi(const SensorRoi& sensorRoi, ResolutionRoi& statsRoi);

private:
    StaticGraphStatus initRunKernel(GraphResolutionConfiguratorKernelRole role, StaticGraphRunKernel*& runKernel);
    StaticGraphStatus initRunKernel(uint32_t kernelUuid, StaticGraphRunKernel*& runKernel);
    StaticGraphStatus initOutputRunKernel();
    StaticGraphStatus initKernelsForUpdate();

    // Calculate ROI in dimensions of pipe downscaler input.
    StaticGraphStatus getDownscalerInputRoi(const RegionOfInterest& userRoi, ResolutionRoi& pipeInputRoi);

    StaticGraphStatus updateRunKernelOfScalers(ResolutionRoi& roi);

    StaticGraphStatus updateRunKernelDownScaler(StaticGraphRunKernel* runKernel, ResolutionRoi& roi, uint32_t& outputWidth, uint32_t& outputHeight);
    StaticGraphStatus updateRunKernelUpScaler(StaticGraphRunKernel* runKernel, ResolutionRoi& roi, StaticGraphKernelResCrop& cropperKernelCrop, uint32_t outputWidth, uint32_t outputHeight);
    StaticGraphStatus updateRunKernelCropper(StaticGraphRunKernel* runKernel, ResolutionRoi& roi, uint32_t inputWidth, uint32_t inputHeight,
        uint32_t outputWidth, uint32_t outputHeight, StaticGraphKernelResCrop& downscalerCropHist);
    StaticGraphStatus updateRunKernelSmurf(StaticGraphRunKernel* smurfRunKernel, StaticGraphRunKernel* deviceRunKernel);

    StaticGraphStatus SanityCheck();
    StaticGraphStatus SanityCheckCrop(StaticGraphKernelResCrop* crop);

    uint32_t _upscalerStepW = 1;
    uint32_t _upscalerStepH = 1;

    StaticGraphKernelResCrop _originalCropOfDownScaler = { 0,0,0,0 };
    StaticGraphKernelResCrop _originalCropOfEspaCropper = { 0,0,0,0 };
    StaticGraphKernelResCrop _originalCropOfOutput = { 0,0,0,0 };
    StaticGraphKernelResCrop _originaHistoryOfOutput = { 0,0,0,0 };

    StaticGraphRunKernel* _downscalerRunKernel;
    StaticGraphRunKernel* _cropperRunKernel;
    StaticGraphRunKernel* _upscalerRunKernel;
    StaticGraphRunKernel* _outputRunKernel;
    std::vector<StaticGraphRunKernel*> _kernelsForUpdateAfterCropper;
    std::vector<StaticGraphRunKernel*> _kernelsForUpdateAfterUpscaler;
    std::vector<std::pair<StaticGraphRunKernel*, StaticGraphRunKernel*>> _smurfKernels;

    // For striping
    OuterNode* _node = nullptr;
    Ipu8FragmentsConfigurator* _fragmentsConfigurator = nullptr;
};
