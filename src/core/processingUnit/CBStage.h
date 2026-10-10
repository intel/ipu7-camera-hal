/*
 * Copyright (C) 2022-2025 Intel Corporation.
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

#include <cstdio>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>

#include "IPipeStage.h"
#include "IpuPacAdaptor.h"
#include "PSysDevice.h"
#if defined(GRC_IPU7X)
#include "Ipu7xTerminalDescriptorAutogen.h"
#elif defined(GRC_IPU75XA)
#include "Ipu75xaTerminalDescriptorAutogen.h"
#elif defined(GRC_IPU8)
#include "Ipu8TerminalDescriptorAutogen.h"
#else
#include "TerminalDescriptorAutogen.h"
#endif
#include "Utils.h"
#include "cb_payload_descriptor.h"

namespace icamera {

class CBStage : public IPipeStage, public IPSysDeviceCallback {
 public:
    CBStage(int cameraId, int streamId, int stageId, uint8_t contextId, uint8_t psysContextId,
            uint8_t resourceId, const std::string& cbName, PSysDevice* psysDevice,
            IpuPacAdaptor* pacAdapt);
    virtual ~CBStage();

    int init();
    void deInit();
    // config the data terminals, init, config and prepare PAC.
    int configure(const StaticGraphNodeKernels& kernelGroup, const GraphLink** links,
                  uint32_t numOfLink, std::unordered_map<uint8_t, TerminalConfig>& terminalConfig);

    // EventSource
    virtual void registerListener(EventType eventType, EventListener* eventListener);
    virtual void removeListener(EventType eventType, EventListener* eventListener);

    // ISchedulerNode
    virtual bool process(int64_t triggerId);

    virtual int start();
    virtual int stop();

#ifdef LINUX_PRIVACY_MODE
    // Mirrors the reinitAic() decision taken by PipeLine::start() for this stream.
    void setNeedReconfigAic(bool need) { mNeedReconfigAic = need; }
#endif

    virtual void setControl(int64_t sequence, const StageControl& control) {}

    // IPSysDeviceCallback
    virtual int bufferDone(int64_t sequence);

 private:
    struct StageTask {
        std::map<uuid, std::shared_ptr<CameraBuffer>> inBuffers;
        std::map<uuid, std::shared_ptr<CameraBuffer>> outBuffers;
        int64_t sequence;
    };

 private:
    int32_t fetchTask(StageTask* task);
    int32_t processTask(StageTask* task);
    void updateInfoAndSendEvents(StageTask* task);

    // There are 4 types of buffer: frame, node2self, metadata and payload buffer.
    int32_t allocateFrameBuffers();
    int allocateNode2SelfBuffers(const PSysLink& psysLink, uint32_t bufferSize);
    int setTerminalLinkAndAllocNode2SelfBuffers(const GraphLink** links, uint8_t numOfLink);

    int allocMetadataBuffer(const GraphLink** links, uint8_t numOfLink,
                            std::unordered_map<uint8_t, TerminalConfig>& terminalConfig);
    int registerMetadataBuffer(aic::IaAicBuffer** iaAicBuf, PacTerminalBufMap& termBufMap);

    int getKernelOffsetFromTerminalDesc(
        cca::cca_cb_kernel_offset& offsets, uint32_t** offsetPtr,
        std::unordered_map<uint8_t, TerminalConfig>& terminalConfig);
    bool kernelExist(const StaticGraphNodeKernels& kernelGroup, uint32_t kernelUuid);
    int getKernelOffsetFromPayloadDesc(const StaticGraphNodeKernels& kernelGroup,
                                       cca::cca_cb_kernel_offset& offsets);
    int pacConfig(const StaticGraphNodeKernels& kernelGroup, aic::IaAicBuffer** iaAicPtr,
                  std::unordered_map<uint8_t, TerminalConfig>& terminalConfig,
                  PacTerminalBufMap& termBufMap,
                  bool reinit = false);
#ifdef LINUX_PRIVACY_MODE
    int reapplyPacConfig();
#endif
    int allocPayloadBuffer(const cca::cca_aic_terminal_config& pacConfig,
                           std::unordered_map<uint8_t, TerminalConfig>& terminalConfig);
    bool isInPlaceTerminal(uint8_t resourceId, uint8_t terminalId);
    int registerPayloadBuffer(aic::IaAicBuffer** iaAicBuf, PacTerminalBufMap& termBufMap);

    void unregisterExtDmaBuf(int64_t sequence);
    int addFrameTerminals(std::unordered_map<uint8_t, TerminalBuffer>* terminalBuffers,
                          const std::map<uuid, std::shared_ptr<CameraBuffer>>& buffers,
                          int64_t sequence = -1);
    int addTask(std::unordered_map<uint8_t, TerminalBuffer>* terminalBuffers,
                const PacTerminalBufMap& bufferMap, int64_t sequence);
    void dumpTerminalData(const PacTerminalBufMap& bufferMap, int64_t sequence);

 private:
    int32_t mCameraId;
    uint32_t mStreamId;
    uint8_t mOuterNodeCtxId;  // from static graph
    uint8_t mContextId;  // psys context id, start at 0;
    uint8_t mResourceId;
    std::string mCBName;
    PSysDevice* mPSysDevice;
    bool mHasStatsTerminal;
    IpuPacAdaptor* mPacAdapt;

    enum LinkStreamMode {
        LINK_STREAMING_MODE_SOFF = 0,
        LINK_STREAMING_MODE_DOFF = 1,
        LINK_STREAMING_MODE_BCLM = 2,
        LINK_STREAMING_MODE_BCSM = 3,
    };
    uint8_t  mLinkStreamMode;

    std::mutex mDataLock;
    static const uint8_t MAX_FRAME_NUM = 2;
    std::list<StageTask> mStageTaskList;

    // Used to dump all used terminal buffers
    // Ignore (psys) ctx id of consumer or producer because they are invalid
    std::list<PSysLink> mTerminalLink;
    std::map<uuid, std::shared_ptr<CameraBuffer>> mInternalOutputBuffers;
    const payload_descriptor_t** sPayloadDesc;
    uint32_t mPayloadDescCount;
    const TerminalDescriptor* sTerminalDesc;
    uint32_t mTerminalDescCount;
    static const uint32_t kMaxSectionCount = 256;
    uint32_t* mKernelOffsetBuf;
    aic::IaAicBuffer* mIaAicBuf;

    static const uint8_t kMaxNode2SelfBufArray = MAX_BUFFER_COUNT;
    uint8_t mNode2SelfBufIndex;
    /**
     * node2self, example:
     * BB:8 -> BB:11, apply on current frame (buffer chasing)
     * BB:8 -> BB:6, apply on the next frame
     */
    // <(output) terminal buffer, buffers>
    std::map<uint8_t, std::vector<TerminalBuffer> > mNode2SelfBuffers;
    // < output terminal, related links>
    std::map<uint8_t, std::vector<PSysLink> > mNode2SelfLinks;

    struct TerminalBufferInfo {
        // first: terminal id, second: metadata buffer
        std::unordered_map<uint8_t, TerminalBuffer> mMetadataBufferMap;
        // first: terminal id, second: payload buffer
        std::unordered_map<uint8_t, TerminalBuffer> mPayloadBufferMap;
    };

    static const uint8_t kMaxTerminalBufArray = MAX_PAC_BUFFERS;
    // first: index, second: TerminalBufferInfo
    std::unordered_map<uint8_t, TerminalBufferInfo> mTerminalBufferMaps;

    // first: user ptr, second: TerminalBuffer
    std::unordered_map<void*, TerminalBuffer> mUserToTerminalBuffer;

    // first: sequence, second:: TerminalBuffer
    std::unordered_multimap<int64_t, TerminalBuffer> mSeqToTerminalBufferMaps;

#ifdef LINUX_PRIVACY_MODE
    // Frame terminal buffers registered with PSysDevice, keyed by their psys fd.
    // Dropped in stop() because the ISYS buffers are re-mmap'd at the same addresses.
    std::unordered_map<int, TerminalBuffer> mFrameTerminalBuffers;

    // Kept from configure() so reapplyPacConfig() can redo configAic + registerAicBuf.
    const StaticGraphNodeKernels* mKernelGroup = nullptr;
    std::unordered_map<uint8_t, TerminalConfig> mStoredTerminalConfig;
    bool mNeedReconfigAic = false;
    bool mFrameBuffersAllocated = false;
#endif
};

}  // namespace icamera
