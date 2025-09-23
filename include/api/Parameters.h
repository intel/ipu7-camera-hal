/*
 * Copyright (C) 2013 The Android Open Source Project
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

/*
 *
 * Filename: Parameters.h
 *
 * ------------------------------------------------------------------------------
 * REVISION HISTORY
 *     Version        0.1        Initialize camera parameters API
 *     Version        0.2        Merge all the types to this file
 *     Version        0.3        Add AE compensation related APIs.
 *     Version        0.31       Add manual color matrix APIs.
 *     Version        0.32       Add manual AE/AWB converge speed APIs.
 *     Version        0.33       Add timestamp variable in camera_buffer_t
 *     Version        0.34       Add AE window weight grid API
 *     Version        0.40       Add Data Structure for HAL 3.3
 *     Version        0.41       Add API getSupportedAeExposureTimeRange
 *                               Add API getSupportedAeGainRange
 *     Version        0.42       Add API updateDebugLevel
 *     Version        0.43       Add API set and get deinterlace mode
 *     Version        0.44       Add API set and get gps processing method
 *                               Add API set and get focal length
 *     Version        0.45       Add get supported static metadata APIs
 *     Version        0.50       Support low level ISP feature control
 *     Version        0.51       Support getting supported ISP control feature list
 *     Version        0.52       Add API set and get awb result
 *     Version        0.53       Add API to get/set enabled ISP control feature list
 *     Version        0.54       Add API to get/set fisheye dewarping mode
 *     Version        0.55       Add API to get/set LTM tuning data
 *     Version        0.56       Add API to get/set LDC/RSC/digital zoom ratio
 *     Version        0.57       Add API to support WFOV mode, including get WFOV mode, get sensor
 *                               mount type
 *                               set/get view projection, rotation and fine adjustments.
 *     Version        0.58       Add API to get/set 3A state, and lens state.
 *     Version        0.59       Add API to get/set AE/AWB lock
 *     Version        0.60       Add API to support get/set camera rotation in WFOV mode.
 *     Version        0.61       Add API to support vertical and horizontal flip.
 *     Version        0.62       Add API to support 3A cadence.
 *     Version        0.63       Add API to enable/disable MONO Downscale feature.
 *     Version        0.64       Add callback message definition.
 *     Version        0.65       Add API to support OUTPUT/INPUT streams.
 *     Version        0.66       modifies callback message definition.
 *     Version        0.67       Add API to support lens.focusDistance and lens.focalLength
 *     Version        0.68       Add API to support shading map.
 *     Version        0.69       Add API to support statistics lens shading map control.
 *     Version        0.70       Add API to support tonemap.
 *     Version        0.71       Add API to support OPAQUE RAW usage for RAW reprocessing.
 *     Version        0.72       Add streamType into supported_stream_config_t.
 *     Version        0.73       Remove supported_stream_config_t structure.
 *     Version        0.74       Add API to support sensor iso.
 *     Version        0.75       Add API to support lens static info about apertures,
 *                               filter densities, min focus densities and hyperfocal distance.
 *     Version        0.76       Remove the marco for lsc grid size
 *     Version        0.77       Add API to support capture intent
 *     Version        0.78       Add API to support edge enhancement
 *     Version        0.79       Add API to support set flags to callback rgbs statistics
 *     Version        0.80       Add API to support set flags to callback tone map curve
 *     Version        0.81       Remove API for unused WFOV/ISP_CONTROL features
 *
 * ------------------------------------------------------------------------------
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <vector>
#include <set>
#include <stdint.h>
#include <cstddef>

#include "ParamDataType.h"

namespace icamera {

/**
 * \class Parameters
 *
 * \brief
 *   Manage parameter's data structure, and provide set and get parameters
 *
 * This class provides a thread safe management to internal parameter's data
 * structure, and helps client to easily set parameters to and get parameters
 * from camera device.
 *
 * \version 0.1
 *
 */
class Parameters {
 public:
    Parameters();
    Parameters(const Parameters& other);
    Parameters& operator=(const Parameters& other);
    ~Parameters();
    /**
     * \brief Merge and update current parameter with other
     *
     * \param[in] Parameters other: parameter source
     *
     * \return void
     */
    void merge(const Parameters& other);

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported fps range list
     *
     * \param[out] camera_range_array_t& ranges
     *
     * \return 0 if fps range supported, otherwise non-0 value is returned.
     */
    int getSupportedFpsRange(camera_range_array_t& ranges) const;

    /**
     * \brief Get supported Stream Config list
     *
     * \param[out] stream_array_t& config
     *
     * \return 0 if Stream Configs supported, otherwise non-0 value is returned.
     */
    int getSupportedStreamConfig(stream_array_t& config) const;

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported sensor exposure time range (microsecond)
     *
     * \param[out] camera_range_t& range
     *
     * \return 0 if it is supported, otherwise non-0 value is returned.
     */
    int getSupportedSensorExposureTimeRange(camera_range_t& range) const;

    // Belows are camera capability related parameters operations
    /**
     * \brief Get supported sensor sensitivity time range
     *
     * \param[out] camera_range_t& range
     *
     * \return 0 if it is supported, otherwise non-0 value is returned.
     */
    int getSupportedSensorSensitivityRange(camera_range_t& range) const;

