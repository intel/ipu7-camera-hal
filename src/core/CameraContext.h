/*
 * Copyright (C) 2022-2025 Intel Corporation.
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

#include <map>
#include <mutex>
#include <memory>
#include <cstring>

#include "AiqSetting.h"
#include "CameraTypes.h"
#include "ParamDataType.h"
// JPEG_ENCODE_S
#include "EXIFMetaData.h"
// JPEG_ENCODE_E

namespace icamera {

class AiqResultStorage;
class GraphConfig;

struct IspParameters {
    camera_edge_mode_t edgeMode;
    camera_nr_mode_t nrMode;
    struct NrLevel {
        bool set;
        camera_nr_level_t nrLevel;
    } nrLevel;
    camera_image_enhancement_t enhancement;

    float digitalZoomRatio;

    IspParameters() {
        memset(this, 0, sizeof(*this));

        nrMode = NR_MODE_LEVEL_2;
    }
};

// JPEG_ENCODE_S
struct JpegParameters {
    double latitude;
    double longitude;
    double altitude;
    char gpsProcessingMethod[MAX_NUM_GPS_PROCESSING_METHOD + 1];
    uint8_t gpsProcessingMethodSize;
    int64_t gpsTimestamp;
    int32_t rotation;
    uint8_t jpegQuality;
    uint8_t thumbQuality;
    camera_resolution_t thumbSize;
    float focalLength;
    float aperture;

    JpegParameters() {
        memset(this, 0, sizeof(*this));

        jpegQuality = DEFAULT_JPEG_QUALITY;
        thumbQuality = DEFAULT_JPEG_QUALITY;
    }
};
// JPEG_ENCODE_E

class DataContext {
 public:
    int64_t mFrameNumber;
    int64_t mSequence;
    int64_t mCcaId;  // Used for CCA algo

    uint8_t mFaceDetectMode;
    camera_mono_downscale_mode_t monoDsMode;
    camera_deinterlace_mode_t deinterlaceMode;

    camera_crop_region_t cropRegion;
    camera_zoom_region_t zoomRegion;

    struct aiq_parameter_t mAiqParams;
    struct IspParameters mIspParams;
// JPEG_ENCODE_S
    struct JpegParameters mJpegParams;
// JPEG_ENCODE_E

    DataContext(int cameraId);
    ~DataContext() {}

    void reset();
    DataContext& operator=(const DataContext& other) {
        mFaceDetectMode = other.mFaceDetectMode;
        monoDsMode = other.monoDsMode;
        deinterlaceMode = other.deinterlaceMode;
        cropRegion = other.cropRegion;
        zoomRegion = other.zoomRegion;
        mAiqParams = other.mAiqParams;
        mIspParams = other.mIspParams;
// JPEG_ENCODE_S
        mJpegParams = other.mJpegParams;
// JPEG_ENCODE_E
        return *this;
    }

    void setFrameNumber(int64_t frameNumber) { mFrameNumber = frameNumber; }
    void setSequence(int64_t sequence) { mSequence = sequence; }
    void setCcaId(int64_t ccaId) { mCcaId = ccaId; }
}; /* DataContext */

class CameraContext {
 public:
    explicit CameraContext(int cameraId);
    ~CameraContext();

    static CameraContext* getInstance(int cameraId);
    static void releaseInstance(int cameraId);

    void reset();
    // used to save aiq, face and statistics results
    AiqResultStorage* getAiqResultStorage();

    // only called when parsing request once
    DataContext* acquireDataContext();
    void updateDataContextMapByFn(int64_t frameNumber, DataContext* context);

    // only called when raw reprocessing
    DataContext* getReprocessingDataContextBySeq(int64_t sequence);

    std::shared_ptr<GraphConfig> getGraphConfig(ConfigMode configMode);
    void storeGraphConfig(std::map<ConfigMode, std::shared_ptr<GraphConfig> > gcs);

    // only called when handling request once
    DataContext* acquireDataContextByFn(int64_t frameNumber);
    void updateDataContextMapBySeq(int64_t sequence, DataContext* context);
    void updateDataContextMapByCcaId(int64_t ccaId, DataContext* context);

    // called runtimely after request has been handled
    const DataContext* getDataContextBySeq(int64_t sequence);
    const DataContext* getDataContextByCcaId(int64_t ccaId);
    bool checkUserRequestBySeq(int64_t sequence);

 private:
    void eraseDataContextMap(const DataContext* context);

 private:
    static std::map<int, CameraContext*> sInstances;
    // Guard for singleton creation
    static std::mutex sLock;
    static const int kContextSize = MAX_SETTING_COUNT;

    int mCameraId;

    int mCurrentIndex;  // range from 0 to MAX_SETTING_COUNT - 1;
    DataContext* mDataContext[kContextSize];

    AiqResultStorage* mAiqResultStorage;

    std::mutex mLock;  // Guard all Maps and public APIs
    std::map<int64_t, DataContext*> mFnToDataContextMap;
    std::map<int64_t, DataContext*> mSeqToDataContextMap;
    std::map<int64_t, DataContext*> mCcaIdToDataContextMap;
    std::map<ConfigMode, std::shared_ptr<GraphConfig> > mGraphConfigMap;
}; /* CameraContext */

} /* namespace icamera */
