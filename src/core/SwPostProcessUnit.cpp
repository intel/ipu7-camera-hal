/*
 * Copyright (C) 2024 Intel Corporation
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

#define LOG_TAG SwPostProcessUnit

#include "SwPostProcessUnit.h"

#include <vector>

#include "iutils/CameraLog.h"

namespace icamera {

SwPostProcessUnit::SwPostProcessUnit(int cameraId)
        : mCameraId(cameraId),
          mPostProcessType(POST_PROCESS_NONE),
          mPostProcessorCore(std::unique_ptr<PostProcessorCore>(
              new PostProcessorCore(cameraId))) {}

SwPostProcessUnit::~SwPostProcessUnit() {}

/* configure
 *
 * Decide post-processing is needed based on input and output stream.
 * The default processing order is rotate -> crop -> scale -> convert -> encode.
 */
status_t SwPostProcessUnit::configure(const stream_t& srcStream, const stream_t& dstStream) {
    LOG1("@%s, srcStream: w:%d, h:%d, f:%d, size:%d, stride:%d", __func__, srcStream.width,
         srcStream.height, srcStream.format, srcStream.size, srcStream.stride);
    LOG1("@%s, dstStream: w:%d, h:%d, f:%d", __func__, dstStream.width, dstStream.height,
         dstStream.format);

    PostProcessInfo info;
    mPostProcessType = POST_PROCESS_NONE;
    std::vector<PostProcessInfo> processingOrder;

    /* Fill the input/output information for the post processing unit.
     * The input info of processing unit is the output info of last unit.
     */
    stream_t inputStreamInfo = srcStream;
    /* GPU could do rotate/scale/crop process for NV12 stream
     * create GPU processor every time, run depends on frame request */
    if (mPostProcessorCore->isPostProcessTypeSupported(POST_PROCESS_GPU) &&
        (inputStreamInfo.format == V4L2_PIX_FMT_NV12)) {
        mPostProcessType |= POST_PROCESS_GPU;
        info.type = POST_PROCESS_GPU;
        info.inputInfo = inputStreamInfo;
        info.inputInfo.stride = inputStreamInfo.width;

        info.outputInfo.format = inputStreamInfo.format;
        info.outputInfo.width = dstStream.width;
        info.outputInfo.height = dstStream.height;
        info.outputInfo.stride = dstStream.width;
        info.outputInfo.size =
            CameraUtils::getFrameSize(info.outputInfo.format, info.outputInfo.width,
                                      info.outputInfo.height, false, false, false);
        LOG1("%s, input %dx%d, output: %dx%d", __func__, srcStream.width,
             srcStream.height, dstStream.width, dstStream.height);

        inputStreamInfo = info.outputInfo;
        processingOrder.push_back(info);
    }

    if (dstStream.orientation > 0 &&
        mPostProcessorCore->isPostProcessTypeSupported(POST_PROCESS_ROTATE)) {
        mPostProcessType |= POST_PROCESS_ROTATE;
        info.angle = dstStream.orientation;
        info.type = POST_PROCESS_ROTATE;
        info.inputInfo = inputStreamInfo;
        info.outputInfo = inputStreamInfo;
        info.outputInfo.width = inputStreamInfo.height;
        info.outputInfo.height = inputStreamInfo.width;
        info.outputInfo.stride = inputStreamInfo.height;
        info.outputInfo.size =
            CameraUtils::getFrameSize(info.outputInfo.format, info.outputInfo.width,
                                      info.outputInfo.height, false, false, false);
        LOG1("%s, Rotate: input %dx%d, output: %dx%d, angle: %d", __func__, inputStreamInfo.width,
             inputStreamInfo.height, info.outputInfo.width, info.outputInfo.height, info.angle);

        inputStreamInfo = info.outputInfo;
        processingOrder.push_back(info);
    }

    // Crop
    if (inputStreamInfo.width * dstStream.height != inputStreamInfo.height * dstStream.width &&
        mPostProcessorCore->isPostProcessTypeSupported(POST_PROCESS_CROP)) {
        mPostProcessType |= POST_PROCESS_CROP;
        info.type = POST_PROCESS_CROP;
        info.inputInfo = inputStreamInfo;

        // Caclulate the best crop size with same aspect ratio
        if (inputStreamInfo.width * dstStream.height < inputStreamInfo.height * dstStream.width) {
            info.outputInfo.width = info.inputInfo.width;
            info.outputInfo.height =
                ALIGN(info.inputInfo.width * dstStream.height / dstStream.width, 2);
        } else {
            info.outputInfo.width =
                ALIGN(info.inputInfo.height * dstStream.width / dstStream.height, 2);
            info.outputInfo.height = info.inputInfo.height;
        }
        info.outputInfo.format = inputStreamInfo.format;
        info.outputInfo.stride = info.outputInfo.width;
        info.outputInfo.size =
            CameraUtils::getFrameSize(info.outputInfo.format, info.outputInfo.width,
                                      info.outputInfo.height, false, false, false);
        LOG1("%s, Crop: input %dx%d, output: %dx%d", __func__, inputStreamInfo.width,
             inputStreamInfo.height, info.outputInfo.width, info.outputInfo.height);

        inputStreamInfo = info.outputInfo;
        processingOrder.push_back(info);
    }

    // Scale
    if (inputStreamInfo.width * inputStreamInfo.height !=
        dstStream.width * dstStream.height &&
        mPostProcessorCore->isPostProcessTypeSupported(POST_PROCESS_SCALING)) {
        mPostProcessType |= POST_PROCESS_SCALING;
        info.type = POST_PROCESS_SCALING;
        info.inputInfo = inputStreamInfo;
        info.outputInfo.width = dstStream.width;
        info.outputInfo.height = dstStream.height;
        info.outputInfo.stride = dstStream.width;
        info.outputInfo.format = inputStreamInfo.format;
        info.outputInfo.size =
            CameraUtils::getFrameSize(info.outputInfo.format, info.outputInfo.width,
                                      info.outputInfo.height, false, false, false);
        LOG1("%s, Scale: input %dx%d, output: %dx%d", __func__, inputStreamInfo.width,
             inputStreamInfo.height, info.outputInfo.width, info.outputInfo.height);

        inputStreamInfo = info.outputInfo;
        processingOrder.push_back(info);
    }

    // Convert
    if (dstStream.format != V4L2_PIX_FMT_JPEG && inputStreamInfo.format != dstStream.format &&
        mPostProcessorCore->isPostProcessTypeSupported(POST_PROCESS_CONVERT)) {
        mPostProcessType |= POST_PROCESS_CONVERT;
        info.type = POST_PROCESS_CONVERT;
        info.inputInfo = inputStreamInfo;
        info.outputInfo.width = dstStream.width;
        info.outputInfo.height = dstStream.height;
        info.outputInfo.stride = dstStream.width;
        info.outputInfo.format = dstStream.format;
        info.outputInfo.size =
            CameraUtils::getFrameSize(info.outputInfo.format, info.outputInfo.width,
                                      info.outputInfo.height, false, false, false);
        LOG1("%s, Convert: input %dx%d, output: %dx%d", __func__, inputStreamInfo.width,
             inputStreamInfo.height, info.outputInfo.width, info.outputInfo.height);

        inputStreamInfo = info.outputInfo;
        processingOrder.push_back(info);
    }

    if (inputStreamInfo.width != dstStream.width ||
        inputStreamInfo.height != dstStream.height) {
        //inputStreamInfo.format !=
        //    HalV3Utils::HALFormatToV4l2Format(mCameraId, stream.format, stream.usage)) {
        LOGE("%s, stream info doesn't match between input and output stream.", __func__);
        return UNKNOWN_ERROR;
    }
    LOG1("<id%d>@%s,mPostProcessType %d, processing unit number: %zu", mCameraId, __func__,
         mPostProcessType, processingOrder.size());
    mPostProcessorCore->configure(processingOrder);

    return OK;
}

int SwPostProcessUnit::getMemoryType() {
    return mPostProcessorCore->getMemoryType();
}

bool SwPostProcessUnit::isBypassed(int64_t sequence) {
    if (!mPostProcessorCore) {
        return false;
    }

    // Currently only GPU processor supports bypass per request
    if (mPostProcessType == POST_PROCESS_GPU) {
        return mPostProcessorCore->isBypassed(sequence);
    }
    return mPostProcessType == POST_PROCESS_NONE;
}

status_t SwPostProcessUnit::doPostProcessing(const std::shared_ptr<CameraBuffer>& inBuf,
                                             std::shared_ptr<CameraBuffer> outBuf) {
    if (inBuf == outBuf) {
        return OK;
    }

    if (mPostProcessType == POST_PROCESS_NONE) {
        MEMCPY_S(outBuf->getBufferAddr(), outBuf->getBufferSize(),
                 inBuf->getBufferAddr(), inBuf->getBufferSize());
        return OK;
    }
    return mPostProcessorCore->doPostProcessing(inBuf, outBuf);
}
}  // namespace icamera
