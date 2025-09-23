/*
 * Copyright (C) 2021-2025 Intel Corporation.
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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef LIBCAMERA_BUILD
#include <libcamera/base/log.h>
#else
#endif

#include <sys/time.h>
#include <time.h>

#include "iutils/LogSink.h"
#include "iutils/Utils.h"

#define TIME_BUF_SIZE 128
#define DEFAULT_FILELOG_PATH "/run/camera/hal_logs.txt"

namespace icamera {
extern const char* cameraDebugLogToString(uint32_t level);

static const uint32_t CAMERA_DEBUG_LOG_LEVEL1 = 1U;
static const uint32_t CAMERA_DEBUG_LOG_LEVEL2 = 2U;   // 1 << 1
static const uint32_t CAMERA_DEBUG_LOG_LEVEL3 = 4U;   // 1 << 2

static const uint32_t CAMERA_DEBUG_LOG_INFO = 16U;    // 1 << 4
static const uint32_t CAMERA_DEBUG_LOG_WARNING = 32U; // 1 << 5;
static const uint32_t CAMERA_DEBUG_LOG_ERR = 64U;     // 1 << 6;

#ifdef LIBCAMERA_BUILD
void LibcameraLogSink::sendOffLog(LogItem logItem) {
    char prefix[32];
    ::snprintf(prefix, sizeof(prefix), " [%s]: ",
            icamera::cameraDebugLogToString(logItem.level));
    libcamera::LogCategory* cat = libcamera::LogCategory::create(logItem.logTags);
    libcamera::LogSeverity sev = (logItem.level == CAMERA_DEBUG_LOG_ERR)     ? libcamera::LogError
                               : (logItem.level == CAMERA_DEBUG_LOG_WARNING) ? libcamera::LogWarning
                               : (logItem.level == CAMERA_DEBUG_LOG_INFO)    ? libcamera::LogInfo
                                                                             : libcamera::LogDebug;
    uint32_t levels = CAMERA_DEBUG_LOG_LEVEL1 | CAMERA_DEBUG_LOG_LEVEL2 | CAMERA_DEBUG_LOG_LEVEL3;
    if (logItem.level & levels) {
        cat->setSeverity(libcamera::LogDebug);
    }
    libcamera::_log(cat, sev).stream() << prefix << logItem.logEntry;
}
#else

#endif
void StdconLogSink::sendOffLog(LogItem logItem) {
    char timeInfo[TIME_BUF_SIZE];
    LogOutputSink::setLogTime(timeInfo);
    fprintf(stdout, "[%s] CamHAL[%s] %s: %s\n", timeInfo,
            icamera::cameraDebugLogToString(logItem.level), logItem.logTags, logItem.logEntry);
}

void LogOutputSink::setLogTime(char* timeBuf) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    const time_t nowtime = tv.tv_sec;
    struct tm local_tm;

    struct tm* nowtm = localtime_r(&nowtime, &local_tm);
    if (nowtm != nullptr) {
        char tmbuf[TIME_BUF_SIZE];
        (void)strftime(tmbuf, TIME_BUF_SIZE, "%m-%d %H:%M:%S", nowtm);
        snprintf(timeBuf, TIME_BUF_SIZE, "%.96s.%d", tmbuf,
                 static_cast<int>((tv.tv_usec / 1000U) % 1000U));
    }
}

#ifdef CAMERA_TRACE
FtraceLogSink::FtraceLogSink() {
    mFtraceFD = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    if (mFtraceFD == -1) {
        fprintf(stderr, "[WAR] Cannot init ftrace sink, [%s] self killing...", strerror(errno));
        raise(SIGABRT);
    }
}

void FtraceLogSink::sendOffLog(LogItem logItem) {
    char timeInfo[TIME_BUF_SIZE];
    setLogTime(timeInfo);
    dprintf(mFtraceFD, "%s CamHAL[%s] %s\n", timeInfo, cameraDebugLogToString(logItem.level),
            logItem.logEntry);
}
#endif

FileLogSink::FileLogSink() {
    static const char* filePath = ::getenv("FILE_LOG_PATH");

    if (filePath == nullptr) {
        filePath = DEFAULT_FILELOG_PATH;
    }

    mFp = fopen(filePath, "w");
}

void FileLogSink::sendOffLog(LogItem logItem) {
    char timeInfo[TIME_BUF_SIZE];
    LogOutputSink::setLogTime(timeInfo);
    fprintf(mFp, "[%s] CamHAL[%s] %s:%s\n", timeInfo,
            icamera::cameraDebugLogToString(logItem.level), logItem.logTags, logItem.logEntry);
    (void)fflush(mFp);
}

}  // namespace icamera
