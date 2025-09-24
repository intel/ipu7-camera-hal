/*
 * Copyright (C) 2022-2024 Intel Corporation
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
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <condition_variable>

#include "CameraEvent.h"
#include "CameraSchedulerPolicy.h"
#include "ISchedulerNode.h"

namespace icamera {

/**
 * \class CameraScheduler
 *
 * The call sequence as follows:
 * 1. configure();
 * 2. registerNode();
 * 3. loop: executeNode();
 * 4. unregisterNode(); (optional)
 */
class CameraScheduler {
 public:
    explicit CameraScheduler(int cameraId);
    ~CameraScheduler();

    int32_t configurate(int32_t graphId);

    int32_t registerNode(ISchedulerNode* node);
    void unregisterNode(ISchedulerNode* node);

    void start();
    void stop();

    /**
     * triggerSource:
     *    empty string: no designated source, will trigger executors WITHOUT trigger sources
     *                  in configuration file.
     * triggerId:
     *    >= 0: will be passed to ISchedulerNode for processing sync.
     *    others: Provide internal trigger count.
     */
    int32_t executeNode(std::string triggerSource, int64_t triggerId = -1);

 private:
    // Aligned with trigger signal sent by CameraScheduler
    class Executor : public Thread {
     public:
        explicit Executor(const char* name);
        virtual ~Executor();

        virtual void start();
        virtual void stop();
        void run() {
            bool ret = true;
            while(ret) {
                ret = threadLoop();
            }
        }
        bool threadLoop();

        void addNode(ISchedulerNode*);
        void removeNode(ISchedulerNode* node);
        void addListener(std::shared_ptr<Executor> executor) { mListeners.push_back(executor); }
        virtual void trigger(int64_t tick);

        const char* getName() { return mName.c_str(); }

     protected:
        virtual int waitTrigger();

     private:
        static const nsecs_t kWaitDuration = 2000000000;  // 2s

        std::string mName;

        std::vector<ISchedulerNode*> mNodes;
        std::vector<std::shared_ptr<Executor>> mListeners;
        std::condition_variable mTriggerSignal;
        bool mActive;

    protected:
        std::mutex mNodeLock;
        int64_t mTriggerTick;

     private:
        DISALLOW_COPY_AND_ASSIGN(Executor);
    };

    // Executor aligned with system time
    class SystemTimerExecutor : public Executor {
     public:
        SystemTimerExecutor(const char* name, int32_t alignedTime)
            : Executor(name), mMsAlignWithSystem(alignedTime) {}
        virtual ~SystemTimerExecutor() {}

        // Executor
        virtual void trigger(int64_t tick);

     private:
        // Executor
        virtual int waitTrigger();

     private:
        int32_t mMsAlignWithSystem;
    };

 private:
    int32_t createExecutors();
    void destroyExecutors();

    std::shared_ptr<Executor> findExecutor(const char* exeName);

 private:
    struct ExecutorGroup {
        std::shared_ptr<Executor> executor;
        std::string triggerSource;  //  empty string means no designated source
        std::vector<std::string> nodeList;
    };

    int mCameraId;
    std::mutex mLock;
    std::vector<ExecutorGroup> mExeGroups;
    // Record owner exe of nodes (after policy switch)
    std::unordered_map<ISchedulerNode*, ExecutorGroup*> mRegisteredNodes;

    int64_t mTriggerCount;

 private:
    CameraSchedulerPolicy* mPolicy;
    int32_t mMsAlignWithSystem;

 private:
    DISALLOW_COPY_AND_ASSIGN(CameraScheduler);
};

}  // namespace icamera
