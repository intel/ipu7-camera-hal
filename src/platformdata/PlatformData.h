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

#include <limits.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <v4l2_device.h>
#include "AiqInitData.h"
#include "CameraTypes.h"
#include "FaceType.h"
#include "ParamDataType.h"
#include "GraphConfig.h"
#include "MediaControl.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

#define RESOLUTION_1_3MP_WIDTH 1280
#define RESOLUTION_1_3MP_HEIGHT 960
#define RESOLUTION_1080P_WIDTH 1920
#define RESOLUTION_1080P_HEIGHT 1080
#define RESOLUTION_720P_WIDTH 1280
#define RESOLUTION_720P_HEIGHT 720
#define RESOLUTION_VGA_WIDTH 640
#define RESOLUTION_VGA_HEIGHT 480
#define RESOLUTION_360P_HEIGHT 360

// VIRTUAL_CHANNEL_S
#define MAX_VC_GROUP_NUMBER 8
// VIRTUAL_CHANNEL_E

#define FACE_ENGINE_DEFAULT_RUNNING_INTERVAL 1

#define FACE_ENGINE_INTEL_PVL 0
#define FACE_ENGINE_GOOGLE_FACESSD 1

#define DEFAULT_TNR_EXTRA_FRAME_NUM 2

#define CSI_PORT_NAME "CSI2"

#define MAX_CAMERA_NUMBER 100
// Temporarily using current path to save aiqd file for none CAL platforms.
#define CAMERA_CACHE_DIR "./"
#define CAMERA_GRAPH_SETTINGS_DIR "gcss/"
#define CAMERA_AIQD_PATH "/run/camera/"

#ifndef CAMERA_DEFAULT_CFG_PATH
#error CAMERA_DEFAULT_CFG_PATH not defined
#endif

#define NVM_DATA_PATH "/sys/bus/i2c/devices/"

#define TNR7US_RESTART_THRESHOLD 5

class StaticMetadata {
 public:
    StaticMetadata() {
        mMountType = static_cast<uint8_t>(WALL_MOUNTED);

        mStaticMetadataToType = {
            {"ae.lockAvailable", TYPE_BYTE},
            {"awb.lockAvailable", TYPE_BYTE},
            {"control.availableModes", TYPE_BYTE},
            {"control.availableSceneModes", TYPE_BYTE},
            {"control.maxRegions", TYPE_INT32},
            {"control.zoomRatioRange", TYPE_FLOAT},
            {"control.enableZsl", TYPE_BYTE},
            {"statistics.info.availableFaceDetectModes", TYPE_BYTE},
            {"statistics.info.maxFaceCount", TYPE_INT32},
            {"sensor.maxAnalogSensitivity", TYPE_INT32},
            {"sensor.info.activeArraySize", TYPE_INT32},
            {"sensor.info.pixelArraySize", TYPE_INT32},
            {"sensor.info.physicalSize", TYPE_FLOAT},
            {"sensor.info.sensitivityRange", TYPE_INT32},
            {"sensor.info.exposureTimeRange", TYPE_INT64},
            {"sensor.info.colorFilterArrangement", TYPE_BYTE},
            {"sensor.info.whitelevel", TYPE_INT32},
            {"sensor.availableTestPatternModes", TYPE_INT32},
            {"sensor.orientation", TYPE_INT32},
            {"sensor.opaqueRawSize", TYPE_INT32},
            {"shading.availableModes", TYPE_BYTE},
            {"lens.facing", TYPE_BYTE},
            {"lens.info.availableApertures", TYPE_FLOAT},
            {"lens.info.availableFilterDensities", TYPE_FLOAT},
            {"lens.info.availableFocalLengths", TYPE_FLOAT},
            {"lens.info.availableOpticalStabilization", TYPE_BYTE},
            {"lens.info.hyperfocalDistance", TYPE_FLOAT},
            {"lens.info.minimumFocusDistance", TYPE_FLOAT},
            {"lens.info.shadingMapSize", TYPE_INT32},
            {"lens.info.focusDistanceCalibration", TYPE_BYTE},
            {"request.maxNumOutputStreams", TYPE_INT32},
            {"request.maxNumInputStreams", TYPE_INT32},
            {"request.pipelineMaxDepth", TYPE_BYTE},
            {"request.availableCapabilities", TYPE_BYTE},
            {"scaler.availableInputOutputFormatsMap", TYPE_INT32},
            {"scaler.availableStreamConfigurations", TYPE_INT32},
            {"scaler.availableMinFrameDurations", TYPE_INT64},
            {"scaler.availableMaxDigitalZoom", TYPE_FLOAT},
            {"scaler.availableStallDurations", TYPE_INT64},
            {"reprocess.maxCaptureStall", TYPE_INT32},
            {"jpeg.maxSize", TYPE_INT32},
            {"jpeg.availableThumbnailSizes", TYPE_INT32},
            {"edge.availableEdgeModes", TYPE_BYTE},
            {"hotPixel.availableHotPixelModes", TYPE_BYTE},
            {"noiseReduction.availableNoiseReductionModes", TYPE_BYTE},
            {"tonemap.maxCurvePoints", TYPE_INT32},
            {"tonemap.availableToneMapModes", TYPE_BYTE},
            {"info.supportedHardwareLevel", TYPE_BYTE},
            {"sync.maxLatency", TYPE_INT32},
        };
    }

