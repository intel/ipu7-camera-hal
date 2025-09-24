/*
 * Copyright (C) 2015-2025 Intel Corporation.
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

#include "CameraTypes.h"
#include "ParamDataType.h"

namespace icamera {

#define DEFAULT_LSC_GRID_SIZE (64 * 64)
#define DEFAULT_TONEMAP_CURVE_POINT_NUM 2048

// HDR_FEATURE_S
typedef enum {
    AEC_SCENE_NONE,
    AEC_SCENE_HDR,
    AEC_SCENE_ULL
} aec_scene_t;
// HDR_FEATURE_E

static const int MAX_CUSTOM_CONTROLS_PARAM_SIZE = 1024;
typedef struct {
    char data[MAX_CUSTOM_CONTROLS_PARAM_SIZE];
    unsigned int length;
} custom_aic_param_t;

/*
 * aiq related parameters
 */
struct aiq_parameter_t {
    frame_usage_mode_t frameUsage;
    camera_ae_mode_t aeMode;
    bool aeForceLock;
    camera_awb_mode_t awbMode;
    bool awbForceLock;
    camera_af_mode_t afMode;
    camera_af_trigger_t afTrigger;
    camera_scene_mode_t sceneMode;
    int64_t manualExpTimeUs;
    float manualGain;
    int32_t manualIso;
    int32_t evSetting;
    float evShift;
    float fps;
    camera_range_t aeFpsRange;
    camera_antibanding_mode_t antibandingMode;
    camera_range_t cctRange;
    camera_coordinate_t whitePoint;
    camera_awb_gains_t awbManualGain;
    camera_awb_gains_t awbGainShift;
    camera_color_transform_t manualColorMatrix;
    camera_color_gains_t manualColorGains;
    camera_window_list_t aeRegions;
    camera_window_list_t afRegions;
    camera_blc_area_mode_t blcAreaMode;
    camera_converge_speed_mode_t aeConvergeSpeedMode;
    camera_converge_speed_mode_t awbConvergeSpeedMode;
    camera_converge_speed_t aeConvergeSpeed;
    camera_converge_speed_t awbConvergeSpeed;
    int run3ACadence;
    camera_ae_distribution_priority_t aeDistributionPriority;
    custom_aic_param_t customAicParam;
    camera_yuv_color_range_mode_t yuvColorRangeMode;
    camera_range_t exposureTimeRange;
    camera_range_t sensitivityGainRange;
    camera_video_stabilization_mode_t videoStabilizationMode;
    camera_resolution_t resolution;
    camera_ldc_mode_t ldcMode;
    camera_rsc_mode_t rscMode;
    camera_flip_mode_t flipMode;
    float digitalZoomRatio;
    camera_range_t evRange;
    camera_rational_t evStep;

    TuningMode tuningMode;

    int lensPosition;
    unsigned long long lensMovementStartTimestamp;
    camera_makernote_mode_t makernoteMode;
    float minFocusDistance;
    float focusDistance;
    camera_shading_mode_t shadingMode;
    camera_lens_shading_map_mode_type_t lensShadingMapMode;
    camera_coordinate_t lensShadingMapSize;

    camera_tonemap_mode_t tonemapMode;
    camera_tonemap_preset_curve_t tonemapPresetCurve;
    float tonemapGamma;
    camera_tonemap_curves_t tonemapCurves;
    float tonemapCurveMem[DEFAULT_TONEMAP_CURVE_POINT_NUM * 3];  // r, g, b
    camera_test_pattern_mode_t testPatternMode;
    bool callbackRgbs;
    bool callbackTmCurve;
    camera_power_mode_t powerMode;
    int64_t totalExposureTarget;

    camera_window_list_t awbRegions;
    camera_effect_mode_t effectMode;
    aiq_parameter_t() { reset(); }
    void reset();
    void dump();
};

} /* namespace icamera */
