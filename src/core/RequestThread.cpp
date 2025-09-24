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

#define LOG_TAG RequestThread

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"

#include "CameraContext.h"
#include "RequestThread.h"

using std::vector;
using std::shared_ptr;

namespace icamera {

RequestThread::RequestThread(int cameraId, AiqUnitBase *a3AControl) :
    mCameraId(cameraId),
    m3AControl(a3AControl),
    mPerframeControlSupport(false),
    mGet3AStatWithFakeRequest(false),
    mRequestsInProcessing(0),
    mFirstRequest(true),
    mState(EXIT),
    mRequestTriggerEvent(NONE_EVENT),
    mLastCcaId(-1),
    mLastEffectSeq(-1),
    mLastAppliedSeq(-1),
    mLastSofSeq(-1),
    mBlockRequest(true),
    mSofEnabled(false) {
    CLEAR(mFakeReqBuf);

    mPerframeControlSupport = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);

    mSofEnabled = PlatformData::isIsysEnabled(cameraId);
    // FILE_SOURCE_S
    mSofEnabled = mSofEnabled || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E
}

RequestThread::~RequestThread() {
}

void RequestThread::requestStart() {
    mState = START;
    Thread::start();
}

void RequestThread::requestStop() {
    mState = EXIT;
    clearRequests();

    Thread::exit();
    {
        AutoMutex l(mPendingReqLock);
        mRequestSignal.notify_one();
    }

    Thread::wait();
}

void RequestThread::clearRequests() {
    LOG1("%s", __func__);

    for (int streamId = 0; streamId < MAX_STREAM_NUMBER; streamId++) {
        FrameQueue& frameQueue = mOutputFrames[streamId];
        AutoMutex lock(frameQueue.mFrameMutex);
        while (!frameQueue.mFrameQueue.empty()) {
            frameQueue.mFrameQueue.pop();
        }
        frameQueue.mFrameAvailableSignal.notify_all();
    }

    AutoMutex l(mPendingReqLock);
    mRequestsInProcessing = 0;
    while (!mPendingRequests.empty()) {
        mPendingRequests.pop_back();
    }

    mLastCcaId = -1;
    mLastEffectSeq = -1;
    mLastAppliedSeq = -1;
    mLastSofSeq = -1;
    mFirstRequest = true;
    mBlockRequest = true;
}

int RequestThread::configure(const stream_config_t *streamList) {
    int previewIndex = -1, videoIndex = -1, stillIndex = -1;
    for (int i = 0; i < streamList->num_streams; i++) {
        if (streamList->streams[i].usage == CAMERA_STREAM_PREVIEW) {
            previewIndex = i;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_VIDEO_CAPTURE) {
            videoIndex = i;
        } else if (streamList->streams[i].usage == CAMERA_STREAM_STILL_CAPTURE) {
            stillIndex = i;
        }
    }

    // Don't block request handling if no 3A stats (from video pipe)
    mBlockRequest = PlatformData::isEnableAIQ(mCameraId) && ((previewIndex >= 0) ||
                                                             (videoIndex >= 0));
    LOG1("%s: user specified Configmode: %d, blockRequest: %d", __func__,
         static_cast<ConfigMode>(streamList->operation_mode), mBlockRequest);

    mGet3AStatWithFakeRequest =
        mPerframeControlSupport ? PlatformData::isPsysContinueStats(mCameraId) : false;
    if (mGet3AStatWithFakeRequest) {
        const int fakeStreamIndex = (previewIndex >= 0) ? previewIndex :
                              ((videoIndex >= 0) ? videoIndex : stillIndex);
        if (fakeStreamIndex < 0) {
            LOGW("There isn't valid stream to trigger stats event");
            mGet3AStatWithFakeRequest = false;
            return OK;
        }

        CLEAR(mFakeReqBuf);
        stream_t &fakeStream = streamList->streams[fakeStreamIndex];
        LOG2("%s: create fake request with stream index %d", __func__, fakeStreamIndex);
        mFakeBuffer = CameraBuffer::create(V4L2_MEMORY_USERPTR, fakeStream.size, 0,
                                           fakeStream.format, fakeStream.width,
                                           fakeStream.height);

        mFakeReqBuf.s = fakeStream;
        mFakeReqBuf.s.memType = V4L2_MEMORY_USERPTR;
        mFakeReqBuf.addr = mFakeBuffer->getUserBuffer()->addr;
    }

    return OK;
}

