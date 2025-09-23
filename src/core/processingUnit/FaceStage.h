/*
 * Copyright (C) 2022 Intel Corporation.
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

#include "CameraStream.h"
#include "ISchedulerNode.h"
#include "IFaceDetection.h"

#include "iutils/Errors.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * \class FaceStage
 *
 * This class is used to process the frame for face detection.
 */
class FaceStage : public CameraStream, public ISchedulerNode {
 public:
    FaceStage (int cameraId, int streamId, const stream_t& stream, bool isPrivate);
    virtual ~FaceStage();

    virtual int start();
    virtual int stop();

    virtual int qbuf(camera_buffer_t* ubuffer, int64_t sequence, bool addExtraBuf = false);
    virtual int onBufferAvailable(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer);

    virtual bool process(int64_t triggerId);
 private:
    bool isFaceEnabled(int64_t sequence);

 private:
    stream_t mStreamInfo;  // The stream info for face detection
    // Maintains a internal buffer pool when face running in async and bind on preview stream
    CameraBufQ mAvailableBufferQ;

    // Store the pending buffers need to process
    // Use the mBufferPoolLock in base class to guard it
    CameraBufQ mPendingBufferQ;

    bool mIsPrivate;  // Indicate this is a internal private stream
    std::unique_ptr<FaceDetection> mFaceDetection;
};

}  // namespace icamera
