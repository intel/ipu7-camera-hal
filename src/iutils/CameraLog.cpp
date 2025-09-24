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

#define LOG_TAG CameraLog

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <sstream>
#include <string>

#ifdef USE_VSYS_LOG
#include <base/logging.h>
#endif

#include "CameraLog.h"
#include "Trace.h"
#include "iutils/Utils.h"

icamera::LogOutputSink* globalLogSink;
extern const char* tagNames[];

GroupDesc globalGroupsDescp[TAGS_MAX_NUM];

namespace icamera {

uint32_t gLogLevel = 0U;
static uint32_t gPerfLevel = 0U;
int32_t gSlowlyRunRatio = 0;
// DUMP_ENTITY_TOPOLOGY_S
static bool gIsDumpMediaTopo = false;
// DUMP_ENTITY_TOPOLOGY_E
static bool gIsDumpMediaInfo = false;

const char* cameraDebugLogToString(uint32_t level) {
    switch (level) {
        case CAMERA_DEBUG_LOG_LEVEL1:
            return "LV1";
        case CAMERA_DEBUG_LOG_LEVEL2:
            return "LV2";
        case CAMERA_DEBUG_LOG_LEVEL3:
            return "LV3";
        case CAMERA_DEBUG_LOG_INFO:
            return "INF";
        case CAMERA_DEBUG_LOG_ERR:
            return "ERR";
        case CAMERA_DEBUG_LOG_WARNING:
            return "WAR";
        default:
            return "UKN";
    }
}

#ifdef USE_VSYS_LOG
__attribute__((__format__(__printf__, 3, 0))) static void printLog(const char* module, int level,
                                                                   const char* fmt, va_list ap) {
    char prefix[64] = {};
    snprintf(prefix, sizeof(prefix), "[%s]: CamHAL_%s:", cameraDebugLogToString(level), module);

    char message[256] = {};
    vsnprintf(message, sizeof(message), fmt, ap);

    switch (level) {
        case CAMERA_DEBUG_LOG_ERR:
            LOG(ERROR) << prefix << message;
            break;
        case CAMERA_DEBUG_LOG_WARNING:
            LOG(WARNING) << prefix << message;
            break;
        default:
            LOG(INFO) << prefix << message;
            break;
    }
}
#else
static void getLogTime(char* timeBuf, int bufLen) {
    // The format of time is: 01-22 15:24:53.071
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    const time_t nowtime = tv.tv_sec;
    struct tm* nowtm = localtime(&nowtime);
    if (nowtm != nullptr) {  // If nowtm is nullptr, simply print nothing for time info
        char tmbuf[bufLen];
        CLEAR(tmbuf);
        (void)strftime(tmbuf, bufLen, "%m-%d %H:%M:%S", nowtm);
        snprintf(timeBuf, bufLen, "%s.%03ld", tmbuf, tv.tv_usec / 1000);
    }
}

__attribute__((__format__(__printf__, 3, 0))) static void printLog(const char* module, int level,
                                                                   const char* fmt, va_list ap) {
    // Add time into beginning of the log.
    const int BUF_LEN = 64;
    char timeBuf[BUF_LEN] = {'\0'};

    getLogTime(timeBuf, BUF_LEN);

    fprintf(stdout, "%s: [%s]: CamHAL_%s:", timeBuf, cameraDebugLogToString(level), module);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
}
#endif

void doLogBody(int logTag, uint32_t level, int grpPosition, const char* fmt, ...) {
    if ((level & static_cast<int>(globalGroupsDescp[grpPosition].level)) == 0U) {
        return;
    }

    char message[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);

    globalLogSink->sendOffLog({message, level, tagNames[grpPosition]});
}

void doLogBody(int logTag, uint32_t level, const char* fmt, ...) {
    if ((level & static_cast<int>(globalGroupsDescp[logTag].level)) == 0U) {
        return;
    }

    char message[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);

    globalLogSink->sendOffLog({message, level, tagNames[logTag]});
}

namespace Log {

static void initLogSinks() {
#ifdef LIBCAMERA_BUILD
    if (globalLogSink == nullptr) {
        globalLogSink = new LibcameraLogSink();
        LOG2("Enable libcamera LOG");
        return;
    }
#else
#endif
    globalLogSink = new StdconLogSink();
    LOG2("Enable Stdcon LOG");
}

static void setLogTagLevel() {
    static const char* LOG_FILE_TAG = "cameraTags";
    char* logFileTag = ::getenv(LOG_FILE_TAG);
    if (logFileTag == nullptr) {
        return;
    }

    std::string s = logFileTag;
    std::istringstream is(s);
    std::string token;

    while (std::getline(is, token, ':')) {
        const auto pos = token.find_first_of('-');

        std::string name;
        std::string levelStr;
        if (pos != std::string::npos) {
            name = token.substr(0, pos);
            levelStr = token.substr(pos + 1);
        } else {
            name = token;
        }

        for (int itemIdx = 0; itemIdx < TAGS_MAX_NUM; ++itemIdx) {
            if (name != tagNames[itemIdx]) {
                continue;
            }

            if (!levelStr.empty()) {
                globalGroupsDescp[itemIdx].level =
                    static_cast<uint32_t>(strtoul(levelStr.c_str(), nullptr, 0));
            }
        }
    }
}

void setDebugLevel(void) {
    initLogSinks();

    const char* PROP_CAMERA_HAL_DEBUG = "cameraDebug";
    const char* PROP_CAMERA_RUN_RATIO = "cameraRunRatio";

    // debug
    char* dbgLevel = getenv(PROP_CAMERA_HAL_DEBUG);
    gLogLevel = CAMERA_DEBUG_LOG_ERR | CAMERA_DEBUG_LOG_WARNING | CAMERA_DEBUG_LOG_INFO;

    if (dbgLevel != nullptr) {
        gLogLevel = static_cast<uint32_t>(strtoul(dbgLevel, nullptr, 0));
        LOG1("Debug level is 0x%x", gLogLevel);
    }

    for (int i = 0; i < TAGS_MAX_NUM; ++i) {
        globalGroupsDescp[i].level = gLogLevel;
    }

    setLogTagLevel();

    char* slowlyRunRatio = getenv(PROP_CAMERA_RUN_RATIO);
    if (slowlyRunRatio != nullptr) {
        gSlowlyRunRatio = strtoul(slowlyRunRatio, nullptr, 0);
        LOG1("Slow run ratio is 0x%x", gSlowlyRunRatio);
    }
    // ASYNC_TRACE_S
    // performance
    const char* PROP_CAMERA_HAL_PERF = "cameraPerf";
    char* perfLevel = getenv(PROP_CAMERA_HAL_PERF);
    if (perfLevel != nullptr) {
        gPerfLevel = static_cast<uint32_t>(strtoul(perfLevel, nullptr, 0));
        LOGI("Performance level is 0x%x", gPerfLevel);

        // bitmask of tracing categories
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_PERF_TRACES)) != 0U) {
            LOG1("Perf KPI start/end trace is not yet supported");
        }
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_PERF_TRACES_BREAKDOWN)) != 0U) {
            LOG1("Perf KPI breakdown trace is not yet supported");
        }
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_PERF_IOCTL_BREAKDOWN)) != 0U) {
            LOG1("Perf IOCTL breakdown trace is not yet supported");
        }
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_PERF_MEMORY)) != 0U) {
            LOG1("Perf memory breakdown trace is not yet supported");
        }
        // DUMP_ENTITY_TOPOLOGY_S
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_MEDIA_TOPO_LEVEL)) != 0U) {
            gIsDumpMediaTopo = true;
        }
        // DUMP_ENTITY_TOPOLOGY_E
        if ((gPerfLevel & static_cast<int>(CAMERA_DEBUG_LOG_MEDIA_CONTROLLER_LEVEL)) != 0U) {
            gIsDumpMediaInfo = true;
        }
        ScopedAtrace::setTraceLevel(gPerfLevel);
    }
    // ASYNC_TRACE_E
}

