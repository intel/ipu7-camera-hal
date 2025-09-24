/*
 * Copyright (C) 2019-2022 Intel Corporation
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

#define LOG_TAG ImageProcessorCore

#include "ImageProcessorCore.h"

#include "iutils/CameraLog.h"
#include "PlatformData.h"

namespace icamera {

libyuv::RotationMode getRotationMode(int angle) {
    switch (angle) {
        case 0: return libyuv::RotationMode::kRotate0;
        case 90: return libyuv::RotationMode::kRotate90;
        case 180: return libyuv::RotationMode::kRotate180;
        case 270: return libyuv::RotationMode::kRotate270;
        default: return libyuv::RotationMode::kRotate0;
    }
}

ImageProcessorCore::ImageProcessorCore() {}

std::unique_ptr<IImageProcessor> IImageProcessor::createImageProcessor() {
    return std::unique_ptr<ImageProcessorCore>(new ImageProcessorCore());
}

// If support this kind of post process type in current OS
bool IImageProcessor::isProcessingTypeSupported(PostProcessType type) {
    int supportedType = POST_PROCESS_CONVERT | POST_PROCESS_JPEG_ENCODING;
    if (PlatformData::useGPUProcessor()) {
        supportedType |= POST_PROCESS_GPU;
    } else {
        supportedType |= POST_PROCESS_ROTATE | POST_PROCESS_SCALING | POST_PROCESS_CROP;
    }

    return supportedType & type;
}

status_t ImageProcessorCore::cropFrame(const std::shared_ptr<CameraBuffer>& input,
                                       std::shared_ptr<CameraBuffer>& output) {
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x", __func__, input->getWidth(),
         input->getHeight(), input->getFormat(), output->getWidth(),
         output->getHeight(), output->getFormat());

    int srcW = input->getStride();
    int srcH = input->getHeight();
    int dstW = output->getStride();
    int dstH = output->getHeight();

    std::unique_ptr<uint8_t[]> srcI420Buf;
    unsigned int srcI420BufSize = srcW * srcH * 3 / 2;
    srcI420Buf.reset(new uint8_t[srcI420BufSize]);

    const uint8_t* srcBufY = static_cast<uint8_t*>(input->getBufferAddr());
    const uint8_t* srcBufUV = srcBufY + srcW * srcH;
    uint8_t* srcI420BufY = static_cast<uint8_t*>(srcI420Buf.get());
    uint8_t* srcI420BufU = srcI420BufY + srcW * srcH;
    uint8_t* srcI420BufV = srcI420BufU + srcW * srcH / 4;
    int ret = libyuv::NV12ToI420(srcBufY, srcW, srcBufUV, srcW, srcI420BufY, srcW, srcI420BufU,
                                 srcW / 2, srcI420BufV, srcW / 2, srcW, srcH);
    CheckAndLogError((ret != 0), UNKNOWN_ERROR, "NV12ToI420 failed");

    std::unique_ptr<uint8_t[]> dstI420BufUV;
    unsigned int dstI420BufUVSize = dstW * dstH / 2;
    dstI420BufUV.reset(new uint8_t[dstI420BufUVSize]);

    uint8_t* dstI420BufU = static_cast<uint8_t*>(dstI420BufUV.get());
    uint8_t* dstI420BufV = dstI420BufU + dstW * dstH / 4;
    int left = (input->getWidth() - output->getWidth()) / 2;
    int top = (srcH - dstH) / 2;
    ret = libyuv::ConvertToI420(static_cast<uint8_t*>(srcI420Buf.get()), srcI420BufSize,
                                static_cast<uint8_t*>(output->getBufferAddr()), dstW, dstI420BufU,
                                (dstW + 1) / 2, dstI420BufV, (dstW + 1) / 2, left, top,
                                srcW, srcH, output->getWidth(), dstH,
                                libyuv::RotationMode::kRotate0, libyuv::FourCC::FOURCC_I420);
    CheckAndLogError(ret != 0, UNKNOWN_ERROR, "ConvertToI420 failed");

    uint8_t* dstBufUV = static_cast<uint8_t*>(output->getBufferAddr()) + dstW * dstH;
    libyuv::MergeUVPlane(dstI420BufU, (dstW + 1) / 2, dstI420BufV, (dstW + 1) / 2, dstBufUV, dstW,
                         (dstW + 1) / 2, (dstH + 1) / 2);

    return OK;
}

status_t ImageProcessorCore::scaleFrame(const std::shared_ptr<CameraBuffer>& input,
                                        std::shared_ptr<CameraBuffer>& output) {
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x", __func__, input->getWidth(),
         input->getHeight(), input->getFormat(),
         output->getWidth(), output->getHeight(), output->getFormat());

    // Y plane
    libyuv::ScalePlane(static_cast<uint8_t*>(input->getBufferAddr()), input->getStride(),
                       input->getWidth(), input->getHeight(),
                       static_cast<uint8_t*>(output->getBufferAddr()), output->getStride(),
                       output->getWidth(), output->getHeight(), libyuv::kFilterNone);

    // UV plane
    int inUVOffsetByte = input->getStride() * input->getHeight();
    int outUVOffsetByte = output->getStride() * output->getHeight();
    libyuv::ScalePlane_16(
        static_cast<uint16_t*>(input->getBufferAddr()) + inUVOffsetByte / sizeof(uint16_t),
        input->getStride() / 2, input->getWidth() / 2, input->getHeight() / 2,
        static_cast<uint16_t*>(output->getBufferAddr()) + outUVOffsetByte / sizeof(uint16_t),
        output->getStride() / 2, output->getWidth() / 2, output->getHeight() / 2,
        libyuv::kFilterNone);

    return OK;
}

status_t ImageProcessorCore::rotateFrame(const std::shared_ptr<CameraBuffer>& input,
                                         std::shared_ptr<CameraBuffer>& output, int angle,
                                         std::vector<uint8_t>& rotateBuf) {
    LOG2("%s: src: %dx%d,format 0x%x, dest: %dx%d format 0x%x", __func__, input->getWidth(),
         input->getHeight(), input->getFormat(),
         output->getWidth(), output->getHeight(), output->getFormat());

    CheckAndLogError(output->getWidth() != input->getHeight() ||
                     output->getHeight() != input->getWidth(), BAD_VALUE,
                     "output resolution mismatch [%d x %d] -> [%d x %d]", input->getWidth(),
                     input->getHeight(), output->getWidth(), output->getHeight());
    CheckAndLogError((angle != 90 && angle != 270), BAD_VALUE, "angle value:%d is wrong", angle);

    const uint8_t* inBuffer = static_cast<uint8_t*>(input->getBufferAddr());
    uint8_t* outBuffer = static_cast<uint8_t*>(output->getBufferAddr());
    int outW = output->getWidth();
    int outH = output->getHeight();
    int outStride = output->getStride();
    int inW = input->getWidth();
    int inH = input->getHeight();
    int inStride = input->getStride();
    if (rotateBuf.size() < input->getBufferSize()) {
        rotateBuf.resize(input->getBufferSize());
    }

    uint8_t* I420Buffer = rotateBuf.data();

    if (getRotationMode(angle) == libyuv::RotationMode::kRotate0) {
        libyuv::CopyPlane(inBuffer, inStride, outBuffer, outStride, inW, inH);
        libyuv::CopyPlane(inBuffer + inH * inStride, inStride, outBuffer + outH * outStride,
                          outStride, inW, inH / 2);
    } else {
        int ret = libyuv::NV12ToI420Rotate(inBuffer, inStride, inBuffer + inH * inStride, inStride,
                                           I420Buffer, outW, I420Buffer + outW * outH, outW / 2,
                                           I420Buffer + outW * outH * 5 / 4, outW / 2, inW, inH,
                                           getRotationMode(angle));
        CheckAndLogError((ret < 0), UNKNOWN_ERROR, "NV12ToI420Rotate failed [%d]", ret);

        ret = libyuv::I420ToNV12(I420Buffer, outW, I420Buffer + outW * outH, outW / 2,
                                 I420Buffer + outW * outH * 5 / 4, outW / 2, outBuffer, outStride,
                                 outBuffer + outStride * outH, outStride, outW, outH);
        CheckAndLogError((ret < 0), UNKNOWN_ERROR, "I420ToNV12 failed [%d]", ret);
    }

    return OK;
}

status_t ImageProcessorCore::convertFrame(const std::shared_ptr<CameraBuffer>& input,
                                          std::shared_ptr<CameraBuffer>& output) {
    LOGE("Doesn't support the image convert: 0x%x -> 0x%x!", input->getFormat(),
         output->getFormat());
    return UNKNOWN_ERROR;
}
} /* namespace icamera */