    /**
     * \brief Get supported feature list.
     *
     * Camera application MUST check if the feature is supported before trying to enable it.
     * Otherwise the behavior is undefined currently, HAL may just ignore the request.
     *
     * \param[out] camera_features_list_t& features: All supported feature will be filled in
     * "features"
     *
     * \return: If no feature supported, features will be empty
     */
    int getSupportedFeatures(camera_features_list_t& features) const;

    /**
     * \brief Get ae compensation range supported by camera device
     *
     * \param[out] camera_range_t& evRange
     *
     * \return 0 if ae compensation supported, non-0 or evRange equals [0, 0] means ae compensation
     * not supported.
     */
    int getAeCompensationRange(camera_range_t& evRange) const;

    /**
     * \brief Get ae compensation step supported by camera device
     *
     * Smallest step by which the exposure compensation can be changed.
     * This is the unit for setAeCompensation. For example, if this key has
     * a value of `1/2`, then a setting of `-2` for setAeCompensation means
     * that the target EV offset for the auto-exposure routine is -1 EV.
     *
     * One unit of EV compensation changes the brightness of the captured image by a factor
     * of two. +1 EV doubles the image brightness, while -1 EV halves the image brightness.
     *
     * \param[out] camera_rational_t& evStep
     *
     * \return 0 if ae compensation supported, non-0 means ae compensation not supported.
     */
    int getAeCompensationStep(camera_rational_t& evStep) const;

    /**
     * \brief Get supported manual exposure time range
     *
     * Different sensors or same sensor in different settings may have different supported exposure
     * time range, so camera application needs to use this API to check if the user's settings is
     * in the supported range, if application pass an out of exposure time, HAL will clip it
     * according to this supported range.
     *
     * \param[out] vector<camera_ae_exposure_time_range_t>& etRanges
     *
     * \return 0 if exposure time range is filled by HAL.
     */
    int getSupportedAeExposureTimeRange(
        std::vector<camera_ae_exposure_time_range_t>& etRanges) const;

    /**
     * \brief Get supported manual sensor gain range
     *
     * Different sensors or same sensor in different settings may have different supported sensor
     * gain range, so camera application needs to use this API to check if the user's settings is
     * in the supported range, if application pass an out of range gain, HAL will clip it according
     * to this supported range.
     *
     * \param[out] vector<camera_ae_gain_range_t>& gainRanges
     *
     * \return 0 if exposure time range is filled by HAL.
     */
    int getSupportedAeGainRange(std::vector<camera_ae_gain_range_t>& gainRanges) const;

    // Belows are AE related parameters operations
    /**
     * \brief Set exposure mode(auto/manual).
     *
     * "auto" means 3a algorithm will control exposure time and gain automatically.
     * "manual" means user can control exposure time or gain, or both of them.
     * Under manual mode, if user only set one of exposure time or gain, then 3a algorithm
     * will help to calculate the other one.
     *
     * \param[in] camera_ae_mode_t aeMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeMode(camera_ae_mode_t aeMode);

    /**
     * \brief Get exposure mode
     *
     * \param[out] aeMode: Currently used ae mode will be set to aeMode if 0 is returned.
     *
     * \return 0 if exposure mode was set, otherwise non-0 value is returned.
     */
    int getAeMode(camera_ae_mode_t& aeMode) const;

    /**
     * \brief Set AE state.
     *
     * \param[in] camera_ae_state_t aeState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeState(camera_ae_state_t aeState);

    /**
     * \brief Get AE state
     *
     * \param[out] aeState: Currently AE state will be set to aeState if 0 is returned.
     *
     * \return 0 if AE state was set, otherwise non-0 value is returned.
     */
    int getAeState(camera_ae_state_t& aeState) const;

    /**
     * \brief Set ae lock
     *
     * \param[in] bool lock
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeLock(bool lock);

    /**
     * \brief Get ae lock
     *
     * \param[out] bool lock
     *
     * \return 0 if lock was set, otherwise non-0 value is returned.
     */
    int getAeLock(bool& lock) const;

    /**
     * \brief Get supported video stabilization mode
     *
     * Camera application MUST check if the video stabilization mode is supported before trying
     * to enable it. Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedModes: All supported video stabilization mode will be filled in
     * "supportedModes"
     *
     * \return: If no mode supported, supportedModes will be empty
     */
    int getSupportedVideoStabilizationMode(camera_video_stabilization_list_t& supportedModes) const;

    /**
     * \brief Get supported ae mode
     *
     * Camera application MUST check if the ae mode is supported before trying to enable it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedAeModes: All supported ae mode will be filled in "supportedAeModes"
     *
     * \return: If no ae mode supported, supportedAeModes will be empty
     */
    int getSupportedAeMode(std::vector<camera_ae_mode_t>& supportedAeModes) const;

    /**
     * \brief Get supported awb mode
     *
     * Camera application MUST check if the awb mode is supported before trying to enable it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedAwbModes: All supported awb mode will be filled in "supportedAwbModes"
     *
     * \return: If no awb mode supported, supportedAwbModes will be empty
     */
    int getSupportedAwbMode(std::vector<camera_awb_mode_t>& supportedAwbModes) const;

