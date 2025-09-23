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

#define LOG_TAG CameraDevice

#include "CameraDevice.h"

#include <vector>

#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

#include "GraphConfig.h"
#include "AiqUtils.h"
#include "I3AControlFactory.h"
#include "PlatformData.h"
#include "V4l2DeviceFactory.h"
// FILE_SOURCE_S
#include "FileSource.h"
// FILE_SOURCE_E
#include "CaptureUnit.h"
#include "CameraContext.h"
#include "StageDescriptor.h"

using std::vector;

namespace icamera {
CameraDevice::CameraDevice(int cameraId)
        : mState(DEVICE_UNINIT),
          mProcessingUnit(nullptr),
          mCameraId(cameraId),
          mStreamNum(0),
          mDataContext(nullptr),
          mGcMgr(nullptr),
          mCallback(nullptr),
          mScheduler(nullptr) {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s", mCameraId, __func__);

    CLEAR(mStreams);
    CLEAR(mFdStream);

    V4l2DeviceFactory::createDeviceFactory(mCameraId);
    CLEAR(mInputConfig);
    mInputConfig.format = -1;

    mDataContext = new DataContext(cameraId);

    // CSI_META_S
    mCsiMetaDevice = new CsiMetaDevice(mCameraId);
    // CSI_META_E

    mProducer = createBufferProducer();

    mSofSource = new SofSource(mCameraId);

    mPerframeControlSupport = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);
    LOG2("%s: support perframe %d", __func__, mPerframeControlSupport);

    mLensCtrl = new LensHw(mCameraId);
    mSensorCtrl = SensorHwCtrl::createSensorCtrl(mCameraId);

    m3AControl = I3AControlFactory::createI3AControl(mCameraId, mSensorCtrl, mLensCtrl);
    mRequestThread = new RequestThread(mCameraId, m3AControl);
    mRequestThread->registerListener(EVENT_PROCESS_REQUEST, this);

    mScheduler = std::make_shared<CameraScheduler>(mCameraId);
}

CameraDevice::~CameraDevice() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s", mCameraId, __func__);
    AutoMutex m(mDeviceLock);

    // Clear the media control when close the device.
    MediaControl* mc = MediaControl::getInstance();
    MediaCtlConf* mediaCtl = PlatformData::getMediaCtlConf(mCameraId);
    if ((mc != nullptr) && (mediaCtl != nullptr)) {
        mc->mediaCtlClear(mCameraId, mediaCtl);
    }

    mRequestThread->removeListener(EVENT_PROCESS_REQUEST, this);

    for (int i = 0; i < MAX_STREAM_NUMBER; i++) {
        delete mStreams[i];
    }

    if (mGcMgr != nullptr) {
        delete mGcMgr;
    }

    delete mLensCtrl;
    delete m3AControl;
    delete mSensorCtrl;
    delete mSofSource;
    delete mProducer;
    // CSI_META_S
    delete mCsiMetaDevice;
    // CSI_META_E
    delete mRequestThread;

    V4l2DeviceFactory::releaseDeviceFactory(mCameraId);

    delete mDataContext;
}

int CameraDevice::init() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s, mState:%d", mCameraId, __func__, mState);
    AutoMutex m(mDeviceLock);

    int ret = mProducer->init();
    CheckAndLogError(ret < 0, ret, "%s: Init capture unit failed", __func__);

    // CSI_META_S
    ret = mCsiMetaDevice->init();
    CheckAndLogError(ret != OK, ret, "@%s: init csi meta device failed", __func__);
    // CSI_META_E

    ret = mSofSource->init();
    CheckAndLogError(ret != OK, ret, "@%s: init sync manager failed", __func__);

    m3AControl->init();

    mRequestThread->requestStart();

    mState = DEVICE_INIT;
    return ret;
}

void CameraDevice::deinit() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s, mState:%d", mCameraId, __func__, mState);
    AutoMutex m(mDeviceLock);

    // deinit should not be call in UNINIT or START STATE
    if (mState == DEVICE_UNINIT) {
        // Do nothing
        return;
    }

    m3AControl->stop();

    if (mState == DEVICE_START) {
        // stop first
        (void)stopLocked();
    }

    // stop request thread
    mRequestThread->requestStop();

    deleteStreams();

    delete mProcessingUnit;

    m3AControl->deinit();

    mSofSource->deinit();

    // CSI_META_S
    mCsiMetaDevice->deinit();
    // CSI_META_E

    mProducer->deinit();

    mState = DEVICE_UNINIT;
}

void CameraDevice::callbackRegister(const camera_callback_ops_t* callback) {
    mCallback = const_cast<camera_callback_ops_t*>(callback);
}

StreamSource* CameraDevice::createBufferProducer() {
    // FILE_SOURCE_S
    if (PlatformData::isFileSourceEnabled()) {
        return new FileSource(mCameraId);
    }
    // FILE_SOURCE_E

    // DUMMY_SOURCE_S
    if (!PlatformData::isIsysEnabled(mCameraId)) {
        return new DummySource;
    }
    // DUMMY_SOURCE_E

    return new CaptureUnit(mCameraId);
}