    stream_array_t mConfigsArray;
    std::vector<double> mFpsRange;
    std::vector<int> mEvRange;
    std::vector<int> mEvStep;
    camera_features_list_t mSupportedFeatures;
    std::vector<struct AeRange> mAeExposureTimeRange;
    std::vector<struct AeRange> mAeGainRange;
    camera_video_stabilization_list_t mVideoStabilizationModes;
    std::vector<camera_ae_mode_t> mSupportedAeMode;
    std::vector<camera_awb_mode_t> mSupportedAwbMode;
    std::vector<camera_scene_mode_t> mSupportedSceneMode;
    std::vector<camera_af_mode_t> mSupportedAfMode;
    std::vector<camera_antibanding_mode_t> mSupportedAntibandingMode;
    std::vector<camera_rotate_mode_t> mSupportedRotateMode;
    uint8_t mMountType;

    std::unordered_map<std::string, enum StaticMetaType> mStaticMetadataToType;
    std::unordered_map<std::string, std::vector<uint8_t>> mByteMetadata;
    std::unordered_map<std::string, std::vector<int32_t>> mInt32Metadata;
    std::unordered_map<std::string, std::vector<int64_t>> mInt64Metadata;
    std::unordered_map<std::string, std::vector<float>> mFloatMetadata;
    std::unordered_map<std::string, std::vector<double>> mDoubleMetadata;
};

class PlatformData {
 private:
    // Prevent to create multiple instances
    PlatformData();
    ~PlatformData();

 public:
    class StaticCfg {
     public:
        StaticCfg() { mCameras.clear(); }
        ~StaticCfg() {}  // not release resource by design

        /**
         * Camera feature info that is specific to camera id
         */
        class CameraInfo {
         public:
            CameraInfo()
                    : sensorName(""),
                      sensorDescription("unset"),
                      mLensName(""),
                      // VIRTUAL_CHANNEL_S
                      mVCCount(0),
                      mVCId(0),
                      mVCGroupId(-1),
                      // VIRTUAL_CHANNEL_E
                      mLensHwType(LENS_NONE_HW),
                      mEnablePdaf(false),
                      mSensorAwb(false),
                      mSensorAe(false),
                      mRunIspAlways(false),
                      // HDR_FEATURE_S
                      mHdrStatsInputBitDepth(0),
                      mHdrStatsOutputBitDepth(0),
                      mUseFixedHdrExposureInfo(true),
                      // HDR_FEATURE_E
                      mSensorExposureNum(2),
                      mSensorExposureType(SENSOR_EXPOSURE_SINGLE),
                      mSensorGainType(SENSOR_GAIN_NONE),
                      mLensCloseCode(0),
                      mEnableAIQ(false),
                      mAiqRunningInterval(1),
                      mStatsRunningRate(false),
                      mEnableMkn(true),
                      mIspTuningUpdate(true),
                      mSkipFrameV4L2Error(false),
                      mCITMaxMargin(0),
                      mYuvColorRangeMode(
                        camera_yuv_color_range_mode_t::CAMERA_FULL_MODE_YUV_COLOR_RANGE),
                      mInitialSkipFrame(0),
                      mMaxRawDataNum(MAX_BUFFER_COUNT),
                      mTopBottomReverse(false),
                      mPsysContinueStats(false),
                      mMaxRequestsInflight(0),
                      mPreferredBufQSize(MAX_BUFFER_COUNT),
                      mDigitalGainLag(-1),
                      mExposureLag(MAX_BUFFER_COUNT),
                      mAnalogGainLag(0),
                      mMaxSensorDigitalGain(0),
                      mSensorDgType(SensorDgType::SENSOR_DG_TYPE_NONE),
                      mISysFourcc(V4L2_PIX_FMT_SGRBG8),
                      mISysRawFormat(V4L2_PIX_FMT_SGRBG10),
                      mUseCrlModule(true),
                      mFacing(FACING_BACK),
                      mOrientation(ORIENTATION_0),
                      mSensorOrientation(ORIENTATION_0),
                      mUseSensorDigitalGain(false),
                      mUseIspDigitalGain(false),
                      mNeedPreRegisterBuffers(false),
                      mEnableAiqd(false),
                      mCurrentMcConf(nullptr),
                      mDVSType(MORPH_TABLE),
                      mPSACompression(false),
                      mOFSCompression(false),
                      mUnregisterExtDmaBuf(false),
                      mFaceAeEnabled(true),
                      mFaceEngineVendor(FACE_ENGINE_INTEL_PVL),
                      mFaceEngineRunningInterval(FACE_ENGINE_DEFAULT_RUNNING_INTERVAL),
                      mFaceEngineRunningIntervalNoFace(FACE_ENGINE_DEFAULT_RUNNING_INTERVAL),
                      mRunFaceWithSyncMode(false),
                      mMaxFaceDetectionNumber(MAX_FACES_DETECTABLE),
                      mPsysBundleWithAic(false),
                      mSwProcessingAlignWithIsp(false),
                      mMaxNvmDataSize(0),
                      mNvmOverwrittenFileSize(0),
                      mGpuTnrEnabled(false),
                      mGpuIpaEnabled(false),
                      mTnrExtraFrameNum(DEFAULT_TNR_EXTRA_FRAME_NUM),
                      mMsPsysAlignWithSystem(0),
                      mDummyStillSink(false),
                      mRemoveCacheFlushOutputBuffer(false),
                      mPLCEnable(false),
                      mStillOnlyPipe(false),
                      mUsePSysProcessor(true) {
            }