    /**
     * \brief Get supported af mode
     *
     * Camera application MUST check if the af mode is supported before trying to enable it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedAfModes: All supported af mode will be filled in "supportedAfModes"
     *
     * \return: If no af mode supported, supportedAfModes will be empty
     */
    int getSupportedAfMode(std::vector<camera_af_mode_t>& supportedAfModes) const;

    /**
     * \brief Get supported scene mode
     *
     * Camera application MUST check if the scene mode is supported before trying to enable it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedSceneModes: All supported scene mode will be filled in
     * "supportedSceneModes"
     *
     * \return: If no scene mode supported, supportedSceneModes will be empty
     */
    int getSupportedSceneMode(std::vector<camera_scene_mode_t>& supportedSceneModes) const;

    /**
     * \brief Get supported antibanding mode
     *
     * Camera application MUST check if the antibanding mode is supported before trying to enable
     * it. Otherwise one error occurring, HAL may just ignore the request.
     *
     * \param[out] supportedAntibindingModes: All supported scene mode will be filled in
     * "supportedAntibindingModes"
     *
     * \return: If no antibanding mode supported, supportedAntibindingModes will be empty
     */
    int getSupportedAntibandingMode(
        std::vector<camera_antibanding_mode_t>& supportedAntibindingModes) const;

    /**
     * \brief Get if ae lock is available
     *
     * Camera application MUST check if ae lock is supported before trying to lock it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \return: true if lock is supported, false if not
     */
    bool getAeLockAvailable() const;

    /**
     * \brief Get if awb lock is available
     *
     * Camera application MUST check if awb lock is supported before trying to lock it.
     * Otherwise one error occurring, HAL may just ignore the request.
     *
     * \return: true if lock is supported, false if not
     */
    bool getAwbLockAvailable() const;

    /**
     * \brief Set AE region
     *
     * Current only fisrt region can take effect when BLC mode is BLC_AREA_MODE_ON;
     * if BLC_AREA_MODE_OFF, AE region function will be disabled.
     *
     * \param[in] camera_window_list_t aeRegions
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeRegions(camera_window_list_t aeRegions);

    /**
     * \brief Get AE region
     *
     * \param[out] camera_window_list_t aeRegions
     *
     * \return 0 if aeRegions were set, otherwise non-0 value is returned.
     */
    int getAeRegions(camera_window_list_t& aeRegions) const;

    /**
     * \brief Set exposure time whose unit is microsecond(us).
     *
     * The exposure time only take effect when ae mode set to manual.
     *
     * \param[in] int64_t exposureTime
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setExposureTime(int64_t exposureTime);

    /**
     * \brief Get exposure time whose unit is microsecond(us).
     *
     * \param[out] int64_t& exposureTime: exposure time if be set in exposureTime if 0 is returned.
     *
     * \return 0 if exposure time was set, non-0 means no exposure time was set.
     */
    int getExposureTime(int64_t& exposureTime) const;

    /**
     * \brief Set sensor gain whose unit is db.
     * The sensor gain only take effect when ae mode set to manual.
     *
     * \param[in] float gain
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityGain(float gain);

    /**
     * \brief Get sensor gain whose unit is db.
     *
     * \param[out] float gain
     *
     * \return 0 if sensor gain was set, non-0 means no sensor gain was set.
     */
    int getSensitivityGain(float& gain) const;

    /**
     * \brief Set sensor ISO, will overwrite value of setSensitivityGain.
     * The sensor ISO only take effect when ae mode set to manual.
     *
     * \param[in] int32 iso
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityIso(int32_t iso);

    /**
     * \brief Get sensor iso.
     *
     * \param[out] int32 iso
     *
     * \return 0 if sensor iso was set, non-0 means no sensor iso was set.
     */
    int getSensitivityIso(int& iso) const;

    /**
     * \brief Set ae compensation whose unit is compensation step.
     *
     * The adjustment is measured as a count of steps, with the
     * step size defined ae compensation step and the allowed range by ae compensation range.
     *
     * For example, if the exposure value (EV) step is 0.333, '6'
     * will mean an exposure compensation of +2 EV; -3 will mean an
     * exposure compensation of -1 EV. One EV represents a doubling of image brightness.
     *
     * In the event of exposure compensation value being changed, camera device
     * may take several frames to reach the newly requested exposure target.
     *
     * \param[in] int ev
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeCompensation(int ev);

    /**
     * \brief Get ae compensation whose unit is compensation step.
     *
     * \param[out] int ev
     *
     * \return 0 if ae compensation was set, non-0 means no ae compensation was set.
     */
    int getAeCompensation(int& ev) const;

    /**
     * \brief Set frame rate
     *
     * \param[in] float fps
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFrameRate(float fps);

    /**
     * \brief Get frame rate
     *
     * \param[out] float& fps
     *
     * \return 0 if frame rate was set, otherwise non-0 value is returned.
     */
    int getFrameRate(float& fps) const;

