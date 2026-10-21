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

#include <memory>
#include <vector>

#include <queue>

#include <libcamera/base/log.h>
#include <libcamera/base/mutex.h>
#include <libcamera/base/thread.h>
#include <libcamera/base/utils.h>

#include <libcamera/camera.h>
#include <libcamera/control_ids.h>
#include <libcamera/formats.h>
#include <libcamera/property_ids.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "libcamera/internal/camera.h"
#include "libcamera/internal/device_enumerator.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/pipeline_handler.h"
#include <libcamera/internal/formats.h>

#include "Frames.h"
#ifdef IPA_SANDBOXING
#include "IPAClient.h"
#include "IGPUIPAClient.h"
#endif
#include "ParameterConverter.h"

// FILE_SOURCE_S
#include "FileSource.h"
// FILE_SOURCE_E

#include "Camera3AMetadata.h"
#include "PrivacyControl.h"
#include "PlatformData.h"
#include "MediaControl.h"
#include "CameraContext.h"
// LIBCAMERA_NONE_UPSTREAM_S
#include "MakerNoteBuilder.h"
// LIBCAMERA_NONE_UPSTREAM_E
#include "AiqUnit.h"
#include "CameraStream.h"
#include "IProcessingUnitFactory.h"
#include "LensHw.h"
#include "RequestThread.h"
#include "SensorHwCtrl.h"
#include "SofSource.h"
#include "StreamSource.h"
// CSI_META_S
#include "CsiMetaDevice.h"
// CSI_META_E
#include "gc/GraphConfigManager.h"
#include "I3AControlFactory.h"
#include "V4l2DeviceFactory.h"
#include "CaptureUnit.h"

namespace icamera::Log {
    void setDebugLevel(void);
}

namespace libcamera {

LOG_DEFINE_CATEGORY(IPU7)

#define UNUSED(param) (void)(param)

using namespace icamera;

class IPU7CameraData : public Camera::Private, public icamera::EventListener {
 public:
    IPU7CameraData(PipelineHandler* pipe, int32_t cameraId);
    ~IPU7CameraData();

    void initializeCapabilities();
    std::vector<SizeRange> availableStreamSizes(const PixelFormat &pixelFormat) const;

    void start();
    void stop();
    bool acquireDevice();
    void releaseDevice();

    void handleNewRequest(Request* request);
    void processNewRequest();

    void returnRequestDone(unsigned int frameNumber);

    // Device operation
    bool validate(const icamera::stream_t& stream);
    int configure(icamera::stream_config_t *streamList);
    void processControls(Request* request, bool isStill = false);
    void updateMetadataResult(int64_t sequence, const ControlList& controls,
                              ControlList& metadata);
    int qbuf(icamera::camera_buffer_t** ubuffer, int bufferNum);
    int dqbuf(int streamId, icamera::camera_buffer_t** ubuffer);

 public:
    int32_t mCameraId;
    /* based on current libcamera design, define Pipeline output stream numbers
    *  to 6, 3 video, 2 still, 1 raw.
    */
    static const int8_t kVideoStreamNum = 3;
    static const int8_t kStillStreamNum = 2;
    static const int8_t kMaxStreamNum = kVideoStreamNum + kStillStreamNum + 1;
    Stream mVideoStreams[kVideoStreamNum];
    Stream mStillStreams[kStillStreamNum];
    Stream mRawStream;

    std::unique_ptr<IPUFrames> mFrameInfo;
    icamera::stream_config_t mStreamList;
    std::map<Stream*, unsigned int> mStreamToStreamIndexMap;

 private:
    void metadataReady(unsigned int frameNumber, int64_t sequence);
    void bufferReady(unsigned int streamId);
    void shutterReady(unsigned int frameNumber, int64_t timestamp);

    void processPrivacySwitch();
    void waitAllRequestsDone();

    int startStream();
    void stopStream();
    int deviceDqbuf(int streamId, camera_buffer_t** ubuffer);
    int deviceQbuf(camera_buffer_t** ubuffer, int bufferNum = 1);
    int deviceConfigure(stream_config_t* streamList);

    StreamSource* createBufferProducer();
    std::map<uuid, stream_t> selectProducerConfig(const stream_config_t* streamList, int mcId);
    bool isProcessorNeeded(const stream_config_t* streamList, const stream_t& producerConfig);
    int analyzeStream(stream_config_t* streamList, int* inputStreamId, int* inputYuvStreamId);
    int assignPortForStreams(const stream_config_t* streamList, int inputStreamId,
                             int inputYuvStreamId, int configuredStreamNum);
    int createStreams(stream_config_t* streamList, int configuredStreamNum);
    int bindStreams(stream_config_t* streamList);
    void deleteStreams();
    void bindListeners();
    void unbindListeners();
    int handleQueueBuffer(int bufferNum, camera_buffer_t** ubuffer, int64_t sequence);
    void handleEvent(EventData eventData);

 private:
    // Pipeline elements
    CameraStream* mCameraStream[MAX_STREAM_NUMBER];
    std::map<int, uuid> mStreamIdToPortMap;
    std::vector<int> mSortedStreamIds;  // Used to save sorted stream ids with descending order.
    StreamSource* mProducer;

    IProcessingUnit* mProcessingUnit;

    LensHw* mLensCtrl;
    SensorHwCtrl* mSensorCtrl;
    SofSource* mSofSource;
    AiqUnitBase* m3AControl;
    // CSI_META_S
    CsiMetaDevice* mCsiMetaDevice;
    // CSI_META_E
    // Internal used variable
    int mStreamNum;
    bool mPerframeControlSupport;

    RequestThread* mRequestThread;
    class ResultHandler : public Object {
     public:
        ResultHandler(IPU7CameraData* cameraData) : mCameraData(cameraData) {}
        ~ResultHandler() {}

        void bufferReady(unsigned int streamId) { mCameraData->bufferReady(streamId); }
        void metadataReady(unsigned int frameNumber, int64_t sequence) {
            mCameraData->metadataReady(frameNumber, sequence);
        }
        void shutterReady(unsigned int frameNumber, int64_t timestamp) {
            mCameraData->shutterReady(frameNumber, timestamp);
        }
     private:
        IPU7CameraData* mCameraData;
    };
    std::unique_ptr<ResultHandler> mResultHandler;
    Thread* mResultThread;

    std::shared_ptr<CameraScheduler> mScheduler;

 private:
    icamera::stream_t mStreams[kMaxStreamNum];

    mutable Mutex mMutex;
    std::queue<Request*> mPendingRequests;

    Camera3AMetadata* mCamera3AMetadata;
    bool mCameraStarted;
    std::map<int32_t, int32_t> mRequestInProgress;
    std::condition_variable mRequestCondition;
    std::mutex mLock;

    icamera::stream_config_t mStreamConfig;
    std::unique_ptr<PrivacyControl> mPrivacyControl;
    bool mPrivacyStarted;
    // LIBCAMERA_NONE_UPSTREAM_S
    std::unique_ptr<MakerNoteBuilder> mMakerNoteBuilder;
    // LIBCAMERA_NONE_UPSTREAM_E
};

class IPU7CameraConfiguration : public CameraConfiguration {
 public:
    IPU7CameraConfiguration(IPU7CameraData* data);

