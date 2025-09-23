/*
 * Copyright (C) 2022-2025 Intel Corporation
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

#define LOG_TAG CameraContext

#include <CameraContext.h>

#include <Utils.h>

#include "PlatformData.h"
#include "AiqResultStorage.h"
#include "iutils/CameraLog.h"

namespace icamera {

std::map<int32_t, CameraContext*> CameraContext::sInstances;
std::mutex CameraContext::sLock;

DataContext::DataContext(int cameraId) :
    mFrameNumber(-1),
    mSequence(-1),
    mCcaId(-1),
    mFaceDetectMode(0),
    monoDsMode(MONO_DS_MODE_OFF),
    deinterlaceMode(DEINTERLACE_OFF) {
    cropRegion = {0, 0, 0};
    zoomRegion = {0, 0, 0, 0, 1.0F, icamera::ROTATE_NONE};

    camera_coordinate_system_t activePixelArray = PlatformData::getActivePixelArray(cameraId);
    if ((activePixelArray.right > activePixelArray.left) &&
        (activePixelArray.bottom > activePixelArray.top)) {
        mAiqParams.resolution.width = activePixelArray.right - activePixelArray.left;
        mAiqParams.resolution.height = activePixelArray.bottom - activePixelArray.top;
    }
    const StaticMetadata *staticMetadata = PlatformData::getStaticMetadata(cameraId);
    if (staticMetadata->mEvRange.size() == 2) {
        mAiqParams.evRange = {static_cast<float>(staticMetadata->mEvRange[0]),
                             static_cast<float>(staticMetadata->mEvRange[1])};
    }
    if (staticMetadata->mEvStep.size() == 2) {
        mAiqParams.evStep = {staticMetadata->mEvStep[0], staticMetadata->mEvStep[1]};
    };

    std::string str = "lens.info.shadingMapSize";
    const auto vI = PlatformData::getInt32StaticMetadata(cameraId, str);
    if (vI.size() == 2) {
        mAiqParams.lensShadingMapSize = {vI[0], vI[1]};
    }
    str = "lens.info.minimumFocusDistance";
    const auto vF = PlatformData::getFloatStaticMetadata(cameraId, str);
    if (vF.size() == 1) {
        mAiqParams.minFocusDistance = vF[0];
    }
}

void DataContext::reset() {
    mFrameNumber = -1;
    mSequence = -1;
    mCcaId = -1;
}

CameraContext* CameraContext::getInstance(int cameraId) {
    std::lock_guard<std::mutex> lock(sLock);

    if (sInstances.find(cameraId) != sInstances.end()) {
        return sInstances[cameraId];
    }

    sInstances[cameraId] = new CameraContext(cameraId);

    return sInstances[cameraId];
}

void CameraContext::releaseInstance(int cameraId) {
    std::lock_guard<std::mutex> lock(sLock);

    if (sInstances.find(cameraId) != sInstances.end()) {
        CameraContext* context = sInstances[cameraId];
        sInstances.erase(cameraId);
        delete context;
    }
}

CameraContext::CameraContext(int cameraId) :
    mCameraId(cameraId),
    mCurrentIndex(-1) {
    LOG1("<id%d> %s", cameraId, __func__);
    for (int i = 0; i < kContextSize; i++) {
        mDataContext[i] = new DataContext(mCameraId);
    }
    mAiqResultStorage = new AiqResultStorage(mCameraId);
}

CameraContext::~CameraContext() {
    LOG1("<id%d> %s", mCameraId, __func__);

    mFnToDataContextMap.clear();
    mSeqToDataContextMap.clear();
    mCcaIdToDataContextMap.clear();

    delete mAiqResultStorage;
    for (int i = 0; i < kContextSize; i++) {
        delete mDataContext[i];
    }
}

void CameraContext::reset() {
    LOG2("<id%d> %s", mCameraId, __func__);
    mFnToDataContextMap.clear();
    mSeqToDataContextMap.clear();
    mCcaIdToDataContextMap.clear();

    for (int i = 0; i < kContextSize; i++) {
        mDataContext[i]->reset();
    }
}

AiqResultStorage* CameraContext::getAiqResultStorage() {
    return mAiqResultStorage;
}

DataContext* CameraContext::acquireDataContext() {
    LOG2("<id%d> %s", mCameraId, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    mCurrentIndex += 1;
    mCurrentIndex %= kContextSize;

    if (mDataContext[mCurrentIndex]->mSequence >= 0) {
        eraseDataContextMap(mDataContext[mCurrentIndex]);
    }

    return mDataContext[mCurrentIndex];
}

void CameraContext::updateDataContextMapByFn(int64_t frameNumber, DataContext* context) {
    LOG2("<id%d:fn%ld> %s", mCameraId, frameNumber, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    context->setFrameNumber(frameNumber);
    mFnToDataContextMap[frameNumber] = context;
}

DataContext* CameraContext::acquireDataContextByFn(int64_t frameNumber) {
    LOG2("<id%d:fn%ld> %s", mCameraId, frameNumber, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    if (mFnToDataContextMap.find(frameNumber) != mFnToDataContextMap.end()) {
        return mFnToDataContextMap[frameNumber];
    }

    LOGW("Failed to find context for fn %ld", frameNumber);
    // if mCurrentIndex is -1, use 0 as default setting
    return mDataContext[(mCurrentIndex == -1) ? 0 : mCurrentIndex];
}

DataContext* CameraContext::getReprocessingDataContextBySeq(int64_t sequence) {
    LOG2("<id%d:seq%ld> %s", mCameraId, sequence, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    if(mSeqToDataContextMap.find(sequence) != mSeqToDataContextMap.end()) {
        return mSeqToDataContextMap[sequence];
    }

    LOGW("Failed to find seq %ld for reprocessing", sequence);

    // create a DataContext for reprocessing with the nearest sequence
    mCurrentIndex += 1;
    mCurrentIndex %= kContextSize;

    if (mDataContext[mCurrentIndex]->mSequence >= 0) {
        eraseDataContextMap(mDataContext[mCurrentIndex]);
    }

    auto it = mSeqToDataContextMap.upper_bound(sequence);
    if (it != mSeqToDataContextMap.begin()) {
        --it;
        *mDataContext[mCurrentIndex] = *(it)->second;
    }
    mDataContext[mCurrentIndex]->setSequence(sequence);
    mSeqToDataContextMap[sequence] = mDataContext[mCurrentIndex];

    return mDataContext[mCurrentIndex];
}

void CameraContext::storeGraphConfig(std::map<ConfigMode, std::shared_ptr<GraphConfig> > gcs) {
    AutoMutex l(mLock);
    mGraphConfigMap = gcs;
}

std::shared_ptr<GraphConfig> CameraContext::getGraphConfig(ConfigMode configMode) {
    AutoMutex l(mLock);
    for (auto& gc : mGraphConfigMap) {
        if (gc.first == configMode) {
            return gc.second;
        }
    }

    return nullptr;
}

void CameraContext::updateDataContextMapBySeq(int64_t sequence, DataContext* context) {
    LOG2("<id%d:seq%ld> %s", mCameraId, sequence, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    context->setSequence(sequence);
    mSeqToDataContextMap[sequence] = context;
}

void CameraContext::updateDataContextMapByCcaId(int64_t ccaId, DataContext* context) {
    LOG2("<id%d:cca%ld> %s", mCameraId, ccaId, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    context->setCcaId(ccaId);
    mCcaIdToDataContextMap[ccaId] = context;
}

void CameraContext::eraseDataContextMap(const DataContext* context) {
    mFnToDataContextMap.erase(context->mFrameNumber);
    mSeqToDataContextMap.erase(context->mSequence);
    mCcaIdToDataContextMap.erase(context->mCcaId);
}

const DataContext* CameraContext::getDataContextBySeq(int64_t sequence) {
    LOG2("<id%d:seq%ld> %s", mCameraId, sequence, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    if(mSeqToDataContextMap.find(sequence) != mSeqToDataContextMap.end()) {
        return mSeqToDataContextMap[sequence];
    }

    // search from the newest result
    for (int i = 0; i < kContextSize; i++) {
        const int tmpIdx = (mCurrentIndex + kContextSize - i) % kContextSize;
        if ((mDataContext[tmpIdx]->mSequence >= 0) &&
            (sequence >= mDataContext[tmpIdx]->mSequence)) {
            return mDataContext[tmpIdx];
        }
    }

    LOGW("Failed to find context for seq %ld", sequence);
    // if mCurrentIndex is -1, use 0 as default setting
    return mDataContext[(mCurrentIndex == -1) ? 0 : mCurrentIndex];
}

const DataContext* CameraContext::getDataContextByCcaId(int64_t ccaId) {
    LOG2("<id%d:cca%ld> %s", mCameraId, ccaId, __func__);

    std::lock_guard<std::mutex> lock(mLock);
    if (mCcaIdToDataContextMap.find(ccaId) != mCcaIdToDataContextMap.end()) {
        return mCcaIdToDataContextMap[ccaId];
    }

    LOGW("Failed to find context for ccaId %ld", ccaId);
    return nullptr;
}

bool CameraContext::checkUserRequestBySeq(int64_t sequence) {
    std::lock_guard<std::mutex> lock(mLock);
    return mSeqToDataContextMap.find(sequence) == mSeqToDataContextMap.end() ? false : true;
}

}  // namespace icamera

