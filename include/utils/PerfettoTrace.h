/*
 * Copyright (C) 2024 Intel Corporation.
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

#include <perfetto/perfetto.h>
#include <unistd.h>

#include <condition_variable>
#include <mutex>

namespace icamera {
extern bool gPerfettoEnabled;
}

#define PERFETTO_CATEGORIES "camera"

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category(PERFETTO_CATEGORIES).SetDescription("Trace from libcamera"));

extern void initPerfettoTrace();

class ScopedPerfetto {
 public:
    ScopedPerfetto(const char* traceInfo, const char* note1 = nullptr, int value1 = 0,
                   const char* note2 = nullptr, int value2 = 0, const char* note3 = nullptr,
                   int value3 = 0);
    ~ScopedPerfetto();

 private:
    bool mEnableAtraceEnd;
};

#define PERFETTO_TRACE_EVENT(...) ScopedPerfetto ptrace(__FUNCTION__, ##__VA_ARGS__)

#define PERF_CAMERA_ATRACE(...) PERFETTO_TRACE_EVENT(__VA_ARGS__);

#define PERF_CAMERA_ATRACE_PARAM1(note1, value1) PERFETTO_TRACE_EVENT(note1, value1)

#define PERF_CAMERA_ATRACE_PARAM2(note1, value1, note2, value2) \
    PERFETTO_TRACE_EVENT(note1, value1, note2, value2)

#define PERF_CAMERA_ATRACE_PARAM3(note1, value1, note2, value2, note3, value3) \
    PERFETTO_TRACE_EVENT(note1, value1, note2, value2, note3, value3)

#define PERF_CAMERA_ATRACE_IMAGING(...) PERFETTO_TRACE_EVENT(__VA_ARGS__)

#define PERF_CAMERA_ATRACE_PARAM1_IMAGING(note1, value1) PERFETTO_TRACE_EVENT(note1, value1)

#define PERF_CAMERA_ATRACE_PARAM2_IMAGING(note1, value1, note2, value2) \
    PERFETTO_TRACE_EVENT(note1, value1, note2, value2)

#define PERF_CAMERA_ATRACE_PARAM3_IMAGING(note1, value1, note2, value2, note3, value3) \
    PERFETTO_TRACE_EVENT(note1, value1, note2, value2, note3, value3)