void CameraDevice::bindListeners() {
    if (mProcessingUnit != nullptr) {
        vector<EventListener*> statsListenerList = m3AControl->getStatsEventListener();
        for (auto statsListener : statsListenerList) {
            // Subscribe PSys statistics.
            mProcessingUnit->registerListener(EVENT_PSYS_STATS_BUF_READY, statsListener);
            mProcessingUnit->registerListener(EVENT_PSYS_STATS_SIS_BUF_READY, statsListener);
        }

        mProcessingUnit->registerListener(EVENT_PSYS_STATS_BUF_READY, mRequestThread);

        // CSI_META_S
        // Listen to meta data when enabled
        if (mCsiMetaDevice->isEnabled()) {
            mCsiMetaDevice->registerListener(EVENT_META, mProcessingUnit);
        }
        // CSI_META_E

        mProcessingUnit->registerListener(EVENT_PSYS_REQUEST_BUF_READY, this);
        mProcessingUnit->registerListener(EVENT_REQUEST_METADATA_READY, this);
    }

    vector<EventListener*> sofListenerList = m3AControl->getSofEventListener();
    for (auto sofListener : sofListenerList) {
        mSofSource->registerListener(EVENT_ISYS_SOF, sofListener);
        // FILE_SOURCE_S
        if (PlatformData::isFileSourceEnabled()) {
            // File source needs to produce SOF event as well when it's enabled.
            mProducer->registerListener(EVENT_ISYS_SOF, sofListener);
        }
        // FILE_SOURCE_E
    }

    if (mPerframeControlSupport || (!PlatformData::isIsysEnabled(mCameraId))) {
        mProcessingUnit->registerListener(EVENT_PSYS_FRAME, mRequestThread);
    } else {
        mProducer->registerListener(EVENT_ISYS_FRAME, mRequestThread);
    }

    mSofSource->registerListener(EVENT_ISYS_SOF, mRequestThread);
    // FILE_SOURCE_S
    if (PlatformData::isFileSourceEnabled()) {
        // File source needs to produce SOF event as well when it's enabled.
        mProducer->registerListener(EVENT_ISYS_SOF, mRequestThread);
    }
    // FILE_SOURCE_E
}

void CameraDevice::unbindListeners() {
    if (mProcessingUnit != nullptr) {
        vector<EventListener*> statsListenerList = m3AControl->getStatsEventListener();
        for (auto statsListener : statsListenerList) {
            mProcessingUnit->removeListener(EVENT_PSYS_STATS_BUF_READY, statsListener);
            mProcessingUnit->removeListener(EVENT_PSYS_STATS_SIS_BUF_READY, statsListener);
        }
        mProcessingUnit->removeListener(EVENT_PSYS_STATS_BUF_READY, mRequestThread);

        // CSI_META_S
        if (mCsiMetaDevice->isEnabled()) {
            mCsiMetaDevice->removeListener(EVENT_META, mProcessingUnit);
        }
        // CSI_META_E

        mProcessingUnit->removeListener(EVENT_PSYS_REQUEST_BUF_READY, this);
        mProcessingUnit->removeListener(EVENT_REQUEST_METADATA_READY, this);
    }
    vector<EventListener*> sofListenerList = m3AControl->getSofEventListener();
    for (auto sofListener : sofListenerList) {
        mSofSource->removeListener(EVENT_ISYS_SOF, sofListener);
        // FILE_SOURCE_S
        if (PlatformData::isFileSourceEnabled()) {
            mProducer->removeListener(EVENT_ISYS_SOF, sofListener);
        }
        // FILE_SOURCE_E
    }

    if (mPerframeControlSupport || (!PlatformData::isIsysEnabled(mCameraId))) {
        mProcessingUnit->removeListener(EVENT_PSYS_FRAME, mRequestThread);
    } else {
        mProducer->removeListener(EVENT_ISYS_FRAME, mRequestThread);
    }

    mSofSource->removeListener(EVENT_ISYS_SOF, mRequestThread);
    // FILE_SOURCE_S
    if (PlatformData::isFileSourceEnabled()) {
        // File source needs to produce SOF event as well when it's enabled.
        mProducer->removeListener(EVENT_ISYS_SOF, mRequestThread);
    }
    // FILE_SOURCE_E
}

int CameraDevice::configureInput(const stream_t* inputConfig) {
    AutoMutex lock(mDeviceLock);
    mInputConfig = *inputConfig;

    return OK;
}

