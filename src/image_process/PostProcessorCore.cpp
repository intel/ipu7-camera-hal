/*
 * Copyright (C) 2019-2024 Intel Corporation
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

#define LOG_TAG PostProcessorCore

#include "PostProcessorCore.h"

#include "iutils/CameraLog.h"

using std::shared_ptr;

namespace icamera {

PostProcessorCore::PostProcessorCore(int cameraId)
        : mCameraId(cameraId),
          mMemoryType(V4L2_MEMORY_USERPTR) {}

bool PostProcessorCore::isPostProcessTypeSupported(PostProcessType type) {
    return IImageProcessor::isProcessingTypeSupported(type);
}

status_t PostProcessorCore::createProcessor() {
    mProcessorVector.clear();
    for (const auto& order : mProcessorsInfo) {
        shared_ptr<PostProcessorBase> processor = nullptr;
        switch (order.type) {
            case POST_PROCESS_SCALING:
                processor = std::make_shared<ScaleProcess>();
                break;
            case POST_PROCESS_ROTATE:
                processor = std::make_shared<RotateProcess>(order.angle);
                break;
            case POST_PROCESS_CROP:
                processor = std::make_shared<CropProcess>();
                break;
            case POST_PROCESS_CONVERT:
                processor = std::make_shared<ConvertProcess>();
                break;
            case POST_PROCESS_NONE:
                break;
            default:
                LOGE("%s, Doesn't support this kind of post-processor", __func__);
                return UNKNOWN_ERROR;
        }

        CheckAndLogError(!processor, UNKNOWN_ERROR, "%s, Failed to create the post processor: 0x%x",
                         __func__, order.type);

        int memoryType = processor->getMemoryType();
        // If DMA buffer is preferred, use DMA buffer for post-processing
        if (memoryType == V4L2_MEMORY_DMABUF) {
            mMemoryType = V4L2_MEMORY_DMABUF;
        }

        mProcessorVector.push_back(processor);
    }

    LOG2("%s, the number of post processor unit is %zu", __func__, mProcessorVector.size());
    return OK;
}

status_t PostProcessorCore::configure(const std::vector<PostProcessInfo>& processorOrder) {
    if (processorOrder.empty()) {
        return OK;
    }

    mProcessorsInfo = processorOrder;
    int ret = createProcessor();
    CheckAndLogError(ret != OK, ret, "%s, Failed to create the post processor", __func__);

    ret = allocateInternalBuffers();
    CheckAndLogError(ret != OK, ret, "%s, Failed allocate the internal buffers", __func__);

    return OK;
}

bool PostProcessorCore::isBypassed(int64_t sequence) {
    for (auto& processor : mProcessorVector) {
        if (!processor->isBypassed(sequence)) {
            return false;
        }
    }
    return true;
}

status_t PostProcessorCore::allocateInternalBuffers() {
    LOG1("<id%d>@%s,mProcessorVector.size: %zu", mCameraId, __func__, mProcessorVector.size());

    mInterBuffersMap.clear();

    for (size_t i = 0; i < mProcessorsInfo.size() - 1; i++) {
        const stream_t& info = mProcessorsInfo[i].outputInfo;
        std::shared_ptr<CameraBuffer> buf = CameraBuffer::create(mMemoryType, info.size, i,
                                      mProcessorsInfo[i].inputInfo.format, info.width, info.height);
        if (!buf) {
            mInterBuffersMap.clear();
            LOGE("Failed to allocate internal buffer: processor: %s",
                 mProcessorVector[i]->getName().c_str());
            return NO_MEMORY;
        }
        mInterBuffersMap[mProcessorVector[i]] = buf;
    }

    return OK;
}

status_t PostProcessorCore::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                             shared_ptr<CameraBuffer> outBuf) {
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    shared_ptr<CameraBuffer> input = inBuf;
    shared_ptr<CameraBuffer> output = nullptr;
    std::vector<std::shared_ptr<PostProcessorBase>> processors;
    for (size_t i = 0; i < mProcessorVector.size(); i++) {
        if (!mProcessorVector[i]->isBypassed(inBuf->getSequence())) {
            processors.push_back(mProcessorVector[i]);
        }
    }
    if (processors.empty()) {
        MEMCPY_S(outBuf->getBufferAddr(), outBuf->getBufferSize(),
                 inBuf->getBufferAddr(), inBuf->getBufferSize());
        return OK;
    }
    for (size_t i = 0; i < processors.size(); i++) {
        if (i == (processors.size() - 1)) {
            output = outBuf;
        } else {
            output = mInterBuffersMap[processors[i]];
        }

        int ret = processors[i]->doPostProcessing(input, output);
        CheckAndLogError(ret != OK, ret, "%s, Failed to do post processing: %s", __func__,
                         processors[i]->getName().c_str());

        input = output;
    }

    return OK;
}
}  // namespace icamera