            std::vector<MediaCtlConf> mMediaCtlConfs;

            std::string sensorName;
            std::string sensorDescription;
            std::string mLensName;
            // VIRTUAL_CHANNEL_S
            int mVCCount;
            int mVCId;
            int mVCGroupId;
            // VIRTUAL_CHANNEL_E
            int mLensHwType;
            bool mEnablePdaf;
            bool mSensorAwb;
            bool mSensorAe;
            bool mRunIspAlways;
            // HDR_FEATURE_S
            int mHdrStatsInputBitDepth;
            int mHdrStatsOutputBitDepth;
            bool mUseFixedHdrExposureInfo;
            // HDR_FEATURE_E
            int mSensorExposureNum;
            int mSensorExposureType;
            int mSensorGainType;
            int mLensCloseCode;
            bool mEnableAIQ;
            int mAiqRunningInterval;
            bool mStatsRunningRate;
            bool mEnableMkn;
            bool mIspTuningUpdate;
            // first: one algo type in imaging_algorithm_t, second: running rate
            std::unordered_map<int, float> mAlgoRunningRateMap;
            bool mSkipFrameV4L2Error;
            int mCITMaxMargin;
            camera_yuv_color_range_mode_t mYuvColorRangeMode;
            unsigned int mInitialSkipFrame;
            unsigned int mMaxRawDataNum;
            bool mTopBottomReverse;
            bool mPsysContinueStats;
            int mMaxRequestsInflight;
            unsigned int mPreferredBufQSize;
            int mDigitalGainLag;
            int mExposureLag;
            int mAnalogGainLag;
            int mMaxSensorDigitalGain;
            SensorDgType mSensorDgType;
            std::string mCustomAicLibraryName;
            std::string mCustom3ALibraryName;
            std::vector<camera_resolution_t> mSupportedISysSizes;  // ascending order request
            std::vector<int> mSupportedISysFormat;
            int mISysFourcc;     // the isys output format
            int mISysRawFormat;  // the isys raw format if scale enabled

            std::vector<TuningConfig> mSupportedTuningConfig;
            std::vector<LardTagConfig> mLardTagsConfig;
            std::vector<ConfigMode> mConfigModesForAuto;

            bool mUseCrlModule;
            int mFacing;
            int mOrientation;
            int mSensorOrientation;
            bool mUseSensorDigitalGain;
            bool mUseIspDigitalGain;
            bool mNeedPreRegisterBuffers;
            bool mEnableAiqd;
            MediaCtlConf* mCurrentMcConf;
            std::map<int, stream_array_t> mStreamToMcMap;

            std::string mGraphSettingsFile;
            std::vector<MultiExpRange> mMultiExpRanges;
            int mDVSType;
            bool mPSACompression;
            bool mOFSCompression;
            bool mUnregisterExtDmaBuf;
            bool mFaceAeEnabled;
            int mFaceEngineVendor;
            int mFaceEngineRunningInterval;
            int mFaceEngineRunningIntervalNoFace;
            bool mRunFaceWithSyncMode;
            unsigned int mMaxFaceDetectionNumber;
            bool mPsysBundleWithAic;
            bool mSwProcessingAlignWithIsp;

            /* key: camera_test_pattern_mode_t, value: sensor test pattern mode */
            std::unordered_map<int32_t, int32_t> mTestPatternMap;

            // This is for binding stream id to ConfigMode, since the stream id from kernel list of
            // a PG might be incorrect. To be removed after stream id mismatch issue fixed.
            std::map<int, int> mConfigModeToStreamId;
            std::vector<UserToPslOutputMap> mOutputMap;
            int mMaxNvmDataSize;
            std::string mNvmDirectory;
            int mNvmOverwrittenFileSize;
            std::string mNvmOverwrittenFile;  // overwrite NVM data
            std::string mCamModuleName;
            std::vector<std::string> mSupportModuleNames;
            /* key: camera module name, value: camera module info */
            // TODO enable camera module after switch to Json
            std::vector<IGraphType::ScalerInfo> mScalerInfo;
            bool mGpuTnrEnabled;
            bool mGpuIpaEnabled;
            int mTnrExtraFrameNum;
            int mMsPsysAlignWithSystem;  // Scheduling aligned with system time
            bool mDummyStillSink;
            bool mRemoveCacheFlushOutputBuffer;
            bool mPLCEnable;
            bool mStillOnlyPipe;
            bool mUsePSysProcessor;