int CameraDevice::configure(stream_config_t* streamList) {
    PERF_CAMERA_ATRACE();
    CheckAndLogError(streamList->streams == nullptr, BAD_VALUE, "%s: No valid stream config", __func__);
    CheckAndLogError(((streamList->num_streams > MAX_STREAM_NUMBER) ||
                      (streamList->num_streams <= 0)), BAD_VALUE,
                      "%s: The stream number(%d) out of range: [1-%d]", __func__,
                     streamList->num_streams, MAX_STREAM_NUMBER);
    CheckAndLogError(
        (mState != DEVICE_STOP) && (mState != DEVICE_INIT) && (mState != DEVICE_CONFIGURE),
        INVALID_OPERATION, "%s: Add streams in wrong state %d", __func__, mState);

    LOG1("<id%d>@%s, operation_mode %x", mCameraId, __func__,
         static_cast<ConfigMode>(streamList->operation_mode));

    AutoMutex lock(mDeviceLock);

    // Release the resource created last time
    deleteStreams();
    delete mProcessingUnit;
    mProcessingUnit = nullptr;
    mProducer->removeAllFrameAvailableListener();

    /*
     * The configure flow for CameraStream
     * 1. Analyze all the streams
     * 2. Config the graph
     * 3. Assign port for each stream
     * 4. Create the CameraStream classes
     * 5. Create the processor
     * 6. Bind the CameraStream to processor
     */
    int inputRawStreamId = -1, preStreamIdForFace = -1, inputYuvStreamId = -1;
    int ret = analyzeStream(streamList, &inputRawStreamId, &preStreamIdForFace, &inputYuvStreamId);
    CheckAndLogError(ret != OK, ret, "@%s, analyzeStream failed", __func__);

    /*
     * According to preview to create a private stream for face detection
     * and the resolution is 640x480(4:3) or 640x360(16:9).
     */
    if (preStreamIdForFace >= 0) {
        CLEAR(mFdStream);
        mFdStream = streamList->streams[preStreamIdForFace];
        mFdStream.usage = static_cast<int32_t>(CAMERA_STREAM_FACE);
        mFdStream.width = RESOLUTION_VGA_WIDTH;
        mFdStream.height = RESOLUTION_VGA_HEIGHT;
        if (streamList->streams[preStreamIdForFace].width * RESOLUTION_VGA_HEIGHT !=
            streamList->streams[preStreamIdForFace].height * RESOLUTION_VGA_WIDTH) {
            mFdStream.height = RESOLUTION_360P_HEIGHT;
        }

        mFdStream.id = streamList->num_streams;
        mFdStream.stride = CameraUtils::getStride(mFdStream.format, mFdStream.width);
        mFdStream.size = CameraUtils::getFrameSize(mFdStream.format, mFdStream.width,
                                                    mFdStream.height, false, false, false);
        LOG2("@%s, face stream(%d), format:%s (%dx%d)", __func__, mFdStream.id,
             CameraUtils::pixelCode2String(mFdStream.format), mFdStream.width, mFdStream.height);
    }

    /* Config the graph
     * The return value is the number of configured streams.
     * The max value of totalStream is streamList->num_streams + 1
     * preStreamIdForFace = -1 is mean doesn't find the preview stream or
     * face detection is not supported
     */
    int mcId = -1;
    int totalStream = 0;

    // Graph config only be used for PSYS processor now
    if ((mGcMgr == nullptr) && PlatformData::isUsePSysProcessor(mCameraId)) {
        mGcMgr = new GraphConfigManager(mCameraId);
    }
    if (mGcMgr != nullptr) {
        std::map<ConfigMode, std::shared_ptr<GraphConfig> > gcs;
        // Clear old gcs
        CameraContext::getInstance(mCameraId)->storeGraphConfig(gcs);
        totalStream = mGcMgr->configStreams(streamList, gcs,
                                           (preStreamIdForFace >= 0) ? &mFdStream : nullptr);
        CheckAndLogError(totalStream < 0, INVALID_OPERATION, "No matching graph config found");
        CameraContext::getInstance(mCameraId)->storeGraphConfig(gcs);
        mcId = mGcMgr->getSelectedMcId();
    }

    if (mGcMgr != nullptr) {
        vector<ConfigMode> configModes;
        PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode,
                                                    configModes);
        // Config the H-Scheduler based on graph id
        std::shared_ptr<GraphConfig> gc =
                CameraContext::getInstance(mCameraId)->getGraphConfig(configModes[0]);
        CheckAndLogError(gc == nullptr, UNKNOWN_ERROR, "Failed to get GraphConfig!");
        const int graphId = gc->getGraphId();
        if (mScheduler != nullptr) {
            ret = mScheduler->configurate(graphId);
            CheckAndLogError(ret != OK, ret, "@%s Failed to configure H-Scheduler", __func__);
        }
    }

    ret = assignPortForStreams(streamList, inputRawStreamId, inputYuvStreamId, totalStream);
    CheckAndLogError(ret < 0, ret, "@%s Failed to assign port for streams", __func__);

    ret = createStreams(streamList, totalStream);
    CheckAndLogError(ret < 0, ret, "@%s Failed to create streams", __func__);
    mRequestThread->configure(streamList);

    std::map<uuid, stream_t> producerConfigs = selectProducerConfig(streamList, mcId);
    CheckAndLogError(producerConfigs.empty(), BAD_VALUE, "The config for producer is invalid.");

    const bool needProcessor = isProcessorNeeded(streamList, producerConfigs[MAIN_INPUT_PORT_UID]);
    for (auto& item : producerConfigs) {
        LOG1("Producer config for port:%d, fmt:%s (%dx%d), needProcessor=%d", item.first,
             CameraUtils::format2string(item.second.format).c_str(), item.second.width,
             item.second.height, needProcessor);
        // Only V4L2_MEMORY_MMAP is supported when using post processor
        if (needProcessor) {
            item.second.memType = V4L2_MEMORY_MMAP;
        }
    }

    ret = mProducer->configure(producerConfigs);
    CheckAndLogError(ret < 0, BAD_VALUE, "@%s Device Configure failed", __func__);

    // CSI_META_S
    ret = mCsiMetaDevice->configure();
    CheckAndLogError(ret != OK, ret, "@%s failed to configure CSI meta device", __func__);
    // CSI_META_E

    // CRL_MODULE_S
    ret = mSensorCtrl->configure();
    CheckAndLogError(ret != OK, ret, "@%s failed to configure sensor HW", __func__);
    // CRL_MODULE_E

    ret = mSofSource->configure();
    CheckAndLogError(ret != OK, ret, "@%s failed to configure SOF source device", __func__);

    m3AControl->configure(streamList);

    if (needProcessor) {
        mProcessingUnit =
            IProcessingUnitFactory::createIProcessingUnit(mCameraId, mScheduler);
        CheckAndLogError(mProcessingUnit == nullptr, UNKNOWN_ERROR,
                         "@%s Failed to create ProcessingUnit", __func__);

        if (mProcessingUnit != nullptr) {
            std::map<uuid, stream_t> outputConfigs;
            for (const auto& item : mStreamIdToPortMap) {
                outputConfigs[item.second] = ((item.first > 0) && (item.first == mFdStream.id)) ?
                                             mFdStream : streamList->streams[item.first];
            }
            vector<ConfigMode> configModes;
            PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode,
                                                        configModes);
            ret = mProcessingUnit->configure(producerConfigs, outputConfigs, configModes[0]);
            CheckAndLogError(ret != OK, ret, "@%s failed to configure ProcessingUnit", __func__);
            mProcessingUnit->setBufferProducer(mProducer);
        }
    }

    ret = bindStreams(streamList);
    CheckAndLogError(ret < 0, ret, "@%s bind stream failed with %d", __func__, ret);

    mState = DEVICE_CONFIGURE;
    return OK;
}