bool RequestThread::blockRequest() {
    if (mPendingRequests.empty()) {
        return true;
    }

    /**
     * Block request processing if:
     * 1. mBlockRequest is true (except the 1st request);
     * 2. Too many requests in flight;
     * 3. if no trigger event is available.
     */
    const int maxRequest = PlatformData::getMaxRequestsInflight(mCameraId);

    return ((mBlockRequest && (mLastCcaId >= 0)) ||
        (mRequestsInProcessing >= maxRequest) ||
        (mPerframeControlSupport && (mRequestTriggerEvent == NONE_EVENT)));
}

int RequestThread::processRequest(int bufferNum, camera_buffer_t **ubuffer) {
    AutoMutex l(mPendingReqLock);
    CameraRequest request;
    request.mBufferNum = bufferNum;
    bool hasVideoBuffer = false;

    for (int id = 0; id < bufferNum; id++) {
        request.mBuffer[id] = ubuffer[id];
        if ((ubuffer[id]->s.usage == CAMERA_STREAM_PREVIEW) ||
            (ubuffer[id]->s.usage == CAMERA_STREAM_VIDEO_CAPTURE)) {
            hasVideoBuffer = true;
        }
    }

    if (mFirstRequest && (!hasVideoBuffer)) {
        LOG2("there is no video buffer in first request, so don't block request processing.");
        mBlockRequest = false;
    }

    mPendingRequests.push_back(request);

    if (mState != PROCESSING) {
        mState = PROCESSING;
    }

    mRequestTriggerEvent |= static_cast<uint32_t>(NEW_REQUEST);
    mRequestSignal.notify_one();
    return OK;
}

int RequestThread::waitFrame(int streamId, camera_buffer_t **ubuffer) {
    FrameQueue& frameQueue = mOutputFrames[streamId];
    std::unique_lock<std::mutex> lock(frameQueue.mFrameMutex);

    if (mState == EXIT) {
        return NO_INIT;
    }
    while (frameQueue.mFrameQueue.empty()) {
        std::cv_status ret = frameQueue.mFrameAvailableSignal.wait_for(
                      lock,
                      std::chrono::nanoseconds(kWaitFrameDuration * SLOWLY_MULTIPLIER));
        if (mState == EXIT) {
            return NO_INIT;
        }

        CheckWarning(ret == std::cv_status::timeout, TIMED_OUT,
                     "<id%d>@%s, time out happens, wait recovery", mCameraId, __func__);
    }

    shared_ptr<CameraBuffer> camBuffer = frameQueue.mFrameQueue.front();
    frameQueue.mFrameQueue.pop();
    *ubuffer = camBuffer->getUserBuffer();

    LOG2("@%s, frame returned. camera id:%d, stream id:%d", __func__, mCameraId, streamId);

    return OK;
}

int RequestThread::wait1stRequestDone() {
    int ret = OK;
    std::unique_lock<std::mutex> lock(mFirstRequestLock);
    if (mFirstRequest) {
        LOG2("%s, waiting the first request done", __func__);
        std::cv_status status = mFirstRequestSignal.wait_for(
                  lock,
                  std::chrono::nanoseconds(kWaitFirstRequestDoneDuration * SLOWLY_MULTIPLIER));
        if (status == std::cv_status::timeout) {
            LOGE("@%s: Wait 1st request timed out", __func__);
            ret = TIMED_OUT;
        }
    }

    return ret;
}

