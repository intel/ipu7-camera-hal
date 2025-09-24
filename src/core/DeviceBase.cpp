/*
 * Copyright (C) 2018-2025 Intel Corporation.
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

#include <string>
#include <vector>

#define LOG_TAG DeviceBase

#include "DeviceBase.h"

#include "CameraEventType.h"
#include "PlatformData.h"
#include "V4l2DeviceFactory.h"
#include "iutils/CameraDump.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "linux/ipu-isys.h"

using std::shared_ptr;

namespace icamera {

DeviceBase::DeviceBase(int cameraId, VideoNodeType nodeType, VideoNodeDirection nodeDirection,
                       DeviceCallback* deviceCB)
        : mCameraId(cameraId),
          mPort(INVALID_PORT),
          mNodeType(nodeType),
          mNodeDirection(nodeDirection),
          mName(GetNodeName(nodeType)),
          mDevice(nullptr),
          mLatestSequence(-1),
          mNeedSkipFrame(false),
          mDeviceCB(deviceCB),
          mMaxBufferNumber(MAX_BUFFER_COUNT),
          mBufferQueuing(false) {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);
    std::string devName;
    const int ret = PlatformData::getDevNameByType(cameraId, nodeType, devName);
    CheckAndLogError(ret != OK, VOID_VALUE,
                     "Failed to get video device name for cameraId: %d, node type: %d", cameraId,
                     nodeType);

    mDevice = new V4L2VideoNode(devName);
}

DeviceBase::~DeviceBase() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);
    delete mDevice;
}

int DeviceBase::openDevice() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    return mDevice->Open(O_RDWR);
}

void DeviceBase::closeDevice() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    // Release V4L2 buffers
    mDevice->Stop(true);
    {
        AutoMutex l(mBufferLock);
        mPendingBuffers.clear();
        mBuffersInDevice.clear();
        mAllocatedBuffers.clear();
    }
    mDevice->Close();
}

int DeviceBase::configure(uuid port, const stream_t& config, uint32_t bufferNum) {
    LOG1("<id%d>%s, device:%s, port:%d", mCameraId, __func__, mName, port);

    mPort = port;
    mMaxBufferNumber = bufferNum;

    int ret = createBufferPool(config);
    CheckAndLogError(ret != OK, NO_MEMORY, "Failed to create buffer pool:%d", ret);

    resetBuffers();

    return OK;
}

int DeviceBase::streamOn() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mFrameSkipNum = PlatformData::getInitialSkipFrame(mCameraId);

    return mDevice->Start();
}

int DeviceBase::streamOff() {
    LOG1("<id%d>%s, device:%s", mCameraId, __func__, mName);

    mDevice->Stop(false);

    return OK;
}

bool DeviceBase::checkAndUpdateBufLength(shared_ptr<CameraBuffer> buffer) {
    /*
     * The DMA buf length should not be smaller than external DMA buf length.
     * So protect the buffer length when using external DMA buffer.
     */
    if (buffer->getMemory() == V4L2_MEMORY_DMABUF) {
        if (mV4L2BufferInfo.empty()) {
            LOGE("%s, No queried V4L2 buffer info.", __func__);
            return false;
        }

        if (mV4L2BufferInfo[0].Length(0) < buffer->getBufferSize()) {
            LOG2("%s, Update buf length, v4l2 buf %u, external DMA buf %u", __func__,
                 mV4L2BufferInfo[0].Length(0), buffer->getBufferSize());

            buffer->setBufferSize(mV4L2BufferInfo[0].Length(0));
        } else if (mV4L2BufferInfo[0].Length(0) > buffer->getBufferSize()) {
            LOGE("%s, external DMA buf size %u is smaller than v4l2 buf %u", __func__,
                 buffer->getBufferSize(), mV4L2BufferInfo[0].Length(0));

            return false;
        }
    }

    return true;
}