/**
 * Select the producer's config from the supported list.
 *
 * How to decide the producer's config?
 * 1. Select the input stream if it's provided
 * 2. Use user's cropRegion or CSI output in graph to select the MC and producerConfigs
 * 3. Try to use the same config as user's required.
 * 4. Select the producerConfigs of SECOND_PORT if DOL enabled
 */
std::map<uuid, stream_t> CameraDevice::selectProducerConfig(const stream_config_t* streamList,
                                                            int mcId) {
    std::map<uuid, stream_t> producerConfigs;
    if (!PlatformData::isIsysEnabled(mCameraId)) {
        // Input stream id is the last one of mSortedStreamIds
        const stream_t& tmp = streamList->streams[mSortedStreamIds.back()];
        if (tmp.streamType == CAMERA_STREAM_INPUT) {
            producerConfigs[MAIN_INPUT_PORT_UID] = tmp;
            LOG2("%s: producer is user input stream", __func__);
            return producerConfigs;
        }
    }

    stream_t biggestStream = streamList->streams[mSortedStreamIds[0]];
    if (mDataContext->cropRegion.flag == 1) {
        // Use crop region to select MC config
        PlatformData::selectMcConf(mCameraId, mInputConfig,
                                   static_cast<ConfigMode>(streamList->operation_mode),
                                   mcId);
    } else {
        stream_t matchedStream = biggestStream;

        if (PlatformData::isUsePSysProcessor(mCameraId)) {
            // Use CSI output to select MC config
            vector<ConfigMode> configModes;
            PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode,
                                                        configModes);
            std::shared_ptr<GraphConfig> gc =
                CameraContext::getInstance(mCameraId)->getGraphConfig(configModes[0]);
            if ((!configModes.empty()) && (gc != nullptr)) {
                camera_resolution_t csiOutput = {0, 0};
                gc->getCSIOutputResolution(csiOutput);
                if ((csiOutput.width > 0) && (csiOutput.height > 0)) {
                    matchedStream.width = csiOutput.width;
                    matchedStream.height = csiOutput.height;
                }
            }
        }

        PlatformData::selectMcConf(mCameraId, matchedStream,
                                   static_cast<ConfigMode>(streamList->operation_mode),
                                   mcId);
    }

    // Select the output format.
    int iSysFmt = biggestStream.format;
    if (mInputConfig.format != -1) {
        if (!PlatformData::isISysSupportedFormat(mCameraId, mInputConfig.format)) {
            LOGE("The given ISYS format %s is unsupported.",
                 CameraUtils::pixelCode2String(mInputConfig.format));
            return producerConfigs;
        }
        iSysFmt = mInputConfig.format;
    }
    PlatformData::selectISysFormat(mCameraId, iSysFmt);

    // Filter the ISYS best resolution with input stream
    const int inputWidth = mInputConfig.width;
    const int inputHeight = mInputConfig.height;
    camera_resolution_t producerRes = {inputWidth, inputHeight};
    if ((inputWidth == 0) && (inputHeight == 0)) {
        // Only get the ISYS resolution when input config is not specified.
        producerRes = PlatformData::getISysBestResolution(
            mCameraId, biggestStream.width, biggestStream.height, biggestStream.field);
    } else if (!PlatformData::isISysSupportedResolution(mCameraId, producerRes)) {
        LOGE("The stream config: (%dx%d) is not supported.", inputWidth, inputHeight);
        return producerConfigs;
    }

    // Update the height according to the field(interlaced).
    stream_t mainConfig = {};
    mainConfig.memType = biggestStream.memType;
    mainConfig.field = biggestStream.field;
    mainConfig.format = PlatformData::getISysFormat(mCameraId);
    mainConfig.width = producerRes.width;
    mainConfig.height = CameraUtils::getInterlaceHeight(mainConfig.field, producerRes.height);

    // configuration with main port
    producerConfigs[MAIN_INPUT_PORT_UID] = mainConfig;

    LOG2("%s: mcId %d", __func__, mcId);
    return producerConfigs;
}

