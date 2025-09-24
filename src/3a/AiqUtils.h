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
#include "IntelCCATypes.h"

namespace icamera {

#define MIN_TONEMAP_POINTS 64

/*!> Top limit for the RGBS grid size */
static const unsigned int MAX_AE_GRID_SIZE = 2048;
/*!> Number of leds AEC algorithm provides output for */
static const unsigned int MAX_EXPOSURES_NUM = 3;
static const unsigned int NUM_FLASH_LEDS = 1;
static const unsigned int MAX_GAMMA_LUT_SIZE = 2048;
static const unsigned int MAX_TONEMAP_LUT_SIZE = 2048;

static const unsigned int MAX_STATISTICS_WIDTH = BXT_RGBS_GRID_MAX_WIDTH;
static const unsigned int MAX_STATISTICS_HEIGHT = BXT_RGBS_GRID_MAX_HEIGHT;

static const unsigned int MAX_LSC_WIDTH = 100;
static const unsigned int MAX_LSC_HEIGHT = 100;

static const int MAX_BAYER_ORDER_NUM = 4;

/**
 *  The normalized awb gain range is (4.0, 1.0) which is just experimental.
 */
static const int AWB_GAIN_NORMALIZED_START = 4.0;
static const int AWB_GAIN_NORMALIZED_END = 1.0;
static const int AWB_GAIN_RANGE_NORMALIZED = AWB_GAIN_NORMALIZED_END - AWB_GAIN_NORMALIZED_START;

static const float AWB_GAIN_MIN = 0;
static const float AWB_GAIN_MAX = 255;
static const float AWB_GAIN_RANGE_USER = AWB_GAIN_MAX - AWB_GAIN_MIN;

namespace AiqUtils {
void dumpAeResults(const cca::cca_ae_results& aeResult);
void dumpAfResults(const cca::cca_af_results& afResult);
void dumpAwbResults(const cca::cca_awb_results& awbResult);
void dumpGbceResults(const cca::cca_gbce_params& gbceResult);
void dumpPaResults(const cca::cca_pa_params& paResult);
void dumpSaResults(const cca::cca_sa_results& saResult);

int convertError(ia_err iaErr);
void convertToAiqFrameParam(const SensorFrameParams& sensor, ia_aiq_frame_params& aiq);

camera_coordinate_t convertCoordinateSystem(const camera_coordinate_system_t& srcSystem,
                                            const camera_coordinate_system_t& dstSystem,
                                            const camera_coordinate_t& srcCoordinate);
camera_coordinate_t convertToIaCoordinate(const camera_coordinate_system_t& srcSystem,
                                          const camera_coordinate_t& srcCoordinate);
camera_window_t convertToIaWindow(const camera_coordinate_system_t& srcSystem,
                                  const camera_window_t& srcWindow);
float normalizeAwbGain(int gain);
int convertToUserAwbGain(float normalizedGain);
float convertSpeedModeToTime(camera_converge_speed_t mode);
// HDR_FEATURE_S
float convertSpeedModeToTimeForHDR(camera_converge_speed_t mode);
// HDR_FEATURE_E

ia_aiq_frame_use convertFrameUsageToIaFrameUsage(int frameUsage);

void applyTonemapGamma(float gamma, cca::cca_gbce_params* results);
void applyTonemapSRGB(cca::cca_gbce_params* results);
void applyTonemapREC709(cca::cca_gbce_params* results);
void applyTonemapCurve(const camera_tonemap_curves_t& curves, cca::cca_gbce_params* results);
void applyAwbGainForTonemapCurve(const camera_tonemap_curves_t& curves,
                                 cca::cca_awb_results* results);

/*!
 * \brief Resize a 2D array with linear interpolation.
 *
 * @param[in,out]
 *  in a_src                pointer to input array (width-major)
 *  in a_src_w              width of the input array
 *  in a_src_h              height of the input array
 *  in a_dst                pointer to output array (width-major)
 *  in a_dst_w              width of the output array
 *  in a_dst_h              height of the output array
 */
template <typename T> int resize2dArray(
    const T* a_src, int a_src_w, int a_src_h,
    T* a_dst, int a_dst_w, int a_dst_h) {
    int i, j, step_size_w, step_size_h, rounding_term;

    if (a_src_w < 2 || a_dst_w < 2 || a_src_h < 2 || a_dst_h < 2) {
        return  -1;
    }

    // Resize a 2D array with linear interpolation
    // For some cases, we need to upscale or downscale a 2D array.
    // For example, Android requests lensShadingMapSize must be smaller than 64*64,
    // but for some sensors, the lens shading map is bigger than this, so need to do resize.
    /* Value of 8 is maximum in order to avoid overflow with 16-bit inputs */
    const unsigned int FRAC_BITS_CURR_LOC = 8;
    const unsigned int FRAC_BASE = (1 << FRAC_BITS_CURR_LOC);

    step_size_w = ((a_src_w - 1) << FRAC_BITS_CURR_LOC) / (a_dst_w - 1);
    step_size_h = ((a_src_h - 1) << FRAC_BITS_CURR_LOC) / (a_dst_h - 1);
    rounding_term = (1 << (2 * FRAC_BITS_CURR_LOC - 1));
    for (j = 0; j < a_dst_h; ++j) {
        unsigned int curr_loc_h, curr_loc_lower_h;
        curr_loc_h = j * step_size_h;
        curr_loc_lower_h = (curr_loc_h > 0) ? (curr_loc_h - 1) >> FRAC_BITS_CURR_LOC : 0;

        for (i = 0; i < a_dst_w; ++i) {
            unsigned int curr_loc_w, curr_loc_lower_w;

            curr_loc_w = i * step_size_w;
            curr_loc_lower_w = (curr_loc_w > 0) ? (curr_loc_w - 1) >> FRAC_BITS_CURR_LOC : 0;

            a_dst[a_dst_w * j + i] =
                (a_src[curr_loc_lower_w + curr_loc_lower_h * a_src_w]  *
                        (((curr_loc_lower_w + 1) << FRAC_BITS_CURR_LOC) - curr_loc_w) *
                        (((curr_loc_lower_h + 1) << FRAC_BITS_CURR_LOC) - curr_loc_h) +
                a_src[curr_loc_lower_w + 1 + curr_loc_lower_h * a_src_w] *
                        (curr_loc_w-((curr_loc_lower_w) << FRAC_BITS_CURR_LOC))   *
                        (((curr_loc_lower_h + 1) << FRAC_BITS_CURR_LOC) - curr_loc_h) +
                a_src[curr_loc_lower_w + (curr_loc_lower_h + 1) * a_src_w]  *
                        (((curr_loc_lower_w + 1) << FRAC_BITS_CURR_LOC) - curr_loc_w) *
                        (curr_loc_h - ((curr_loc_lower_h) << FRAC_BITS_CURR_LOC)) +
                a_src[curr_loc_lower_w + 1 + (curr_loc_lower_h + 1) * a_src_w] *
                        (curr_loc_w - ((curr_loc_lower_w) << FRAC_BITS_CURR_LOC))   *
                        (curr_loc_h - ((curr_loc_lower_h) << FRAC_BITS_CURR_LOC))
                + rounding_term) / (FRAC_BASE * FRAC_BASE);
        }
    }

    return 0;
}

template int resize2dArray<float>(
    const float* a_src, int a_src_w, int a_src_h,
    float* a_dst, int a_dst_w, int a_dst_h);
template int resize2dArray<unsigned short>(
    const unsigned short* a_src, int a_src_w, int a_src_h,
    unsigned short* a_dst, int a_dst_w, int a_dst_h);
template int resize2dArray<int>(
    const int* a_src, int a_src_w, int a_src_h,
    int* a_dst, int a_dst_w, int a_dst_h);

float calculateHyperfocalDistance(const cca::cca_cmc &cmc);
}  // namespace AiqUtils
}  // namespace icamera