int DeviceBase::queueBuffer(int64_t sequence) {
    LOG2("<id%d>%s, device:%s", mCameraId, __func__, mName);

    shared_ptr<CameraBuffer> buffer;
    {
        AutoMutex l(mBufferLock);
        if (mBufferQueuing) {
            LOG2("buffer is queuing");
            return DEV_BUSY;
        }

        if (mPendingBuffers.empty()) {
            LOG2("Device:%s has no pending buffer to be queued.", mName);
            return OK;
        }

        buffer = mPendingBuffers.front();

        const bool valid = checkAndUpdateBufLength(buffer);
        if (!valid) {
            return BAD_VALUE;
        }

        mBufferQueuing = true;
    }

    int ret = onQueueBuffer(sequence, buffer);
    if (ret == OK) {
        ret = mDevice->PutFrame(&buffer->getV4L2Buffer());

        if (ret >= 0) {
            AutoMutex l(mBufferLock);
            mPendingBuffers.pop_front();
            mBuffersInDevice.push_back(buffer);
        } else {
            LOGE("%s, index:%u size:%u, memory:%u, used:%u", __func__, buffer->getIndex(),
                 buffer->getBufferSize(), buffer->getMemory(), buffer->getBytesused());
        }
    } else {
        LOGE("Device:%s failed to preprocess the buffer with ret=%d", mName, ret);
    }

    {
        AutoMutex l(mBufferLock);
        mBufferQueuing = false;
    }

    return ret;
}

int DeviceBase::dequeueBuffer() {
    LOG2("<id%d>%s, device:%s", mCameraId, __func__, mName);

    shared_ptr<CameraBuffer> camBuffer = getFirstDeviceBuffer();
    CheckAndLogError(camBuffer == nullptr, UNKNOWN_ERROR, "No buffer in device:%s.", mName);

    int ret = OK;
    const int targetIndex = camBuffer->getIndex();

    V4L2Buffer& vbuf = camBuffer->getV4L2Buffer();
    int actualIndex = mDevice->GrabFrame(&vbuf);

    CheckAndLogError(actualIndex < 0, BAD_VALUE, "Device grabFrame failed:%d", actualIndex);
    if (actualIndex != targetIndex) {
        LOGE("%s, CamBuf index isn't same with index used by kernel", __func__);
        ret = BAD_VALUE;
    }

    mNeedSkipFrame = needQueueBack(camBuffer);
    popBufferFromDevice();

    PERF_CAMERA_ATRACE_PARAM3("grabFrame SeqID", camBuffer->getSequence(), "csi2_port",
                              camBuffer->getCsi2Port(), "virtual_channel",
                              camBuffer->getVirtualChannel());
    (void)onDequeueBuffer(camBuffer);

    // Skip initial frames if needed.
    if (mFrameSkipNum > 0) {
        mFrameSkipNum--;
    }
    return ret;
}

int DeviceBase::getBufferNumInDevice() {
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.size();
}

void DeviceBase::resetBuffers() {
    AutoMutex l(mBufferLock);

    mBuffersInDevice.clear();
    mPendingBuffers.clear();
    for (const auto& buffer : mAllocatedBuffers) {
        mPendingBuffers.push_back(buffer);
    }
}

bool DeviceBase::hasPendingBuffer() {
    AutoMutex l(mBufferLock);

    return !mPendingBuffers.empty();
}

void DeviceBase::addPendingBuffer(const shared_ptr<CameraBuffer>& buffer) {
    AutoMutex l(mBufferLock);

    mPendingBuffers.push_back(buffer);
}

int64_t DeviceBase::getPredictSequence() {
    AutoMutex l(mBufferLock);

    return mLatestSequence + mFrameSkipNum + mBuffersInDevice.size();
}

shared_ptr<CameraBuffer> DeviceBase::getFirstDeviceBuffer() {
    AutoMutex l(mBufferLock);

    return mBuffersInDevice.empty() ? nullptr : mBuffersInDevice.front();
}

void DeviceBase::popBufferFromDevice() {
    AutoMutex l(mBufferLock);
    if (mBuffersInDevice.empty()) {
        return;
    }

    shared_ptr<CameraBuffer> camBuffer = mBuffersInDevice.front();
    mBuffersInDevice.pop_front();
    mLatestSequence = camBuffer->getSequence();

    if (mNeedSkipFrame) {
        mPendingBuffers.push_back(camBuffer);
    }
}

void DeviceBase::dumpFrame(const shared_ptr<CameraBuffer>& buffer) {
    if (!CameraDump::isDumpTypeEnable(DUMP_ISYS_BUFFER)) {
        return;
    }

    LOG2("@%s, ISYS: fmt:%s(%dx%d), stride:%d, len:%d", __func__,
         CameraUtils::format2string(buffer->getFormat()).c_str(), buffer->getWidth(),
         buffer->getHeight(), buffer->getStride(), buffer->getBufferSize());

    CameraDump::dumpImage(mCameraId, buffer, M_ISYS, mPort);
}