/**
 * Check if post processor is needed.
 * The processor is needed when:
 * 1. At least one of the given streams does not match with the producer's output.
 * 2. To support specific features such as HW weaving or dewarping.
 */
bool CameraDevice::isProcessorNeeded(const stream_config_t* streamList,
                                     const stream_t& producerConfig) const {
    if (mDataContext->cropRegion.flag == 1) {
        return true;
    }

    if (producerConfig.field != V4L2_FIELD_ANY) {
        if (mDataContext->deinterlaceMode == DEINTERLACE_WEAVING) {
            return true;
        }
    }

    if (producerConfig.field != V4L2_FIELD_ALTERNATE) {
        const int streamCounts = streamList->num_streams;
        for (int streamId = 0; streamId < streamCounts; streamId++) {
            if ((producerConfig.width != streamList->streams[streamId].width) ||
                (producerConfig.height != streamList->streams[streamId].height) ||
                (producerConfig.format != streamList->streams[streamId].format)) {
                return true;
            }
        }
    }

    if (mDataContext->monoDsMode != MONO_DS_MODE_OFF) {
        return true;
    }

    return false;
}

int CameraDevice::createStreams(const stream_config_t* streamList, int configuredStreamNum) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    const int streamCounts = streamList->num_streams;
    for (int streamId = 0; streamId < streamCounts; streamId++) {
        stream_t& streamConf = streamList->streams[streamId];
        LOG1("@%s, stream_number:%d, stream configure: format:%s (%dx%d)", __func__, streamCounts,
             CameraUtils::pixelCode2String(streamConf.format), streamConf.width, streamConf.height);

        CameraStream* stream = nullptr;
            // Create a normal CameraStream
            stream = new CameraStream(mCameraId, streamId, streamConf);

        stream->registerListener(EVENT_FRAME_AVAILABLE, mRequestThread);
        stream->registerListener(EVENT_FRAME_AVAILABLE, this);
        mStreams[streamId] = stream;
        mStreamNum++;

        LOG2("@%s: automation checkpoint: interlaced: %d", __func__, streamConf.field);
    }

    return OK;
}

/**
 * 1. Checking if the streams is supported or not
 * 2. According resolution and format to store the streamId in descending order.
 */
int CameraDevice::analyzeStream(const stream_config_t* streamList, int* inputRawStreamId,
                                int* preStreamIdForFace, int* inputYuvStreamId) {
    LOG1("<id%d>@%s", mCameraId, __func__);

    mSortedStreamIds.clear();
    int opaqueRawStreamId = -1;

    for (int i = 0; i < streamList->num_streams; i++) {
        stream_t& stream = streamList->streams[i];
        stream.id = i;
        stream.max_buffers = PlatformData::getMaxPipelineDepth(mCameraId);

        if (stream.streamType == CAMERA_STREAM_INPUT) {
            CheckAndLogError(*inputRawStreamId >= 0, BAD_VALUE, "Don't support two INPUT streams!");
            if ((stream.usage == CAMERA_STREAM_PREVIEW) ||
                (stream.usage == CAMERA_STREAM_VIDEO_CAPTURE)) {
                *inputYuvStreamId = i;
            } else {
                *inputRawStreamId = i;
            }
            continue;
        }

        if ((stream.usage == CAMERA_STREAM_OPAQUE_RAW) &&
            (stream.streamType != CAMERA_STREAM_INPUT)) {
            CheckAndLogError(opaqueRawStreamId >= 0, BAD_VALUE, "Don't support two RAW streams!");
            opaqueRawStreamId = i;
            continue;
        }

        if ((stream.usage == CAMERA_STREAM_PREVIEW) && (stream.format != V4L2_PIX_FMT_JPEG)) {
            *preStreamIdForFace = i;
        }

        if (mDataContext->cropRegion.flag == 0) {
            bool valid = PlatformData::isSupportedStream(mCameraId, stream);
            CheckAndLogError(!valid, BAD_VALUE, "Stream config is not supported. format:%s (%dx%d)",
                             CameraUtils::pixelCode2String(stream.format), stream.width,
                             stream.height);
        }

        bool saved = false;
        // Store the streamId in descending order.
        for (size_t j = 0U; j < mSortedStreamIds.size(); j++) {
            stream_t& tmp = streamList->streams[mSortedStreamIds[j]];
            if (stream.width * stream.height > tmp.width * tmp.height) {
                mSortedStreamIds.insert(std::next(mSortedStreamIds.begin(), j), i);
                saved = true;
                break;
            }
        }
        if (!saved) {
            mSortedStreamIds.push_back(i);
        }
    }

    // Set opaque RAW stream as last one
    if (opaqueRawStreamId >= 0) {
        mSortedStreamIds.push_back(opaqueRawStreamId);
    }

    // Don't create stream for face if it is not supported
    if (!PlatformData::isFaceDetectionSupported(mCameraId)) {
        *preStreamIdForFace = -1;
    }

    return OK;
}

