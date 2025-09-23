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
#include <map>
#include <list>
#include <mutex>
#include <unordered_map>

#include "modules/ipu_desc/ipu-psys.h"
#include "iutils/Thread.h"

namespace icamera {

class IPSysDeviceCallback {
 public:
    virtual int bufferDone(int64_t sequence) = 0;
};

static const uint8_t INVALID_TERMINAL_ID = 0xFF;
static const uint8_t MAX_NODE_NUM = 5;
static const uint8_t MAX_LINK_NUM = 10;
static const uint8_t MAX_TASK_NUM = 8;
static const uint8_t MAX_TERMINAL_NUM = 26;

struct TerminalConfig {
    uint32_t payloadSize = 0;  // buffer needed for terminal
};

struct PSysBitmapGroup {
    uint32_t teb[2] = {0};
    uint32_t deb[4] = {0};
    uint32_t rbm[4] = {0};
    uint32_t reb[4] = {0};
};

struct PSysNode {
    uint8_t nodeCtxId = 0;
    uint8_t nodeRsrcId = 0;
    PSysBitmapGroup bitmaps;
    // first: terminal id, second: TerminalConfig
    std::unordered_map<uint8_t, TerminalConfig> terminalConfig;
};

struct PSysLink {
    uint8_t srcNodeCtxId = 0;
    uint8_t srcTermId = 0;
    uint8_t dstNodeCtxId = 0;
    uint8_t dstTermId = 0;

    uint8_t streamingMode = 0;
    uint8_t delayedLink = 0;
};

struct PSysGraph {
    std::list<PSysNode> nodes;
    std::list<PSysLink> links;
};

struct TerminalBuffer {
    void* userPtr;
    uint64_t handle;
    uint32_t size;
    uint32_t flags;
    struct ipu_psys_buffer psysBuf;
    bool isExtDmaBuf;
};

struct PSysTask {
    uint8_t nodeCtxId = 0;
    int64_t sequence = 0;
    // first: terminal id, second: TerminalBuffer
    std::unordered_map<uint8_t, TerminalBuffer> terminalBuffers;
};

/**
 * PSysDevice abstracts the PSYS function
 */
class PSysDevice {
 public:
    explicit PSysDevice(int cameraId);
    virtual ~PSysDevice();

    virtual int init();
    virtual void deinit();
    virtual void registerPSysDeviceCallback(uint8_t contextId, IPSysDeviceCallback* callback);

    virtual int addGraph(const PSysGraph& graph);
    virtual int closeGraph();
    virtual int addTask(const PSysTask& task);

    virtual int registerBuffer(TerminalBuffer* buf);
    virtual void unregisterBuffer(const TerminalBuffer* buf);

    virtual int poll();

 private:
    int wait(ipu_psys_event& event);
    int poll(short events, int timeout);
    void handleEvent(const ipu_psys_event& event);
    void updatePsysBufMap(TerminalBuffer* buf);
    void erasePsysBufMap(const TerminalBuffer* buf);
    bool getPsysBufMap(TerminalBuffer* buf);

 private:
    static const int kEventTimeout = 800;
    PollThread<PSysDevice>* mPollThread;
    bool mExitPending;

    // Set once during pipeline setup, and no lock protection
    std::unordered_map<uint8_t, IPSysDeviceCallback*> mPSysDeviceCallbackMap;

    static const uint8_t INVALID_GRAPH_ID = 255;
    static const uint8_t MAX_DRV_FRAME_ID = 255;

    int mCameraId;
    int32_t mFd;
    int mGraphId;

    int32_t mEventFd;

    uint8_t mFrameId[MAX_NODE_NUM];
    std::mutex mDataLock;
    int64_t mFrameIdToSeqMap[MAX_NODE_NUM][MAX_TASK_NUM];

    struct graph_node *mGraphNode;
    struct ipu_psys_term_buffers *mTaskBuffers[MAX_GRAPH_NODES];

    std::unordered_map<int, TerminalBuffer> mFdToTermBufMap;
    std::unordered_map<void*, TerminalBuffer> mPtrToTermBufMap;
};  /* PSysDevice */

} /* namespace icamera */

