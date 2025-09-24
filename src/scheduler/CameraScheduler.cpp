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

#define LOG_TAG Scheduler

#include "src/scheduler/CameraScheduler.h"

#include <sstream>
#include <utility>

#include "CameraLog.h"
#include "Errors.h"
#include "PlatformData.h"
#include "iutils/Utils.h"

namespace icamera {

CameraScheduler::CameraScheduler(int cameraId) : mCameraId(cameraId), mTriggerCount(0) {
    mPolicy = CameraSchedulerPolicy::getInstance();
    mMsAlignWithSystem = PlatformData::getMsOfPsysAlignWithSystem(mCameraId);
    LOG2("%s: msAlignWithSystem %d", __func__, mMsAlignWithSystem);
}

CameraScheduler::~CameraScheduler() {
    destroyExecutors();
}

int32_t CameraScheduler::configurate(int32_t graphId) {
    int ret = mPolicy->setConfig(graphId);
    CheckAndLogError(ret != OK, ret, "configurate %d error", graphId);

    mTriggerCount = 0;
    destroyExecutors();
    return createExecutors();
}

int32_t CameraScheduler::createExecutors() {
    std::map<const char*, const char*> executors;
    int32_t exeNumber = mPolicy->getExecutors(&executors);
    CheckAndLogError(exeNumber <= 0, UNKNOWN_ERROR, "Can't get Executors' names");

    std::lock_guard<std::mutex> l(mLock);
    for (auto& exe : executors) {
        ExecutorGroup group;
        if (mMsAlignWithSystem) {
            group.executor =
                std::shared_ptr<Executor>(new SystemTimerExecutor(exe.first, mMsAlignWithSystem));
        } else {
            group.executor = std::shared_ptr<Executor>(new Executor(exe.first));
            group.triggerSource = exe.second;
        }
        if (!group.triggerSource.empty()) {
            // Check if trigger source is one executor
            std::shared_ptr<Executor> source = findExecutor(group.triggerSource.c_str());
            if (source != nullptr) {
                source->addListener(group.executor);
            }
        }
        mPolicy->getNodeList(exe.first, &group.nodeList);

        mExeGroups.push_back(group);
    }
    return OK;
}

void CameraScheduler::destroyExecutors() {
    std::lock_guard<std::mutex> l(mLock);
    mRegisteredNodes.clear();
    mExeGroups.clear();
}

int32_t CameraScheduler::registerNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mLock);

    ExecutorGroup* group = nullptr;
    for (size_t i = 0U; i < mExeGroups.size(); i++) {
        for (auto& nodeName : mExeGroups[i].nodeList) {
            if (strcmp(nodeName.c_str(), node->getName()) == 0) {
                group = &mExeGroups[i];
                break;
            }
        }
    }
    CheckWarning(!group, BAD_VALUE, "register node %s fail", node->getName());

    group->executor->addNode(node);
    mRegisteredNodes[node] = group;
    return OK;
}

void CameraScheduler::unregisterNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mLock);
    if (mRegisteredNodes.find(node) != mRegisteredNodes.end()) {
        mRegisteredNodes[node]->executor->removeNode(node);
        mRegisteredNodes.erase(node);
    }
}

void CameraScheduler::start() {
    for (auto& group : mExeGroups) {
        group.executor->start();
    }
}

void CameraScheduler::stop() {
    for (auto& group : mExeGroups) {
        group.executor->stop();
    }
}

int32_t CameraScheduler::executeNode(std::string triggerSource, int64_t triggerId) {
    // System timer aligned, ignore external trigger
    if (mMsAlignWithSystem) {
        return OK;
    }

    mTriggerCount++;
    for (auto& group : mExeGroups) {
        if (group.triggerSource == triggerSource)
            group.executor->trigger(triggerId < 0 ? mTriggerCount : triggerId);
    }
    return OK;
}

std::shared_ptr<CameraScheduler::Executor> CameraScheduler::findExecutor(const char* exeName) {
    if (!exeName) {
        return nullptr;
    }

    for (auto& group : mExeGroups) {
        if (strcmp(group.executor->getName(), exeName) == 0) {
            return group.executor;
        }
    }

    return nullptr;
}