            StaticMetadata mStaticMetadata;
        };

        std::vector<CameraInfo> mCameras;
        CommonConfig mCommonConfig;
    };

 private:
    StaticCfg mStaticCfg;

    std::vector<AiqInitData*> mAiqInitData;

 private:
    /**
     * Get access to the platform singleton.
     *
     * Note: this is implemented in PlatformFactory.cpp
     */
    static PlatformData* sInstance;
    static Mutex sLock;
    static PlatformData* getInstance();

    /**
     * Release GraphConfigNodes in StaticCfg::CameraInfo
     */
    void releaseGraphConfigNodes();

    /**
     * Get MediaCtlConf via MC ID.
     */
    static MediaCtlConf* getMcConfByMcId(const StaticCfg::CameraInfo& cameraInfo, int mcId);

    /**
     * Get MediaCtlConf via stream config.
     */
    static MediaCtlConf* getMcConfByStream(const StaticCfg::CameraInfo& cameraInfo,
                                           const stream_t& stream);

    /**
     * Get MediaCtlConf via ConfigMode.
     */
    static MediaCtlConf* getMcConfByConfigMode(const StaticCfg::CameraInfo& cameraInfo,
                                               const stream_t& stream, ConfigMode mode);

    /**
     * Check if video node is enabled via camera Id and video node type.
     */
    static bool isVideoNodeEnabled(int cameraId, VideoNodeType type);

 public:
    /**
     * releaseInstance
     * This function must be called when the hal is destroyed.
     */
    static void releaseInstance();

    /**
     * init PlatformData
     *
     * \return OK if init PlatformData successfully, otherwise return ERROR.
     */
    static int init();

    /**
     * Parse graph descriptor and settings from configuration files.
     */
    static void parseGraphFromXmlFile();

    /**
     * get the camera numbers
     *
     * \return int: the camera numbers
     */
    static int numberOfCameras();

    /**
     * get the camera number in xml
     *
     * \return int: the camera numbers in xml
     */
    static int getXmlCameraNumber();

    /**
     * get the sensor name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return char*: the sensor name string.
     */
    static const char* getSensorName(int cameraId);

    /**
     * get the sensor ratio
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return float: the sensor ratio.
     */
    static float getSensorRatio(int cameraId);

    /**
     * get the sensor description
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return const char*: the sensor description string.
     */
    static const char* getSensorDescription(int cameraId);

    /**
     * get the Lens name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return char*: the lens name string.
     */
    static const char* getLensName(int cameraId);

    /**
     * get the Lens HW type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the Lens HW type
     */
    static int getLensHwType(int cameraId);

    /**
     * check if PDAF is supported or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if PDAF is supported.
     */
    static bool isPdafEnabled(int cameraId);

    /**
     * get the sensor AWB
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the sensor AWB enable
     */
    static bool getSensorAwbEnable(int cameraId);

    /**
     * get the sensor AE
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the sensor AE enable
     */
    static bool getSensorAeEnable(int cameraId);

    /**
     * get the run ISP rate
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return bool: the run ISP rate
     */
    static bool getRunIspAlways(int cameraId);

    /**
     * get the DVS type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the DVS type
     */
    static int getDVSType(int cameraId);

    /**
     * get the PSA compression flag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if PSA compression is enabled
     */
    static bool getPSACompression(int cameraId);

    /**
     * get the OFS compression flag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if OFS compression is enabled
     */
    static bool getOFSCompression(int cameraId);

    /**
     * get the max coarse integration time margin
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the value of max coarse integration time margin.
     */
    static int getCITMaxMargin(int cameraId);

    /**
     * Check AIQ is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if AIQ is enabled or not.
     */
    static bool isEnableAIQ(int cameraId);

    /**
     * get running rate of AIQ
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return int: the running rate of AIQ
     */
    static int getAiqRunningInterval(int cameraId);

    /**
     * Check Mkn is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if Mkn is enabled or not.
     */
    static bool isEnableMkn(int cameraId);

    /**
     * get running rate of every Algorithms
     *
     * \param algo: one type of imaging_algorithm_t
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return float: the running rate of every Algorithms, otherwise return 0.0
     */
    static float getAlgoRunningRate(int algo, int cameraId);

    /**
     * if running rate is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \\return if running rate is supported or not.
     */
    static bool isStatsRunningRateSupport(int cameraId);

    /**
     * Check if sensor digital gain is used or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if sensor gain is used or not.
     */
    static bool isUsingSensorDigitalGain(int cameraId);

    /**
     * Check if using isp digital gain or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if using isp gain or not.
     */
    static bool isUsingIspDigitalGain(int cameraId);

    /**
     * Check if need to pre-register buffers or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if pre-register buffers or not.
     */
    static bool isNeedToPreRegisterBuffer(int cameraId);

