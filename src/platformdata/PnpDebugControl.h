/*
 * Copyright (C) 2021-2024 Intel Corporation.
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

/**
 *\File PnpDebugParser.h
 *
 * parser for the pnp control xml configuration file
 *
 * This file calls the libexpat ditectly. The libexpat is one xml parser.
 * It will parse the camera configuration out firstly.
 * Then other module can call the methods of it to get the real configuration.
 */

#pragma once

#include "CameraTypes.h"
#include "JsonParserBase.h"
#include "PlatformData.h"
#include "iutils/Utils.h"

namespace icamera {

/**
 * \class PnpDebugParser
 *
 * This class is used to parse the pnp configuration file.
 * The configuration file is xml format.
 * This class will use the expat lib to do the xml parser.
 */

class PnpDebugControl {
 public:
    /**
     * check if using mock AAL layer for PNP test
     *
     * \return true if feature is enabled, otherwise return false.
     */
    static bool useMockAAL();

    /**
     * the fake fps of mock AAL layer
     *
     * \return fps if set, otherwise return 30.
     */

    static int mockAPPFps();
    /**
     * check if 3A algo is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isBypass3A();

    /**
     * check if run PAL is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isBypassPAC();

    /**
     * check if CB is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isBypassCB();

    /**
     * check if Face Dection Feature is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isFaceDisabled();

    /**
     * check if Face AE Feature is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isFaceAeDisabled();

    /**
     * check if Face Dection Algo is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isBypassFDAlgo();

    /**
     * check if ISys is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isBypassISys();

    /**
     * check if PSys is skipped for PNP test
     *
     * \return true if feature is skipped, otherwise return false.
     */
    static bool isUsingMockPSys();

    /**
     * check if using mock camhal for PNP test
     *
     * \return true if feature is enabled, otherwise return false.
     */
    static bool useMockHal();

    /**
     * check if use mock pipeManager
     *
     * \return true if to use mock pipeManager, otherwise return false.
     */
    static bool useMockPipes();

    static void updateConfig();

    static void releaseInstance();

 public:
    struct StaticCfg {
     public:
        StaticCfg()
                : useMockAAL(false),
                  mockAPPFps(30),
                  isBypass3A(false),
                  isBypassPAC(false),
                  isBypassCB(false),
                  isFaceDisabled(false),
                  isFaceAeDisabled(false),
                  isBypassFDAlgo(false),
                  isBypassISys(false),
                  useMockPSys(false),
                  useMockHal(false),
                  useMockPipes(false) {}
        bool useMockAAL;
        int mockAPPFps;
        bool isBypass3A;
        bool isBypassPAC;
        bool isBypassCB;
        bool isFaceDisabled;
        bool isFaceAeDisabled;
        bool isBypassFDAlgo;
        bool isBypassISys;
        bool useMockPSys;
        bool useMockHal;
        bool useMockPipes;
    };

 private:
    PnpDebugControl();
    ~PnpDebugControl() {}

 private:
    struct StaticCfg mStaticCfg;
    static PnpDebugControl* sInstance;
    static Mutex sLock;
    static PnpDebugControl* getInstance();
};

class PnpDebugParser : public JsonParserBase {
 public:
    explicit PnpDebugParser(PnpDebugControl::StaticCfg* cfg);
    ~PnpDebugParser() {}

    bool run(const std::string& filename) final override;

 private:
    PnpDebugControl::StaticCfg* mStaticCfg;

    // prevent copy constructor and assignment operator
    DISALLOW_COPY_AND_ASSIGN(PnpDebugParser);
};

}  // namespace icamera
