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

#pragma once
#ifdef IPA_SANDBOXING
#include "CcaClient.h"
#else
#include "modules/algowrapper/IntelCca.h"
#endif

#include "CameraEvent.h"

#include "AiqSetting.h"
#include "AiqEngine.h"

namespace icamera {

class SensorHwCtrl;
class LensHw;

/*
 * \class AiqUnit
 * This class is used for upper layer to control 3a engine.
 */

class AiqUnitBase{

public:
    AiqUnitBase() {}
    virtual ~AiqUnitBase() {}

    virtual void init() {}
    virtual void deinit() {}
    virtual int configure(const stream_config_t * /*streamList*/) { return OK; }
    virtual int start() { return OK; }
    virtual void stop() {}
    virtual int run3A(int64_t ccaId, int64_t applyingSeq, int64_t frameNumber,
                      int64_t * /*effectSeq*/)  { return OK; }

    virtual std::vector<EventListener*> getSofEventListener()
    {
        std::vector<EventListener*> eventListenerList;
        return eventListenerList;
    }
    virtual std::vector<EventListener*> getStatsEventListener()
    {
        std::vector<EventListener*> eventListenerList;
        return eventListenerList;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(AiqUnitBase);

};

class AiqUnit : public AiqUnitBase {

public:
    AiqUnit(int cameraId, SensorHwCtrl *sensorHw, LensHw *lensHw);
    ~AiqUnit();

    /**
     * \brief Init 3a related objects
     */
    virtual void init();

    /**
     * \brief Deinit 3a related objects
     */
    virtual void deinit();

    /**
     * \brief configure 3a engine with stream configuration
     */
    virtual int configure(const stream_config_t *streamList);

    /**
     * \brief Start 3a Engine
     */
    virtual int start();

    /**
     * \brief Stop 3a Engine
     */
    virtual void stop();

    /**
     * \brief Run 3a to get new 3a settings.
     *
     * ccaId: unique cca id set by RequestThread;
     * applyingSeq: sequence id indicates which SOF sequence to set the settings,
     *             -1 means no target sequence to set the settings;
     * frameNumber: frame number set in request;
     * effectSeq: sequence id is an output parameter and indicates the settings is taken effect
     *            on the frame.
     *
     * Return 0 if the operation succeeds.
     */
    virtual int run3A(int64_t ccaId, int64_t applyingSeq, int64_t frameNumber, int64_t* effectSeq);

    /**
     * \brief Get software EventListener
     */
    virtual std::vector<EventListener*> getSofEventListener();

    /**
     * \brief Get stats EventListener
     */
    virtual std::vector<EventListener*> getStatsEventListener();

private:
    DISALLOW_COPY_AND_ASSIGN(AiqUnit);

private:
    int initIntelCcaHandle(const std::vector<ConfigMode> &configModes);
    void deinitIntelCcaHandle();
    void dumpCcaInitParam(const cca::cca_init_params& params);

private:
    int mCameraId;
    enum AiqUnitState {
        AIQ_UNIT_NOT_INIT = 0,
        AIQ_UNIT_INIT,
        AIQ_UNIT_CONFIGURED,
        AIQ_UNIT_START,
        AIQ_UNIT_STOP,
        AIQ_UNIT_MAX
    } mAiqUnitState;

    // The operation mode of the streams
    uint32_t mOperationMode;

    AiqEngine *mAiqEngine;

    // Guard for AiqUnit public API.
    Mutex mAiqUnitLock;

    std::vector<TuningMode> mTuningModes;
    bool mCcaInitialized;
    size_t mActiveStreamCount;
};

} /* namespace icamera */