    /**
     * Get exposure number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param multiExposure: true or false
     * \return the value of exposure number according to different cases
     */
    static int getExposureNum(int cameraId, bool multiExposure);

    // HDR_FEATURE_S
    /**
     * Check HDR is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if HDR is enabled or not.
     */
    static bool isEnableHDR(int cameraId);

    /**
     * Get HDR stats input bit depth
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of HDR stats input bit depth
     */
    static int getHDRStatsInputBitDepth(int cameraId);

    /**
     * Get HDR stats output bit depth
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of HDR stats output bit depth
     */
    static int getHDRStatsOutputBitDepth(int cameraId);

    /**
     * Get if HDR exposure info is fixed or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if HDR exposure info is fixed
     */
    static int isUseFixedHDRExposureInfo(int cameraId);
    // HDR_FEATURE_E

    /**
     * Get if multi exposure cases or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode: tuningMode
     * \return true if multi exposure case
     */
    static bool isMultiExposureCase(int cameraId, TuningMode tuningMode);

    /**
     * Get sensor exposure type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of sensor exposure type
     */
    static int getSensorExposureType(int cameraId);

    /**
     * Get sensor gain type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of sensor gain type
     */
    static int getSensorGainType(int cameraId);

    /**
     * Get sensor's initial skip frame number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of initial skip frame number
     */
    static unsigned int getInitialSkipFrame(int cameraId);

    /**
     * Get max raw data number
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of max raw data number
     */
    static unsigned int getMaxRawDataNum(int cameraId);

    /**
     * Get sensor's top bottom filed reverse option
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of top bottom filed reverse value
     */
    static bool getTopBottomReverse(int cameraId);

    /*
     * Check if Psys continuous stats is needed or not.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if it is needed
     */
    static bool isPsysContinueStats(int cameraId);

    /**
     * Check if unregister external DMA buffer
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if unregister external DMA buffer.
     */
    static bool unregisterExtDmaBuf(int cameraId);

    /**
     * Get preferred buffer queue size
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of preferred buffer queue size
     */
    static unsigned int getPreferredBufQSize(int cameraId);

    /**
     * Check face engine is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face engine is enabled or not.
     */
    static bool isFaceDetectionSupported(int cameraId);

    /**
     * Check face AE is enabled or not, only for debug
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face ae is enabled or not.
     */
    static bool isFaceAeEnabled(int cameraId);

    /**
     * get face engine's vendor
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine's vendor.
     */
    static int faceEngineVendor(int cameraId);

    /**
     * get face engine's running interval
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine running interval value.
     */
    static int faceEngineRunningInterval(int cameraId);

    /**
     * get face engine's running interval when face is not found
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the face engine running interval value when face is not found.
     */
    static int faceEngineRunningIntervalNoFace(int cameraId);

    /**
     * Check face detection runs synchronously or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if face detection runs synchronously or not.
     */
    static bool runFaceWithSyncMode(int cameraId);

    /**
     * Whether IPU has a single output to support face detection
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true or false.
     */
    static bool isIPUSupportFD(int cameraId);

    /**
     * get the max number of face detection
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the max number of face detection.
     */
    static unsigned int getMaxFaceDetectionNumber(int cameraId);

    /**
     * get dvs supported status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true: dvs supported; false: dvs not supported.
     */
    static bool isDvsSupported(int cameraId);

    /**
     * Check running psys bundle with aic is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if running psys bundle with aic is enabled or not.
     */
    static bool psysBundleWithAic(int cameraId);

    /**
     * Check software processing align with isp is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if software processing align with isp is enabled or not.
     */
    static bool swProcessingAlignWithIsp(int cameraId);

    /**
     * Get the max digital gain of sensor
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of max digital gain
     */
    static int getMaxSensorDigitalGain(int cameraId);

    /**
     * Get sensor digital gain type
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the sensor digital gain type
     */
    static SensorDgType sensorDigitalGainType(int cameraId);

    /**
     * Get sensor's digital gain lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of digital gain lag
     */
    static int getDigitalGainLag(int cameraId);
    /**
     * Get sensor's exposure lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of exposure lag
     */
    static int getExposureLag(int cameraId);

    /**
     * Get sensor's analog gain lag
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of analog gain lag
     */
    static int getAnalogGainLag(int cameraId);

    /**
     * According to stream info to select MC
     * this function will compare the format/resolutions/interlace to find the MediaCtlConf
     * and then store it into cameraInfo.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param stream: the stream info
     * \param mode: the stream operation mode
     */
    static void selectMcConf(int cameraId, stream_t stream, ConfigMode mode, int mcId);

    /**
     * to get the current MediaCtlConf
     * after the media controller has been analyzed, the media controller information will be stored
     * in the mMediaCtlConfs.
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return MediaCtlConf*, if it doesn't find one, this function will return nullptr.
     */
    static MediaCtlConf* getMediaCtlConf(int cameraId);