void RequestThread::handleEvent(EventData eventData) {
    if (mState == EXIT) {
        return;
    }

    /* Notes:
      * There should be only one of EVENT_ISYS_FRAME
      * and EVENT_PSYS_FRAME registered.
      * There should be only one of EVENT_xx_STATS_BUF_READY
      * registered.
      */
    switch (eventData.type) {
        case EVENT_ISYS_FRAME:
        case EVENT_PSYS_FRAME:
            {
                AutoMutex l(mPendingReqLock);
                if (mRequestsInProcessing > 0) {
                    mRequestsInProcessing--;
                }
                // Just in case too many requests are pending in mPendingRequests.
                if (!mPendingRequests.empty()) {
                    mRequestTriggerEvent |= static_cast<uint32_t>(NEW_FRAME);
                    mRequestSignal.notify_one();
                }
            }
            break;
        case EVENT_PSYS_STATS_BUF_READY:
            {
                TRACE_LOG_POINT("RequestThread", "receive the stat event");
                AutoMutex l(mPendingReqLock);
                if (mBlockRequest) {
                    mBlockRequest = false;
                }
                mRequestTriggerEvent |= static_cast<uint32_t>(NEW_STATS);
                mRequestSignal.notify_one();
            }
            break;
        case EVENT_ISYS_SOF:
            {
                AutoMutex l(mPendingReqLock);
                mLastSofSeq = eventData.data.sync.sequence;
                mRequestTriggerEvent |= static_cast<uint32_t>(NEW_SOF);
                mRequestSignal.notify_one();
            }
            break;
        case EVENT_FRAME_AVAILABLE:
            {
                if (eventData.buffer->getUserBuffer() != &mFakeReqBuf) {
                    const int streamId = eventData.data.frameDone.streamId;
                    FrameQueue& frameQueue = mOutputFrames[streamId];

                    AutoMutex lock(frameQueue.mFrameMutex);
                    const bool needSignal = frameQueue.mFrameQueue.empty();
                    frameQueue.mFrameQueue.push(eventData.buffer);
                    if (needSignal) {
                        frameQueue.mFrameAvailableSignal.notify_one();
                    }
                } else {
                    LOG2("%s: fake request return %u", __func__, eventData.buffer->getSequence());
                }

                AutoMutex l(mPendingReqLock);
                // Insert fake request if no any request in the HAL to keep 3A running
                if (mGet3AStatWithFakeRequest &&
                    (eventData.buffer->getSequence() >= mLastEffectSeq) &&
                    mPendingRequests.empty()) {
                    LOGW("No request, insert fake req after req %ld to keep 3A stats update",
                         mLastCcaId);
                    CameraRequest fakeRequest;
                    fakeRequest.mBufferNum = 1;
                    fakeRequest.mBuffer[0] = &mFakeReqBuf;
                    mFakeReqBuf.sequence = -1;
                    mPendingRequests.push_back(fakeRequest);
                    mRequestTriggerEvent |= static_cast<uint32_t>(NEW_REQUEST);
                    mRequestSignal.notify_one();
                }
            }
            break;
        default:
            {
                LOGW("Unknown event type %d", eventData.type);
            }
            break;
    }
}

/**
 * Get the next request for processing.
 * Return false if no pending requests or it is not ready for reconfiguration.
 */
bool RequestThread::fetchNextRequest(CameraRequest& request) {
    std::unique_lock<std::mutex> lock(mPendingReqLock);
    if (mPendingRequests.empty()) {
        return false;
    }

    request = mPendingRequests.front();
    mRequestsInProcessing++;
    mPendingRequests.pop_front();
    LOG2("@%s, mRequestsInProcessing %d", __func__, mRequestsInProcessing);
    return true;
}