MainDevice::MainDevice(int cameraId, VideoNodeType nodeType, DeviceCallback* deviceCB)
        : DeviceBase(cameraId, nodeType, INPUT_VIDEO_NODE, deviceCB) {
    LOG1("<id%d>%s, device:%s", DeviceBase::mCameraId, __func__, DeviceBase::mName);
}

MainDevice::~MainDevice() {}

int MainDevice::createBufferPool(const stream_t& config) {
    LOG1("<id%d>%s, fmt:%s(%dx%d) field:%d", DeviceBase::mCameraId, __func__,
         CameraUtils::pixelCode2String(config.format), config.width, config.height, config.field);

    struct v4l2_format v4l2fmt;
    v4l2fmt.fmt.pix_mp.field = config.field;

    v4l2fmt.fmt.pix.width = config.width;
    v4l2fmt.fmt.pix.height = config.height;
    v4l2fmt.fmt.pix.pixelformat = config.format;
    v4l2fmt.fmt.pix.bytesperline = config.width;
    v4l2fmt.fmt.pix.sizeimage = 0;

    v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    V4L2Format tmpbuf{v4l2fmt};
    int ret = mDevice->SetFormat(tmpbuf);
    CheckAndLogError(ret != OK, ret, "set v4l2 format failed ret=%d", ret);
    v4l2fmt = *tmpbuf.Get();

    const int realBufferSize = v4l2fmt.fmt.pix.sizeimage;
    const int calcBufferSize =
        CameraUtils::getFrameSize(config.format, config.width, config.height);
    CheckAndLogError(calcBufferSize < realBufferSize, BAD_VALUE,
                     "realBufferSize %d is larger than calcBufferSize %d.", realBufferSize,
                     calcBufferSize);

    LOG2("@%s: realBufSize:%d, calcBufSize:%d", __func__, realBufferSize, calcBufferSize);

    mV4L2BufferInfo.clear();
    int bufNum = mDevice->SetupBuffers(DeviceBase::mMaxBufferNumber, true,
                                       static_cast<enum v4l2_memory>(config.memType),
                                       &this->mV4L2BufferInfo);
    CheckAndLogError(bufNum < 0, BAD_VALUE, "request buffers failed return=%d", bufNum);

    return OK;
}

int MainDevice::onDequeueBuffer(shared_ptr<CameraBuffer> buffer) {
    mDeviceCB->onDequeueBuffer();

    if (DeviceBase::mNeedSkipFrame) {
        return OK;
    }

    LOG2("<seq%u>@%s, field:%d, timestamp: sec=%ld, usec=%ld", buffer->getSequence(),
         __func__, buffer->getField(), buffer->getTimestamp().tv_sec,
         buffer->getTimestamp().tv_usec);

    DeviceBase::dumpFrame(buffer);

    for (auto& consumer : DeviceBase::mConsumers) {
        consumer->onBufferAvailable(DeviceBase::mPort, buffer);
    }

    if (DeviceBase::mPort == MAIN_INPUT_PORT_UID) {
        EventData frameData;
        frameData.type = EVENT_ISYS_FRAME;
        frameData.buffer = nullptr;
        frameData.data.frame.sequence = buffer->getSequence();
        frameData.data.frame.timestamp.tv_sec = buffer->getTimestamp().tv_sec;
        frameData.data.frame.timestamp.tv_usec = buffer->getTimestamp().tv_usec;
        notifyListeners(frameData);
    }

    return OK;
}

bool MainDevice::needQueueBack(shared_ptr<CameraBuffer> buffer) {
    bool needSkipFrame = (DeviceBase::mFrameSkipNum > 0);

    const V4L2Buffer& vbuf = buffer->getV4L2Buffer();
    // Check for STR2MMIO Error from kernel space
    if (((vbuf.Flags() & static_cast<uint32_t>(V4L2_BUF_FLAG_ERROR)) != 0U) &&
         (PlatformData::isSkipFrameOnSTR2MMIOErr(this->mCameraId))) {
        // On STR2MMIO error, enqueue this buffer back to V4L2 before notifying the
        // listener/consumer and return
        needSkipFrame = true;
        LOGW("<seq%u>%s: buffer error", buffer->getSequence(), __func__);
    }
    return needSkipFrame;
}
}  // namespace icamera