    /**
     * \brief Set antibanding mode
     *
     * \param[in] camera_antibanding_mode_t bandingMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAntiBandingMode(camera_antibanding_mode_t bandingMode);

    /**
     * \brief Get antibanding mode
     *
     * \param[out] camera_antibanding_mode_t& bandingMode
     *
     * \return 0 if antibanding mode was set, otherwise non-0 value is returned.
     */
    int getAntiBandingMode(camera_antibanding_mode_t& bandingMode) const;

    /**
     * \brief Set AE distribution priority.
     *
     * \param[in] camera_ae_distribution_priority_t priority: the AE distribution priority to be
     * set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeDistributionPriority(camera_ae_distribution_priority_t priority);

    /**
     * \brief Get AE distribution priority.
     *
     * \param[out] camera_ae_distribution_priority_t priority: the AE distribution priority.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeDistributionPriority(camera_ae_distribution_priority_t& priority) const;

    /**
     * \brief set exposure time range
     *
     * \param[in] camera_range_t: the exposure time range to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setExposureTimeRange(camera_range_t exposureTimeRange);

    /**
     * \brief get exposure time range
     *
     * \param[out] camera_range_t: the exposure time had been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getExposureTimeRange(camera_range_t& exposureTimeRange) const;

    /**
     * \brief set sensitivity gain range
     *
     * \param[in] camera_range_t: the sensitivity gain range(the unit is db) to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSensitivityGainRange(camera_range_t sensitivityGainRange);

    /**
     * \brief get sensitivity gain range
     *
     * \param[out] camera_range_t: the sensitivity gain(the unit is db) had been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getSensitivityGainRange(camera_range_t& sensitivityGainRange) const;

    /**
     * \brief Set weight grid mode (deprecated)
     *
     * \param[in] camera_weight_grid_mode_t weightGridMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setWeightGridMode(camera_weight_grid_mode_t weightGridMode);

    /**
     * \brief Get weight grid mode (deprecated)
     *
     * \param[out] camera_weight_grid_mode_t& weightGridMode
     *
     * \return 0 if weight grid mode was set, otherwise non-0 value is returned.
     */
    int getWeightGridMode(camera_weight_grid_mode_t& weightGridMode) const;

    /**
     * \brief Set BLC (backlight compensation) area mode
     *
     * \param[in] camera_blc_area_mode_t blcAreaMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setBlcAreaMode(camera_blc_area_mode_t blcAreaMode);

    /**
     * \brief Get BLC (backlight compensation) area mode
     *
     * \param[out] camera_blc_area_mode_t& blcAreaMode
     *
     * \return 0 if BLC area mode was set, otherwise non-0 value is returned.
     */
    int getBlcAreaMode(camera_blc_area_mode_t& blcAreaMode) const;

    int setFpsRange(camera_range_t fpsRange);
    int getFpsRange(camera_range_t& fpsRange) const;

    // Belows are AWB related parameters operations
    /**
     * \brief Set white balance mode
     *
     * White balance mode could be one of totally auto, preset cct range, customized cct range,
     * customized white area, customize gains.
     *
     * \param[in] camera_awb_mode_t awbMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbMode(camera_awb_mode_t awbMode);

    /**
     * \brief Get white balance mode currently used.
     *
     * \param[out] camera_awb_mode_t& awbMode
     *
     * \return 0 if awb mode was set, non-0 means no awb mode was set.
     */
    int getAwbMode(camera_awb_mode_t& awbMode) const;

    /**
     * \brief Set AWB state.
     *
     * \param[in] camera_awb_state_t awbState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbState(camera_awb_state_t awbState);

    /**
     * \brief Get AWB state
     *
     * \param[out] awbState: Currently AWB state will be set to awbState if 0 is returned.
     *
     * \return 0 if AWB state was set, otherwise non-0 value is returned.
     */
    int getAwbState(camera_awb_state_t& awbState) const;

    /**
     * \brief Set awb lock
     *
     * \param[in] bool lock
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbLock(bool lock);

    /**
     * \brief Get awb lock
     *
     * \param[out] bool lock
     *
     * \return 0 if lock was set, otherwise non-0 value is returned.
     */
    int getAwbLock(bool& lock) const;

    /**
     * \brief Set customized cct range.
     *
     * Customized cct range only take effect when awb mode is set to AWB_MODE_MANUAL_CCT_RANGE
     *
     * \param[in] camera_range_t cct range, which specify min and max cct for 3a algorithm to use.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbCctRange(camera_range_t cct);

    /**
     * \brief Get customized cct range currently used.
     *
     * \param[out] camera_range_t& cct range
     *
     * \return 0 if cct range was set, non-0 means no cct range was set.
     */
    int getAwbCctRange(camera_range_t& cct) const;

    /**
     * \brief Set customized awb gains.
     *
     * Customized awb gains only take effect when awb mode is set to AWB_MODE_MANUAL_GAIN
     *
     * The range of each gain is (0, 255).
     *
     * \param[in] camera_awb_gains_t awb gains, which specify r,g,b gains for overriding awb result.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbGains(camera_awb_gains_t awbGains);

    /**
     * \brief Get customized awb gains currently used.
     *
     * \param[out] camera_awb_gains_t& awb gains
     *
     * \return 0 if awb gain was set, non-0 means no awb gain was set.
     */
    int getAwbGains(camera_awb_gains_t& awbGains) const;