    Status validate() override;

 private:
    const unsigned int kMaxBufferCount = 6;
    IPU7CameraData* mData;
};

class PipelineHandlerIPU7 : public PipelineHandler {
 public:
    PipelineHandlerIPU7(CameraManager* manager);
    virtual ~PipelineHandlerIPU7();

    std::unique_ptr<CameraConfiguration>
    generateConfiguration(Camera* camera, Span<const StreamRole> roles) override;
    int configure(Camera* camera, CameraConfiguration* config) override;

    int exportFrameBuffers(Camera* camera, Stream* stream,
                           std::vector<std::unique_ptr<FrameBuffer>>* buffers) override;

    int start(Camera* camera, const ControlList* controls) override;
    void stopDevice(Camera* camera) override;

    int queueRequestDevice(Camera* camera, Request* request) override;

    bool match(DeviceEnumerator* enumerator) override;
    bool acquireDevice(Camera *camera) override;
    void releaseDevice(Camera *camera) override;

 private:
    IPU7CameraData *cameraData(Camera* camera) {
        return static_cast<IPU7CameraData*>(camera->_d());
    }

    int registerCameras();
    void setup();
    void tearDown();
 private:
    // hal setup done and cameras registered
    bool mHalInitialized;
};

IPU7CameraData::IPU7CameraData(PipelineHandler* pipe, int32_t cameraId)
    : Camera::Private(pipe),
      mCameraId(cameraId),
      mProcessingUnit(nullptr),
      mStreamNum(0),
      mScheduler(nullptr),
      mCameraStarted(false),
      mPrivacyStarted(false) {
    LOG(IPU7, Info) << "<id" << mCameraId << ">@" << __func__;

    icamera::MediaControl* mc = icamera::MediaControl::getInstance();
    if (!mc) {
        LOG(IPU7, Error) << "MediaControl init failed";
        return;
    }
    mc->resetAllLinks();

    bool zslEnable = PlatformData::isHALZslSupported(mCameraId);
    mFrameInfo = std::make_unique<IPUFrames>(zslEnable);

    V4l2DeviceFactory::createDeviceFactory(mCameraId);
    mProducer = createBufferProducer();
    mSofSource = new SofSource(mCameraId);
    // CSI_META_S
    mCsiMetaDevice = new CsiMetaDevice(mCameraId);
    // CSI_META_E
    mPerframeControlSupport = PlatformData::isFeatureSupported(mCameraId, PER_FRAME_CONTROL);

    mLensCtrl = new LensHw(mCameraId);
    mSensorCtrl = SensorHwCtrl::createSensorCtrl(mCameraId);

    m3AControl = I3AControlFactory::createI3AControl(mCameraId, mSensorCtrl, mLensCtrl);
    mRequestThread = new RequestThread(mCameraId, m3AControl);

    mScheduler = std::make_shared<CameraScheduler>(mCameraId);

    mCamera3AMetadata = new Camera3AMetadata(mCameraId);
    mPrivacyControl = std::make_unique<PrivacyControl>(mCameraId);
    mPrivacyStarted = mPrivacyControl->getPrivacyMode();

    // LIBCAMERA_NONE_UPSTREAM_S
    mMakerNoteBuilder = std::make_unique<MakerNoteBuilder>();
    // LIBCAMERA_NONE_UPSTREAM_E

    mResultThread = new Thread();
    mResultHandler = std::make_unique<ResultHandler>(this);
    mResultHandler->moveToThread(mResultThread);
}

IPU7CameraData::~IPU7CameraData() {
    LOG(IPU7, Info) << "<id" << mCameraId << ">@" << __func__;

    delete mResultThread;
    // Clear the media control when close the device.
    MediaControl* mc = MediaControl::getInstance();
    MediaCtlConf* mediaCtl = PlatformData::getMediaCtlConf(mCameraId);
    if (mc && mediaCtl) {
        mc->mediaCtlClear(mCameraId, mediaCtl);
    }

    for (int i = 0; i < MAX_STREAM_NUMBER; i++) delete mCameraStream[i];

    delete mLensCtrl;
    delete m3AControl;
    delete mSensorCtrl;
    delete mSofSource;
    // CSI_META_S
    delete mCsiMetaDevice;
    // CSI_META_E
    delete mProducer;
    delete mRequestThread;

    V4l2DeviceFactory::releaseDeviceFactory(mCameraId);

    if (mCamera3AMetadata) delete mCamera3AMetadata;
    mPrivacyControl = nullptr;

    icamera::CameraContext::releaseInstance(mCameraId);
}

void IPU7CameraData::initializeCapabilities() {
    ParameterConverter::initProperties(mCameraId, properties_);
    ControlInfoMap::Map controls{};
    ParameterConverter::initializeCapabilities(mCameraId, properties_, controls);

    controlInfo_ = ControlInfoMap(std::move(controls), controls::controls);
}

void IPU7CameraData::start() {
    bindListeners();
    mResultThread->start();
    mRequestThread->requestStart();
    mScheduler->start();
}

void IPU7CameraData::stop() {
    // mCameraStarted is accessed only in capture thread, no need to lock
    if (!mCameraStarted) return;

    mCameraStarted = false;
    if (mPrivacyStarted)
        mPrivacyControl->stop();
    else
        stopStream();

    mScheduler->stop();
    mRequestThread->requestStop();
    mResultThread->exit();
    mResultThread->wait();
    unbindListeners();
}

bool IPU7CameraData::acquireDevice() {
    memset(&mStreamList, 0, sizeof(mStreamList));
    mStreamList.streams = mStreams;
    CLEAR(mCameraStream);
    mStreamNum = 0;
    if (mProducer->init() < 0) {
        LOG(IPU7, Error) << "Init capture unit failed";
        return false;
    }

    if (mSofSource->init() != icamera::OK) {
        LOG(IPU7, Error) << "Init sync manager failed";
        return false;
    }
    // CSI_META_S
    if (mCsiMetaDevice->init() != icamera::OK) {
        LOG(IPU7, Error) << "Init csi meta device failed";
        return false;
    }
    // CSI_META_E

    if (m3AControl->init() != icamera::OK) {
        LOG(IPU7, Error) << "Init 3A Unit falied";
        return false;
    }

    if (mLensCtrl->init() != icamera::OK) {
        LOG(IPU7, Error) << "Init Lens falied";
        return false;
    }

    return true;
}

void IPU7CameraData::releaseDevice() {
    deleteStreams();
    if (mProcessingUnit) {
        delete mProcessingUnit;
        mProcessingUnit = nullptr;
    }

    m3AControl->deinit();
    mSofSource->deinit();
    // CSI_META_S
    mCsiMetaDevice->deinit();
    // CSI_META_E
    mProducer->deinit();
}

void IPU7CameraData::handleNewRequest(Request* request) {
    processPrivacySwitch();
    {
    MutexLocker locker(mMutex);

    mPendingRequests.push(request);
    }

    processNewRequest();
}

void IPU7CameraData::processNewRequest() {
    MutexLocker locker(mMutex);

    if (mPendingRequests.empty()) return;

    Request* request = mPendingRequests.front();
    Info* info = mFrameInfo->create(request);
    if (!info) {
        LOG(IPU7, Warning) << "Failed to create Info";
        return;
    }

    for (int i = 0; i < kStillStreamNum; i++) {
        if (request->findBuffer(&mStillStreams[i]) != nullptr) {
            info->isStill = true;
            break;
        }
    }
    processControls(request, info->isStill);

    icamera::camera_buffer_t* halBuffer[kMaxStreamNum] = {nullptr};
    int8_t bufferNum = 0;
    for (auto const &buffer : request->buffers()) {
        LOG(IPU7, Debug) << " request stream " << buffer.first;
        Stream* usrStream = const_cast<Stream*>(buffer.first);
        unsigned int id = mStreamToStreamIndexMap[usrStream];
        icamera::stream_t halStream = mStreamList.streams[id];

        bool status = mFrameInfo->getBuffer(info, halStream, buffer.second,
                                            &info->halBuffer[bufferNum]);
        if (!status) {
            LOG(IPU7, Error) << "Failed to get buffer id " << id;
            mFrameInfo->recycle(info);
            return;
        }
        halBuffer[bufferNum] = &info->halBuffer[bufferNum];
        bufferNum++;
    }

    int ret = qbuf(halBuffer, bufferNum);
    if (ret != 0) {
        LOG(IPU7, Error) << "Failed to queue buffers";
        mFrameInfo->recycle(info);
        return;
    }

    mPendingRequests.pop();

    LOG(IPU7, Debug) << " request processing " << info->id;
}

void IPU7CameraData::returnRequestDone(unsigned int frameNumber) {
    Info* info = mFrameInfo->requestComplete(frameNumber);
    if (!info) return;

    mFrameInfo->recycle(info);

    pipe()->completeRequest(info->request);

    LOG(IPU7, Debug) << " request done " << info->id << " frameNumber " << frameNumber;
    processNewRequest();
}

void IPU7CameraData::metadataReady(unsigned int frameNumber, int64_t sequence) {
    Info* info = mFrameInfo->find(frameNumber);
    ControlList metadata;
    if (info) {
        updateMetadataResult(sequence, info->request->controls(), metadata);
        pipe()->completeMetadata(info->request, metadata);
    }

    mFrameInfo->metadataReady(frameNumber, sequence, metadata);

    returnRequestDone(frameNumber);
}

void IPU7CameraData::bufferReady(unsigned int streamId) {
    icamera::camera_buffer_t* buffer = nullptr;

    int ret = dqbuf(streamId, &buffer);
    if (ret != 0) {
        LOG(IPU7, Error) << "Failed to dequeue buffer";
        return;
    }

    Info* info = mFrameInfo->find(buffer->frameNumber);
    if (info && info->outBuffers.find(streamId) != info->outBuffers.end()) {
        FrameBuffer* frameBuffer = info->outBuffers[streamId];
        pipe()->completeBuffer(info->request, frameBuffer);

        mFrameInfo->bufferReady(buffer->frameNumber, streamId);

        returnRequestDone(buffer->frameNumber);
    }
}

void IPU7CameraData::shutterReady(unsigned int frameNumber, int64_t timestamp) {
    Info* info = mFrameInfo->find(frameNumber);
    ControlList metadata;
    if (info) {
        // LIBCAMERA_NONE_UPSTREAM_S
        if (info->isStill) {
            mMakerNoteBuilder->buildMakerNoteMedata(mCameraId, timestamp, metadata);
        }
        // LIBCAMERA_NONE_UPSTREAM_E
        metadata.set(controls::SensorTimestamp, timestamp);
        pipe()->completeMetadata(info->request, metadata);
    }

    mFrameInfo->shutterReady(frameNumber, timestamp);
}

bool IPU7CameraData::validate(const icamera::stream_t& stream) {
    return PlatformData::isSupportedStream(mCameraId, stream);
}

std::vector<SizeRange> IPU7CameraData::availableStreamSizes(const PixelFormat &pixelFormat) const {
    auto staticMetadata = PlatformData::getStaticMetadata(mCameraId);
    std::vector<SizeRange> sizes;
    if (staticMetadata && staticMetadata->mConfigsArray.size() > 0) {
        // currently only NV12 format output is supported, SGRBG10 not supported in config file
        int streamFormat =
            pixelFormat == formats::SGRBG10 ? V4L2_PIX_FMT_SGRBG10 : V4L2_PIX_FMT_NV12;
        for (auto& stream : staticMetadata->mConfigsArray) {
            if (stream.format == streamFormat) {
                // size is sorted in descending order in config file
                sizes.emplace_back(Size(stream.width, stream.height));
            }
        }
    }

    return sizes;
}

int IPU7CameraData::configure(icamera::stream_config_t *streamList) {

    mPrivacyControl->configure(streamList);
    mStreamConfig = *streamList;

    return deviceConfigure(streamList);
}

void IPU7CameraData::processControls(Request* request, bool isStill) {
    if (!request || mPrivacyStarted) return;

    auto cameraContext = icamera::CameraContext::getInstance(mCameraId);
    icamera::DataContext* dataContext = cameraContext->acquireDataContext();

    cameraContext->updateDataContextMapByFn(request->sequence(), dataContext);

    dataContext->mAiqParams.makernoteMode = icamera::MAKERNOTE_MODE_OFF;
    if (isStill) {
        dataContext->mAiqParams.frameUsage = icamera::FRAME_USAGE_STILL;
        dataContext->mAiqParams.makernoteMode = icamera::MAKERNOTE_MODE_JPEG;
    } else {
        dataContext->mAiqParams.frameUsage = icamera::FRAME_USAGE_PREVIEW;
    }

    ParameterConverter::controls2DataContext(mCameraId, request->controls(), dataContext);
}

void IPU7CameraData::updateMetadataResult(int64_t sequence, const ControlList& controls,
                                          ControlList& metadata) {
    if (mPrivacyStarted) return mPrivacyControl->updateMetadataResult(metadata);

    auto cameraContext = icamera::CameraContext::getInstance(mCameraId);
    auto dataContext = cameraContext->getDataContextBySeq(sequence);
    auto resultStorage = cameraContext->getAiqResultStorage();
    const AiqResult* aiqResult = resultStorage->getAiqResult(sequence);
    const auto faceResult = resultStorage->getFaceResult();
    mCamera3AMetadata->process3Astate(aiqResult, dataContext, controls, metadata);

    ParameterConverter::dataContext2Controls(mCameraId, dataContext, faceResult, aiqResult,
                                             metadata);
}

int IPU7CameraData::qbuf(icamera::camera_buffer_t** ubuffer, int bufferNum) {
    {
        std::unique_lock<std::mutex> lock(mLock);
        for (int i = 0; i < bufferNum; i++) {
            int streamId = ubuffer[i]->s.id;
            mRequestInProgress[streamId]++;
        }
    }

    int ret = OK;
    if (mPrivacyStarted) ret = mPrivacyControl->qbuf(ubuffer, bufferNum);
    else ret = deviceQbuf(ubuffer, bufferNum);
    // Start camera after the first buffer queued
    if (!mCameraStarted) {
        if (mPrivacyStarted)
            mPrivacyControl->start();
        else
            startStream();

        mCameraStarted = true;
    }

    return ret;
}

int IPU7CameraData::dqbuf(int streamId, icamera::camera_buffer_t** ubuffer) {
    int ret = OK;
    if (mPrivacyStarted) ret = mPrivacyControl->dqbuf(streamId, ubuffer);
    else ret = deviceDqbuf(streamId, ubuffer);

    {
        std::unique_lock<std::mutex> lock(mLock);
        mRequestInProgress[streamId]--;
        mRequestCondition.notify_one();
    }

    return ret;
}

void IPU7CameraData::waitAllRequestsDone() {
    std::unique_lock<std::mutex> lock(mLock);
    for ( auto& it : mRequestInProgress) {
        while (it.second > 0) {
            auto duration = std::chrono::duration<float, std::ratio<2 / 1>>(10);
            mRequestCondition.wait_for(lock, duration);
        }
    }
}

void IPU7CameraData::processPrivacySwitch() {
    bool needSwitch = mPrivacyStarted != mPrivacyControl->getPrivacyMode();
    if (!needSwitch) return;
    LOG(IPU7, Debug) << __func__;
    waitAllRequestsDone();

    if (mPrivacyStarted) {
        mPrivacyControl->stop();
    } else {
        stopStream();
        deviceConfigure(&mStreamConfig);
    }

    mCameraStarted = false;
    /* mPrivacyStarted only written here, at this time, the qbuf/start/stop is blocked, and all
    ** requests dqbuf are done. locked is not needed
    */
    mPrivacyStarted = !mPrivacyStarted;
}

StreamSource* IPU7CameraData::createBufferProducer() {
    // FILE_SOURCE_S
    if (PlatformData::isFileSourceEnabled()) {
        return new FileSource(mCameraId);
    }
    // FILE_SOURCE_E

    return new CaptureUnit(mCameraId);
}

void IPU7CameraData::bindListeners() {
    mRequestThread->registerListener(EVENT_PROCESS_REQUEST, this);
    if (mProcessingUnit) {
        std::vector<EventListener*> statsListenerList = m3AControl->getStatsEventListener();
        for (auto statsListener : statsListenerList) {
            // Subscribe PSys statistics.
            mProcessingUnit->registerListener(EVENT_PSYS_STATS_BUF_READY, statsListener);
            mProcessingUnit->registerListener(EVENT_PSYS_STATS_SIS_BUF_READY, statsListener);
        }
        // CSI_META_S
        if (mCsiMetaDevice->isEnabled()) {
            mCsiMetaDevice->registerListener(EVENT_META, mProcessingUnit);
        }
        // CSI_META_E
        mProcessingUnit->registerListener(EVENT_PSYS_STATS_BUF_READY, mRequestThread);
        mProcessingUnit->registerListener(EVENT_PSYS_REQUEST_BUF_READY, this);
        mProcessingUnit->registerListener(EVENT_REQUEST_METADATA_READY, this);
    }

    std::vector<EventListener*> sofListenerList = m3AControl->getSofEventListener();
    for (auto sofListener : sofListenerList) {
        mSofSource->registerListener(EVENT_ISYS_SOF, sofListener);
        // FILE_SOURCE_S
        if (PlatformData::isFileSourceEnabled()) {
            // File source needs to produce SOF event as well when it's enabled.
            mProducer->registerListener(EVENT_ISYS_SOF, sofListener);
        }
        // FILE_SOURCE_E
    }

    if (mPerframeControlSupport || !PlatformData::isIsysEnabled(mCameraId)) {
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
    mPrivacyControl->frameEvents.connect(this, &IPU7CameraData::handleEvent);
}

void IPU7CameraData::unbindListeners() {
    mRequestThread->removeListener(EVENT_PROCESS_REQUEST, this);
    if (mProcessingUnit) {
        std::vector<EventListener*> statsListenerList = m3AControl->getStatsEventListener();
        for (auto statsListener : statsListenerList) {
            mProcessingUnit->removeListener(EVENT_PSYS_STATS_BUF_READY, statsListener);
            mProcessingUnit->removeListener(EVENT_PSYS_STATS_SIS_BUF_READY, statsListener);
        }
        // CSI_META_S
        if (mCsiMetaDevice->isEnabled()) {
            mCsiMetaDevice->removeListener(EVENT_META, mProcessingUnit);
        }
        // CSI_META_E
        mProcessingUnit->removeListener(EVENT_PSYS_STATS_BUF_READY, mRequestThread);
        mProcessingUnit->removeListener(EVENT_PSYS_REQUEST_BUF_READY, this);
        mProcessingUnit->removeListener(EVENT_REQUEST_METADATA_READY, this);
    }
    std::vector<EventListener*> sofListenerList = m3AControl->getSofEventListener();
    for (auto sofListener : sofListenerList) {
        mSofSource->removeListener(EVENT_ISYS_SOF, sofListener);
        // FILE_SOURCE_S
        if (PlatformData::isFileSourceEnabled()) {
            mProducer->removeListener(EVENT_ISYS_SOF, sofListener);
        }
        // FILE_SOURCE_E
    }

    if (mPerframeControlSupport || !PlatformData::isIsysEnabled(mCameraId)) {
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
    mPrivacyControl->frameEvents.disconnect(this, &IPU7CameraData::handleEvent);
}

int IPU7CameraData::deviceConfigure(stream_config_t* streamList) {
    // Release the resource created last time
    deleteStreams();
    if (mProcessingUnit) delete mProcessingUnit;
    mProcessingUnit = nullptr;
    mProducer->removeAllFrameAvailableListener();

    /*
     * The configure flow for CameraStream
     * 1. Analyze all the streams
     * 2. Config the graph
     * 3. Assign port for each stream
     * 4. Create the CameraStream classes
     * 5. Create the procesor
     * 6. Bind the CameraStream to processor
     */
    int inputRawStreamId = -1, inputYuvStreamId = -1;
    if (analyzeStream(streamList, &inputRawStreamId, &inputYuvStreamId) < 0) {
        LOG(IPU7, Error) << "analyzeStream failed";
        return icamera::BAD_VALUE;
    }

    int mcId = -1;
    int totalStream = 0;

    std::unique_ptr<GraphConfigManager> gcMgr = std::make_unique<GraphConfigManager>(mCameraId);
    if (gcMgr != nullptr) {
        std::map<ConfigMode, std::shared_ptr<GraphConfig> > gcs;
        // Clear old gcs
        CameraContext::getInstance(mCameraId)->storeGraphConfig(gcs);
        totalStream = gcMgr->configStreams(streamList, gcs, nullptr);
        LOG(IPU7, Debug) << "Total stream number: " << totalStream;
        CameraContext::getInstance(mCameraId)->storeGraphConfig(gcs);
        mcId = gcMgr->getSelectedMcId();
    }

    // Config the H-Scheduler based on graph id
    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);

    std::shared_ptr<GraphConfig> gc =
            CameraContext::getInstance(mCameraId)->getGraphConfig(configModes[0]);
    if (!gc) {
        LOG(IPU7, Error) << "Failed to get GraphConfig!";
        return icamera::BAD_VALUE;
    }
    int graphId = gc->getGraphId();
    if (!mScheduler || mScheduler->configurate(graphId) < 0) {
        LOG(IPU7, Error) << "Faield to configure H-Scheduler";
        return icamera::BAD_VALUE;
    }

    if (assignPortForStreams(streamList, inputRawStreamId, inputYuvStreamId, totalStream) < 0) {
        LOG(IPU7, Error) << "Faield to assign port for streams";
        return icamera::BAD_VALUE;
    }

    if (createStreams(streamList, totalStream) < 0) {
        LOG(IPU7, Error) << "Faield to create streams";
        return icamera::BAD_VALUE;
    }
    mRequestThread->configure(streamList);

    std::map<uuid, stream_t> producerConfigs = selectProducerConfig(streamList, mcId);
    if (producerConfigs.empty()) {
        LOG(IPU7, Error) << "The config for producer is invalid";
        return icamera::BAD_VALUE;
    }

    bool needProcessor = isProcessorNeeded(streamList, producerConfigs[MAIN_INPUT_PORT_UID]);
    for (auto& item : producerConfigs) {
        if (needProcessor) {
            item.second.memType = V4L2_MEMORY_MMAP;
        }
    }

    if (mProducer->configure(producerConfigs, configModes) < 0) {
        LOG(IPU7, Error) << "Device Configure failed";
        return icamera::BAD_VALUE;
    }

    // CSI_META_S
    if (mCsiMetaDevice->configure() < 0) {
        LOG(IPU7, Error) << "Failed to configure CSI meta device";
        return icamera::BAD_VALUE;
    }
    // CSI_META_E

    if (mSofSource->configure() < 0) {
        LOG(IPU7, Error) << "Failed to configure SOF source device";
        return icamera::BAD_VALUE;
    }

    m3AControl->configure(streamList);

    if (needProcessor) {
        mProcessingUnit = IProcessingUnitFactory::createIProcessingUnit(mCameraId, mScheduler);
        if (!mProcessingUnit) {
            LOG(IPU7, Error) << "Failed to create ProcessingUnit";
            return icamera::BAD_VALUE;
        }

        if (mProcessingUnit) {
            std::map<uuid, stream_t> outputConfigs;
            for (const auto& item : mStreamIdToPortMap) {
                outputConfigs[item.second] = streamList->streams[item.first];
            }
            if (mProcessingUnit->configure(producerConfigs, outputConfigs, configModes[0]) < 0) {
                LOG(IPU7, Error) << "Failed to configure ProcessingUnit";
                return icamera::BAD_VALUE;
            }
            mProcessingUnit->setBufferProducer(mProducer);
        }
    }

    if (bindStreams(streamList) < 0) {
        LOG(IPU7, Error) << "Failed to bind streams";
        return icamera::BAD_VALUE;
    }

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
std::map<uuid, stream_t> IPU7CameraData::selectProducerConfig(const stream_config_t* streamList,
                                                              int mcId) {
    std::map<uuid, stream_t> producerConfigs;
    if (!PlatformData::isIsysEnabled(mCameraId)) {
        // Input stream id is the last one of mSortedStreamIds
        const stream_t& tmp = streamList->streams[mSortedStreamIds.back()];
        if (tmp.streamType == CAMERA_STREAM_INPUT) {
            producerConfigs[MAIN_INPUT_PORT_UID] = tmp;
            LOG(IPU7, Debug) << "producer is user input stream";
            return producerConfigs;
        }
    }

    stream_t biggestStream = streamList->streams[mSortedStreamIds[0]];
    // Use CSI output to select MC config
    std::vector<ConfigMode> configModes;
    PlatformData::getConfigModesByOperationMode(mCameraId, streamList->operation_mode, configModes);
    stream_t matchedStream = biggestStream;
    std::shared_ptr<GraphConfig> gc =
        CameraContext::getInstance(mCameraId)->getGraphConfig(configModes[0]);
    if (!configModes.empty() && gc) {
        camera_resolution_t csiOutput = {0, 0};
        gc->getCSIOutputResolution(csiOutput);
        if (csiOutput.width > 0 && csiOutput.height > 0) {
            matchedStream.width = csiOutput.width;
            matchedStream.height = csiOutput.height;
        }
    }
    PlatformData::selectMcConf(mCameraId, matchedStream, (ConfigMode)streamList->operation_mode,
                               mcId);

    // Select the output format.
    PlatformData::selectISysFormat(mCameraId, biggestStream.format);

    // Use the ISYS output if it's provided in media config section of config file.
    stream_t mainConfig = PlatformData::getISysOutputByPort(mCameraId, MAIN_INPUT_PORT_UID);
    mainConfig.memType = biggestStream.memType;
    mainConfig.field = biggestStream.field;

    if (mainConfig.width != 0 && mainConfig.height != 0) {
        producerConfigs[MAIN_INPUT_PORT_UID] = mainConfig;
        LOG(IPU7, Debug) << "mcId" << mcId << " select the biggest stream";
        return producerConfigs;
    }

    camera_resolution_t producerRes = PlatformData::getISysBestResolution(
        mCameraId, biggestStream.width, biggestStream.height, biggestStream.field);

    // Update the height according to the field(interlaced).
    mainConfig.format = PlatformData::getISysFormat(mCameraId);
    mainConfig.width = producerRes.width;
    mainConfig.height = CameraUtils::getInterlaceHeight(mainConfig.field, producerRes.height);

    // configuration with main port
    producerConfigs[MAIN_INPUT_PORT_UID] = mainConfig;

    return producerConfigs;
}

/**
 * Check if post processor is needed.
 * The processor is needed when:
 * 1. At least one of the given streams does not match with the producer's output.
 * 2. To support specific features such as HW weaving or dewarping.
 */
bool IPU7CameraData::isProcessorNeeded(const stream_config_t* streamList,
                                       const stream_t& producerConfig) {
    if (producerConfig.field != V4L2_FIELD_ALTERNATE) {
        int streamCounts = streamList->num_streams;
        for (int streamId = 0; streamId < streamCounts; streamId++) {
            if (producerConfig.width != streamList->streams[streamId].width ||
                producerConfig.height != streamList->streams[streamId].height ||
                producerConfig.format != streamList->streams[streamId].format) {
                return true;
            }
        }
    }

    return false;
}

int IPU7CameraData::createStreams(stream_config_t* streamList, int configuredStreamNum) {
    int streamCounts = streamList->num_streams;
    for (int streamId = 0; streamId < streamCounts; streamId++) {
        stream_t& streamConf = streamList->streams[streamId];
        LOG(IPU7, Debug) << "stream_number: " << streamCounts << "stream configure: format: " <<
            streamConf.width << "x" << streamConf.height;

        CameraStream* stream = new CameraStream(mCameraId, streamId, streamConf);
        stream->registerListener(EVENT_FRAME_AVAILABLE, mRequestThread);
        stream->registerListener(EVENT_FRAME_AVAILABLE, this);
        mCameraStream[streamId] = stream;
        mStreamNum++;
        LOG(IPU7, Debug) << "automation checkpoint: interlaced: " << streamConf.field;
    }

    return OK;
}

/**
 * 1. Checking if the streams is supported or not
 * 2. According resolution and format to store the streamId in descending order.
 */
int IPU7CameraData::analyzeStream(stream_config_t* streamList, int* inputRawStreamId,
                                  int* inputYuvStreamId) {
    mSortedStreamIds.clear();
    int opaqueRawStreamId = -1;

    for (int i = 0; i < streamList->num_streams; i++) {
        stream_t& stream = streamList->streams[i];
        stream.id = i;
        stream.max_buffers = PlatformData::getMaxRequestsInflight(mCameraId);

        if (stream.streamType == CAMERA_STREAM_INPUT) {
            if (*inputRawStreamId >= 0) {
                LOG(IPU7, Error) << "Don't support two INPUT streams!";
                return icamera::BAD_VALUE;
            }
            if (stream.usage == CAMERA_STREAM_PREVIEW ||
                stream.usage == CAMERA_STREAM_VIDEO_CAPTURE) {
                *inputYuvStreamId = i;
            } else {
                *inputRawStreamId = i;
            }
            continue;
        }

        if (stream.usage == CAMERA_STREAM_OPAQUE_RAW && stream.streamType != CAMERA_STREAM_INPUT) {
            if (opaqueRawStreamId >= 0) {
                LOG(IPU7, Error) << "Don't support two RAW streams!";
                return icamera::BAD_VALUE;
            }
            opaqueRawStreamId = i;
            continue;
        }

        bool saved = false;
        // Store the streamId in descending order.
        for (size_t j = 0; j < mSortedStreamIds.size(); j++) {
            stream_t& tmp = streamList->streams[mSortedStreamIds[j]];
            if (stream.width * stream.height > tmp.width * tmp.height) {
                mSortedStreamIds.insert((mSortedStreamIds.begin() + j), i);
                saved = true;
                break;
            }
        }
        if (!saved) mSortedStreamIds.push_back(i);
    }

    // Set opaque RAW stream as last one
    if (opaqueRawStreamId >= 0) {
        mSortedStreamIds.push_back(opaqueRawStreamId);
    }

    return OK;
}

int IPU7CameraData::assignPortForStreams(const stream_config_t* streamList, int inputRawStreamId,
                                       int inputYuvStreamId, int configuredStreamNum) {
    mStreamIdToPortMap.clear();

    for (size_t i = 0; i < mSortedStreamIds.size(); i++) {
        mStreamIdToPortMap[mSortedStreamIds[i]] = USER_STREAM_PORT_UID(i);
    }

    // Handle input stream
    if (inputRawStreamId >= 0) {
        if (mSortedStreamIds.empty()) {
            LOG(IPU7, Error) << "There is no output stream!!";
            return icamera::BAD_VALUE;
        }
        // Push input stream index to the end of vector mSortedStreamIds
        mSortedStreamIds.push_back(inputRawStreamId);
        // Use MAIN PORT for input stream
        mStreamIdToPortMap[inputRawStreamId] = MAIN_INPUT_PORT_UID;
    }

    if (inputYuvStreamId >= 0) {
        if (mSortedStreamIds.empty()) {
            LOG(IPU7, Error) << "There is no output stream!!";
            return icamera::BAD_VALUE;
        }
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
int IPU7CameraData::bindStreams(stream_config_t* streamList) {
    for (auto& iter : mStreamIdToPortMap) {
        mCameraStream[iter.first]->setPort(iter.second);
        if (mProcessingUnit)
            mCameraStream[iter.first]->setBufferProducer(mProcessingUnit);
        else
            mCameraStream[iter.first]->setBufferProducer(mProducer);
    }

    return OK;
}

int IPU7CameraData::startStream() {
    mRequestThread->wait1stRequestDone();

    for (int i = 0; i < mStreamNum; i++) {
        if(mCameraStream[i]->start() < 0) {
            LOG(IPU7, Error) << "Start stream: " << i <<  " failed";
            return icamera::BAD_VALUE;
        }
    }

    if (!mProcessingUnit || mProcessingUnit->start() < 0) {
        LOG(IPU7, Error) << "Start image processor failed";
        return icamera::BAD_VALUE;
    }

    if (mProducer->start() < 0) {
        LOG(IPU7, Error) << "Start capture unit failed";
        return icamera::BAD_VALUE;
    }

    // CSI_META_S
    if (mCsiMetaDevice->start() < 0) {
        LOG(IPU7, Error) << "Start CSI meta failed";
        return icamera::BAD_VALUE;
    }
    // CSI_META_E

    if (mSofSource->start() < 0) {
        LOG(IPU7, Error) << "Start SOF event source failed";
        return icamera::BAD_VALUE;
    }

    return OK;
}

void IPU7CameraData::stopStream() {
    mRequestThread->clearRequests();
    mSofSource->stop();
    m3AControl->stop();

    // CSI_META_S
    mCsiMetaDevice->stop();
    // CSI_META_E

    mProducer->stop();
    if (mProcessingUnit) mProcessingUnit->stop();
}

/**
 * Delegate it to RequestThread, make RequestThread manage all buffer related actions.
 */
int IPU7CameraData::deviceDqbuf(int streamId, camera_buffer_t** ubuffer) {
    int ret = mRequestThread->waitFrame(streamId, ubuffer);
    while (ret == TIMED_OUT) ret = mRequestThread->waitFrame(streamId, ubuffer);

    if (ret == icamera::NO_INIT) return ret;
    if (!*ubuffer || ret != OK) {
        LOG(IPU7, Error) << "failed to get ubuffer from stream " << streamId;
        return ret;
    }

    return ret;
}

int IPU7CameraData::handleQueueBuffer(int bufferNum, camera_buffer_t** ubuffer, int64_t sequence) {
    // All streams need to be queued with either a real buffer from user or an empty buffer.
    for (int streamId = 0; streamId < mStreamNum; streamId++) {
        if (mCameraStream[streamId] == nullptr) {
            LOG(IPU7, Error) << "Stream " << streamId << " is nullptr";
            return icamera::BAD_VALUE;
        }
        bool isBufferQueued = false;
        // Find if user has queued a buffer for mCameraStream[streamId]
        for (int bufferId = 0; bufferId < bufferNum; bufferId++) {
            camera_buffer_t* buffer = ubuffer[bufferId];
            int streamIdInBuf = buffer->s.id;
            if (streamIdInBuf < 0 || streamIdInBuf > mStreamNum) {
                LOG(IPU7, Error) << "Wrong stream id: " << streamIdInBuf;
                return icamera::BAD_VALUE;
            }

            if (streamIdInBuf == streamId) {
                if (mCameraStream[streamId]->qbuf(buffer, sequence) < 0) {
                    LOG(IPU7, Error) << "Queue buffer failed: ";
                    return icamera::BAD_VALUE;
                }
                isBufferQueued = true;
                break;
            }
        }

        // If streamId is not found in buffers queued by user, then we need to queue
        // an empty buffer to keep the BufferQueue run.
        if (!isBufferQueued && mCameraStream[streamId]->qbuf(nullptr, sequence) < 0) {
            LOG(IPU7, Error) << "Queue empty buffer failed: ";
            return icamera::BAD_VALUE;
        }
    }

    return OK;
}

int IPU7CameraData::deviceQbuf(camera_buffer_t** ubuffer, int bufferNum) {
    // Start 3A before the 1st buffer queued
    if (!mCameraStarted && m3AControl->start() < 0) {
        LOG(IPU7, Error) << "Start 3a unit failed";
        return icamera::NO_INIT;
    }
    return mRequestThread->processRequest(bufferNum, ubuffer);
}

// Destroy all the streams
void IPU7CameraData::deleteStreams() {
    for (int streamId = 0; streamId < mStreamNum; streamId++) {
        mCameraStream[streamId]->stop();
        delete mCameraStream[streamId];
        mCameraStream[streamId] = nullptr;
    }
    mStreamNum = 0;
}

void IPU7CameraData::handleEvent(EventData eventData) {
    switch (eventData.type) {
        case EVENT_PROCESS_REQUEST: {
            const EventRequestData& request = eventData.data.request;
            if (!((request.buffer[0]->s.streamType == CAMERA_STREAM_INPUT) &&
                  IS_INPUT_BUFFER(request.buffer[0]->timestamp, request.buffer[0]->sequence))) {
                auto cameraContext = CameraContext::getInstance(mCameraId);
                auto dataContext = cameraContext->getDataContextBySeq(request.settingSeq);
                // Set test pattern mode
                if (PlatformData::isTestPatternSupported(mCameraId)) {
                    int32_t sensorTestPattern =
                        PlatformData::getSensorTestPattern(mCameraId,
                                                           dataContext->mAiqParams.testPatternMode);
                    if (sensorTestPattern >= 0) {
                        if (mSensorCtrl->setTestPatternMode(sensorTestPattern) < 0) {
                            LOG(IPU7, Error) << "Set test Pattern Mode failed";
                        }
                    }
                }
            }

            handleQueueBuffer(request.bufferNum, request.buffer, request.settingSeq);
            break;
        }
        case EVENT_PSYS_REQUEST_BUF_READY: {
            const EventRequestReady& requestReady = eventData.data.requestReady;
            mResultHandler->invokeMethod(&ResultHandler::shutterReady, ConnectionTypeQueued,
                                         requestReady.frameNumber, requestReady.timestamp);
            break;
        }

        case EVENT_REQUEST_METADATA_READY: {
            const EventRequestReady& requestReady = eventData.data.requestReady;
            mResultHandler->invokeMethod(&ResultHandler::metadataReady, ConnectionTypeQueued,
                                         requestReady.frameNumber, requestReady.sequence);
            break;
        }

        case EVENT_FRAME_AVAILABLE: {
            mResultHandler->invokeMethod(&ResultHandler::bufferReady, ConnectionTypeQueued,
                                         eventData.data.frameDone.streamId);
            break;
        }

        default:
            LOG(IPU7, Error) << "Not supported event type: " << eventData.type;
            break;
    }
}

IPU7CameraConfiguration::IPU7CameraConfiguration(IPU7CameraData* data)
    : CameraConfiguration() {
    LOG(IPU7, Info) << "IPU7CameraConfiguration()";
    mData = data;
}

CameraConfiguration::Status IPU7CameraConfiguration::validate() {
    LOG(IPU7, Info) << "IPU7CameraConfiguration::validate()";
    if (config_.empty() || config_.size() > IPU7CameraData::kMaxStreamNum) return Invalid;

    int videoStreamIndex = 0;
    int stillStreamIndex = 0;
    for (unsigned int i = 0; i < config_.size(); ++i) {
        const StreamConfiguration originalCfg = config_[i];
        StreamConfiguration *cfg = &config_[i];
        switch (cfg->role) {
        case StreamRole::StillCapture:
            if (stillStreamIndex >= IPU7CameraData::kStillStreamNum) return Invalid;
            cfg->setStream(&(mData->mStillStreams[stillStreamIndex]));
            stillStreamIndex++;
            break;
        case StreamRole::VideoRecording:
        case StreamRole::Viewfinder:
            if (videoStreamIndex >= IPU7CameraData::kVideoStreamNum) return Invalid;
            cfg->setStream(&(mData->mVideoStreams[videoStreamIndex]));
            videoStreamIndex++;
            break;
        case StreamRole::Raw:
            cfg->setStream(&(mData->mRawStream));
            break;
        }

        const PixelFormatInfo &info = PixelFormatInfo::info(config_[i].pixelFormat);
        if (info.colourEncoding == PixelFormatInfo::ColourEncodingRAW) {
            cfg->pixelFormat = formats::SGRBG10;
            cfg->stride = info.stride(cfg->size.width, 0, 64);
            cfg->frameSize = info.frameSize(cfg->size, 64);
            // TODO get buffer count from HAL
            cfg->bufferCount = kMaxBufferCount;
        } else {
            cfg->pixelFormat = formats::NV12;
            cfg->bufferCount = kMaxBufferCount;
            cfg->stride = info.stride(cfg->size.width, 0, 1);
            cfg->frameSize = info.frameSize(cfg->size, 1);
        }

        icamera::stream_t stream = {};
        stream.width = cfg->size.width;
        stream.height = cfg->size.height;
        stream.format = cfg->pixelFormat.fourcc();
        stream.field = 0;
        if (!mData->validate(stream)) return Invalid;

        LOG(IPU7, Info) << " stream " << cfg->toString() << " Valid";
    }

    return Valid;
}

PipelineHandlerIPU7::PipelineHandlerIPU7(CameraManager* manager)
    : PipelineHandler(manager),
      mHalInitialized(false) {
    LOG(IPU7, Info) << __FUNCTION__ << " Construct " << this;
}

PipelineHandlerIPU7::~PipelineHandlerIPU7() {
    LOG(IPU7, Info) << __FUNCTION__ << " Destroy " << this;
    if (mHalInitialized) {
#ifdef IPA_SANDBOXING
        IPAClient::removeInstance();
        if (icamera::PlatformData::isUsingGpuIpa()) {
            IGPUIPAClient::removeInstance();
        }
#endif

        tearDown();
    }
}

std::unique_ptr<CameraConfiguration>
PipelineHandlerIPU7::generateConfiguration(Camera* camera, Span<const StreamRole> roles) {
    LOG(IPU7, Info) << "PipelineHandlerIPU7::generateConfiguration()";
    IPU7CameraData *data = cameraData(camera);
    std::unique_ptr<IPU7CameraConfiguration> config =
        std::make_unique<IPU7CameraConfiguration>(data);

    if (roles.empty()) return config;

    for (const StreamRole role : roles) {
        std::map<PixelFormat, std::vector<SizeRange>> streamFormats;
        PixelFormat pixelFormat = (role == StreamRole::Raw) ? formats::SGRBG10 : formats::NV12;
        std::vector<SizeRange> sizes = data->availableStreamSizes(pixelFormat);
        if (sizes.size() == 0) return {};
        streamFormats[pixelFormat] = sizes;
        StreamFormats formats(streamFormats);
        StreamConfiguration cfg(formats);
        // set default size to max size
        cfg.size = sizes.at(0).max;
        cfg.pixelFormat = pixelFormat;
        cfg.role = role;
        config->addConfiguration(cfg);
        LOG(IPU7, Debug) << "generate config " << pixelFormat << " size " << cfg.toString();
    }

    if (config->validate() == CameraConfiguration::Invalid) return {};

    LOG(IPU7, Debug) << "generateConfiguration done, config count " << config->size();
    return config;
}

int PipelineHandlerIPU7::configure(Camera* camera, CameraConfiguration* c) {
    // do not call camera device config durning match() configuration
    // Fix me, needed add PipelineHandler CameraConfiguration check here
    if (!mHalInitialized) return 0;

    IPU7CameraConfiguration *config =
        static_cast<IPU7CameraConfiguration *>(c);
    IPU7CameraData *data = cameraData(camera);
    data->mStreamList.num_streams = config->size();

    for (unsigned int i = 0; i < config->size(); ++i) {
        StreamConfiguration &cfg = config->at(i);
        data->mStreamList.streams[i].width = cfg.size.width;
        data->mStreamList.streams[i].height = cfg.size.height;
        data->mStreamList.streams[i].stride = cfg.stride;
        data->mStreamList.streams[i].size = cfg.frameSize;
        data->mStreamList.streams[i].field = 0;
        data->mStreamList.streams[i].format = cfg.pixelFormat.fourcc();
        data->mStreamList.streams[i].max_buffers = cfg.bufferCount;
        data->mStreamList.streams[i].id = i;
        data->mStreamList.streams[i].streamType = icamera::CAMERA_STREAM_OUTPUT;
        LOG(IPU7, Info) << "configure(): " << cfg.toString() << " role " << cfg.role;

        switch (cfg.role) {
            case StreamRole::StillCapture:
                data->mStreamList.streams[i].usage = icamera::CAMERA_STREAM_STILL_CAPTURE;
                break;
            case StreamRole::VideoRecording:
                data->mStreamList.streams[i].usage = icamera::CAMERA_STREAM_VIDEO_CAPTURE;
                break;
            case StreamRole::Viewfinder:
                data->mStreamList.streams[i].usage = icamera::CAMERA_STREAM_PREVIEW;
                break;
            case StreamRole::Raw:
                data->mStreamList.streams[i].usage = icamera::CAMERA_STREAM_OPAQUE_RAW;
                break;
        }

        data->mStreamList.streams[i].memType = V4L2_MEMORY_USERPTR;

        LOG(IPU7, Debug) << " stream " << cfg.stream() << " size: " << cfg.toString();

        data->mStreamToStreamIndexMap[cfg.stream()] = i;
    }
    if (data->mStreamList.num_streams > 0) data->configure(&data->mStreamList);

    return 0;
}

int PipelineHandlerIPU7::exportFrameBuffers(Camera* camera, Stream* stream,
                                            std::vector<std::unique_ptr<FrameBuffer>> *buffers) {
    LOG(IPU7, Info) << "exportFrameBuffers()";
    UNUSED(camera);
    UNUSED(stream);
    UNUSED(buffers);
    return 0;
}

int PipelineHandlerIPU7::start(Camera* camera, [[maybe_unused]] const ControlList* controls) {
    LOG(IPU7, Info) << "PipelineHandlerIPU7::start()";

    IPU7CameraData *data = cameraData(camera);
    data->start();
    return 0;
}

void PipelineHandlerIPU7::stopDevice(Camera* camera) {
    LOG(IPU7, Info) << "PipelineHandlerIPU7::stopDevice()";

    IPU7CameraData *data = cameraData(camera);
    data->stop();
}

int PipelineHandlerIPU7::queueRequestDevice(Camera* camera, Request* request) {
    LOG(IPU7, Debug) << "PipelineHandlerIPU7::queueRequestDevice()";

    IPU7CameraData *data = cameraData(camera);
    data->handleNewRequest(request);
    return 0;
}

bool PipelineHandlerIPU7::match(DeviceEnumerator* enumerator) {
    LOG(IPU7, Info) << "PipelineHandlerIPU7::match()";

    DeviceMatch isysDm("intel-ipu7");
    MediaDevice* isysDev = acquireMediaDevice(enumerator, isysDm);
    if (!isysDev) return false;

    setup();

#ifdef IPA_SANDBOXING
    IPAClient::createInstance(this);
    bool fine = IPAClient::getInstance()->isIPAFine();
    if (fine) {
        LOG(IPU7, Info) << "IPA connection is successful";
    } else {
        LOG(IPU7, Error) << "IPA connection is not successful";
        return false;
    }

    if (icamera::PlatformData::isUsingGpuIpa()) {
        IGPUIPAClient::createInstance(this);
        fine = IGPUIPAClient::getInstance()->isIPAFine();
        if (fine) {
            LOG(IPU7, Info) << "Connected to GPU IPA";
        } else {
            LOG(IPU7, Error) << "GPU IPA connection failed";
            IGPUIPAClient::removeInstance();
            return false;
        }
    }
#endif

    int ret = registerCameras();
    mHalInitialized = true;

    return ret == 0;
}

int PipelineHandlerIPU7::registerCameras() {
    int numCameras = PlatformData::numberOfCameras();

    for (int cameraId = 0; cameraId < numCameras; cameraId++) {
        const std::string cameraName = std::string("camera") + std::to_string(cameraId);
        std::unique_ptr<IPU7CameraData> data = std::make_unique<IPU7CameraData>(this, cameraId);
        data->initializeCapabilities();

        std::set<Stream*> streams;
        for (int i = 0; i < IPU7CameraData::kVideoStreamNum; i++) {
            streams.insert(&(data->mVideoStreams[i]));
        }
        for (int i = 0; i < IPU7CameraData::kStillStreamNum; i++) {
            streams.insert(&(data->mStillStreams[i]));
        }
        streams.insert(&(data->mRawStream));

        std::shared_ptr<Camera> camera = Camera::create(std::move(data), cameraName, streams);

        registerCamera(std::move(camera));
        LOG(IPU7, Info) << "Registered Camera[" << cameraId << "] \"" << cameraName << "\"";
    }
    return numCameras ? 0 : -ENODEV;
}

bool PipelineHandlerIPU7::acquireDevice(Camera *camera) {
    return cameraData(camera)->acquireDevice();
}

void PipelineHandlerIPU7::releaseDevice(Camera *camera) {
    cameraData(camera)->releaseDevice();
}

void PipelineHandlerIPU7::setup() {
    LOG(IPU7, Info) << __func__;
    icamera::Log::setDebugLevel();
    icamera::CameraDump::setDumpLevel();
    icamera::PlatformData::init();
}

void PipelineHandlerIPU7::tearDown() {
    LOG(IPU7, Info) << __func__;
    icamera::PlatformData::releaseInstance();
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerIPU7)

} /* namespace libcamera */
