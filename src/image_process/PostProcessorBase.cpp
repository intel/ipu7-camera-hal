/*
 * Copyright (C) 2019-2024 Intel Corporation.
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
#define LOG_TAG PostProcessorBase

#include "PostProcessorBase.h"

#include <vector>
#ifdef LIBCAMERA_BUILD
#else
#endif

#include "iutils/CameraLog.h"
#include "stdlib.h"

using std::shared_ptr;

namespace icamera {

PostProcessorBase::PostProcessorBase(std::string processName)
        : mName(processName),
          mMemoryType(V4L2_MEMORY_USERPTR),
          mProcessor(nullptr) {}

ScaleProcess::ScaleProcess() : PostProcessorBase("Scaler") {
    LOG1("@%s create scaler processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t ScaleProcess::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                        shared_ptr<CameraBuffer>& outBuf) {
    PERF_CAMERA_ATRACE_PARAM1(mName.c_str(), 0);
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->scaleFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

RotateProcess::RotateProcess(int angle) : PostProcessorBase("Rotate"), mAngle(angle) {
    LOG1("@%s create rotate processor, degree: %d", __func__, mAngle);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t RotateProcess::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                         shared_ptr<CameraBuffer>& outBuf) {
    PERF_CAMERA_ATRACE_PARAM1(mName.c_str(), 0);
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);
    std::vector<uint8_t> rotateBuf;

    int ret = mProcessor->rotateFrame(inBuf, outBuf, mAngle, rotateBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

CropProcess::CropProcess() : PostProcessorBase("Crop") {
    LOG1("@%s create crop processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t CropProcess::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                       shared_ptr<CameraBuffer>& outBuf) {
    PERF_CAMERA_ATRACE_PARAM1(mName.c_str(), 0);
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->cropFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

ConvertProcess::ConvertProcess() : PostProcessorBase("Convert") {
    LOG1("@%s create convert processor", __func__);
    mProcessor = IImageProcessor::createImageProcessor();
}

status_t ConvertProcess::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                          shared_ptr<CameraBuffer>& outBuf) {
    PERF_CAMERA_ATRACE_PARAM1(mName.c_str(), 0);
    LOG1("@%s processor name: %s", __func__, mName.c_str());
    CheckAndLogError(!inBuf, UNKNOWN_ERROR, "%s, the inBuf is nullptr", __func__);
    CheckAndLogError(!outBuf, UNKNOWN_ERROR, "%s, the outBuf is nullptr", __func__);

    int ret = mProcessor->convertFrame(inBuf, outBuf);
    CheckAndLogError(ret != OK, UNKNOWN_ERROR, "Failed to do post processing, name: %s",
                     mName.c_str());

    return OK;
}

}  // namespace icamera
