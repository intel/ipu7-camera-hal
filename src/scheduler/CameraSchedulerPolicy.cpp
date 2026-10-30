/*
 * Copyright (C) 2022-2025 Intel Corporation
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

#define LOG_TAG SchedPolicy

#include "src/scheduler/CameraSchedulerPolicy.h"

#include <utility>
#include <vector>

#include "CameraLog.h"
#include "Errors.h"
#include "PlatformData.h"

namespace icamera {

#define SCHEDULER_POLICY_FILE_NAME "pipe_scheduler_profiles.json"

CameraSchedulerPolicy* CameraSchedulerPolicy::sInstance = nullptr;
std::mutex CameraSchedulerPolicy::sLock;

CameraSchedulerPolicy* CameraSchedulerPolicy::getInstance() {
    std::lock_guard<std::mutex> lock(sLock);
    if (!sInstance) {
        sInstance = new CameraSchedulerPolicy();
    }
    return sInstance;
}

void CameraSchedulerPolicy::releaseInstance() {
    std::lock_guard<std::mutex> lock(sLock);
    if (sInstance) {
        delete sInstance;
        sInstance = nullptr;
    }
}

CameraSchedulerPolicy::CameraSchedulerPolicy()
        : mActiveConfig(nullptr) {
    LOG1("%s", __func__);

    std::string filename(SCHEDULER_POLICY_FILE_NAME);
    std::string fullPath = PlatformData::getCameraCfgPath() + filename;
    CameraSchedulerPolicy::run(fullPath);

    if (!mPolicyConfigs.empty()) {
        mActiveConfig = &mPolicyConfigs.front();
    }
}

CameraSchedulerPolicy::~CameraSchedulerPolicy() {
    LOG1("%s", __func__);
}

int32_t CameraSchedulerPolicy::setConfig(uint32_t graphId) {
    for (auto& iter : mPolicyConfigs) {
        if (iter.graphId == graphId) {
            mActiveConfig = &iter;
            LOG1("%s: config id %u, graphId %u", __func__, iter.configId, graphId);
            return OK;
        }
    }

    LOGE("%s: no config for graphId %u", __func__, graphId);
    return BAD_VALUE;
}

int32_t CameraSchedulerPolicy::getExecutors(std::map<const char*, const char*>* executors) const {
    CheckAndLogError(!executors, 0, "%s: nullptr", __func__);
    CheckAndLogError(!mActiveConfig, 0, "%s: No config", __func__);

    for (auto& iter : mActiveConfig->exeList) {
        (*executors)[iter.exeName.c_str()] = iter.triggerName.c_str();
    }
    return mActiveConfig->exeList.size();
}

int32_t CameraSchedulerPolicy::getNodeList(const char* exeName,
                                           std::vector<std::string>* nodeList) const {
    CheckAndLogError(!nodeList, BAD_VALUE, "nullptr input");
    CheckAndLogError(!mActiveConfig, BAD_VALUE, "No config");

    for (auto& exe : mActiveConfig->exeList) {
        if (strcmp(exe.exeName.c_str(), exeName) == 0) {
            *nodeList = exe.nodeList;
            return OK;
        }
    }
    return BAD_VALUE;
}

void CameraSchedulerPolicy::parseExecutorsObject(const Json::Value& node,
                                                 PolicyConfigDesc* desc) {
    for (Json::Value::ArrayIndex i = 0; i < node.size(); i++) {
        auto ele = node[i];
        ExecutorDesc exe;

        if (ele.isMember("name")) {
            exe.exeName = ele["name"].asString();
        }
        if (ele.isMember("trigger")) {
            exe.triggerName = ele["trigger"].asString();
        }
        if (ele.isMember("nodes")) {
            for (Json::Value::ArrayIndex j = 0; j < ele["nodes"].size(); j++)
                exe.nodeList.push_back(ele["nodes"][j].asString());
        }

        desc->exeList.push_back(std::move(exe));
    }
}

bool CameraSchedulerPolicy::run(const std::string& filename) {
    auto root = openJsonFile(filename);
    if (root.empty()) {
        return true;
    }

    if (!root.isMember("PipeSchedulerPolicy")) {
        return false;
    }

    const Json::Value& node = root["PipeSchedulerPolicy"];
    if (node.isMember("schedulers")) {
        auto scheduler = node["schedulers"];
        for (Json::Value::ArrayIndex i = 0; i < scheduler.size(); i++) {
            auto ele = scheduler[i];
            PolicyConfigDesc desc;

            if (ele.isMember("id")) {
                desc.configId = ele["id"].asUInt();
            }
            if (ele.isMember("graphId")) {
                desc.graphId = ele["graphId"].asUInt();
            }
            if (ele.isMember("pipe_executors")) {
                parseExecutorsObject(ele["pipe_executors"], &desc);
            }

            mPolicyConfigs.push_back(std::move(desc));
        }
    }

    return true;
}

}  // namespace icamera