    /**
     * \brief Set awb gain shift.
     *
     * Customized awb gain shift only take effect when awb mode is NOT set to AWB_MODE_MANUAL_GAIN
     *
     * The range of each gain shift is (0, 255).
     *
     * \param[in] camera_awb_gains_t awb gain shift, which specify r,g,b gains for updating awb
     * result.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbGainShift(camera_awb_gains_t awbGainShift);

    /**
     * \brief Get customized awb gains shift currently used.
     *
     * \param[out] camera_awb_gains_t& awb gain shift
     *
     * \return 0 if awb gain shift was set, non-0 means no awb gain shift was set.
     */
    int getAwbGainShift(camera_awb_gains_t& awbGainShift) const;

    /**
     * \brief Set awb result.
     *
     * \param[in] data: The data used to override awb result.
     *
     * Note: data is allocated by the caller and NULL is for erasing the param.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbResult(void* data);

    /**
     * \brief Get awb result currently used.
     *
     * \param[out] data: the awb result pointer to user
     *
     * Note: data is allocated by the caller and it must never be NULL.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbResult(void* data) const;

    /**
     * \brief Set manual white point coordinate.
     *
     * Only take effect when awb mode is set to AWB_MODE_MANUAL_WHITE_POINT.
     * The coordinate system is based on frame which is currently displayed.
     *
     * \param[in] camera_coordinate_t white point
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbWhitePoint(camera_coordinate_t whitePoint);

    /**
     * \brief Get manual white point coordinate.
     *
     * \param[out] camera_coordinate_t& white point
     *
     * \return 0 if white point was set, non-0 means no white point was set.
     */
    int getAwbWhitePoint(camera_coordinate_t& whitePoint) const;

    /**
     * \brief Set customized color transform which is a 3x3 matrix.
     *
     *  Manual color transform only takes effect when awb mode set to
     * AWB_MODE_MANUAL_COLOR_TRANSFORM.
     *
     * \param[in] camera_color_transform_t colorTransform: a 3x3 matrix for color convertion.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setColorTransform(camera_color_transform_t colorTransform);

    /**
     * \brief Get color transform matrix currently used.
     *
     * \param[out] camera_color_transform_t& color transform matrix
     *
     * \return 0 if color transform matrix was set, non-0 means no color transform matrix was set.
     */
    int getColorTransform(camera_color_transform_t& colorTransform) const;

    /**
     * \brief Set customized color correction gains which is a 4 array.
     *
     *  Manual color correction gains only takes effect when awb mode set to
     * AWB_MODE_MANUAL_COLOR_TRANSFORM.
     *
     * \param[in] camera_color_gains_t colorGains: a 4 array for color correction gains.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setColorGains(camera_color_gains_t colorGains);
    /**
     * \brief Get color correction gains currently used.
     *
     * \param[out] camera_color_gains_t& color correction gains
     *
     * \return 0 if color correction gains was set, non-0 means no color correction gains was set.
     */
    int getColorGains(camera_color_gains_t& colorGains) const;

    int setAwbRegions(camera_window_list_t awbRegions);
    int getAwbRegions(camera_window_list_t& awbRegions) const;

    // Belows are convergence speed related parameters operations
    /**
     * \brief Set customized Ae converge speed.
     *
     * \param[in] camera_converge_speed_t speed: the converge speed to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeConvergeSpeed(camera_converge_speed_t speed);

    /**
     * \brief Get customized Ae converge speed.
     *
     * \param[out] camera_converge_speed_t& speed: the converge speed been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeConvergeSpeed(camera_converge_speed_t& speed) const;

    /**
     * \brief Set customized Awb converge speed.
     *
     * \param[in] camera_converge_speed_t speed: the converge speed to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbConvergeSpeed(camera_converge_speed_t speed);

    /**
     * \brief Get customized Awb converge speed.
     *
     * \param[out] camera_converge_speed_t& speed: the converge speed been set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbConvergeSpeed(camera_converge_speed_t& speed) const;

    /**
     * \brief Set customized Ae converge speed mode.
     *
     * \param[in] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAeConvergeSpeedMode(camera_converge_speed_mode_t mode);

    /**
     * \brief Get customized Ae converge speed mode.
     *
     * \param[out] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAeConvergeSpeedMode(camera_converge_speed_mode_t& mode) const;

    /**
     * \brief Set customized Awb converge speed mode.
     *
     * \param[in] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAwbConvergeSpeedMode(camera_converge_speed_mode_t mode);

    /**
     * \brief Get customized Awb converge speed mode.
     *
     * \param[out] camera_converge_speed_mode_t mode: the converge speed mode to be set.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getAwbConvergeSpeedMode(camera_converge_speed_mode_t& mode) const;

    // Belows are ISP related parameters operations

    /**
     * \brief Set edge mode.
     *
     * \param[in] camera_edge_mode_t edgeMode: the edge mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setEdgeMode(camera_edge_mode_t edgeMode);

    /**
     * \brief Get edge mode.
     *
     * \param[out] camera_edge_mode_t mode: the edge mode to be get.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getEdgeMode(camera_edge_mode_t& edgeMode) const;

    int setNrMode(camera_nr_mode_t nrMode);
    int getNrMode(camera_nr_mode_t& nrMode) const;

    int setNrLevel(camera_nr_level_t level);
    int getNrLevel(camera_nr_level_t& level) const;

    /**
     * \brief Set YUV color range mode
     *
     * \param[in] camera_yuv_color_range_mode_t colorRange: the YUV color range mode to be set.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setYuvColorRangeMode(camera_yuv_color_range_mode_t colorRange);

    /**
     * \brief Get YUV color range mode
     *
     * \param[out] camera_yuv_color_range_mode_t colorRange: the YUV color range mode.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getYuvColorRangeMode(camera_yuv_color_range_mode_t& colorRange) const;

    /**
     * \brief Set customized effects.
     *
     * One of sharpness, brightness, contrast, hue, saturation could be controlled by this API.
     * Valid range should be [-128, 127]
     *
     * \param[in] camera_image_enhancement_t effects
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setImageEnhancement(camera_image_enhancement_t effects);

    /**
     * \brief Get customized effects.
     *
     * \param[out] effects
     *
     * \return 0 if effects was set, non-0 return value means no effects was set.
     */
    int getImageEnhancement(camera_image_enhancement_t& effects) const;

