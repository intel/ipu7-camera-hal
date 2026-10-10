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

#define LOG_TAG CameraStream

#include "CameraStream.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"
#include "StageDescriptor.h"

using std::shared_ptr;

namespace icamera {

CameraStream::CameraStream(int cameraId, int streamId, const stream_t& stream)
        : mCameraId(cameraId),
          mStreamId(streamId),
#ifdef LINUX_PRIVACY_MODE
          mLastUserSequence(-1),
#else
          mBufferInProcessing(0),
#endif
          mPort(USER_DEFAULT_PORT_UID) {
    LOG2("<id%d>@%s: streamid:%d automation checkpoint: %dx%d, format: %s", mCameraId, __func__,
         streamId, stream.width, CameraUtils::getInterlaceHeight(stream.field, stream.height),
         CameraUtils::pixelCode2String(stream.format));
}

CameraStream::~CameraStream() {}

int CameraStream::start() {
    LOG1("<id%d>@%s, %p", mCameraId, __func__, this);

#ifdef LINUX_PRIVACY_MODE
    mLastUserSequence = -1;
#endif

    return OK;
}

int CameraStream::stop() {
    LOG1("<id%d>@%s, %p", mCameraId, __func__, this);

    if (mBufferProducer != nullptr) {
        mBufferProducer->removeFrameAvailableListener(this);
    }

    AutoMutex poolLock(mBufferPoolLock);
#ifdef LINUX_PRIVACY_MODE
    mBufferInProcessing.clear();
    mLastUserSequence = -1;
#else
    mBufferInProcessing = 0;
#endif
    mInputBuffersPool.clear();

    return OK;
}

/*
 * Allocate memory to the stream processor which should be
 * set by the CameraDevice
 */
int CameraStream::allocateMemory(camera_buffer_t* ubuffer) {
    LOG1("<id%d>@%s, ubuffer %p", mCameraId, __func__, ubuffer);

    int ret = BAD_VALUE;
    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    CheckAndLogError(camBuffer == nullptr, ret, "@%s: fail to alloc CameraBuffer", __func__);

    // mBufferProducer will not change after start
    if (mBufferProducer != nullptr) {
        ret = mBufferProducer->allocateMemory(mPort, camBuffer);
    }

    return ret;
}

shared_ptr<CameraBuffer> CameraStream::userBufferToCameraBuffer(camera_buffer_t* ubuffer) {
    if (ubuffer == nullptr) {
        return nullptr;
    }

    shared_ptr<CameraBuffer> camBuffer = nullptr;

    AutoMutex l(mBufferPoolLock);
    for (auto buffer = mInputBuffersPool.begin(); buffer != mInputBuffersPool.end(); buffer++) {
        /* check if the buffer is already in the pool, when the userBuffer is same as ubuffer
         * will continue to check the buffer, because the data addr in ubuffer may change */
        if ((*buffer)->getUserBuffer() == ubuffer) {
            /* when memType matches, the dmafd or the addr should match */
            if (((*buffer)->getMemory() == static_cast<uint32_t>(ubuffer->s.memType)) &&
                (((ubuffer->addr != nullptr) &&
                ((*buffer)->getUserBuffer()->addr == ubuffer->addr)) || ((ubuffer->dmafd >= 0) &&
                ((*buffer)->getUserBuffer()->dmafd == ubuffer->dmafd)))) {
                camBuffer = *buffer;
            } else {
                mInputBuffersPool.erase(buffer);
            }
            break;
        }
    }

    if (camBuffer == nullptr) {  // Not found in the pool, so create a new CameraBuffer for it.
        ubuffer->index = mInputBuffersPool.size();
        camBuffer = CameraBuffer::create(ubuffer->s.memType, ubuffer->s.size, ubuffer->index,
                                         ubuffer);
        CheckAndLogError(camBuffer == nullptr, nullptr, "@%s: fail to alloc CameraBuffer",
                         __func__);
        mInputBuffersPool.push_back(camBuffer);
    } else {
        camBuffer->setUserBufferInfo(ubuffer);
        // Update the v4l2 flags
        camBuffer->updateFlags();
    }

    return camBuffer;
}

// Q buffers to the stream processor which should be set by the CameraDevice
int CameraStream::qbuf(camera_buffer_t* ubuffer, int64_t sequence, bool addExtraBuf) {
    UNUSED(addExtraBuf);

    shared_ptr<CameraBuffer> camBuffer = userBufferToCameraBuffer(ubuffer);
    if (camBuffer != nullptr) {
        camBuffer->setSettingSequence(sequence);
        LOG2("<id%d:seq%ld>@%s, mStreamId:%d, CameraBuffer:%p for port:%d, ubuffer:%p, addr:%p",
             mCameraId, sequence, __func__, mStreamId, camBuffer.get(), mPort, ubuffer,
             ubuffer->addr);
    }

    int ret = BAD_VALUE;
    // GstBuffer release can race with redirectBufferProducer() on another thread
    // (e.g. a shutter switch), so mBufferProducer must not be read without the lock.
    AutoMutex l(mBufferPoolLock);
    if (mBufferProducer != nullptr) {
        ret = mBufferProducer->qbuf(mPort, camBuffer);
        if (ret == OK) {
#ifdef LINUX_PRIVACY_MODE
            mBufferInProcessing.push_back(camBuffer);
#else
            mBufferInProcessing++;
#endif
        }
    }
    return ret;
}

#ifdef LINUX_PRIVACY_MODE
// Redirect active stream to a new buffer producer and transfer pending buffers.
int CameraStream::redirectBufferProducer(BufferProducer* producer) {
    AutoMutex l(mBufferPoolLock);
    BufferProducer* oldProducer = mBufferProducer;

    if (oldProducer != nullptr) {
        oldProducer->removeFrameAvailableListener(this);
    }

    BufferConsumer::setBufferProducer(producer);
    if ((producer == nullptr) || (oldProducer == nullptr)) {
        return 0;
    }

    int transferred = 0;
    for (auto& buffer : mBufferInProcessing) {
        // The new producer starts a new sequence epoch, so a settingSequence from the
        // previous one would stall the buffer in needExecutePipe() until the input
        // sequence caught up. -1 means no particular input frame is required.
        buffer->setSettingSequence(-1);
        int ret = producer->qbuf(mPort, buffer);
        if (ret == OK) {
            ++transferred;
        } else {
            LOGW("<id%d>@%s: failed to transfer buffer %p, ret=%d", mCameraId, __func__,
                 buffer.get(), ret);
        }
    }

    LOG1("<id%d>@%s: transferred %d buffer(s) to the new producer", mCameraId, __func__,
         transferred);
    return transferred;
}
#endif

int CameraStream::onBufferAvailable(uuid port, const shared_ptr<CameraBuffer>& camBuffer) {
    // Ignore if the buffer is not for this stream.
    if (mPort != port) {
        return OK;
    }
    if (camBuffer->getStreamId() != mStreamId) {
        return OK;
    }

    LOG2("<id%d>@%s: mStreamId:%d, CameraBuffer:%p for port:%d", mCameraId, __func__, mStreamId,
         camBuffer.get(), port);

    // Update the user buffer info before return back
    camBuffer->updateUserBuffer();

    camera_buffer_t* ubuffer = camBuffer->getUserBuffer();
#ifdef LINUX_PRIVACY_MODE
    if (mLastUserSequence < 0) {
        mLastUserSequence = ubuffer->sequence;
    } else {
        if (static_cast<int64_t>(ubuffer->sequence) <= mLastUserSequence) {
            ubuffer->sequence = static_cast<uint32_t>(mLastUserSequence + 1);
        }
        mLastUserSequence = ubuffer->sequence;
    }
#endif
    LOG2("ubuffer:%p, addr:%p, timestamp:%lu, sequence:%ld", ubuffer, ubuffer->addr,
         ubuffer->timestamp, ubuffer->sequence);

    PERF_CAMERA_ATRACE_PARAM3("sequence", camBuffer->getSequence(), "csi2_port",
                              camBuffer->getCsi2Port(), "virtual_channel",
                              camBuffer->getVirtualChannel());

    {
        AutoMutex l(mBufferPoolLock);
#ifdef LINUX_PRIVACY_MODE
        auto it = std::find(mBufferInProcessing.begin(), mBufferInProcessing.end(), camBuffer);
        if (it != mBufferInProcessing.end()) {
            mBufferInProcessing.erase(it);
        }

        LOG2("%s, buffer in processing: %zu for stream: %p", __func__, mBufferInProcessing.size(), this);
#else
        if (mBufferInProcessing > 0) {
            mBufferInProcessing--;
        }
        LOG2("%s, buffer in processing: %d for stream: %p", __func__, mBufferInProcessing, this);
#endif
    }

    EventFrameAvailable frameData;
    frameData.streamId = mStreamId;
    EventData eventData;
    eventData.type = EVENT_FRAME_AVAILABLE;
    eventData.buffer = camBuffer;
    eventData.data.frameDone = frameData;
    notifyListeners(eventData);

    return OK;
}

}  // namespace icamera
