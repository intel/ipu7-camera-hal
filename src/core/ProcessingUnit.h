/*
 * Copyright (C) 2017-2025 Intel Corporation.
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
#include <queue>
#include <set>

#include "CameraScheduler.h"
#include "IProcessingUnit.h"
#include "IspSettings.h"
#include "CameraContext.h"
#include "iutils/RWLock.h"
#include "processingUnit/IPipeManager.h"

namespace icamera {

class IPipeManager;
class AiqResult;

typedef std::map<uuid, std::shared_ptr<CameraBuffer>> CameraBufferPortMap;

/**
 * ProcessingUnit runs the Image Process Algorithm in the PSYS.
 * It implements the BufferConsumer and BufferProducer Interface
 */
class ProcessingUnit : public IProcessingUnit, public PipeManagerCallback {
 public:
    ProcessingUnit(int cameraId, std::shared_ptr<CameraScheduler> scheduler);
    virtual ~ProcessingUnit();
    virtual int configure(const std::map<uuid, stream_t>& inputInfo,
                          const std::map<uuid, stream_t>& outputInfo, const ConfigMode configModes);
    virtual int start();
    virtual void stop();

    // Overwrite PipeManagerCallback API, used for returning back buffers from PipeManager.
    virtual void onTaskDone(const PipeTaskData& result);
    virtual void onBufferDone(int64_t sequence, uuid port,
                              const std::shared_ptr<CameraBuffer>& camBuffer);
    virtual void onMetadataReady(int64_t sequence, const CameraBufferPortMap& outBuf);
    virtual void onStatsReady(EventData& eventData);

 private:
    DISALLOW_COPY_AND_ASSIGN(ProcessingUnit);

 private:
    virtual int processNewFrame();
    std::shared_ptr<CameraBuffer> allocStatsBuffer(int index);

    status_t prepareTask(CameraBufferPortMap* srcBuffers, CameraBufferPortMap* dstBuffers);
    void dispatchTask(CameraBufferPortMap& inBuf, CameraBufferPortMap& outBuf,
                      bool fakeTask = false, bool callbackRgbs = false);
    int setParameters(const DataContext* dataContext);

    int64_t getSettingSequence(const CameraBufferPortMap& outBuf) const;
    bool needSkipOutputFrame(int64_t sequence);
    bool needExecutePipe(int64_t settingSequence, int64_t inputSequence) const;
    bool needHoldOnInputFrame(int64_t settingSequence, int64_t inputSequence) const;

    void outputRawImage(std::shared_ptr<CameraBuffer>& srcBuf,
                        std::shared_ptr<CameraBuffer>& dstBuf);
    status_t handleYuvReprocessing(const CameraBufferPortMap* buffersMap);
    void handleRawReprocessing(CameraBufferPortMap* srcBuffers, CameraBufferPortMap* dstBuffers,
                               bool* allBufDone, bool* hasRawOutput, bool* hasRawInput);
    bool isBufferHoldForRawReprocess(int64_t sequence);
    void saveRawBuffer(CameraBufferPortMap* srcBuffers);
    void returnRawBuffer();
    void sendPsysFrameDoneEvent(const CameraBufferPortMap* dstBuffers);
    void sendPsysRequestEvent(const CameraBufferPortMap* dstBuffers, int64_t sequence,
                              uint64_t timestamp, EventType eventType);

    status_t getTnrTriggerInfo();
    int getTnrFrameCount(const AiqResult* aiqResult);
    void handleExtraTasksForTnr(int64_t sequence, CameraBufferPortMap* dstBuffers,
                                const AiqResult* aiqResult);

    void extractZslInfo(const CameraBufferPortMap* dstBuffers, bool& reprocess,
                        CameraBufferPortMap& videoBuf, CameraBufferPortMap& stillBuf,
                        int64_t& zslSequence);
    void handleZslReprocessing(int64_t sequence, const CameraBufferPortMap& videoBuf,
                               CameraBufferPortMap stillBuf, bool& allBufDone,
                               CameraBufferPortMap* dstBuffers);
 private:
    int mCameraId;
    static const nsecs_t kWaitDuration = 1000000000;  // 1000ms
    static const nsecs_t kQueueTimeout = 66000000; // 66ms

    IspSettings mIspSettings;
    RWLock mIspSettingsLock;

    // Since the isp settings may be re-used in all modes, so the buffer size of
    // isp settings should be equal to frame buffer size.
    static const int IA_PAL_CONTROL_BUFFER_SIZE = 10;

    // Save the sequences which are being processed.
    std::multiset<int64_t> mSequencesInflight;

    std::unique_ptr<IPipeManager> mPipeManager;
    ConfigMode mConfigMode;
    // Active tuning mode
    TuningMode mTuningMode;

    std::queue<EventDataMeta> mMetaQueue;
    // Guard for the metadata queue
    Mutex mMetaQueueLock;
    std::condition_variable mMetaAvailableSignal;

    uuid mRawPort;

    // variables for opaque raw
    std::set<uuid> mOpaqueRawPorts;
    std::mutex mBufferMapLock;
    std::map<int64_t, CameraBufferPortMap> mRawBufferMap;

    enum { PIPELINE_UNCREATED = 0, PIPELINE_CREATED } mStatus;

    std::shared_ptr<CameraScheduler> mScheduler;

    std::map<uuid, stream_t> mYuvInputInfo;

    tnr7us_trigger_info_t mTnrTriggerInfo;
    // Indicate the latest sequence of raw buffer used in still TNR
    int64_t mLastStillTnrSequence;

    // <stream id, pipe streamId>
    std::map<int32_t, int32_t> mStreamIdToPipeId;
};  // End of class ProcessingUnit

}  // namespace icamera