    // Belows are other parameters operations
    int setIrisMode(camera_iris_mode_t irisMode);
    int getIrisMode(camera_iris_mode_t& irisMode);

    int setIrisLevel(int level);
    int getIrisLevel(int& level);

    // HDR_FEATURE_S
    /**
     * \brief Set WDR mode
     *
     * \param[in] camera_wdr_mode_t wdrMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setWdrMode(camera_wdr_mode_t wdrMode);

    /**
     * \brief Get WDR mode currently used.
     *
     * \param[out] camera_wdr_mode_t& wdrMode
     *
     * \return 0 if awb mode was set, non-0 means no awb mode was set.
     */
    int getWdrMode(camera_wdr_mode_t& wdrMode) const;
    // HDR_FEATURE_E

    /**
     * \brief Set WDR Level
     *
     * \param[in] uint8_t level
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setWdrLevel(uint8_t level);

    /**
     * \brief Get WDR level currently used.
     *
     * \param[out] uint8_t& level
     *
     * \return 0 if get WDR level, non-0 means error.
     */
    int getWdrLevel(uint8_t& level) const;

    /**
     * \brief Set effect scene mode
     *
     * \param[in] camera_scene_mode_t: scene mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setEffectSceneMode(camera_scene_mode_t sceneMode);

    /**
     * \brief Get effect scene mode based on runtime
     *
     * \param[out] camera_scene_mode_t&: scene mode
     *
     * \return 0 if get scene mode, non-0 means error.
     */
    int getEffectSceneMode(camera_scene_mode_t& sceneMode) const;

    /**
     * \brief Set scene mode
     *
     * \param[in] camera_scene_mode_t: scene mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setSceneMode(camera_scene_mode_t sceneMode);

    /**
     * \brief Get scene mode current set by user.
     *
     * \param[out] camera_scene_mode_t&: scene mode
     *
     * \return 0 if get scene mode, non-0 means error.
     */
    int getSceneMode(camera_scene_mode_t& sceneMode) const;

    /**
     * \brief Set deinterlace mode
     *
     * \param[in] camera_deinterlace_mode_t deinterlaceMode
     *
     * Setting deinterlace mode only takes effect before camera_device_config_streams called
     * That's it cannot be changed after camera_device_config_streams.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setDeinterlaceMode(camera_deinterlace_mode_t deinterlaceMode);

    /**
     * \brief Get deinterlace mode
     *
     * \param[out] camera_deinterlace_mode_t& deinterlaceMode
     *
     * \return 0 if deinterlace mode was set, non-0 means no deinterlace mode was set.
     */
    int getDeinterlaceMode(camera_deinterlace_mode_t& deinterlaceMode) const;

    /**
     * \brief Set Custom Aic Param
     *
     * \param[in] const void* data: the pointer of data.
     * \param[in] int length: the length of the data.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setCustomAicParam(const void* data, unsigned int length);

    /**
     * \brief Get Custom Aic Param
     *
     * \param[out] void* data: the pointer of destination buffer.
     * \param[in/out] in: the buffer size; out: the buffer used size.
     *
     * \return 0 if get successfully, otherwise non-0 value is returned.
     */
    int getCustomAicParam(void* data, unsigned int* length) const;

