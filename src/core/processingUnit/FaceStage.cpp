/*
 * Copyright (C) 2022-2024 Intel Corporation.
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

#define LOG_TAG FaceStage

#include "FaceStage.h"
#include "iutils/CameraLog.h"
#include "PlatformData.h"
#include "CameraContext.h"

using std::shared_ptr;

namespace icamera {

FaceStage::FaceStage(int cameraId, int streamId, const stream_t& stream, bool isPrivate)
        : CameraStream(cameraId, streamId, stream),
          ISchedulerNode("face"),
          mStreamInfo(stream),
          mIsPrivate(isPrivate),
          mFaceDetection(nullptr) {
    LOG1("%s, mIsPrivate: %d, width: %d, height: %d", __func__, mIsPrivate,
         mStreamInfo.width, mStreamInfo.height);

    mFaceDetection = std::unique_ptr<FaceDetection>(
            IFaceDetection::createFaceDetection(mCameraId, stream.width, stream.height));

    if (mIsPrivate || !PlatformData::runFaceWithSyncMode(mCameraId)) {
        for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
            std::shared_ptr<CameraBuffer> buffer = CameraBuffer::create(
                mFaceDetection->getMemoryType(), stream.size, i, stream.format, stream.width,
                stream.height);
            if (buffer == nullptr) {
                while (mAvailableBufferQ.empty() == false) {
                    mAvailableBufferQ.pop();
                }
                LOGE("failed to alloc %d internal buffers", i);
                break;
            }
            mAvailableBufferQ.push(buffer);
        }
    }
}

FaceStage::~FaceStage() {
    LOG1("%s, mIsPrivate: %d", __func__, mIsPrivate);
}

int FaceStage::start() {
    LOG1("<id%d>@%s, %p, mIsPrivate: %d", mCameraId, __func__, this, mIsPrivate);

    if ((mIsPrivate || !PlatformData::runFaceWithSyncMode(mCameraId))) {
        if (mAvailableBufferQ.empty()) {
            return NO_MEMORY;
        }
    }

    return OK;
}

int FaceStage::stop() {
    CameraStream::stop();

    while (mPendingBufferQ.empty() == false) {
        const auto buffer = mPendingBufferQ.front();
        mPendingBufferQ.pop();

        mAvailableBufferQ.push(buffer);
    }

    return OK;
}

// Q buffers to the stream processor which should be set by the CameraDevice
int FaceStage::qbuf(camera_buffer_t* ubuffer, int64_t sequence, bool addExtraBuf) {
    LOG2("<seq:%ld>, %s, mIsPrivate: %d, addExtraBuf: %d", sequence, __func__,
         mIsPrivate, addExtraBuf);

    int ret = BAD_VALUE;
    if (mIsPrivate) {
        shared_ptr<CameraBuffer> camBuffer = nullptr;
        if (addExtraBuf) {
            AutoMutex l(mBufferPoolLock);
            // use the internal buffer pool for private stream
            CheckAndLogError(mAvailableBufferQ.empty(), ret, "no buffer pool for private stream");

            camBuffer = mAvailableBufferQ.front();
            LOG2("<id%d:seq%ld>@%s, mStreamId:%d, CameraBuffer:%p for port:%d, addr:%p",
                 mCameraId, sequence, __func__, mStreamId, camBuffer.get(), mPort,
                 camBuffer->getBufferAddr());

            camBuffer->setSequence(0);
            struct timeval ts = {0};
            camBuffer->setTimestamp(ts);

            camBuffer->setSettingSequence(sequence);
        }

        if (mBufferProducer != nullptr) {
            ret = mBufferProducer->qbuf(mPort, camBuffer);
            if (ret == OK) {
                AutoMutex l(mBufferPoolLock);
                mAvailableBufferQ.pop();

                mBufferInProcessing++;
                LOG2("%s, mIsPrivate: %d, buffer in processing: %d", __func__, mIsPrivate,
                     mBufferInProcessing);
            }
        }
    } else {
        // use normal preview stream for face detection
        ret = CameraStream::qbuf(ubuffer, sequence);
    }

    return ret;
}

bool FaceStage::isFaceEnabled(int64_t sequence) {
    auto cameraContext = CameraContext::getInstance(mCameraId);
    auto dataContext = cameraContext->getDataContextBySeq(sequence);
    if (dataContext->mFaceDetectMode != CAMERA_STATISTICS_FACE_DETECT_MODE_OFF ||
        PlatformData::isFaceAeEnabled(mCameraId))
        return true;

    return false;
}

int FaceStage::onBufferAvailable(uuid port, const shared_ptr<CameraBuffer>& camBuffer) {
    // Ignore if the buffer is not for this stream.
    if (mPort != port) {
        return OK;
    }

    int64_t sequence = camBuffer->getSequence();
    if (mIsPrivate) {
        if (isFaceEnabled(sequence) && mFaceDetection->needRunFace(sequence) &&
            !PlatformData::runFaceWithSyncMode(mCameraId)) {
            AutoMutex l(mBufferPoolLock);
            LOG2("<seq%ld>%s, run face with ASYNC mode. mIsPrivate: %d",
                 sequence, __func__, mIsPrivate);
            mPendingBufferQ.push(camBuffer);
            return OK;
        }

        if (isFaceEnabled(sequence) && mFaceDetection->needRunFace(sequence)) {
            LOG2("<seq%ld>%s, run face with SYNC mode. mIsPrivate: %d",
                 sequence, __func__, mIsPrivate);
            mFaceDetection->runFaceDetection(camBuffer);
        }

        AutoMutex l(mBufferPoolLock);
        if (this->mBufferInProcessing > 0) {
            this->mBufferInProcessing--;
        }
        mAvailableBufferQ.push(camBuffer);
    } else {
        if (isFaceEnabled(sequence) && mFaceDetection->needRunFace(sequence)) {
            if (PlatformData::runFaceWithSyncMode(mCameraId)) {
                LOG2("<seq%ld>%s, run face with SYNC mode. mIsPrivate: %d",
                     sequence, __func__, mIsPrivate);
                mFaceDetection->runFaceDetection(camBuffer);
            } else {
                LOG2("<seq%ld>%s, run face with ASYNC mode. mIsPrivate: %d",
                     sequence, __func__, mIsPrivate);

                shared_ptr<CameraBuffer> faceBuffer;
                {
                    AutoMutex l(mBufferPoolLock);
                    CheckAndLogError(mAvailableBufferQ.empty(), NO_MEMORY, "No detection buffer");
                    faceBuffer = mAvailableBufferQ.front();
                }

                CameraBufferMapper mapper(camBuffer);
                CheckAndLogError(camBuffer->getBufferAddr() == nullptr, BAD_VALUE,
                                 "%s, Failed to get addr for camBuffer", __func__);

                MEMCPY_S(faceBuffer->getBufferAddr(), faceBuffer->getBufferSize(),
                         camBuffer->getBufferAddr(), camBuffer->getBufferSize());

                faceBuffer->setSequence(camBuffer->getSequence());

                AutoMutex l(mBufferPoolLock);
                mAvailableBufferQ.pop();
                mPendingBufferQ.push(faceBuffer);
            }
        }
        CameraStream::onBufferAvailable(port, camBuffer);
    }
    return OK;
}

bool FaceStage::process(int64_t triggerId) {
    PERF_CAMERA_ATRACE_PARAM1(getName(), triggerId);
    LOG2("%s, triggerId: %ld. mPrivate: %d", __func__, triggerId, mIsPrivate);
    shared_ptr<CameraBuffer> faceBuffer = nullptr;
    {
        AutoMutex l(mBufferPoolLock);
        if (mPendingBufferQ.empty()) {
            return true;
        }
        faceBuffer = mPendingBufferQ.front();
        mPendingBufferQ.pop();

        if (mPendingBufferQ.size() > PlatformData::getMaxPipelineDepth(mCameraId)) {
            LOG2("%s, Skip this time due to many buffer in pendding: %d", __func__,
                 mPendingBufferQ.size());

            if (mIsPrivate && this->mBufferInProcessing > 0) {
                this->mBufferInProcessing--;
            }
            mAvailableBufferQ.push(faceBuffer);
            return true;
        }
    }
    CheckAndLogError(!faceBuffer, false, "%s, the faceBuffer is nullptr", __func__);

    LOG2("<seq%ld>%s, run face detection. triggerId: %ld. mPrivate: %d",
         faceBuffer->getSequence(), __func__, triggerId, mIsPrivate);
    mFaceDetection->runFaceDetection(faceBuffer);

    AutoMutex l(mBufferPoolLock);
    if (mIsPrivate && this->mBufferInProcessing > 0) {
        this->mBufferInProcessing--;
    }
    mAvailableBufferQ.push(faceBuffer);

    return true;
}
}  // namespace icamera
