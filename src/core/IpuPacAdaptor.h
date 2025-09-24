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

#include <memory>
#include <vector>
#include <list>
#include <map>

#include "iutils/Errors.h"
#include "CameraBuffer.h"
#include "CameraTypes.h"
#include "PlatformData.h"
#include "AiqResultStorage.h"
#include "3a/AiqResult.h"
#include "IspSettings.h"

#ifdef IPA_SANDBOXING
#include "CcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

namespace icamera {

struct PacTerminalBuf {
    size_t size;
    void *payloadPtr;
    PacTerminalBuf() : size(0), payloadPtr(nullptr) {}
};

// terminal id -> PacTerminalBuf
typedef std::map<uint8_t, PacTerminalBuf> PacTerminalBufMap;

class IpuPacAdaptor {
public:
    explicit IpuPacAdaptor(int cameraId);
    virtual ~IpuPacAdaptor() {}

    int init(std::vector<int> streamIds);
    int reinitAic(const int32_t aicId);
    int deinit();
    void clearAicResult();

    status_t pacConfig(int streamId, const cca::cca_aic_config& aicConfig,
                       const cca::cca_aic_kernel_offset& kernelOffset, uint32_t* offsetPtr,
                       cca::cca_aic_terminal_config* termCfg, const int32_t* statsBufToTermIds);
    void* allocateBuffer(int streamId, uint8_t contextId, uint32_t termId, size_t size);
    void releaseBuffer(int streamId, uint8_t contextId, uint32_t termId, void* addr);
    status_t setPacTerminalData(int streamId, uint8_t contextId,
                                const PacTerminalBufMap& bufferMap);
    status_t registerBuffer(int streamId, const cca::cca_aic_terminal_config& termCfg);
    status_t runAIC(const IspSettings* ispSettings, int64_t settingSequence, int32_t streamId);
    status_t updateResolutionSettings(int streamId, const cca::cca_aic_config& aicConfig,
                                      bool isKeyResChanged);

    status_t getAllBuffers(int streamId, uint8_t contextId, int64_t sequenceId,
                           PacTerminalBufMap& bufferMap);
    status_t decodeStats(int streamId, uint8_t contextId, int64_t sequenceId,
                         unsigned long long timestamp);

 private:
    void* allocateBufferL(int streamId, uint8_t contextId, uint32_t termId, size_t size);
    void releaseBufferL(int streamId, uint8_t contextId, uint32_t termId, void* addr);
    status_t storeTerminalResult(int64_t sequence, int32_t streamId);
    void applyMediaFormat(const AiqResult* aiqResult,
                          ia_media_format* mediaFormat, bool* useLinearGamma,
                          int64_t sequence);
    bool isStatsUsed(int streamId, int64_t sequence);
    void dumpAicOutput(int64_t sequence, int streamId, cca::cca_multi_pal_output pacOutput);

    DISALLOW_COPY_AND_ASSIGN(IpuPacAdaptor);

    void dumpPALParams(uint8_t contextId, int64_t sequenceId, ia_binary_data binaryData);

 private:
    typedef std::map<uint8_t, std::vector<PacTerminalBuf> > CBTermBufferVec;
    enum IpuAdaptorState {
        PAC_ADAPTOR_NOT_INIT,
        PAC_ADAPTOR_INIT
    } mPacAdaptorState;

    typedef struct _CBTerminalResult {
        int64_t sequence;
        PacTerminalBufMap termResult;
    } CBTerminalResult;

    int mCameraId;
    int memIndex = 0;

    //Guard for IpuPacAdaptor public API
    Mutex mPacAdaptorLock;
    IntelCca *mIntelCca;
    AiqResultStorage* mAiqResultStorage;

    // Guard lock for payload buffer
    Mutex mIpuParamLock;
    // key: pair<streamId, contextId> -> PacTerminalBufMap
    std::map<std::pair<int, uint8_t>, PacTerminalBufMap> mTerminalData;

    std::map<std::pair<int, uint8_t>, std::vector<CBTerminalResult> > mTerminalResult;
    std::map<int, cca::cca_pal_input_params*> mStreamIdToInputParams;

    int64_t mLastStatsSequence;
    static const uint8_t MAX_CACHE_PAC_HIST = 6;
    // key: pair<streamId, sequence> --> bool (true means stats decoded)
    std::map<std::pair<int, int64_t>, bool> mPacRunHistMap;
};
} // namespace icamera
