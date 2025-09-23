/*
 * Copyright (C) 2016-2024 Intel Corporation
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

#define LOG_TAG JpegMaker

#include "JpegMaker.h"

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"
#include "CameraContext.h"
#include "AiqResultStorage.h"

namespace icamera {

JpegMaker::JpegMaker() {
    LOG2("@%s", __func__);
    mExifMaker = std::unique_ptr<EXIFMaker>(new EXIFMaker());
}

JpegMaker::~JpegMaker() {
    LOG2("@%s", __func__);
}

status_t JpegMaker::setupExifWithMetaData(int bufWidth, int bufHeight, int64_t sequence,
                                          uint64_t timestamp, int cameraId,
                                          ExifMetaData* metaData) {
    LOG2("@%s", __func__);

    auto cameraContext = CameraContext::getInstance(cameraId);
    auto dataContext = cameraContext->getDataContextBySeq(sequence);
    auto resultStorage = cameraContext->getAiqResultStorage();
    const AiqResult* aiqResult = resultStorage->getAiqResult(sequence);
    if (aiqResult == nullptr) {
        LOGW("Can't find Aiq Result for sequence %ld, use latest result", __func__);
        aiqResult = resultStorage->getAiqResult();
    }

    status_t status = processJpegSettings(aiqResult, dataContext, metaData);
    CheckAndLogError(status != OK, status, "@%s: Process settngs for JPEG failed!", __func__);

    mExifMaker->initialize(bufWidth, bufHeight);
    mExifMaker->pictureTaken(metaData);

    mExifMaker->enableFlash(metaData->flashFired, metaData->v3AeMode, metaData->flashMode);
    mExifMaker->updateSensorInfo(dataContext, cameraId);
    mExifMaker->saveMakernote(cameraId, timestamp);

    status = processExifSettings(dataContext, metaData);
    if (status != OK) {
        LOGE("@%s: Process settngs for Exif! %d", __func__, status);
        return status;
    }

    mExifMaker->initializeLocation(metaData);
    mExifMaker->setSensorAeConfig(aiqResult, dataContext);

    if (metaData->software != nullptr) {
        mExifMaker->setSoftware(metaData->software);
    }

    return status;
}

status_t JpegMaker::getExif(const EncodePackage& thumbnailPackage, uint8_t* exifPtr,
                            uint32_t* exifSize) {
    if (thumbnailPackage.encodedDataSize > 0 && thumbnailPackage.quality > 0) {
        mExifMaker->setThumbnail(static_cast<unsigned char*>(thumbnailPackage.outputData),
                                 thumbnailPackage.encodedDataSize, thumbnailPackage.outputWidth,
                                 thumbnailPackage.outputHeight);
    }
    *exifSize = mExifMaker->makeExif(exifPtr);
    return *exifSize > 0 ? OK : UNKNOWN_ERROR;
}

status_t JpegMaker::processExifSettings(const DataContext* dataContext, ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    status = processGpsSettings(dataContext, metaData);
    status |= processColoreffectSettings(dataContext, metaData);
    status |= processScalerCropSettings(dataContext, metaData);

    return status;
}

/* copy exif data into output buffer */
void JpegMaker::writeExifData(EncodePackage* package) {
    CheckAndLogError(package == nullptr, VOID_VALUE, "@%s, package is nullptr", __func__);

    if (package->exifDataSize == 0) return;

    CheckAndLogError(!package->outputData, VOID_VALUE, "@%s, outputData is nullptr", __func__);
    CheckAndLogError(!package->exifData, VOID_VALUE, "@%s, exifData is nullptr", __func__);

    unsigned int jSOISize = sizeof(mJpegMarkerSOI);
    unsigned char* jpegOut = reinterpret_cast<unsigned char*>(package->outputData);
    MEMCPY_S(jpegOut, jSOISize, mJpegMarkerSOI, jSOISize);
    jpegOut += jSOISize;

    MEMCPY_S(jpegOut, package->exifDataSize, reinterpret_cast<unsigned char*>(package->exifData),
             package->exifDataSize);
}

/**
 * processJpegSettings
 *
 * Store JPEG settings to the exif metadata
 *
 * \ dataContext [IN] jpeg parameters
 * \ aiqResult [IN] jpeg parameters
 *
 * \ metaData [out] metadata of the request
 *
 */
status_t JpegMaker::processJpegSettings(const AiqResult* aiqResult, const DataContext* dataContext,
                                        ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    CheckAndLogError(!metaData, UNKNOWN_ERROR, "MetaData struct not initialized");

    metaData->mJpegSetting.jpegQuality = dataContext->mJpegParams.jpegQuality;

    metaData->mJpegSetting.jpegThumbnailQuality = dataContext->mJpegParams.thumbQuality;
    metaData->mJpegSetting.thumbWidth = dataContext->mJpegParams.thumbSize.width;
    metaData->mJpegSetting.thumbHeight = dataContext->mJpegParams.thumbSize.height;

    metaData->mJpegSetting.orientation = dataContext->mJpegParams.rotation;

    LOG1("jpegQuality=%d,thumbQuality=%d,thumbW=%d,thumbH=%d,orientation=%d",
         metaData->mJpegSetting.jpegQuality, metaData->mJpegSetting.jpegThumbnailQuality,
         metaData->mJpegSetting.thumbWidth, metaData->mJpegSetting.thumbHeight,
         metaData->mJpegSetting.orientation);

    metaData->aeMode = dataContext->mAiqParams.aeMode;
    metaData->awbMode = dataContext->mAiqParams.awbMode;

    metaData->currentFocusDistance = 0.0;
    float focusDistance = aiqResult->mAfDistanceDiopters;
    if (focusDistance != 0) {
        metaData->currentFocusDistance = ceil(1000.0 / focusDistance);
    }
    LOG2("aeMode=%d, awbMode=%d, currentFocusDistance=%f", metaData->aeMode, metaData->awbMode,
         metaData->currentFocusDistance);

    return status;
}

/**
 * This function will get GPS metadata from request setting
 *
 * \param[in] settings The Android metadata to process GPS settings from
 * \param[out] metadata The EXIF data where the GPS setting are written to
 */
status_t JpegMaker::processGpsSettings(const DataContext* dataContext, ExifMetaData* metadata) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    // gps latitude
    metadata->mGpsSetting.latitude = dataContext->mJpegParams.latitude;

    metadata->mGpsSetting.longitude = dataContext->mJpegParams.longitude;

    metadata->mGpsSetting.altitude = dataContext->mJpegParams.altitude;

    // gps timestamp
    metadata->mGpsSetting.gpsTimeStamp = dataContext->mJpegParams.gpsTimestamp;

    // gps processing method
    if (strlen(dataContext->mJpegParams.gpsProcessingMethod) != 0) {
        snprintf(metadata->mGpsSetting.gpsProcessingMethod,
                 sizeof(metadata->mGpsSetting.gpsProcessingMethod), "%s",
                 dataContext->mJpegParams.gpsProcessingMethod);
    }

    return status;
}

status_t JpegMaker::processColoreffectSettings(const DataContext* dataContext,
                                               ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    metaData->effectMode = dataContext->mAiqParams.effectMode;
    LOG2("effect mode=%d", metaData->effectMode);

    return status;
}

status_t JpegMaker::processScalerCropSettings(const DataContext* dataContext,
                                              ExifMetaData* metaData) {
    LOG2("@%s:", __func__);
    status_t status = OK;

    return status;
}

}  // namespace icamera
