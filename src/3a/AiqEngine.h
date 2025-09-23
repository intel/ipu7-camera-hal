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
#include "AiqCore.h"
#include "AiqResult.h"
#include "AiqResultStorage.h"
#include "AiqStatistics.h"
#include "CameraEvent.h"
#include "LensManager.h"
#include "SensorManager.h"

namespace icamera {

/*
 * \class AiqEngine
 * This class is used to parse Stats, control \class AiqEngine
 * This class is used to parse Stats, control running AIQ algorithms
 * and set result to HW layer.
 * This is sub thread class.
 */
class AiqEngine : public EventListener {
 public:
    AiqEngine(int cameraId, SensorHwCtrl* sensorHw, LensHw* lensHw);
    ~AiqEngine();

    /**
     * \brief Init AiqResult, AiqCore and SensorManager
     */
    void init();

    /**
     * \brief Deinit AiqResult, AiqCore and SensorManager
     */
    void deinit();

    /**
     * \brief reset default settings.
     */
    void reset();

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
    int run3A(int64_t ccaId, int64_t applyingSeq, int64_t frameNumber, int64_t* effectSeq);

    /**
     * \brief Get SOF EventListener
     */
    EventListener* getSofEventListener();

    /**
     * \brief handle event
     */
    virtual void handleEvent(EventData eventData);

    int prepareStatsParams(const aiq_parameter_t& aiqParams,
                           cca::cca_stats_params* statsParams, const AiqStatistics* aiqStatistics,
                           AiqResult* aiqResult);

    // Handle AIQ results except Exposure results which are handled in setSensorExposure
    void setAiqResult(const aiq_parameter_t& aiqParams, AiqResult* aiqResult, bool skip);
    void setSensorExposure(AiqResult* aiqResult, int64_t applyingSeq = -1);

    int getSkippingNum(const AiqResult* aiqResult);

    bool needRun3A(const AiqStatistics* aiqStatistics, int64_t ccaId);

    enum AiqState {
        AIQ_STATE_IDLE = 0,
        AIQ_STATE_WAIT,
        AIQ_STATE_INPUT_PREPARE,
        AIQ_STATE_RUN,
        AIQ_STATE_RESULT_SET,
        AIQ_STATE_DONE,
        AIQ_STATE_ERROR,
        AIQ_STATE_MAX
    };

    AiqState prepareInputParam(AiqStatistics* aiqStats, AiqResult* aiqResult,
                               aiq_parameter_t& aiqParams);
    AiqState runAiq(int64_t ccaId, int64_t applyingSeq, AiqResult* aiqResult, bool* aiqRun);
    AiqState handleAiqResult(const aiq_parameter_t& aiqParams, AiqResult* aiqResult);
    AiqState done(const aiq_parameter_t& aiqParams, AiqResult* aiqResult);

    int run();

    // For manual ISP settings
    int applyManualTonemaps(const aiq_parameter_t& aiqParams, AiqResult* aiqResult);

 private:
    static const int kMaxExposureAppliedDelay = 5;

 private:
    int mCameraId;
    AiqResultStorage* mAiqResultStorage;
    AiqCore* mAiqCore;
    SensorManager* mSensorManager;
    LensManager* mLensManager;

    int mRun3ACadence;
    bool mFirstAiqRunning;
    bool mAiqRunningForPerframe;

    // Guard for public API of AiqEngine.
    Mutex mEngineLock;

    struct AiqRunningHistory {
        AiqResult* aiqResult;
        int64_t ccaId;
        int64_t statsSequence;
    };
    AiqRunningHistory mAiqRunningHistory;

 private:
    DISALLOW_COPY_AND_ASSIGN(AiqEngine);
};

} /* namespace icamera */