    /**
     * \brief Set digital zoom ratio
     *
     * \param[in] float ratio
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setDigitalZoomRatio(float ratio);

    /**
     * \brief Get digital zoom ratio
     *
     * \param[out] float& ratio
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getDigitalZoomRatio(float& ratio) const;

    /**
     * \brief Set lens distortion correction mode
     *
     * \param[in] camera_ldc_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setLdcMode(camera_ldc_mode_t mode);

    /**
     * \brief Get lens distortion correction mode
     *
     * \param[out] camera_ldc_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getLdcMode(camera_ldc_mode_t& mode) const;

    /**
     * \brief Set rolling shutter correction mode
     *
     * \param[in] camera_rsc_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setRscMode(camera_rsc_mode_t mode);

    /**
     * \brief Get rolling shutter correction mode
     *
     * \param[out] camera_rsc_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getRscMode(camera_rsc_mode_t& mode) const;

    /**
     * \brief flip mode
     *
     * \param[in] camera_flip_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFlipMode(camera_flip_mode_t mode);

    /**
     * \brief Get flip mode
     *
     * \param[out] camera_flip_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getFlipMode(camera_flip_mode_t& mode) const;

    /**
     * \brief set frame interval to run 3A
     *
     * \param[in] int cadence which is frame interval
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setRun3ACadence(int cadence);

    /**
     * \brief Get frame interval to run 3A
     *
     * \param[out] int& cadence which is frame interval
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getRun3ACadence(int& cadence) const;

    /**
     * \brief mono downscale mode
     *
     * \param[in] camera_mono_downscale_mode_t mode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setMonoDsMode(camera_mono_downscale_mode_t mode);

    /**
     * \brief Get mono downscale mode
     *
     * \param[out] camera_mono_downscale_mode_t& mode
     *
     * \return 0 if find the corresponding data, otherwise non-0 value is returned.
     */
    int getMonoDsMode(camera_mono_downscale_mode_t& mode) const;