int CameraDevice::assignPortForStreams(const stream_config_t* streamList, int inputRawStreamId,
                                       int inputYuvStreamId, int configuredStreamNum) {
    mStreamIdToPortMap.clear();

    for (size_t i = 0U; i < mSortedStreamIds.size(); i++) {
        /*
         * If PSYS Processor isn't used, treat as ISYS only case.
         * Then set port to INPUT_STREAM_PORT_UID directly.
         */
        if (PlatformData::isUsePSysProcessor(mCameraId)) {
            mStreamIdToPortMap[mSortedStreamIds[i]] = USER_STREAM_PORT_UID(i);
        } else {
            mStreamIdToPortMap[mSortedStreamIds[i]] = INPUT_STREAM_PORT_UID(i);
        }

        // Dump the stream info by descending order.
        const stream_t& stream = streamList->streams[mSortedStreamIds[i]];
        LOG1("%s  streamId: %d, %dx%d(%s)", __func__, mSortedStreamIds[i], stream.width,
             stream.height, CameraUtils::format2string(stream.format).c_str());
    }

    // This indicates IPU output for face detection
    if (configuredStreamNum > streamList->num_streams) {
        mSortedStreamIds.push_back(mFdStream.id);
        mStreamIdToPortMap[mFdStream.id] = USER_STREAM_PORT_UID(mFdStream.id);
    }

    // Handle input stream
    if (inputRawStreamId >= 0) {
        CheckAndLogError(mSortedStreamIds.empty(), BAD_VALUE, "There is no output stream!");
        // Push input stream index to the end of vector mSortedStreamIds
        mSortedStreamIds.push_back(inputRawStreamId);
        // Use MAIN PORT for input stream
        mStreamIdToPortMap[inputRawStreamId] = MAIN_INPUT_PORT_UID;
    }

    if (inputYuvStreamId >= 0) {
        CheckAndLogError(mSortedStreamIds.empty(), BAD_VALUE, "There is no output stream!");
        // Use YUV reprocessing port for input stream
        mStreamIdToPortMap[inputYuvStreamId] = YUV_REPROCESSING_INPUT_PORT_ID;
    }

    return OK;
}

/**
 * Bind all streams to their producers and to the correct port.
 *
 * Bind the streams to uuid in resolution descending order:
 * Stream with max resolution            --> MAIN_PORT
 * Stream with intermediate resolution   --> SECOND_PORT
 * Stream with min resolution            --> THIRD_PORT
 */
int CameraDevice::bindStreams(stream_config_t* streamList) {
    for (auto& iter : mStreamIdToPortMap) {
        mStreams[iter.first]->setPort(iter.second);
        if (mProcessingUnit != nullptr) {
            mStreams[iter.first]->setBufferProducer(mProcessingUnit);
        } else {
            mStreams[iter.first]->setBufferProducer(mProducer);
        }
    }

    return OK;
}

int CameraDevice::start() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s, mState:%d", mCameraId, __func__, mState);

    // Not protected by mDeviceLock because it is required in qbufL()
    mRequestThread->wait1stRequestDone();

    AutoMutex m(mDeviceLock);
    CheckAndLogError(mState != DEVICE_BUFFER_READY, BAD_VALUE, "start camera in wrong status %d",
                     mState);
    CheckAndLogError(mStreamNum == 0, BAD_VALUE, "@%s: device doesn't add any stream yet.",
                     __func__);

    mScheduler->start();
    const int ret = startLocked();
    if (ret != OK) {
        LOGE("Camera device starts failed.");
        (void)stopLocked();  // There is error happened, stop all related units.
        return INVALID_OPERATION;
    }

    mState = DEVICE_START;
    return OK;
}

int CameraDevice::stop() {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s, mState:%d", mCameraId, __func__, mState);
    AutoMutex m(mDeviceLock);

    mRequestThread->clearRequests();

    m3AControl->stop();
    mLensCtrl->stop();

    if (mState == DEVICE_START) {
        (void)stopLocked();
    }

    mScheduler->stop();
    mState = DEVICE_STOP;

    return OK;
}

