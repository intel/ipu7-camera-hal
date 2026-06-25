/*
 * Copyright (C) 2025 Intel Corporation.
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

#define LOG_TAG InputEventMonitor

#include "Errors.h"
#include "InputEventMonitor.h"
#include "iutils/CameraLog.h"

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/ioctl.h>

namespace icamera {

static const struct EventIoctlMode {
    uint32_t type;
    uint32_t max;
    uint32_t rq;
} EVENTIOCTLMODES[] = {
    { EV_KEY, KEY_MAX, EVIOCGKEY(KEY_MAX) },
    { EV_LED, LED_MAX, EVIOCGLED(LED_MAX) },
    { EV_SND, SND_MAX, EVIOCGSND(SND_MAX) },
    { EV_SW, SW_MAX, EVIOCGSW(SW_MAX) },
};

static const uint8_t U32_BITS = sizeof(uint32_t) * 8;

InputEventMonitor::InputEventMonitor() : mCheckThread(nullptr),
                                         mEventType(-1),
                                         mEventCode(-1),
                                         mFd(-1),
                                         mValue(-1) {
    mCheckThread = new WorkThread(this);
}

InputEventMonitor::~InputEventMonitor() {
    if (nullptr != mCheckThread) {
        delete mCheckThread;
        mCheckThread = nullptr;
    }
    if (mFd >= 0) {
        ::close(mFd);
    }
}

int InputEventMonitor::configure(int eventType, int eventCode) {
    AutoMutex l(mLock);
    if (eventType < 0 || eventCode < 0) {
        LOGE("Invalid event type %d code %d", eventType, eventCode);
        return BAD_VALUE;
    }

    DIR *dir = ::opendir("/dev/input/");
    if (!dir) {
        LOGE("Unable to open input directory");
        return BAD_VALUE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            std::string devicePath = std::string("/dev/input/") + entry->d_name;
            if (checkDeviceCapabilities(devicePath, eventType, eventCode)) {
                int status = ::open(devicePath.c_str(), O_RDONLY);
                if (status >= 0) {
                    mFd = status;
                    mEventType = eventType;
                    mEventCode = eventCode;
                    for (uint32_t i = 0; i < sizeof(EVENTIOCTLMODES) / sizeof(EVENTIOCTLMODES[0]); ++i) {
                        if (mEventType == EVENTIOCTLMODES[i].type) {
                            mEventIoctlModesIndex = i;
                            break;
                        }
                    }
                    LOG1("Use %s for event type %d code %d", devicePath.c_str(), eventType,
                            eventCode);
                    status = readRawValue();
                } else {
                    LOGE("open %s fail: %d", devicePath.c_str(), status);
                }
                ::closedir(dir);
                return status;
            }
        }
    }

    ::closedir(dir);
    LOG1("No device support event type %d code %d", eventType, eventCode);
    return NO_ENTRY;
}

int InputEventMonitor::getValue() {
    AutoMutex l(mLock);
    return mValue;
}

int InputEventMonitor::readRawValue() {
    // Only used inside locked context so no need to get lock again
    if (mFd < 0 || mEventType < 0 || mEventCode < 0) return -1;

    int status = -1;
    uint32_t bits = EVENTIOCTLMODES[mEventIoctlModesIndex].max;
    uint32_t req = EVENTIOCTLMODES[mEventIoctlModesIndex].rq;
    uint32_t codeBits[bits / U32_BITS + 1];

    memset(codeBits, 0, sizeof(codeBits));
    status = ioctl(mFd, req, codeBits);
    if (status >= 0) {
        mValue = ((codeBits[mEventCode / U32_BITS] & (1UL << (mEventCode % U32_BITS))) != 0U) ? 1 : 0;
    }

    return status;
}

int InputEventMonitor::start() {
    AutoMutex l(mLock);
    int status = readRawValue();
    if (status < 0) {
        LOGE("read initial value fail: %d", status);
        return status;
    }
    mCheckThread->start();
    return OK;
}

int InputEventMonitor::stop() {
    {
        AutoMutex l(mLock);
        mCheckThread->exit();
    }
    mCheckThread->wait();
    return OK;
}

bool InputEventMonitor::check() {
    if (mFd < 0 || mEventType < 0 || mEventCode < 0) {
        return false;
    }

    struct pollfd pfd = {mFd, POLLIN};
    int status = ::poll(&pfd, 1, 33);
    if (status < 0) {
        LOGE("poll fd %d error: %d", mFd, status);
        return false;
    }
    if (status == 0) {
        // timeout
        return true;
    }

    struct input_event ev;
    CLEAR(ev);
    ssize_t n = read(mFd, &ev, sizeof(struct input_event));
    if (n <= 0) {
        LOGE("read event fail: %lld", n);
    } else {
        if (ev.type == mEventType && ev.code == mEventCode) {
            LOG1("read input event value %d", ev.value);
            {
                AutoMutex l(mLock);
                if (ev.value != mValue) {
                    mValue = ev.value;
                    EventData e;
                    e.type = EVENT_INPUT_EVENT;
                    e.buffer = nullptr;
                    e.data.inputEvent = {ev.type, ev.code, ev.value};
                    EventSource::notifyListeners(e);
                }
            }
        }
    }

    return true;
}

bool InputEventMonitor::checkDeviceCapabilities(const std::string &device, const int eventType, const int eventCode) {
    LOG1("check if %s supports event type %d code %d", device.c_str(), eventType, eventCode);
    bool ret = false;

    int status = ::open(device.c_str(), O_RDONLY);
    CheckAndLogError(status < 0, status, "open device %s fail: %d", device.c_str(), status);
    const int fd = status;

    // To store event type bitmap
    uint32_t typeBits[EV_MAX / U32_BITS + 1];
    memset(typeBits, 0, sizeof(typeBits));

    // To store event code bitmap
    uint32_t codeBits[KEY_MAX / U32_BITS + 1];
    memset(codeBits, 0, sizeof(codeBits));

    status = ioctl(fd, EVIOCGBIT(0, EV_MAX), typeBits);
    CheckAndClean(status < 0, status, ::close(fd), "%s query event types fail: %d", device.c_str(), status);

    if ((typeBits[eventType / U32_BITS] & (1U << (eventType % U32_BITS))) != 0U) {
        status = ioctl(fd, EVIOCGBIT(eventType, KEY_MAX), codeBits);
        CheckAndClean(status < 0, status, ::close(fd), "%s query key event codes fail: %d", device.c_str(), status);

        if ((codeBits[eventCode / U32_BITS] & (1U << (eventCode % U32_BITS))) != 0U) {
            ret = true;
        }
    }

    ::close(fd);
    return ret;
}

bool InputEventMonitor::isConfigured() const {
    return mFd >= 0;
}

}   // namespace icamera