    /**
     * \brief Fill device info according to given camera id
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[out] device_info_t info
     *
     */
    static void getDeviceInfo(int cameraId, device_info_t& info);

// VIRTUAL_CHANNEL_S
    /**
     * \brief Fill vc info according to given camera id
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[out] vc_info_t vc
     *
     * \return 0 if succeed, other value indicates failed.
     */
    static int getVCInfo(int cameraId, vc_info_t& vc);
// VIRTUAL_CHANNEL_E

    /**
     * \brief get static metadata according to given camera id
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     *
     * \return const point of static metadata.
     */
    static const StaticMetadata *getStaticMetadata(int cameraId);

    /**
     * \brief get values per str
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param str: used to find metadata
     *
     * \return vector values.
     */
    static const std::vector<uint8_t> getByteStaticMetadata(int cameraId, const std::string str);
    static const std::vector<int32_t> getInt32StaticMetadata(int cameraId, const std::string str);
    static const std::vector<int64_t> getInt64StaticMetadata(int cameraId, const std::string str);
    static const std::vector<float> getFloatStaticMetadata(int cameraId, const std::string str);
    static const std::vector<double> getDoubleStaticMetadata(int cameraId, const std::string str);

    /**
     * \brief Check if the camera_features feature is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] camera_features feature
     *
     * \return true if supported, otherwise return false.
     */
    static bool isFeatureSupported(int cameraId, camera_features feature);

    /**
     * \brief Check if the HAL based ZSL is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     *
     * \return true if supported, otherwise return false.
     */
    static bool isHALZslSupported(int cameraId);

    /**
     * \brief Check if the given stream config is supported
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] stream_t conf
     *
     * \return true if supported, otherwise return false.
     */
    static bool isSupportedStream(int cameraId, const stream_t& conf);

    /**
     * get the isys supported size list
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param sizes: the function will fill the isys supported size list to the sizes
     */
    static void getSupportedISysSizes(int cameraId, std::vector<camera_resolution_t>& resolutions);

    /**
     * get the isys supported format list
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param formats: the function will fill the isys supported format list to the formats
     * \return true if success, return false if it fails.
     */
    static bool getSupportedISysFormats(int cameraId, std::vector<int>& formats);

    /**
     * Format for the ISYS output
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the format for the isys output
     */
    static int getISysFormat(int cameraId);

    /**
     * Set ISYS output format
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param format: the isys output format
     */
    static void selectISysFormat(int cameraId, int format);

    /**
     * If ISYS supported format.
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     */
    static bool isISysSupportedFormat(int cameraId, int format);

    /**
     * if the resolution is supported by Isys
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param resolution: the requested resolution
     * \return true if the resolution is supported by isys, otherwise false
     */
    static bool isISysSupportedResolution(int cameraId, camera_resolution_t resolution);

    /**
     * Check if the frame needs to be skipped when STR2MMIO error occurs
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return if corrupted frame needs to be skipped or not.
     */
    static bool isSkipFrameOnSTR2MMIOErr(int cameraId);

    /**
     * Format for the ISYS RAW output
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the RAW format if isys scale enabled
     */
    static int getISysRawFormat(int cameraId);

    // CSI_META_S
    /**
     * get CSI meta enabled status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if CSI meta is enabled, otherwise return false
     */
    static bool isCsiMetaEnabled(int cameraId);
    // CSI_META_E

    /**
     * get the format by device name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param devName: device name
     * \format: return param for format
     * \return the status
     */
    static int getFormatByDevName(int cameraId, const std::string& devName, McFormat& format);

    /**
     * get the video node name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param videoNodeType: value of enum VideoNodeType
     * \param videoNodeName: return param for the video node name
     * \return the status
     */
    static int getVideoNodeNameByType(int cameraId, VideoNodeType videoNodeType,
                                      std::string& videoNodeName);

    /**
     * get the hardware device name
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param videoNodeType: value of enum VideoNodeType
     * \param devName: return param for the device name
     * \return the status
     */
    static int getDevNameByType(int cameraId, VideoNodeType videoNodeType, std::string& devName);

    /**
     * Check if ISYS is enabled or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if ISYS is enabled, otherwise return false
     */
    static bool isIsysEnabled(int cameraId);

    static int calculateFrameParams(int cameraId, SensorFrameParams& sensorFrameParams);

    /**
     * Get the optimized resolutions that supported by input system
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param width:    The width of the request frame
     * \param height:   The height of the request frame
     * \param field:    The field of the request frame
     *
     * \return camera_resolution_t: The optimized resolution that used to configure the ISYS.
     */
    static camera_resolution_t getISysBestResolution(int cameraId, int width, int height,
                                                     int field);

    /**
     * to get supported psys dag config
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configs: the function will fill supported psys dag config list to the configs
     */
    static void getSupportedTuningConfig(int cameraId, std::vector<TuningConfig>& configs);

    /**
     * to get the ConfigMode by operation Mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param operationMode: the operation mode
     * \param configModes: the function will fill available ConfigMode for this operation mode
     * \return OK if get ConfigMode, otherwise return INVALID_OPERATION
     */
    static int getConfigModesByOperationMode(int cameraId, uint32_t operationMode,
                                             std::vector<ConfigMode>& configModes);