    /**
     * \brief Set Fisheye Dewarping Mode
     *
     * \param[in] camera_fisheye_dewarping_mode_t dewarpingMode
     *
     * Setting dewarping mode only takes effect before camera_device_config_streams called
     * That's it cannot be changed after camera_device_config_streams.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t dewarpingMode);

    /**
     * \brief Get Fisheye Dewarping Mode
     *
     * \param[out] camera_fisheye_dewarping_mode_t &dewarpingMode
     *
     * \return 0 if dewarping mode was set, non-0 means no dewarping mode was set.
     */
    int getFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t& dewarpingMode) const;

    // Belows are Jpeg related parameters operations
    int getJpegQuality(uint8_t* quality) const;
    int setJpegQuality(uint8_t quality);

    int getJpegThumbnailQuality(uint8_t* quality) const;
    int setJpegThumbnailQuality(uint8_t quality);

    int setJpegThumbnailSize(const camera_resolution_t& res);
    int getJpegThumbnailSize(camera_resolution_t& res) const;

    int getJpegRotation(int& rotation) const;
    int setJpegRotation(int rotation);

    int setJpegGpsCoordinates(const double* coordinates);
    int getJpegGpsLatitude(double& latitude) const;
    int getJpegGpsLongitude(double& longitude) const;
    int getJpegGpsAltitude(double& altiude) const;

    int getJpegGpsTimeStamp(int64_t& timestamp) const;
    int setJpegGpsTimeStamp(int64_t timestamp);

    int getJpegGpsProcessingMethod(int& processMethod) const;
    int setJpegGpsProcessingMethod(int processMethod);

    int getJpegGpsProcessingMethod(int size, char* processMethod) const;
    int setJpegGpsProcessingMethod(const char* processMethod);

    int getImageEffect(camera_effect_mode_t& effect) const;
    int setImageEffect(camera_effect_mode_t effect);

    int getVideoStabilizationMode(camera_video_stabilization_mode_t& mode) const;
    int setVideoStabilizationMode(camera_video_stabilization_mode_t mode);

    int getFocalLength(float& focal) const;
    int setFocalLength(float focal);

    /**
     * \brief Get aperture value currently used
     *
     * \param[in] float& aperture
     *
     * \return 0 if aperture was set, non=0 means no aperture was set
     */
    int getAperture(float& aperture) const;
    /**
     * \brief Set aperture value
     *
     * \param[in] float aperture
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAperture(float aperture);

    /**
     * \brief Get focus distance value currently used
     *
     * \param[in] float& distance
     *
     * \return 0 if distance was set, non-0 means no focus distance was set
     */
    int getFocusDistance(float& distance) const;
    /**
     * \brief Set focus distance value
     *
     * \param[in] float distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFocusDistance(float distance);

    /**
     * \brief Get focus range value currently used
     *
     * \param[in] camera_range_t& focusRange
     *
     * \return 0 if focus range was set, non-0 means no focus range was set
     */
    int getFocusRange(camera_range_t& focusRange) const;
    /**
     * \brief Set focus range value
     *
     * \param[in] camera_range_t focusRange
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setFocusRange(const camera_range_t& focusRange);

    /**
     * \brief Set af mode
     *
     * \param[in] camera_af_mode_t afMode
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfMode(camera_af_mode_t afMode);

    /**
     * \brief Get af mode currently used.
     *
     * \param[out] camera_af_mode_t& afMode
     *
     * \return 0 if af mode was set, non-0 means no af mode was set.
     */
    int getAfMode(camera_af_mode_t& afMode) const;

    /**
     * \brief Trigger or cancel auto focus
     *
     * \param[in] camera_af_trigger_t afTrigger
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfTrigger(camera_af_trigger_t afTrigger);

    /**
     * \brief Get auto focus trigger value
     *
     * \param[out] camera_af_trigger_t afTrigger
     *
     * \return 0 if af trigger was set, otherwise non-0 value is returned.
     */
    int getAfTrigger(camera_af_trigger_t& afTrigger) const;

    /**
     * \brief Set AF state.
     *
     * \param[in] camera_af_state_t afState
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfState(camera_af_state_t afState);

    /**
     * \brief Get AF state
     *
     * \param[out] afState: Currently AF state will be set to afState if 0 is returned.
     *
     * \return 0 if AF state was set, otherwise non-0 value is returned.
     */
    int getAfState(camera_af_state_t& afState) const;

    /**
     * \brief Set lens state.
     *
     * \param[in] if lens is moving
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setLensState(bool lensMoving);

    /**
     * \brief Get lens state
     *
     * \param[out] isMoving: if lens is moving currently
     *
     * \return 0 if lens state was set, otherwise non-0 value is returned.
     */
    int getLensState(bool& lensMoving) const;

    /**
     * \brief Get lens aperture.
     *
     * \param[out] aperture
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensAperture(float& aperture) const;

    /**
     * \brief Get lens filter density.
     *
     * \param[out] filter density
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensFilterDensity(float& filterDensity) const;

    /**
     * \brief Get lens min focus distance.
     *
     * \param[out] min focus distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensMinFocusDistance(float& minFocusDistance) const;

    /**
     * \brief Get lens hyperfocal distance.
     *
     * \param[out] hyperfocal distance
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int getLensHyperfocalDistance(float& hyperfocalDistance) const;

    /**
     * \brief Set af region
     *
     * \param[in] camera_window_list_t afRegions
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setAfRegions(camera_window_list_t afRegions);

    /**
     * \brief Get af region
     *
     * \param[out] camera_window_list_t afRegions
     *
     * \return 0 if afRegions were set, otherwise non-0 value is returned.
     */
    int getAfRegions(camera_window_list_t& afRegions) const;

    /**
     * \brief Get camera sensor mount type
     *
     * \param[out] sensorMountType sensor mount type: WALL_MOUNT or CEILING_MOUNT
     *
     * \return 0 if sensorMountType was set, otherwise non-0 value is returned.
     */
    int getSensorMountType(camera_mount_type_t& sensorMountType) const;

    int updateDebugLevel();

    /**
     * \brief Set camera test pattern mode
     *
     * \param[in] mode: the camera device test pattern mode.
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setTestPatternMode(camera_test_pattern_mode_t mode);

    /**
     * \brief Get camera test pattern mode
     *
     * \param[out] mode: the camera device test pattern mode.
     *
     * \return 0 if test pattern mode was set, otherwise non-0 value is returned.
     */
    int getTestPatternMode(camera_test_pattern_mode_t& mode) const;

    /**
     * \brief Set crop region
     *
     * \param[in] cropRegion  the crop region related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setCropRegion(camera_crop_region_t cropRegion);

    /**
     * \brief Get crop region
     *
     * \param[out] cropRegion  the crop related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getCropRegion(camera_crop_region_t& cropRegion) const;

    /**
     * \brief Set control scene mode
     *
     * \param[in] sceneModeValue the control scene mode related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setControlSceneMode(uint8_t sceneModeValue);

    /**
     * \brief Set face detect mode
     *
     * \param[in] faceDetectMode the face detect mode related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setFaceDetectMode(uint8_t faceDetectMode);

    /**
     * \brief Get face detect mode
     *
     * \param[out] faceDetectMode the face detect mode related parameters, 0:OFF 1:SIMPLE 2:FULL
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getFaceDetectMode(uint8_t& faceDetectMode) const;

    /**
     * \brief Set face id
     *
     * \param[in] int *faceIds, int faceNum
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setFaceIds(const int* faceIds, int faceNum);

    /**
     * Get sensor active array size
     *
     * \param[out] camera_coordinate_system_t& arraySize
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getSensorActiveArraySize(camera_coordinate_system_t& arraySize) const;

    /**
     * \brief Set shading  mode
     *
     * \param[in] shadingMode the shading mode related parameters
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int setShadingMode(camera_shading_mode_t shadingMode);

    /**
     * \brief Get shading  mode
     *
     * \param[out] shadingMode the shading mode related parameters, 0:OFF 1:FAST 2:HIGH_QUALITY
     *
     * \return 0 if successfully, otherwise non-0 value is returned.
     */
    int getShadingMode(camera_shading_mode_t& shadingMode) const;

    /**
     * \brief Set scale & crop region
     *
     * \param[in] camera_zoom_region_t region
     *
     * \return 0 if set successfully, otherwise non-0 value is returned.
     */
    int setZoomRegion(const camera_zoom_region_t& region);

    /**
     * \brief Get scale & crop region
     *
     * \param[in] camera_zoom_region_t region
     *
     * \return 0 if flag was set, otherwise non-0 value is returned.
     */
    int getZoomRegion(camera_zoom_region_t* region) const;

 private:
    friend class ParameterHelper;
    void* mData;  // The internal data to save the all of the parameters.
};                // class Parameters
/*******************End of Camera Parameters Definition**********************/

}  // namespace icamera

#endif // PARAMETERS_H