bool isDebugLevelEnable(uint32_t level) {
    return gLogLevel & level;
}

bool isLogTagEnabled(int tag) {
    if ((tag < 0) || (tag >= TAGS_MAX_NUM)) {
        return false;
    }
    return globalGroupsDescp[tag].level > 0U;
}

// DUMP_ENTITY_TOPOLOGY_S
bool isDumpMediaTopo(void) {
    return gIsDumpMediaTopo;
}
// DUMP_ENTITY_TOPOLOGY_E

bool isDumpMediaInfo(void) {
    return gIsDumpMediaInfo;
}

__attribute__((__format__(__printf__, 1, 0))) void ccaPrintError(const char* fmt, va_list ap) {
    if ((gLogLevel & static_cast<int>(CAMERA_DEBUG_LOG_CCA)) != 0U) {
        printLog("CCA_DEBUG", CAMERA_DEBUG_LOG_ERR, fmt, ap);
    }
}

__attribute__((__format__(__printf__, 1, 0))) void ccaPrintInfo(const char* fmt, va_list ap) {
    if ((gLogLevel & static_cast<int>(CAMERA_DEBUG_LOG_CCA)) != 0U) {
        printLog("CCA_DEBUG", CAMERA_DEBUG_LOG_INFO, fmt, ap);
    }
}

}  // namespace Log

}  // namespace icamera
