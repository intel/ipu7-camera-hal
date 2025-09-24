/*
 * Copyright (C) 2016-2025 Intel Corporation.
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

#pragma once
#include "AiqUnit.h"
#include "CameraStream.h"
#include "IProcessingUnitFactory.h"
#include "LensHw.h"
#include "RequestThread.h"
#include "SensorHwCtrl.h"
#include "SofSource.h"
#include "StreamSource.h"
#include "CameraContext.h"

// CSI_META_S
#include "CsiMetaDevice.h"
// CSI_META_E

#include "gc/GraphConfigManager.h"

namespace icamera {

class RequestThread;

/**
 * CameraDevice : Create elements to construct the streaming pipeline
 * Each element must be a producer or a consumer or Both.
 *
 * These are the typical pipelines:
 *
 * For a single SOC YUV capture in by pass mode
 * StreamSource -> CameraStream
 *
 * For a single NV12 capture of TPG/Sensor using SwImageProcess
 * StreamSource -> SwImageProcessor -> CameraStream
 *
 * For a single NV12 capture of TPG/Sensor using PSYS processor
 * StreamSource -> PsysProcessor -> CameraStream
 *
 * For a dual streams NV12 video capture of TPG/Sensor using PSYS processor
 * StreamSource -> PsysProcessor | -> CameraStream (For video recording)
 *                              | -> CameraStream (for Preview)
 *
 * For a SDV(Snapshot during video) capture of TPG/Sensor uinsg PSYS processor
 * StreamSource | -> PsysProcessor | -> CameraStream (For video recording)
 *             |                  | -> CameraStream (for Preview)
 *             | -> PsysProcessor | -> CameraStream (For still YUV)
 *
 * The buffer notification between the Class is based on Interface defined
 * in BufferQueue. The upstream element use "onBufferAvailable" message to notfiy
 * downstream elements that the buffers are ready for further processing
 *
 * Following singleton instances are created and maintained by CameraDevice
 * 1. GraphConfigManager
 * 2. AiqResultStorage
 */
class CameraDevice : public EventListener {
 public:
    explicit CameraDevice(int cameraId);
    ~CameraDevice();

    /**
     * \brief Camera device class init
     *
     * 1.Related classes init: StreamSource, SofSource, 3AControl, lensCtrl
     // CSI_META_S
     *                         CsiMeta
     // CSI_META_E
     * 2.Register listener if enable AIQ
     * 3.Set the default parameters
     *
     * \return OK if succeed, other value indicates failed
     */
    int init();

    /**
     * \brief Camera device class deinit
     *
     * 1.Change the state
     * 2.Destroy the listeners
     * 3.Delete the streams
     * 4.Deinit the Related classes.
     */
    void deinit();

    /**
     * \brief Camera device start
     *
     * 1. Start all streams
     * 2. Related classes start
     *
     * \return OK if succeed, other value indicates failed
     */
    int start();

    /**
     * \brief Camera device stop
     *
     * 1. Stop all related class module.
     * 2. Change the state
     */
    int stop();

    /**
     * \brief Allocate memory according to user buffer
     *
     * 1. Convert user buffer to CameraBuffer and push it into UserBufferQueue
     * 2. Calling CameraStream class to allocateMemory.
     *
     * \return OK if succeed, other value indicates failed
     */
    int allocateMemory(camera_buffer_t* ubuffer);

    /**
     * \brief dequeue buffer from cameraStream.
     *
     * 1. Dequeue one CameraBuffer from CameraStream
     * 2. Fill the user buffer base on CameraBuffer
     *
     * \return 0 if succeed, other value indicates failed
     */
    int dqbuf(int streamId, camera_buffer_t** ubuffer);

    /**
     * \brief Queue one buffer to CameraStream
     *
     * 1. Get the CameraBuffer base on the ubuffer
     * 2. Calling CameraStream to queue one CameraBuffer
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int qbuf(camera_buffer_t** ubuffer, int bufferNum = 1);

    /**
     * \brief Configure the device sensor input
     *
     *
     * \param inputConfig: the Output format/resolution of the isys
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int configureInput(const stream_t* inputConfig);

    /**
     * \brief Configure the streams, devices and post processor.
     *
     * Configure the streams according to the streamList info
     * Extra processor is needed if the format isn't supported in Psys output
     *
     * \param streamList: all the streams info
     *
     * \return OK if succeed and BAD_VALUE if failed
     */
    int configure(stream_config_t* streamList);

    /**
     * \brief Set the parameters
     *
     * Merge the param to internal parameters and set them to 3A
     * and processor.
     *
     * \param dataContext: the parameters need to set
     *
     * \return OK if succeed, other value indicates failed
     */
    int setParameters(const DataContext& dataContext);

    void callbackRegister(const camera_callback_ops_t* callback);

 private:
    StreamSource* createBufferProducer();
    std::map<uuid, stream_t> selectProducerConfig(const stream_config_t* streamList, int mcId);
    bool isProcessorNeeded(const stream_config_t* streamList, const stream_t& producerConfig) const;
    int analyzeStream(const stream_config_t* streamList, int* inputRawStreamId,
                      int* preStreamIdForFace, int* inputYuvStreamId);
    int assignPortForStreams(const stream_config_t* streamList, int inputRawStreamId,
                             int inputYuvStreamId, int configuredStreamNum);
    int createStreams(const stream_config_t* streamList, int configuredStreamNum);
    int bindStreams(stream_config_t* streamList);
    void deleteStreams();

    /**
     * Bind all event listeners with their source.
     */
    void bindListeners();

    /**
     * Unbind all event listeners from their source.
     */
    void unbindListeners();

    // The second phase of qbuf(), done in RequestThread
    int handleQueueBuffer(int bufferNum, camera_buffer_t** ubuffer, int64_t sequence);

    virtual void handleEvent(EventData eventData);

    int startLocked();
    int stopLocked();

 private:
    enum {
        DEVICE_UNINIT = 0,
        DEVICE_INIT,
        DEVICE_CONFIGURE,  // means stream configured
        DEVICE_START,
        DEVICE_STOP,
        DEVICE_BUFFER_READY,  // At least one buffer is queued to ISP
    } mState;

    // Guard for CameraDevice public API
    Mutex mDeviceLock;

    // Pipeline elements
    CameraStream* mStreams[MAX_STREAM_NUMBER];
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
    int mCameraId;
    int mStreamNum;
    DataContext* mDataContext;
    bool mPerframeControlSupport;
    GraphConfigManager* mGcMgr;

    RequestThread* mRequestThread;
    stream_t mInputConfig;
    camera_callback_ops_t* mCallback;
    stream_t mFdStream;
    std::shared_ptr<CameraScheduler> mScheduler;

 private:
    DISALLOW_COPY_AND_ASSIGN(CameraDevice);
};

}  // namespace icamera

