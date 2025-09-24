/*
 * Copyright (C) 2022 Intel Corporation
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

#define LOG_TAG PipeManagerStub

#include "src/core/processingUnit/PipeManagerStub.h"

#include "ImageScalerCore.h"
#include "SwImageConverter.h"
#include "CameraContext.h"
#include "iutils/CameraLog.h"

namespace icamera {

#define STATS_BUFFER_SIZE 285736

PipeManagerStub::PipeManagerStub(int cameraId, PipeManagerCallback* callback)
        : mCameraId(cameraId),
          mTuningMode(TUNING_MODE_MAX),
          mExitPending(false),
          mPipeManagerCallback(callback),
          mIntermBuffer(nullptr) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mStatsBuffer = CameraBuffer::create(V4L2_MEMORY_USERPTR, sizeof(ia_binary_data), 0, -1, -1, -1);
    mPacAdaptor = new IpuPacAdaptor(mCameraId);
    ia_binary_data* buffer = (ia_binary_data*)mStatsBuffer->getBufferAddr();
    buffer->size = STATS_BUFFER_SIZE;
    buffer->data = malloc(STATS_BUFFER_SIZE);
}

PipeManagerStub::~PipeManagerStub() {
    LOG1("<id%d>@%s", mCameraId, __func__);
    mPacAdaptor->deinit();
    delete mPacAdaptor;
    // stub doesn't have stats result, use a pre allocated buffer as stats data
    ia_binary_data* buffer = (ia_binary_data*)mStatsBuffer->getBufferAddr();
    free(buffer->data);
    buffer->size = 0;
}

int PipeManagerStub::configure(const std::map<uuid, stream_t>& inputInfo,
                               const std::map<uuid, stream_t>& outputInfo, ConfigMode configMode,
                               TuningMode tuningMode, const std::map<uuid, stream_t>* yuvInputInfo) {
    LOG1("<id%d>@%s", mCameraId, __func__);
    UNUSED(yuvInputInfo);

    mConfigMode = configMode;
    mTuningMode = tuningMode;
    mInputFrameInfo = inputInfo;
    mDefaultMainInputPort = inputInfo.begin()->first;

    int fmt = outputInfo.begin()->second.format;
    int width = mInputFrameInfo[mDefaultMainInputPort].width;
    int height = mInputFrameInfo[mDefaultMainInputPort].height;
    uint32_t size = CameraUtils::getFrameSize(fmt, width, height, true);
    mIntermBuffer = CameraBuffer::create(V4L2_MEMORY_USERPTR, size, 0, fmt, width, height);

    status_t ret = OK;
    mGraphConfig = CameraContext::getInstance(mCameraId)->getGraphConfig(mConfigMode);
    CheckAndLogError(!mGraphConfig, UNKNOWN_ERROR, "Failed to get GraphConfig in PipeManager!");

    mActiveStreamIds.clear();
    ret = mGraphConfig->graphGetStreamIds(mActiveStreamIds);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to get the streamIds");

    ret = mPacAdaptor->init(mActiveStreamIds);
    CheckAndLogError(ret != OK, ret, "Init pac Adaptor failed, tuningMode %d", mTuningMode);
    return ret;
}

int PipeManagerStub::start() {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mExitPending = false;
    Thread::start();
    return OK;
}

int PipeManagerStub::stop() {
    mExitPending = true;
    Thread::exit();
    mTaskReadyCondition.notify_one();
    Thread::wait();
    LOG1("<id%d>@%s size %d", mCameraId, __func__, mOngoingTasks.size());
    return OK;
}

void PipeManagerStub::addTask(PipeTaskData taskParam) {
    LOG2("<id%d>@%s", mCameraId, __func__);

    TaskInfo task = {};
    // Save the task data into mOngoingTasks
    task.mTaskData = taskParam;
    // Count how many valid output buffers need to be returned.
    for (auto& outBuf : taskParam.mOutputBuffers) {
        if (outBuf.second) {
            task.mNumOfValidBuffers++;
        }
    }
    LOG2("%s:<id%d:seq%u> push task with %d output buffers", __func__, mCameraId,
         taskParam.mInputBuffers.at(mDefaultMainInputPort)->getSequence(),
         task.mNumOfValidBuffers);

    {
        std::unique_lock<std::mutex> lock(mTaskLock);
        mOngoingTasks.push_back(task);
    }

    int64_t sequence = taskParam.mInputBuffers.at(mDefaultMainInputPort)->getSequence();
    for (auto& id : mActiveStreamIds) {
        prepareIpuParams(&taskParam.mIspSettings, sequence, id);
    }
    // queue buffers to pipeLine here in virtual PipeManager
    queueBuffers();
}

int PipeManagerStub::queueBuffers() {
    mTaskReadyCondition.notify_one();
    return OK;
}

int PipeManagerStub::prepareIpuParams(IspSettings* settings, int64_t sequence, int streamId) {
    LOG2("<id%d>@%s", mCameraId, __func__);
    UNUSED(settings);

    bool validStream = false;
    for (auto id : mActiveStreamIds) {
        if (id == streamId) {
            validStream = true;
            break;
        }
    }
    if (!validStream) {
        return BAD_VALUE;
    }

    {
        // Make sure the AIC is executed once.
        AutoMutex l(mOngoingPalMapLock);

        if (mOngoingPalMap.find(sequence) != mOngoingPalMap.end()) {
            // Check if stream id is available.
            if (mOngoingPalMap[sequence].find(VIDEO_STREAM_ID) != mOngoingPalMap[sequence].end()) {
                // This means aic for the sequence has been executed.
                return OK;
            }
        }
    }
    int ret = mPacAdaptor->runAIC(settings, sequence, streamId);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "%s, <seq%ld> Failed to run AIC: streamId: %d",
                     __func__, sequence, streamId);
    // Store the new sequence.
    AutoMutex l(mOngoingPalMapLock);
    mOngoingPalMap[sequence].insert(streamId);

    return ret;
}

void PipeManagerStub::onMetadataReady(int64_t sequence) {
    LOG2("<seq%ld> %s", sequence, __func__);

    std::unique_lock<std::mutex> lock(mTaskLock);
    for (auto it = mOngoingTasks.begin(); it != mOngoingTasks.end(); it++) {
        // Check if the returned buffer belong to the task.
        if (sequence != it->mTaskData.mInputBuffers.at(mDefaultMainInputPort)->getSequence()) {
            continue;
        }

        if (it->mTaskData.mCallbackRgbs && mPipeManagerCallback) {
            mPipeManagerCallback->onMetadataReady(sequence, it->mTaskData.mOutputBuffers);
        }
        return;
    }
}

int PipeManagerStub::onBufferDone(uuid port, const std::shared_ptr<CameraBuffer>& buffer) {
    if (!buffer) {
        return OK;  // No need to handle if the buffer is nullptr.
    }

    int64_t sequence = buffer->getSequence();
    LOG2("<id%d:seq%ld>@%s", mCameraId, sequence, __func__);

    bool needReturn = false;
    uuid outputPort = INVALID_PORT;
    PipeTaskData result;
    {
        std::unique_lock<std::mutex> lock(mTaskLock);
        for (auto it = mOngoingTasks.begin(); it != mOngoingTasks.end(); it++) {
            // Check if the returned buffer belong to the task.
            if (sequence != it->mTaskData.mInputBuffers.at(mDefaultMainInputPort)->getSequence()) {
                continue;
            }

            // Check if buffer belongs to the task because input buffer maybe reused
            for (auto& buf : it->mTaskData.mOutputBuffers) {
                if (buf.second && (buffer->getUserBuffer() == buf.second->getUserBuffer())) {
                    outputPort = buf.first;
                }
            }
            if (outputPort == INVALID_PORT) {
                continue;
            }

            it->mNumOfReturnedBuffers++;
            if (it->mNumOfReturnedBuffers >= it->mNumOfValidBuffers) {
                result = it->mTaskData;
                needReturn = true;
                LOG2("<Id%d:seq%ld> finish task with %d returned output buffers, ", mCameraId,
                     sequence, it->mNumOfReturnedBuffers);
                // Remove the task data from mOngoingTasks since it's already processed.
                mOngoingTasks.erase(it);

                // Remove the sequence when finish to process the task
                AutoMutex l(mOngoingPalMapLock);
                mOngoingPalMap.erase(sequence);
            }
            break;
        }
    }

    if (mPipeManagerCallback) {
        CheckAndLogError(outputPort == INVALID_PORT, INVALID_OPERATION, "outputPort is invalid");
        // Return buffer
        mPipeManagerCallback->onBufferDone(sequence, outputPort, buffer);
    }

    if (needReturn && mPipeManagerCallback) {
        mPipeManagerCallback->onTaskDone(result);
    }

    return OK;
}

int PipeManagerStub::processTask(const PipeTaskData& task) {
    LOG2("<id%d>@%s", mCameraId, __func__);
    std::shared_ptr<CameraBuffer> cInBuffer;
    std::shared_ptr<CameraBuffer> cOutBuffer;

    for (auto& inputFrame : task.mInputBuffers) {
        if (inputFrame.second) {
            cInBuffer = inputFrame.second;
            break;
        }
    }

    uint32_t sequence = cInBuffer->getSequence();

    // trigger fake stats done event
    EventDataStatsReady statsReadyData;
    statsReadyData.sequence = sequence;
    statsReadyData.timestamp.tv_sec = cInBuffer->getTimestamp().tv_sec;
    statsReadyData.timestamp.tv_usec = cInBuffer->getTimestamp().tv_usec;
    EventData eventData;
    eventData.type = EVENT_PSYS_STATS_BUF_READY;
    eventData.buffer = mStatsBuffer;
    eventData.data.statsReady = statsReadyData;
    eventData.pipeType = VIDEO_STREAM_ID;
    if (mPipeManagerCallback) {
        mPipeManagerCallback->onStatsReady(eventData);
    }
    onMetadataReady(sequence);

    if (sequence < kStartingFrameCount) {
        int ret = SwImageConverter::convertFormat(
            cInBuffer->getWidth(), cInBuffer->getHeight(),
            static_cast<unsigned char*>(cInBuffer->getBufferAddr()), cInBuffer->getBufferSize(),
            cInBuffer->getFormat(), static_cast<unsigned char*>(mIntermBuffer->getBufferAddr()),
            mIntermBuffer->getBufferSize(), mIntermBuffer->getFormat());
        CheckAndLogError((ret < 0), ret, "format conversion failed with %d", ret);
    }
    for (auto& outputFrame : task.mOutputBuffers) {
        if (outputFrame.second) {
            cOutBuffer = outputFrame.second;

            if (sequence < kStartingFrameCount) {
                CameraBufferMapper mapper(cOutBuffer);

                ImageScalerCore::downScaleImage(
                    mIntermBuffer->getBufferAddr(), mapper.addr(), cOutBuffer->getWidth(),
                    cOutBuffer->getHeight(), cOutBuffer->getStride(), mIntermBuffer->getWidth(),
                    mIntermBuffer->getHeight(), mIntermBuffer->getStride(),
                    mIntermBuffer->getFormat());
            }
            cOutBuffer->updateV4l2Buffer(*cInBuffer->getV4L2Buffer().Get());
            onBufferDone(outputFrame.first, cOutBuffer);
        }
    }

    return OK;
}

bool PipeManagerStub::threadLoop() {
    if (mExitPending) {
        return false;
    }

    PipeTaskData task = {};
    LOG2("<id%d>@%s", mCameraId, __func__);
    {
        std::unique_lock<std::mutex> lock(mTaskLock);
        if (mOngoingTasks.empty()) {
            std::cv_status ret =
                mTaskReadyCondition.wait_for(lock, std::chrono::nanoseconds(2000000000));

            if (mOngoingTasks.empty() || ret == std::cv_status::timeout) {
                LOGW("[%p]%s, wait task ready time out", this, __func__);
                return true;
            }
        }
        task = mOngoingTasks[0].mTaskData;
    }
    // assume pipe line cost 10ms to process each frame
    usleep(10000);
    processTask(task);
    return true;
}

}  // namespace icamera
