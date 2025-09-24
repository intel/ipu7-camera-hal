/*
 * Copyright (C) 2019-2025 Intel Corporation
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
#include <ia_types.h>

#include "CameraBuffer.h"
#include "FaceType.h"
#include "iutils/Utils.h"
#include "AiqUnit.h"

namespace icamera {

struct RatioInfo {
    camera_coordinate_system_t sysCoord;
    int verticalCrop;
    int horizontalCrop;
    bool imageRotationChanged;
};

class FaceDetection {
 public:
    FaceDetection(int cameraId, int width, int height, int memoryType = V4L2_MEMORY_USERPTR);
    virtual ~FaceDetection();

    bool needRunFace(int64_t sequence);
    virtual void runFaceDetection(const std::shared_ptr<CameraBuffer>& camBuffer) = 0;
    int getMemoryType() { return mMemoryType; }

 protected:
    void printfFDRunRate();
    void convertFaceCoordinate(camera_coordinate_system_t& sysCoord, int* left, int* top,
                               int* right, int* bottom);

 private:
    void initRatioInfo(struct RatioInfo* ratioInfo);
    int getFaceNum();

 protected:
    int mCameraId;
    bool mInitialized;
    int mWidth;
    int mHeight;
    unsigned int mMaxFaceNum;
    int mMemoryType;

    // Guard for mLastFaceNum
    std::mutex mFaceResultLock;
    int mLastFaceNum;
    struct RatioInfo mRatioInfo;

 private:
    unsigned int mDefaultInterval;  // FD running's interval frames.
    unsigned int mNoFaceInterval;   // FD running's interval frames without face.
    unsigned int mRunInterval;      // run 1 frame every mFDRunInterval frames.
};

}  // namespace icamera

