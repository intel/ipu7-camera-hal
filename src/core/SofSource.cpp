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
#define LOG_TAG SofSource

#include "SofSource.h"

#include <fcntl.h>
#include <poll.h>

#include "PlatformData.h"
#include "V4l2DeviceFactory.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {

SofSource::SofSource(int cameraId)
        : mPollThread(nullptr),
          mCameraId(cameraId),
          mIsysReceiverSubDev(nullptr),
          mExitPending(false) {
    LOG1("%s: SofSource is constructed", __func__);

    mFlushFd[0] = -1;
    mFlushFd[1] = -1;

    int ret = pipe(mFlushFd);
    if (ret >= 0) {
        ret = fcntl(mFlushFd[0], F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            LOG1("failed to set flush pipe flag: %s", strerror(errno));
            close(mFlushFd[0]);
            close(mFlushFd[1]);
            mFlushFd[0] = -1;
            mFlushFd[1] = -1;
        }
        LOG1("%s, mFlushFd [%d-%d]", __func__, mFlushFd[0], mFlushFd[1]);
    }

    mSofDisabled = !PlatformData::isIsysEnabled(cameraId);
    // FILE_SOURCE_S
    mSofDisabled = mSofDisabled || PlatformData::isFileSourceEnabled();
    // FILE_SOURCE_E
}

SofSource::~SofSource() {
    LOG1("%s: SofSource is distructed.", __func__);
    if (mFlushFd[0] != -1) {
        close(mFlushFd[0]);
    }
    if (mFlushFd[1] != -1) {
        close(mFlushFd[1]);
    }
}

int SofSource::init() {
    if (mSofDisabled) {
        return OK;
    }

    mPollThread = new PollThread<SofSource>(this);

    return OK;
}

int SofSource::deinit() {
    if (mSofDisabled) {
        return OK;
    }

    const int status = deinitDev();
    mPollThread->wait();
    delete mPollThread;
    return status;
}

int SofSource::initDev() {
    // Create and open receiver subdevice.
    std::string subDeviceNodeName;

    if (PlatformData::getDevNameByType(mCameraId, VIDEO_ISYS_RECEIVER, subDeviceNodeName) == OK) {
        LOG1("%s: found ISYS receiver subdevice %s", __func__, subDeviceNodeName.c_str());
    }

    (void)deinitDev();

    mIsysReceiverSubDev = V4l2DeviceFactory::getSubDev(mCameraId, subDeviceNodeName);

    int vcId = 0;
    // VIRTUAL_CHANNEL_S
    /* The value of virtual channel id is 0, 1, 2, 3, ... if virtual channel supported */
    vcId = PlatformData::getVirtualChannelId(mCameraId);
    // VIRTUAL_CHANNEL_E
    const int status = mIsysReceiverSubDev->SubscribeEvent(V4L2_EVENT_FRAME_SYNC, vcId);
    CheckAndLogError(status != OK, status, "Failed to subscribe sync event %d", vcId);
    LOG1("%s: Using SOF event id %d for sync", __func__, vcId);

    return OK;
}

int SofSource::deinitDev() {
    if (mIsysReceiverSubDev == nullptr) {
        return OK;
    }

    int vcId = 0;
    // VIRTUAL_CHANNEL_S
    /* The value of virtual channel sequence is 0, 1, 2, 3, ... if virtual channel supported. */
    vcId = PlatformData::getVirtualChannelId(mCameraId);
    // VIRTUAL_CHANNEL_E
    const int status = mIsysReceiverSubDev->UnsubscribeEvent(V4L2_EVENT_FRAME_SYNC, vcId);
    if (status == OK) {
        LOG1("%s: Unsubscribe SOF event id %d done", __func__, vcId);
    } else {
        LOGE("Failed to unsubscribe SOF event %d", vcId);
    }
    mIsysReceiverSubDev = nullptr;

    return status;
}

int SofSource::configure() {
    if (mSofDisabled) {
        return OK;
    }

    return initDev();
}

int SofSource::start() {
    LOG1("%s", __func__);
    if (mSofDisabled) {
        return OK;
    }

    if (mFlushFd[0] != -1) {
        // read pipe just in case there is data in pipe.
        char readBuf;
        int readSize = read(mFlushFd[0], reinterpret_cast<void*>(&readBuf), sizeof(char));
        LOG1("%s, readSize %d", __func__, readSize);
    }

    mExitPending = false;
    mPollThread->start();

    return OK;
}

int SofSource::stop() {
    LOG1("%s", __func__);
    if (mSofDisabled) {
        return OK;
    }

    if (mFlushFd[1] != -1) {
        char buf = 0xf;  // random value to write to flush fd.
        const int size = write(mFlushFd[1], &buf, sizeof(char));
        LOG1("%s, write size %d", __func__, size);
    }

    mPollThread->exit();
    mExitPending = true;
    mPollThread->wait();

    return OK;
}

int SofSource::poll() {
    int ret = 0;
    const int pollTimeoutCount = 10;
    const int pollTimeout = 1000;

    std::vector<V4L2Device*> pollDevs;
    pollDevs.push_back(mIsysReceiverSubDev);
    V4L2DevicePoller poller{pollDevs, mFlushFd[0]};

    std::vector<V4L2Device*> readyDevices;

    int timeOutCount = pollTimeoutCount;

    while (((timeOutCount--) != 0) && (ret == 0)) {
        if (mExitPending) {
            LOG2("%s: mExitPending is true, exit", __func__);
            return -1;
        }

        ret = poller.Poll(pollTimeout, POLLPRI | POLLIN | POLLOUT | POLLERR, &readyDevices);
    }

    if (mExitPending) {
        return -1;
    }

    // handle the poll error
    if (ret < 0) {
        LOGE("Poll error");
        return ret;
    } else if (ret == 0) {
        LOGI("Sof poll time out.");
        return 0;
    }

    struct v4l2_event event;
    CLEAR(event);
    mIsysReceiverSubDev->DequeueEvent(&event);

    EventDataSync syncData;
    syncData.sequence = event.u.frame_sync.frame_sequence;
    syncData.timestamp.tv_sec = event.timestamp.tv_sec;
    syncData.timestamp.tv_usec = (event.timestamp.tv_nsec / 1000);
    LOG2("<seq%ld> %s:sof event, event.id %u", syncData.sequence, __func__, event.id);
    TRACE_LOG_POINT("SofSource", "receive sof event", MAKE_COLOR(syncData.sequence),
                    syncData.sequence);
    EventData eventData;
    eventData.type = EVENT_ISYS_SOF;
    eventData.buffer = nullptr;
    eventData.data.sync = syncData;
    notifyListeners(eventData);

    return 0;
}

}  // namespace icamera
