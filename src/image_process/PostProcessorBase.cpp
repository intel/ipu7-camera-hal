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

// JPEG_ENCODE_S
JpegProcess::JpegProcess(int cameraId)
        : PostProcessorBase("JpegEncode"),
          mCameraId(cameraId),
          mCropBuf(nullptr),
          mScaleBuf(nullptr),
          mThumbOut(nullptr),
          mExifData(nullptr) {
    LOG1("@%s create jpeg encode processor", __func__);

    mProcessor = IImageProcessor::createImageProcessor();
    mJpegEncoder = IJpegEncoder::createJpegEncoder();
    mMemoryType = mJpegEncoder->getMemoryType();
    mJpegMaker = std::unique_ptr<JpegMaker>(new JpegMaker());
}

void JpegProcess::attachJpegBlob(const EncodePackage& package) {
    LOG2("@%s, encoded data size: %d, exif data size: %d", __func__, package.encodedDataSize,
         package.exifDataSize);
    uint8_t* resultPtr = static_cast<uint8_t*>(package.outputData) + package.outputSize -
                         sizeof(struct camera3_jpeg_blob);

    // save jpeg size at the end of file
    auto* blob = reinterpret_cast<struct camera3_jpeg_blob*>(resultPtr);
    blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
    blob->jpeg_size = package.encodedDataSize + package.exifDataSize;
}

std::shared_ptr<CameraBuffer> JpegProcess::cropAndDownscaleThumbnail(
    int thumbWidth, int thumbHeight, const shared_ptr<CameraBuffer>& inBuf) {
    LOG2("@%s, input size: %dx%d, thumbnail info: %dx%d", __func__,
         inBuf->getWidth(), inBuf->getHeight(), thumbWidth, thumbHeight);

    if (thumbWidth <= 0 || thumbHeight <= 0) {
        LOGW("@%s, skip, thumbWidth:%d, thumbHeight:%d", __func__, thumbWidth, thumbHeight);
        return nullptr;
    }

    int ret = OK;
    shared_ptr<CameraBuffer> tempBuffer = inBuf;

    // Do crop first if needed
    if (IImageProcessor::isProcessingTypeSupported(POST_PROCESS_CROP) &&
        inBuf->getWidth() * thumbHeight != inBuf->getHeight() * thumbWidth) {
        int width = 0, height = 0;
        if (inBuf->getWidth() * thumbHeight < inBuf->getHeight() * thumbWidth) {
            width = inBuf->getWidth();
            height = ALIGN(inBuf->getWidth() * thumbHeight / thumbWidth, 2);
        } else {
            width = ALIGN(inBuf->getHeight() * thumbWidth / thumbHeight, 2);
            height = inBuf->getHeight();
        }

        if (mCropBuf && (mCropBuf->getWidth() != width || mCropBuf->getHeight() != height)) {
            mCropBuf.reset();
        }
        if (!mCropBuf) {
            int bufSize = CameraUtils::getFrameSize(inBuf->getFormat(), width, height,
                                                    false, false, false);
            mCropBuf = CameraBuffer::create(mMemoryType, bufSize, 0, inBuf->getFormat(),
                                            width, height);
            CheckAndLogError(!mCropBuf, nullptr,
                             "%s, Failed to allocate the internal crop buffer", __func__);
        }

        LOG2("@%s, Crop the main buffer from %dx%d to %dx%d", __func__, inBuf->getWidth(),
             inBuf->getHeight(), width, height);
        ret = mProcessor->cropFrame(inBuf, mCropBuf);
        CheckAndLogError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mCropBuf;
    }

    if (IImageProcessor::isProcessingTypeSupported(POST_PROCESS_SCALING)) {
        if (mScaleBuf &&
            (mScaleBuf->getWidth() != thumbWidth || mScaleBuf->getHeight() != thumbHeight))
            mScaleBuf.reset();
        if (!mScaleBuf) {
            int bufSize = CameraUtils::getFrameSize(inBuf->getFormat(), thumbWidth, thumbHeight,
                                                    false, false, false);
            mScaleBuf = CameraBuffer::create(mMemoryType, bufSize, 0, inBuf->getFormat(),
                                             thumbWidth, thumbHeight);
            CheckAndLogError(!mScaleBuf, nullptr,
                             "%s, Failed to allocate the internal crop buffer", __func__);
        }

        LOG2("@%s, Scale the buffer from %dx%d to %dx%d", __func__, inBuf->getWidth(),
             inBuf->getHeight(), thumbWidth, thumbHeight);
        ret = mProcessor->scaleFrame(tempBuffer, mScaleBuf);
        CheckAndLogError(ret != OK, nullptr, "%s, Failed to crop the frame", __func__);
        tempBuffer = mScaleBuf;
    }

    if (tempBuffer->getWidth() != thumbWidth || tempBuffer->getHeight() != thumbHeight) {
        LOGE("%s, Failed to crop & downscale the main buffer to thumbnail buffer", __func__);
        return nullptr;
    }

    return tempBuffer;
}