// No Lock for this function as it doesn't update any class member
int CameraDevice::allocateMemory(camera_buffer_t* ubuffer) {
    PERF_CAMERA_ATRACE();
    LOG1("<id%d>@%s", mCameraId, __func__);
    CheckAndLogError(mState < DEVICE_CONFIGURE, BAD_VALUE, "@%s: Wrong state id %d", __func__,
                     mState);
    CheckAndLogError((ubuffer->s.id < 0) || (ubuffer->s.id >= mStreamNum), BAD_VALUE,
                     "@%s: Wrong stream id %d", __func__, ubuffer->s.id);

    const int ret = mStreams[ubuffer->s.id]->allocateMemory(ubuffer);
    CheckAndLogError(ret < 0, ret, "@%s: failed, index: %d", __func__, ubuffer->index);

    return ret;
}

/**
 * Delegate it to RequestThread, make RequestThread manage all buffer related actions.
 */
int CameraDevice::dqbuf(int streamId, camera_buffer_t** ubuffer) {
    CheckAndLogError((streamId < 0) || (streamId > mStreamNum), BAD_VALUE,
                     "@%s: the given stream(%d) is invalid.", __func__, streamId);
    PERF_CAMERA_ATRACE();
    LOG2("<id%d>@%s, stream id:%d", mCameraId, __func__, streamId);

    int ret;
    do {
        ret = mRequestThread->waitFrame(streamId, ubuffer);
    } while (ret == TIMED_OUT);

    if (ret == NO_INIT) {
        return ret;
    }

    CheckAndLogError(((*ubuffer) == nullptr) || (ret != OK),
                     ret, "failed to get ubuffer from stream %d", streamId);

    return ret;
}

int CameraDevice::handleQueueBuffer(int bufferNum, camera_buffer_t** ubuffer, int64_t sequence) {
    LOG2("<id%d:seq%ld>@%s", mCameraId, sequence, __func__);
    CheckAndLogError(mState < DEVICE_CONFIGURE, BAD_VALUE, "@%s: Wrong state id %d", __func__,
                     mState);

    // All streams need to be queued with either a real buffer from user or an empty buffer.
    for (int streamId = 0; streamId < mStreamNum; streamId++) {
        CheckAndLogError(mStreams[streamId] == nullptr, BAD_VALUE, "@%s: stream %d is nullptr",
                         __func__, streamId);

        bool isBufferQueued = false;
        bool hasInput = false, hasYuvOutput = false;
        // Find if user has queued a buffer for mStreams[streamId]
        for (int bufferId = 0; bufferId < bufferNum; bufferId++) {
            camera_buffer_t* buffer = ubuffer[bufferId];
            const int streamIdInBuf = buffer->s.id;
            CheckAndLogError((streamIdInBuf < 0) || (streamIdInBuf > mStreamNum), BAD_VALUE,
                             "@%s: Wrong stream id %d", __func__, streamIdInBuf);

            if (IS_INPUT_BUFFER(buffer->timestamp, buffer->sequence)) {
                hasInput = true;
            }
            if ((buffer->s.usage == CAMERA_STREAM_PREVIEW) ||
                (buffer->s.usage == CAMERA_STREAM_VIDEO_CAPTURE)) {
                hasYuvOutput = true;
            }

            if (streamIdInBuf == streamId) {
                const int ret = mStreams[streamId]->qbuf(buffer, sequence);
                CheckAndLogError(ret < 0, ret, "@%s: queue buffer:%p failed:%d", __func__, buffer,
                                 ret);
                isBufferQueued = true;
                break;
            }
        }

        // If streamId is not found in buffers queued by user, then we need to queue
        // an empty buffer to keep the BufferQueue run.
        if (!isBufferQueued) {
            int ret = OK;
            if (streamId == mFdStream.id) {
                // Use private CameraStream to run face detection for non-reprocessing case
                // and there is one YUV output stream at least
                const bool addExtraBuf = ((!hasInput) && hasYuvOutput) ? true : false;
                ret = mStreams[streamId]->qbuf(nullptr, sequence, addExtraBuf);
            } else {
                ret = mStreams[streamId]->qbuf(nullptr, sequence);
            }
            CheckAndLogError(ret < 0, ret, "@%s: queue empty buffer failed:%d", __func__, ret);
        }
    }

    return OK;
}

int CameraDevice::qbuf(camera_buffer_t** ubuffer, int bufferNum) {
    PERF_CAMERA_ATRACE();
    LOG2("<id%d>@%s", mCameraId, __func__);

    {
        AutoMutex m(mDeviceLock);
        if ((mState == DEVICE_CONFIGURE) || (mState == DEVICE_STOP)) {
            // Start 3A here then the HAL can run 3A for request
            mLensCtrl->start();
            const int ret = m3AControl->start();
            CheckAndLogError((ret != OK), BAD_VALUE, "Start 3a unit failed with ret:%d.", ret);

            mState = DEVICE_BUFFER_READY;
        }
    }

    return mRequestThread->processRequest(bufferNum, ubuffer);
}

