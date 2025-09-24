/*
 * Copyright (C) 2024 Intel Corporation.
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

#define LOG_TAG GPUPostStage

#include "src/core/processingUnit/GPUPostStage.h"

#include "PlatformData.h"
#include "iutils/CameraLog.h"

namespace icamera {

GPUPostStage::GPUPostStage(int cameraId, int stageId, const std::string& stageName)
        : IPipeStage(stageName.c_str(), stageId),
          mCameraId(cameraId),
          mInputPort(INVALID_PORT),
          mOutputBuffersNum(0),
          mTnr7usParam(nullptr) {
    LOG1("%s, %d", __func__, mCameraId);
    mTnr7Stage = std::unique_ptr<IntelTNR7Stage>(IntelTNR7Stage::createIntelTNR(cameraId));
}

GPUPostStage::~GPUPostStage() {}

void GPUPostStage::setFrameInfo(const std::map<uuid, stream_t>& inputInfo,
                                const std::map<uuid, stream_t>& outputInfo) {
    CheckWarningNoReturn(inputInfo.size() > 1, "Only support one input");
    if (mTnr7Stage) {
        int ret =
            mTnr7Stage->init(inputInfo.begin()->second.width, inputInfo.begin()->second.height);
        if (ret != 0) {
            mTnr7Stage = nullptr;
        }
    }

    if (mTnr7Stage) {
        mTnr7usParam = mTnr7Stage->allocTnr7ParamBuf();
        CheckAndLogError(!mTnr7usParam, VOID_VALUE, "Allocate Param buffer failed");
        CLEAR(*mTnr7usParam);
    }
    BufferQueue::setFrameInfo(inputInfo, outputInfo);
    mInputPort = mInputFrameInfo.begin()->first;  // Only support one input currently
    stream_t input = mInputFrameInfo[mInputPort];
    mOutputBuffersNum = mOutputFrameInfo.size();
}

int GPUPostStage::qbuf(uuid port, const std::shared_ptr<CameraBuffer>& camBuffer) {
    AutoMutex l(mBufferQueueLock);
    CheckAndLogError(mPendingOutBuffers.find(port) != mPendingOutBuffers.end(), INVALID_OPERATION,
                     "%s: already have buffer for port %x", getName(), port);

    mPendingOutBuffers[port] = camBuffer;
    // Wait for all output buffers come for one request.
    // Assume no buffer of request n+1 comes before stage gets all output buffers of request n.
    if (mPendingOutBuffers.size() < mOutputBuffersNum) {
        return OK;
    }

    int64_t sequence = -1;
    for (auto& item : mPendingOutBuffers) {
        CameraBufQ& output = mOutputQueue[item.first];
        output.push(item.second);
        if (item.second != nullptr) {
            sequence = item.second->getSettingSequence();
        }
    }

    // Select input buffers for the request
    std::shared_ptr<CameraBuffer> inBuffer;
    if (!fetchRequestBuffer(sequence, inBuffer)) {
        return INVALID_OPERATION;
    }
    mBufferProducer->qbuf(mInputPort, inBuffer);

    mPendingOutBuffers.clear();
    return OK;
}

bool GPUPostStage::fetchRequestBuffer(int64_t sequence, std::shared_ptr<CameraBuffer>& inBuffer) {
    CameraBufVector& bufV = mInternalBuffers[mInputPort];
    CheckAndLogError(bufV.empty(), false, "%s: queued %d, no available buffer", getName(),
                     mQueuedInputBuffers.size());
    inBuffer = bufV.back();
    bufV.pop_back();
    mQueuedInputBuffers.push(inBuffer);
    inBuffer->setSettingSequence(sequence);
    return true;
}

bool GPUPostStage::process(int64_t triggerId) {
    PERF_CAMERA_ATRACE_PARAM1(getName(), triggerId);

    std::map<uuid, std::shared_ptr<CameraBuffer> > inBuffers;
    std::map<uuid, std::shared_ptr<CameraBuffer> > outBuffers;

    {
        AutoMutex l(mBufferQueueLock);
        if (getFreeBuffersInQueue(inBuffers, outBuffers) != OK) {
            return true;
        }
    }

    std::shared_ptr<CameraBuffer> inBuffer = inBuffers.begin()->second;
    v4l2_buffer_t inV4l2Buf = *inBuffer->getV4L2Buffer().Get();
    int64_t sequence = inBuffer->getSequence();
    for (auto& output : outBuffers) {
        if (!output.second) {
            continue;
        }
        if (mTnr7Stage) {
            mTnr7usParam->bc.is_first_frame = 1;
            mTnr7Stage->runTnrFrame(inBuffer->getBufferAddr(), output.second->getBufferAddr(),
                                    inBuffer->getBufferSize(), output.second->getBufferSize(),
                                    mTnr7usParam, output.second->getFd());
        } else {
            MEMCPY_S(output.second->getBufferAddr(), output.second->getBufferSize(),
                     inBuffer->getBufferAddr(), inBuffer->getBufferSize());
        }
        uuid outPort = output.first;
        LOG2("<seq%ld>%s: handle port %x in async", sequence, getName(), outPort);
        updateInfoAndSendEvents(inV4l2Buf, output.second, outPort);
    }

    returnBuffers(inBuffers, outBuffers);
    return true;
}

void GPUPostStage::updateInfoAndSendEvents(const v4l2_buffer_t& inV4l2Buf,
                                           std::shared_ptr<CameraBuffer> outBuffer,
                                           int32_t outPort) {
    outBuffer->updateV4l2Buffer(inV4l2Buf);

    EventData bufferEvent;
    bufferEvent.type = EVENT_STAGE_BUF_READY;
    bufferEvent.data.stageBufReady.sequence = inV4l2Buf.sequence;
    bufferEvent.data.stageBufReady.uuid = outPort;
    bufferEvent.buffer = outBuffer;
    notifyListeners(bufferEvent);
}

void GPUPostStage::returnBuffers(std::map<uuid, std::shared_ptr<CameraBuffer> >& inBuffers,
                                 std::map<uuid, std::shared_ptr<CameraBuffer> >& outBuffers) {
    // Check and return internal input buffer
    if (inBuffers.find(mInputPort) != inBuffers.end() && !mQueuedInputBuffers.empty()) {
        AutoMutex l(mBufferQueueLock);
        if (mQueuedInputBuffers.front() == inBuffers[mInputPort]) {
            mInternalBuffers[mInputPort].push_back(mQueuedInputBuffers.front());
            mQueuedInputBuffers.pop();
            inBuffers.erase(mInputPort);
        }
    }

    // Don't return input buffer to producer here because it happens only when stage gets outputs
    inBuffers.clear();
    BufferQueue::returnBuffers(inBuffers, outBuffers);
}

int32_t GPUPostStage::allocateBuffers() {
    mInternalBuffers.clear();
    while (!mQueuedInputBuffers.empty()) mQueuedInputBuffers.pop();
    CheckAndLogError(!mBufferProducer, BAD_VALUE, "@%s: Buffer Producer is nullptr", __func__);

    if (mInputFrameInfo.empty()) {
        return OK;
    }

    // Only support one input currently
    const stream_t& input = mInputFrameInfo[mInputPort];
    LOG1("%s fmt:%s (%dx%d)", __func__, CameraUtils::format2string(input.format).c_str(),
         input.width, input.height);

    int32_t size = CameraUtils::getFrameSize(input.format, input.width, input.height);
    for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
        std::shared_ptr<CameraBuffer> camBuffer;
        if (mTnr7Stage) {
            void* bufferAddr = mTnr7Stage->allocCamBuf(size, i);
            camBuffer =
                CameraBuffer::create(input.width, input.height, size, input.format, i, bufferAddr);
        } else {
            camBuffer = CameraBuffer::create(V4L2_MEMORY_DMABUF, size, i, input.format, input.width,
                                             input.height);
        }

        CheckAndLogError(!camBuffer, NO_MEMORY, "Allocate producer userptr buffer failed");

        mInternalBuffers[mInputPort].push_back(camBuffer);
    }
    return OK;
}

int GPUPostStage::start() {
    return allocateBuffers();
}

}  // namespace icamera