    /**
     * to get the TuningMode by Config Mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: type of ConfigMode (except auto)
     * \param tuningMode: return related TuningMode
     * \return OK if get TuningMode, otherwise return INVALID_OPERATION
     */
    static int getTuningModeByConfigMode(int cameraId, ConfigMode configMode,
                                         TuningMode& tuningMode);

    /**
     * to get tuning config by ConfigMode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param mode: ConfigMode
     * \param config: return related TuningConfig
     * \return OK if get TuningConfig, otherwise return INVALID_OPERATION
     */
    static int getTuningConfigByConfigMode(int cameraId, ConfigMode mode, TuningConfig& config);

    /*
     * Get stream id by the given configMode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param configMode: ConfigMode
     * \return the stream id if succeeds, otherwise return -1.
     */
    static int getStreamIdByConfigMode(int cameraId, ConfigMode configMode);

    /*
     * Get max pipeline depth
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the max pipeline depth
     */
    static int32_t getMaxPipelineDepth(int cameraId);

    /*
     * Get the max requests number in flight
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the max requests number in flight
     */
    static int getMaxRequestsInflight(int cameraId);

    /**
     * get yuv color range mode
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the corresponding camera_yuv_color_range_mode_t.
     */
    static camera_yuv_color_range_mode_t getYuvColorRangeMode(int cameraId);

    /**
     * Get aiqd
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param tuningMode: mode
     * \return ia_binary_data
     */
    static ia_binary_data* getAiqd(int cameraId, TuningMode mode);

    /**
     * Save aiqd
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param tuningMode: mode
     * \param ia_binary_data: data
     */
    static void saveAiqd(int cameraId, TuningMode tuningMode, const ia_binary_data& data);

    /**
     * Get cpf
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER]
     * \param mode: tuning mode
     * \param aiqbData: cpf
     * \return OK if it is successful.
     */
    static int getCpf(int cameraId, TuningMode mode, ia_binary_data* aiqbData);

    /**
     * if ISYS CSI Back End capture enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if the current sensor is CSI Back End capture or not
     */
    static bool isCSIBackEndCapture(int cameraId);

    /**
     * if AIQD enabled
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if AIQD is enabled or not
     */
    static bool isAiqdEnabled(int cameraId);

    /**
     * if image from tpg
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if frame is from tpg or not
     */
    static bool isTPGReceiver(int cameraId);

    static int getSupportAeExposureTimeRange(int cameraId, camera_scene_mode_t sceneMode,
                                             camera_range_t& etRange);
    static int getSupportAeGainRange(int cameraId, camera_scene_mode_t sceneMode,
                                     camera_range_t& gainRange);

    /**
     * if CrlModule is used
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if CrlModule driver is used, otherwise return false
     */
    static bool isUsingCrlModule(int cameraId);

    /**
     * to get the MultiExpRange of CameraInfo
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the MultiExpRange for current camera id.
     */
    static std::vector<MultiExpRange> getMultiExpRanges(int cameraId);

    // FILE_SOURCE_S
    /**
     * Get the injected file.
     *
     * \return the injected file or nullptr if cameraInjectFile isn't set.
     */
    static const char* getInjectedFile();

    /**
     * Check if FileSource is enabled.
     *
     * \return true cameraInjectFile is set, otherwise return false.
     */
    static bool isFileSourceEnabled();
    // FILE_SOURCE_E

    // VIRTUAL_CHANNEL_S
    /**
     * Get virtual channel id
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return id if Virtual channel supported, otherwise return 0.
     */
    static int getVirtualChannelId(int cameraId);
    // VIRTUAL_CHANNEL_E

    /**
     * Check if test pattern is supported or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if mTestPatternMap is defined, otherwise return false.
     */
    static bool isTestPatternSupported(int cameraId);

    /**
     * get sensor test pattern
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param mode: camera_test_pattern_mode_t
     * \return corresponding sensor test pattern if provided in xml file, otherwise return -1.
     */
    static int32_t getSensorTestPattern(int cameraId, int32_t mode);

    /**
     * Get the nvm
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return ia_binary_data
     */
    static ia_binary_data* getNvm(int cameraId);

    /**
     * Get sensor active array size
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return the value of camera_coordinate_system_t.
     */
    static camera_coordinate_system_t getActivePixelArray(int cameraId);

    /**
     * Get camera cfg path from environment variable
     *
     * \param void
     * \return the value of camera cfg path.
     */
    static std::string getCameraCfgPath();

    /**
     * Get camera graph setting file path.
     *
     * \param void
     * \return the value of camera graph setting file path.
     */
    static std::string getGraphSettingFilePath();

    /*
     * Get sensor value for the digital gain.
     *
     * Since the calculation formula may be different between sensors,
     * so we need to get this value based on sensor digital gain type.
     * For imx274, the magnification = 2^x (x is the register value).
     *
     * Need to specify the sensorDgType, maxSensorDg and useIspDigitalGain in xml.
     */
    static int getSensorDigitalGain(int cameraId, float realDigitalGain);