bool RequestThread::threadLoop() {
    if (mState == EXIT) {
        return false;
    }

    int64_t applyingSeq = -1;
    {
         std::unique_lock<std::mutex> lock(mPendingReqLock);

         if (blockRequest()) {
            std::cv_status ret = mRequestSignal.wait_for(lock,
                std::chrono::nanoseconds(kWaitDuration * SLOWLY_MULTIPLIER));
            CheckWarning(ret == std::cv_status::timeout, true,
                         "wait event time out, %d requests processing, %zu requests in HAL",
                         mRequestsInProcessing, mPendingRequests.size());

            if (blockRequest()) {
                LOG2("Pending request processing, mBlockRequest %d, Req in processing %d",
                     mBlockRequest, mRequestsInProcessing);
                mRequestTriggerEvent = static_cast<uint32_t>(NONE_EVENT);
                return true;
            }
        }

        /* for perframe control cases, one request should be processed in one SOF period only.
         * 1, for new SOF, processes request for current sequence if no request processed for it;
         * 2, for new stats, processes request for next sequence;
         * 3, for new request or frame done, processes request only no buffer processed in HAL.
         */
        if (mPerframeControlSupport && (mRequestTriggerEvent != NONE_EVENT)) {
            if (((mRequestTriggerEvent & static_cast<uint32_t>(NEW_SOF)) != 0U) &&
                (mLastSofSeq > mLastAppliedSeq)) {
                applyingSeq = mLastSofSeq;
            } else if (((mRequestTriggerEvent & static_cast<uint32_t>(NEW_STATS)) != 0U) &&
                       (mLastSofSeq >= mLastAppliedSeq)) {
                applyingSeq = mLastSofSeq + 1;
            } else if (((mRequestTriggerEvent & (static_cast<uint32_t>(NEW_REQUEST)
                        | static_cast<uint32_t>(NEW_FRAME))) != 0U) &&
                       (mRequestsInProcessing == 0U)) {
                applyingSeq = mLastSofSeq + 1;
            } else {
                mRequestTriggerEvent = static_cast<uint32_t>(NONE_EVENT);
                return true;
            }

            mLastAppliedSeq = applyingSeq;
            if ((mLastAppliedSeq + PlatformData::getExposureLag(mCameraId)) <= mLastEffectSeq) {
                mRequestTriggerEvent = static_cast<uint32_t>(NONE_EVENT);
                LOG2("%s, skip processing request for AE delay issue", __func__);
                return true;
            }
            LOG2("%s, trigger event %x, SOF %ld, predict %ld, processed %d request id %ld",
                 __func__, mRequestTriggerEvent, mLastSofSeq, mLastAppliedSeq,
                 mRequestsInProcessing, mLastCcaId);
        }
    }

    if (mState == EXIT) {
        return false;
    }

    CameraRequest request;
    if (fetchNextRequest(request)) {
        handleRequest(request, applyingSeq);
        {
            AutoMutex l(mPendingReqLock);
            mRequestTriggerEvent = static_cast<uint32_t>(NONE_EVENT);
        }
    }
    return true;
}

void RequestThread::handleRequest(CameraRequest& request, int64_t applyingSeq) {
    int64_t effectSeq = 0;
    {
        std::lock_guard<std::mutex> l(mPendingReqLock);
        effectSeq = mLastEffectSeq + 1;
    }
    // Reprocessing case, don't run 3A.
    if (IS_INPUT_BUFFER(request.mBuffer[0]->timestamp, request.mBuffer[0]->sequence)) {
        effectSeq = request.mBuffer[0]->sequence;
        LOG2("%s: Reprocess request: seq %ld, out buffer %d", __func__,
             effectSeq, request.mBufferNum);
    } else {
        int64_t ccaId = -1;
        {
            AutoMutex l(mPendingReqLock);
            if (mState != EXIT) {
                ++mLastCcaId;
                ccaId = mLastCcaId;
            }
        }

        if (ccaId >= 0) {
            m3AControl->run3A(ccaId, applyingSeq, request.mBuffer[0]->frameNumber,
                              mSofEnabled ? &effectSeq : nullptr);
        }

        {
            AutoMutex l(mPendingReqLock);
            if (mState == EXIT) {
                return;
            }

            // Check the final prediction value from 3A
            if (effectSeq <= mLastEffectSeq) {
                LOG2("predict effectSeq %ld, last effect %ld", effectSeq, mLastEffectSeq);
            }

            mLastEffectSeq = effectSeq;

            LOG2("%s: Process cca id: %ld:%ld, out buffer %d", __func__,
                 mLastCcaId, effectSeq, request.mBufferNum);
        }

        auto cameraContext = CameraContext::getInstance(mCameraId);
        const auto dataContext =
            cameraContext->acquireDataContextByFn(request.mBuffer[0]->frameNumber);

        cameraContext->updateDataContextMapBySeq(effectSeq, dataContext);
        cameraContext->updateDataContextMapByCcaId(mLastCcaId, dataContext);
    }

    // Sent event to handle request buffers
    EventRequestData requestData;
    requestData.bufferNum = request.mBufferNum;
    requestData.buffer = request.mBuffer;
    requestData.settingSeq = effectSeq;
    EventData eventData;
    eventData.type = EVENT_PROCESS_REQUEST;
    eventData.data.request = requestData;
    notifyListeners(eventData);

    {
        AutoMutex l(mFirstRequestLock);
        if (mFirstRequest) {
            LOG1("%s: first request done", __func__);
            mFirstRequest = false;
            mFirstRequestSignal.notify_one();
        }
    }
}

} //namespace icamera
