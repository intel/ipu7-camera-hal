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
#define LOG_TAG PnpDebugControl

#include "src/platformdata/PnpDebugControl.h"

#include <expat.h>
#include <string.h>

#include "iutils/CameraLog.h"

namespace icamera {
PnpDebugControl* PnpDebugControl::sInstance = nullptr;
Mutex PnpDebugControl::sLock;

PnpDebugControl* PnpDebugControl::getInstance() {
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new PnpDebugControl();
    }

    return sInstance;
}

void PnpDebugControl::releaseInstance() {
    AutoMutex lock(sLock);

    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

void PnpDebugControl::updateConfig() {
    PnpDebugParser PnpDebugParser(&(getInstance()->mStaticCfg));
}

PnpDebugControl::PnpDebugControl() {
    PnpDebugParser PnpDebugParser(&mStaticCfg);
}

bool PnpDebugControl::useMockAAL() {
    return getInstance()->mStaticCfg.useMockAAL;
}

int PnpDebugControl::mockAPPFps() {
    return getInstance()->mStaticCfg.mockAPPFps;
}

bool PnpDebugControl::isBypass3A() {
    return getInstance()->mStaticCfg.isBypass3A;
}

bool PnpDebugControl::isBypassPAC() {
    return getInstance()->mStaticCfg.isBypassPAC;
}

bool PnpDebugControl::isBypassCB() {
    return getInstance()->mStaticCfg.isBypassCB;
}

bool PnpDebugControl::isFaceDisabled() {
    return getInstance()->mStaticCfg.isFaceDisabled;
}

bool PnpDebugControl::isFaceAeDisabled() {
    return getInstance()->mStaticCfg.isFaceDisabled ? true
                                                    : getInstance()->mStaticCfg.isFaceAeDisabled;
}

bool PnpDebugControl::isBypassFDAlgo() {
    return !getInstance()->mStaticCfg.isFaceDisabled && getInstance()->mStaticCfg.isBypassFDAlgo;
}

bool PnpDebugControl::isBypassISys() {
    return getInstance()->mStaticCfg.isBypassISys;
}

bool PnpDebugControl::isUsingMockPSys() {
    return getInstance()->mStaticCfg.useMockPSys;
}

bool PnpDebugControl::useMockHal() {
    return getInstance()->mStaticCfg.useMockHal;
}

bool PnpDebugControl::useMockPipes() {
    return getInstance()->mStaticCfg.useMockPipes;
}

#define PNP_DEBUG_FILE_NAME "pnp_profiles.json"
PnpDebugParser::PnpDebugParser(PnpDebugControl::StaticCfg* cfg)
        : mStaticCfg(cfg) {
    std::string filename(PNP_DEBUG_FILE_NAME);
    std::string fullpath = PlatformData::getCameraCfgPath() + filename;
    bool ret = run(fullpath);

    CheckAndLogError(!ret, VOID_VALUE, "Failed to get policy profiles data from %s",
                     PNP_DEBUG_FILE_NAME);
}

bool PnpDebugParser::run(const std::string& filename) {
    auto root = openJsonFile(filename);
    if (root.empty()) {
        return false;
    }

    if (!root.isMember("PnpDebugConfig")) {
        return false;
    }

    const Json::Value& node = root["PnpDebugConfig"];
    if (node.isMember("Power")) {
        const Json::Value& ele = node["Power"];

        if (ele.isMember("useMockAAL")) {
            mStaticCfg->useMockAAL = ele["useMockAAL"].asBool();
        }
        if (ele.isMember("useMockHal")) {
            mStaticCfg->useMockHal = ele["useMockHal"].asBool();
        }
        if (ele.isMember("useMockPipes")) {
            mStaticCfg->useMockPipes = ele["useMockPipes"].asBool();
        }
        if (ele.isMember("pnpMockFps")) {
            mStaticCfg->mockAPPFps = ele["pnpMockFps"].asInt();
        }
        if (ele.isMember("bypass3A")) {
            mStaticCfg->isBypass3A = ele["bypass3A"].asBool();
        }
        if (ele.isMember("bypassPAC")) {
            mStaticCfg->isBypassPAC = ele["bypassPAC"].asBool();
        }
        if (ele.isMember("bypassCB")) {
            mStaticCfg->isBypassCB = ele["bypassCB"].asBool();
        }
        if (ele.isMember("disableFace")) {
            mStaticCfg->isFaceDisabled = ele["disableFace"].asBool();
        }
        if (ele.isMember("disableFaceAe")) {
            mStaticCfg->isFaceAeDisabled = ele["disableFaceAe"].asBool();
        }
        if (ele.isMember("bypassFDAlgo")) {
            mStaticCfg->isBypassFDAlgo = ele["bypassFDAlgo"].asBool();
        }
        if (ele.isMember("bypassISys")) {
            mStaticCfg->isBypassISys = ele["bypassISys"].asBool();
        }
        if (ele.isMember("useMockPSys")) {
            mStaticCfg->useMockPSys = ele["useMockPSys"].asBool();
        }
    }

    if (node.isMember("Performance")) {
        // future usage
    }

    return true;
}

}  // namespace icamera