int CameraDevice::setParameters(const DataContext& dataContext) {
    PERF_CAMERA_ATRACE();
    LOG2("<id%d>@%s", mCameraId, __func__);
    AutoMutex m(mDeviceLock);
    int ret = OK;

    // Set test pattern mode
    const camera_test_pattern_mode_t testPatternMode = dataContext.mAiqParams.testPatternMode;
    if (PlatformData::isTestPatternSupported(mCameraId)) {
        const int32_t sensorTestPattern =
            PlatformData::getSensorTestPattern(mCameraId, testPatternMode);
        if (sensorTestPattern >= 0) {
            ret = mSensorCtrl->setTestPatternMode(sensorTestPattern);
        }
    }

    *mDataContext = dataContext;

    return ret;
}

// Private Functions, these functions are called with device lock hold

// Destroy all the streams
void CameraDevice::deleteStreams() {
    PERF_CAMERA_ATRACE();
    LOG2("<id%d>%s, streams:%d", mCameraId, __func__, mStreamNum);

    for (int streamId = 0; streamId < mStreamNum; streamId++) {
        mStreams[streamId]->stop();
        delete mStreams[streamId];
        mStreams[streamId] = nullptr;
    }
    mStreamNum = 0;
}

// Internal start without lock hold
int CameraDevice::startLocked() {
    int ret = OK;

    bindListeners();

    // Start all the streams
    for (int i = 0; i < mStreamNum; i++) {
        ret = mStreams[i]->start();
        CheckAndLogError(ret < 0, BAD_VALUE, "Start stream %d failed with ret:%d.", i, ret);
    }

    if (mProcessingUnit != nullptr) {
        ret = mProcessingUnit->start();
        CheckAndLogError((ret < 0), BAD_VALUE, "Start image processor failed with ret:%d.", ret);
    }

    // Start the CaptureUnit for streamon
    ret = mProducer->start();
    CheckAndLogError((ret < 0), BAD_VALUE, "Start capture unit failed with ret:%d.", ret);

    // CSI_META_S
    ret = mCsiMetaDevice->start();
    CheckAndLogError((ret != OK), BAD_VALUE, "Start CSI meta failed with ret:%d.", ret);
    // CSI_META_E

    ret = mSofSource->start();
    CheckAndLogError((ret != OK), BAD_VALUE, "Start SOF event source failed with ret:%d.", ret);

    return OK;
}

// Internal stop without lock hold
int CameraDevice::stopLocked() {
    PERF_CAMERA_ATRACE();
    LOG2("<id%d>%s", mCameraId, __func__);

    mSofSource->stop();

    // CSI_META_S
    mCsiMetaDevice->stop();
    // CSI_META_E

    // Stop the CaptureUnit for streamon
    mProducer->stop();

    if (mProcessingUnit != nullptr) {
        mProcessingUnit->stop();
    }

    unbindListeners();

    return OK;
}

void CameraDevice::handleEvent(EventData eventData) {
    LOG2("%s, event type:%d", __func__, eventData.type);

    switch (eventData.type) {
        case EVENT_PROCESS_REQUEST: {
            const EventRequestData& request = eventData.data.request;
            if (!IS_INPUT_BUFFER(request.buffer[0]->timestamp, request.buffer[0]->sequence)) {
                auto cameraContext = CameraContext::getInstance(mCameraId);
                auto dataContext = cameraContext->getDataContextBySeq(request.settingSeq);
                // Set test pattern mode
                if (PlatformData::isTestPatternSupported(mCameraId)) {
                    const int32_t sensorTestPattern =
                        PlatformData::getSensorTestPattern(mCameraId,
                                                           dataContext->mAiqParams.testPatternMode);
                    if (sensorTestPattern >= 0) {
                        if (mSensorCtrl->setTestPatternMode(sensorTestPattern) < 0) {
                            LOGE("%s, set test Pattern Mode failed", __func__);
                        }
                    }
                }
            }

            (void)handleQueueBuffer(request.bufferNum, request.buffer, request.settingSeq);
            break;
        }
        case EVENT_PSYS_REQUEST_BUF_READY: {
            if (mCallback != nullptr) {
                camera_msg_data_t data = {CAMERA_ISP_BUF_READY, {}};
                const int64_t sequence = eventData.data.requestReady.sequence;

                data.data.buffer_ready.timestamp = eventData.data.requestReady.timestamp;
                data.data.buffer_ready.frameNumber = eventData.data.requestReady.frameNumber;
                PlatformData::updateMakernoteTimeStamp(mCameraId, sequence,
                                                       data.data.buffer_ready.timestamp);
                mCallback->notify(mCallback, data);
            }
            break;
        }

        case EVENT_REQUEST_METADATA_READY: {
            if (mCallback != nullptr) {
                camera_msg_data_t data = {CAMERA_METADATA_READY, {}};

                data.data.metadata_ready.sequence = eventData.data.requestReady.sequence;
                data.data.metadata_ready.frameNumber = eventData.data.requestReady.frameNumber;
                mCallback->notify(mCallback, data);
            }
            break;
        }

        case EVENT_FRAME_AVAILABLE: {
            if (mCallback != nullptr) {
                camera_msg_data_t data = {CAMERA_FRAME_DONE, };

                data.data.frame_ready.streamId = eventData.data.frameDone.streamId;
                mCallback->notify(mCallback, data);
            }
            break;
        }

        default:
            LOGE("Not supported event type:%d", eventData.type);
            break;
    }
}

}  // namespace icamera