CameraScheduler::Executor::Executor(const char* name)
        : mName(name ? name : "unknown"),
          mActive(false),
          mTriggerTick(0) {}

CameraScheduler::Executor::~Executor() {
    LOG1("%s: destroy", getName());
    CameraScheduler::Executor::stop();
}

void CameraScheduler::Executor::addNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mNodeLock);
    if (mActive) {
        return;
    }
    mNodes.push_back(node);
    LOG1("%s: %s added to %s, pos %d", __func__, node->getName(), getName(), mNodes.size());
}

void CameraScheduler::Executor::removeNode(ISchedulerNode* node) {
    std::lock_guard<std::mutex> l(mNodeLock);
    if (mActive) {
        return;
    }
    for (size_t i = 0U; i < mNodes.size(); i++) {
        if (mNodes[i] == node) {
            LOG1("%s: %s moved from %s", __func__, node->getName(), getName());
            mNodes.erase(mNodes.begin() + i);
            break;
        }
    }
}

void CameraScheduler::Executor::start() {
    LOG2("%s: %s", getName(), __func__);
    {
        std::lock_guard<std::mutex> l(mNodeLock);
        mActive = true;
        mTriggerTick = 0;
    }
    Thread::start();
}

void CameraScheduler::Executor::stop() {
    LOG2("%s: %s", getName(), __func__);
    Thread::exit();
    {
        std::lock_guard<std::mutex> l(mNodeLock);
        mActive = false;
        mTriggerSignal.notify_one();
    }
    Thread::wait();
}

void CameraScheduler::Executor::trigger(int64_t tick) {
    PERF_CAMERA_ATRACE_PARAM1(getName(), tick);
    std::lock_guard<std::mutex> l(mNodeLock);
    mTriggerTick = tick;
    mTriggerSignal.notify_one();
}

int CameraScheduler::Executor::waitTrigger() {
    std::unique_lock<std::mutex> lock(mNodeLock);
    std::cv_status ret = mTriggerSignal.wait_for(
        lock, std::chrono::nanoseconds(kWaitDuration * SLOWLY_MULTIPLIER));
    CheckWarning(ret == std::cv_status::timeout, true, "%s: wait trigger time out", getName());
    return mTriggerTick;
}

bool CameraScheduler::Executor::threadLoop() {
    int64_t tick = waitTrigger();

    {
        std::lock_guard<std::mutex> l(mNodeLock);
        if (!mActive) {
            return false;
        }
    }

    LOG3("%s process, tick %d", getName(), tick);
    for (auto& node : mNodes) {
        bool ret = node->process(tick);
        CheckAndLogError(!ret, true, "%s: node %s process error", getName(), node->getName());
    }

    for (auto listener : mListeners) {
        LOG2("%s: trigger listener %s", getName(), listener->getName());
        listener->trigger(tick);
    }
    return true;
}

void CameraScheduler::SystemTimerExecutor::trigger(int64_t tick) {
    // Ignore external trigger
    UNUSED(tick);
}

int CameraScheduler::SystemTimerExecutor::waitTrigger() {
// Allow +/- 3ms delay
// TODO: check and ignore repeating without sleep (if no task to be handled)
#define SYS_TRIGGER_DELTA    (3)

    if (mMsAlignWithSystem) {
        timeval curTime;
        gettimeofday(&curTime, nullptr);
        int64_t ms = (curTime.tv_usec / 1000) % mMsAlignWithSystem;
        int64_t waitMs = 0;

        if ((ms <= SYS_TRIGGER_DELTA) || ((mMsAlignWithSystem - ms) <= SYS_TRIGGER_DELTA))
            waitMs = 0;
        else
            waitMs  = mMsAlignWithSystem - ms;

        if (waitMs) {
            LOG2("%s: need wait %ld to trigger", getName(), waitMs);
            usleep(waitMs * 1000);
        }
    }

    std::lock_guard<std::mutex> l(Executor::mNodeLock);
    PERF_CAMERA_ATRACE_PARAM1(getName(), mTriggerTick);
    mTriggerTick++;
    return mTriggerTick;
}

}  // namespace icamera