void JpegProcess::fillEncodeInfo(const shared_ptr<CameraBuffer>& inBuf,
                                 const shared_ptr<CameraBuffer>& outBuf,
                                 EncodePackage& package) {
    package.inputWidth = inBuf->getWidth();
    package.inputHeight = inBuf->getHeight();
    package.inputStride = inBuf->getStride();
    package.inputFormat = inBuf->getFormat();
    package.inputSize = inBuf->getBufferSize();

    if (inBuf->getMemory() == V4L2_MEMORY_DMABUF && outBuf->getMemory() == V4L2_MEMORY_DMABUF) {
        camera_buffer_t* uInBuf = inBuf->getUserBuffer();
        package.inputBufferHandle = &(uInBuf->privateHandle);

        camera_buffer_t* uOutBuf = outBuf->getUserBuffer();
        package.outputBufferHandle = &(uOutBuf->privateHandle);
    }

    package.inputData = inBuf->getBufferAddr();
    package.outputData = outBuf->getBufferAddr();

    package.outputWidth = outBuf->getWidth();
    package.outputHeight = outBuf->getHeight();
    package.outputSize = outBuf->getBufferSize();
}

status_t JpegProcess::doPostProcessing(const shared_ptr<CameraBuffer>& inBuf,
                                       shared_ptr<CameraBuffer>& outBuf) {
    PERF_CAMERA_ATRACE_PARAM1(mName.c_str(), 0);
    LOG1("@%s processor name: %s", __func__, mName.c_str());

    bool isEncoded = false;

    icamera::ExifMetaData exifMetadata;
    status_t status = mJpegMaker->setupExifWithMetaData(inBuf->getWidth(), inBuf->getHeight(),
                          inBuf->getSequence(), TIMEVAL2NSECS(inBuf->getTimestamp()),mCameraId,
                          &exifMetadata);
    CheckAndLogError(status != OK, UNKNOWN_ERROR, "@%s, Setup exif metadata failed.", __func__);
    LOG2("@%s: setting exif metadata done!", __func__);

    std::shared_ptr<CameraBuffer> thumbInput = cropAndDownscaleThumbnail(
        exifMetadata.mJpegSetting.thumbWidth, exifMetadata.mJpegSetting.thumbHeight, inBuf);

    EncodePackage thumbnailPackage;
    if (thumbInput) {
        if (mThumbOut == nullptr ||
            mThumbOut->getWidth() != exifMetadata.mJpegSetting.thumbWidth ||
            mThumbOut->getHeight() != exifMetadata.mJpegSetting.thumbHeight ||
            mThumbOut->getFormat() != outBuf->getFormat()) {
            int bufSize = CameraUtils::getFrameSize(inBuf->getFormat(),
                                                    exifMetadata.mJpegSetting.thumbWidth,
                                                    exifMetadata.mJpegSetting.thumbHeight,
                                                    false, false, false);

            mThumbOut = CameraBuffer::create(mMemoryType, bufSize, 0, V4L2_PIX_FMT_JPEG,
                                             exifMetadata.mJpegSetting.thumbWidth,
                                             exifMetadata.mJpegSetting.thumbHeight);
            CheckAndLogError(!mThumbOut, NO_MEMORY,
                             "%s, Failed to allocate the internal crop buffer", __func__);
        }

        // encode thumbnail image
        fillEncodeInfo(thumbInput, mThumbOut, thumbnailPackage);
        thumbnailPackage.quality = exifMetadata.mJpegSetting.jpegThumbnailQuality;
        // the exifDataSize should be 0 for encoding thumbnail
        thumbnailPackage.exifData = nullptr;
        thumbnailPackage.exifDataSize = 0;

        do {
            isEncoded = mJpegEncoder->doJpegEncode(&thumbnailPackage);
            thumbnailPackage.quality -= 5;
        } while (thumbnailPackage.encodedDataSize > THUMBNAIL_SIZE_LIMITATION &&
                 thumbnailPackage.quality > 0);

        if (!isEncoded || thumbnailPackage.quality < 0) {
            LOGW(
                "Failed to generate thumbnail, isEncoded: %d, encoded thumbnail size: %d, "
                "quality:%d",
                isEncoded, thumbnailPackage.encodedDataSize, thumbnailPackage.quality);
        }
    }

    // save exif data
    uint32_t exifBufSize = ENABLE_APP2_MARKER ? EXIF_SIZE_LIMITATION * 2 : EXIF_SIZE_LIMITATION;
    if (mExifData == nullptr) {
        mExifData = std::unique_ptr<unsigned char[]>(new unsigned char[exifBufSize]);
    }
    uint8_t* finalExifDataPtr = static_cast<uint8_t*>(mExifData.get());
    uint32_t finalExifDataSize = 0;
    status = mJpegMaker->getExif(thumbnailPackage, finalExifDataPtr, &finalExifDataSize);
    CheckAndLogError(status != OK, status, "@%s, Failed to get Exif", __func__);
    LOG2("%s, exifBufSize %d, finalExifDataSize %d", __func__, exifBufSize, finalExifDataSize);

    // encode main image
    EncodePackage finalEncodePackage;
    fillEncodeInfo(inBuf, outBuf, finalEncodePackage);
    finalEncodePackage.quality = exifMetadata.mJpegSetting.jpegQuality;
    finalEncodePackage.exifData = finalExifDataPtr;
    finalEncodePackage.exifDataSize = finalExifDataSize;
    isEncoded = mJpegEncoder->doJpegEncode(&finalEncodePackage);
    CheckAndLogError(!isEncoded, UNKNOWN_ERROR, "@%s, Failed to encode main image", __func__);
    mJpegMaker->writeExifData(&finalEncodePackage);
    attachJpegBlob(finalEncodePackage);

    return OK;
}
// JPEG_ENCODE_E
}  // namespace icamera
