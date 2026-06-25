/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#define LOG_TAG AiqResult

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "AiqResult.h"

namespace icamera {

AiqResult::AiqResult(int cameraId) :
    mCameraId(cameraId),
    mTimestamp(0),
    mSequence(-1),
    mFrameId(-1),
    mTuningMode(TUNING_MODE_VIDEO),
    mAfDistanceDiopters(0.0f),
    mSkip(false),
    mLensPosition(0),
    mSceneMode(SCENE_MODE_AUTO),
    mFrameDuration(0),
    mRollingShutter(0) {
    CLEAR(mCustomControls);
    CLEAR(mCustomControlsParams);
    CLEAR(mAwbResults);
    CLEAR(mGbceResults);
    CLEAR(mPaResults);
    CLEAR(mAeResults);
    CLEAR(mAfResults);
    CLEAR(mOutStats);
    CLEAR(mFocusRange);
    CLEAR(mLensShadingMap);
}

AiqResult::~AiqResult() {
    deinit();
}

AiqResult::AiqResult(const AiqResult& other) :
    mCameraId(other.mCameraId),
    mTimestamp(other.mTimestamp),
    mSequence(other.mSequence),
    mFrameId(other.mFrameId),
    mTuningMode(other.mTuningMode),
    mAfDistanceDiopters(other.mAfDistanceDiopters),
    mSkip(other.mSkip),
    mFocusRange(other.mFocusRange),
    mLensPosition(other.mLensPosition),
    mSceneMode(other.mSceneMode),
    mAeResults(other.mAeResults),
    mAwbResults(other.mAwbResults),
    mAfResults(other.mAfResults),
    mGbceResults(other.mGbceResults),
    mPaResults(other.mPaResults),
    mOutStats(other.mOutStats),
    mFrameDuration(other.mFrameDuration),
    mRollingShutter(other.mRollingShutter) {
    CLEAR(mCustomControls);
    CLEAR(mCustomControlsParams);

    mOutStats.rgbs_grid[0].blocks_ptr = mOutStats.rgbs_blocks[0];

    mCustomControls.count = other.mCustomControls.count;
    mCustomControls.parameters = mCustomControlsParams;
    for (int i = 0; i < mCustomControls.count; i++) {
        mCustomControlsParams[i] = other.mCustomControlsParams[i];
    }
    MEMCPY_S(mLensShadingMap, sizeof(mLensShadingMap),
             other.mLensShadingMap, sizeof(other.mLensShadingMap));
}

int AiqResult::init() {
    CLEAR(mAeResults);
    CLEAR(mAfResults);
    CLEAR(mAwbResults);
    CLEAR(mPaResults);
    CLEAR(mOutStats);
    mOutStats.rgbs_grid[0].blocks_ptr = mOutStats.rgbs_blocks[0];

    /*AE results init */
    mAeResults.num_exposures = 1;

    /* Custom Controls init */
    mCustomControls.parameters = mCustomControlsParams;

    return OK;
}

int AiqResult::deinit() const {
    return OK;
}

AiqResult &AiqResult::operator=(const AiqResult &other) {
    mCameraId = other.mCameraId;
    mSequence = other.mSequence;
    mFrameId = other.mFrameId;
    mTimestamp = other.mTimestamp;
    mTuningMode = other.mTuningMode;
    mAfDistanceDiopters = other.mAfDistanceDiopters;
    mSkip = other.mSkip;
    mLensPosition = other.mLensPosition;
    mSceneMode = other.mSceneMode;
    mFocusRange = other.mFocusRange;

    mAeResults = other.mAeResults;
    mAwbResults = other.mAwbResults;
    mAfResults = other.mAfResults;
    mGbceResults = other.mGbceResults;
    mPaResults = other.mPaResults;
    mOutStats = other.mOutStats;
    mOutStats.rgbs_grid[0].blocks_ptr = mOutStats.rgbs_blocks[0];

    mCustomControls.count = other.mCustomControls.count;
    for (int i = 0; i < mCustomControls.count; i++) {
        mCustomControlsParams[i] = other.mCustomControlsParams[i];
    }
    MEMCPY_S(mLensShadingMap, sizeof(mLensShadingMap),
             other.mLensShadingMap, sizeof(other.mLensShadingMap));

    mFrameDuration = other.mFrameDuration;
    mRollingShutter = other.mRollingShutter;

    return *this;
}
} /* namespace icamera */
