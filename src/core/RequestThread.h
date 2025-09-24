/*
 * Copyright (C) 2016-2025 Intel Corporation.
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
#include <atomic>
#include <condition_variable>
#include <deque>

#include "iutils/Thread.h"
#include "PlatformData.h"
#include "AiqUnit.h"

namespace icamera {

/*
 * The RequestThread is used to assist CameraDevice to handle request(qbuf/dqbuf).
 */
class RequestThread : public Thread, public EventSource, public EventListener {
public:
    RequestThread(int cameraId, AiqUnitBase *a3AControl);
    ~RequestThread();

    void requestStart();
    void requestStop();

    virtual void handleEvent(EventData eventData);

    /**
     * \Clear pending requests.
     */
    void clearRequests();

    /**
     * \Accept requests from user.
     */
    int processRequest(int bufferNum, camera_buffer_t **ubuffer);

    int waitFrame(int streamId, camera_buffer_t **ubuffer);

    /**
     * \Block the caller until the first request is processed.
     */
    int wait1stRequestDone();

    /**
     * \brief configure and create buffer for fake request.
     *
     * \param streamList: all the streams info
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int configure(const stream_config_t *streamList);

private:
    virtual void run() {
        bool ret = true;
        while (ret) {
            ret = threadLoop();
        }
    }
    virtual bool threadLoop();

    int mCameraId;
    AiqUnitBase *m3AControl;
    bool mPerframeControlSupport;
    bool mGet3AStatWithFakeRequest;
    camera_buffer_t mFakeReqBuf;
    std::shared_ptr<CameraBuffer> mFakeBuffer;

    struct CameraRequest {
        CameraRequest() : mBufferNum(0) {
            CLEAR(mBuffer);
        }

        int mBufferNum;
        camera_buffer_t *mBuffer[MAX_STREAM_NUMBER];
    };

    /**
     * \Fetch one request from pending request Q for processing.
     */
    bool fetchNextRequest(CameraRequest& request);

    void handleRequest(CameraRequest& request, int64_t applyingSeq);
    bool blockRequest();

    static const int kMaxRequests = MAX_BUFFER_COUNT;
    static const nsecs_t kWaitFrameDuration = 5000000000; // 5s
    static const nsecs_t kWaitDuration = 2000000000; // 2s
    static const nsecs_t kWaitFirstRequestDoneDuration = 1000000000; // 1s

    //Guard for all the pending requests
    Mutex mPendingReqLock;
    std::condition_variable mRequestSignal;
    std::deque <CameraRequest> mPendingRequests;
    int mRequestsInProcessing;

    // Guard for the first request.
    Mutex mFirstRequestLock;
    std::condition_variable mFirstRequestSignal;
    bool mFirstRequest;

    struct FrameQueue {
        Mutex mFrameMutex;
        std::condition_variable mFrameAvailableSignal;
        CameraBufQ mFrameQueue;
    };
    FrameQueue mOutputFrames[MAX_STREAM_NUMBER];

    enum State {
        START,
        PROCESSING,
        EXIT
    };
    std::atomic<State> mState;

    enum RequestTriggerEvent : uint32_t {
        NONE_EVENT  = 0U,
        NEW_REQUEST = 1U,
        NEW_FRAME   = 1U << 1,
        NEW_STATS   = 1U << 2,
        NEW_SOF     = 1U << 3,
    };
    uint32_t mRequestTriggerEvent;

    int64_t mLastCcaId;
    int64_t mLastEffectSeq;  // Last sequence is which last results had been taken effect on
    int64_t mLastAppliedSeq; // Last sequence id which last results had been set on
    int64_t mLastSofSeq;
    bool mBlockRequest;  // Process the 2nd or 3th request after the 1st 3A event
                         // to avoid unstable AWB at the beginning of stream on
    bool mSofEnabled;
};

} //namespace icamera