    /*
     * Get the isp gain
     *
     * Separate real digital to sensorDg and ispDg, and the ispDg >= 1
     */
    static float getIspDigitalGain(int cameraId, float realDigitalGain);

    /**
     * \brief init Makernote
     * allocate memory resource for Makernote
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    static int initMakernote(int cameraId, TuningMode tuningMode);

    /**
     * \brief deinit Makernote
     * free memory resource for Makernote
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] TuningMode tuningMode
     *
     * return OK if it is successful, fails for other values.
     */
    static int deinitMakernote(int cameraId, TuningMode tuningMode);

    /**
     * \brief Save Makernote by ia_mkn_trg mode
     *
     * param[in] int cameraId: camera ID
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] camera_makernote_mode_t: MAKERNOTE_MODE_JPEG is corresponding
     *           to ia_mkn_trg_section_1 for Normal Jpeg capture;
     *           MAKERNOTE_MODE_RAW is corresponding to ia_mkn_trg_section_2
     *           for Raw image capture.
     * \param[in] int64_t sequence: the sequence in latest AiqResult
     * param[in] TuningMode tuningMode: tuning mode
     *
     * \return OK if get Makernote successfully, otherwise return ERROR.
     */
    static int saveMakernoteData(int cameraId, camera_makernote_mode_t makernoteMode,
                                 int64_t sequence, TuningMode tuningMode);

    /**
     * \brief Update Makernote timestamp.
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] sequence: the sequence in frame buffer;
     * \param[in] timestamp: the frame timestamp corresponding sequence
     *
     */
    static void updateMakernoteTimeStamp(int cameraId, int64_t sequence, uint64_t timestamp);

    /**
     * \brief acquire Makernote data.
     *
     * \param[in] cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param[in] sequence: acquire MakerNote per timestamp
     * \param[out] buf: Makernote data will be saved.
     * \param[out] size: the size of Makernote data will be saved.
     *
     */
    static void acquireMakernoteData(int cameraId, uint64_t timestamp, uint8_t* buf,
                                     uint32_t& size);

    /*
     * Get the scaler info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param streamId: hal stream id
     * \param sclscalerWidth and scalerHeight : return related scaler info
     * \return OK.
     */
    static int getScalerInfo(int cameraId, int32_t streamId, float* scalerWidth,
                             float* scalerHeight);

    /*
     * Set the scaler info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param scalerInfo related IGraphType::ScalerInfo
     */
    static void setScalerInfo(int cameraId, std::vector<IGraphType::ScalerInfo> scalerInfo);

    /**
     * Check gpu tnr is enabled or not
     *
     * \return true if tnr is enabled.
     */
    static bool isGpuTnrEnabled(int cameraId);

    /**
     * get the video stream number supported
     *
     * \return HAL video stream number.
     */
    static int getVideoStreamNum();

    /**
     * Check if support to update tuning data or not
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if enable update tuning.
     */
    static bool supportUpdateTuning(int cameraId);

    /**
     * Check should connect gpu algo or not
     * should connect gpu algo service if any gpu algorithm is used
     * \return true if should connect gpu algo.
     */
    static bool isUsingGpuAlgo();

    /**
     * Check should connect gpu ipa or not on libcamera
     * should connect gpu algo service if any gpu algorithm is used
     * \return true if should connect gpu ipa.
     */
    static bool isUsingGpuIpa();

    /**
     * the extra frame count for still stream
     */
    static int getTnrExtraFrameCount(int cameraId);

    /**
     * Get time interval to align psys processing
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return time interval.
     */
    static int getMsOfPsysAlignWithSystem(int cameraId);
    /*
     * Set the orientation Info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \param sensor orientation
     */
    static void setSensorOrientation(int cameraId, int orientation);

    /*
     * Get the orientation Info
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return sensor orientation
     */
    static int getSensorOrientation(int cameraId);

    /**
     * check if support dummy still stream
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if supported.
     */
    static bool isDummyStillSink(int cameraId);

    /*
     * check if removing cache flush output buffer
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if removing cache flush output buffer.
     */
    static bool removeCacheFlushOutputBuffer(int cameraId);

    /*
     * Get PLC Enable status
     *
     * \param cameraId: [0, MAX_CAMERA_NUMBER - 1]
     * \return true if supported.
     */
    static bool getPLCEnable(int cameraId);

    /**
     * Check if use GPU to do post process or not
     *
     * \return true if use GPU to do post process.
     */
    static bool useGPUProcessor();

    /**
     * Check support of still-only pipe is enabled or not
     *
     * \return true if is enabled.
     */
    static bool isStillOnlyPipeEnabled(int cameraId);

    /**
     * Check if use IPU psys processor or not
     *
     * \return true if use IPU psys processor.
     */
    static bool isUsePSysProcessor(int cameraId);

};
} /* namespace icamera */
