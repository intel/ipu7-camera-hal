/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2015-2022 Intel Corporation
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

#define LOG_TAG MediaControl

#include "MediaControl.h"

#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <stack>
#include <string>

#include "ParamDataType.h"
#include "PlatformData.h"
#include "SysCall.h"
#include "V4l2DeviceFactory.h"
#include "iutils/CameraLog.h"
#include "iutils/Errors.h"
#include "iutils/Utils.h"

using std::string;
using std::vector;

namespace icamera {
MediaControl* MediaControl::sMediaControl = nullptr;
Mutex MediaControl::sLock;

void MediaControl::createInstance(libcamera::MediaDevice* isysDev) {
    AutoMutex lock(sLock);
    if (!sMediaControl) {
        sMediaControl = new MediaControl(isysDev);
    }
}

MediaControl* MediaControl::getInstance() {
    if (!sMediaControl)
        LOGE("sMediaControl is nullptr");

    return sMediaControl;
}

void MediaControl::releaseInstance() {
    LOG1("%s", __func__);
    AutoMutex lock(sLock);

    if (sMediaControl) {
        delete sMediaControl;
        sMediaControl = nullptr;
    }
}

MediaControl::MediaControl(libcamera::MediaDevice* isysDev) : mIsysDev(isysDev) {
    LOG1("@%s", __func__);
}

MediaControl::~MediaControl() {
    LOG1("@%s", __func__);
}

int MediaControl::getEntityIdByName(const std::string &name) {
    libcamera::MediaEntity* entity = mIsysDev->getEntityByName(name);
    if (!entity) {
        return -1;
    }

    LOG1("@%s name:%s, id:%d", __func__, name.c_str(), entity->id());
    return entity->id();
}

int MediaControl::resetAllLinks() {
    LOG1("@%s", __func__);

    return mIsysDev->disableLinks();
}

int MediaControl::resetAllRoutes(int cameraId) {
    LOG1("<id%d> %s", cameraId, __func__);

    return OK;
}

libcamera::MediaEntity* MediaControl::getEntityById(uint32_t id) {
    bool next = id & MEDIA_ENT_ID_FLAG_NEXT;

    id &= ~MEDIA_ENT_ID_FLAG_NEXT;

    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();
    for (auto* entity : entities) {
        if ((entity->id() == id && !next) || (entity->id() > id && next)) {
            return entity;
        }
    }

    return nullptr;
}

void MediaControl::setMediaMcCtl(int cameraId, vector<McCtl> ctls) {
    setSensorOrientation(cameraId);

    for (auto& ctl : ctls) {
        libcamera::MediaEntity* entity = mIsysDev->getEntityByName(ctl.entityName);
        if (!entity) {
            LOGW("Failed to get entity:%s device", ctl.entityName.c_str());
            continue;
        }

        V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->deviceNode());
        LOG1("set Ctl %s [%d] cmd %s [0x%08x] value %d", ctl.entityName.c_str(), ctl.entity,
             ctl.ctlName.c_str(), ctl.ctlCmd, ctl.ctlValue);
        if (subDev->SetControl(ctl.ctlCmd, ctl.ctlValue) != OK) {
            LOGW("set Ctl %s [%d] cmd %s [0x%08x] value %d failed.", ctl.entityName.c_str(),
                 ctl.entity, ctl.ctlName.c_str(), ctl.ctlCmd, ctl.ctlValue);
        }
    }
}

int MediaControl::setupLink(McLink &mclink) {
    int ret = 0;
    media_link_desc ulink;
    libcamera::MediaLink* link = nullptr;

    link = mIsysDev->link(mclink.srcEntityName, mclink.srcPad, mclink.sinkEntityName,
                          mclink.sinkPad);
    CheckAndLogError(!link, NAME_NOT_FOUND, "Failed to get link");

    ret = link->setEnabled(mclink.enable);
    CheckAndLogError(ret != 0, ret, "Unable to setup link (%s)", strerror(errno));

    if (Log::isDumpMediaInfo()) dumpLinkDesc(&ulink, 1);

    return OK;
}

int MediaControl::setMediaMcLink(vector<McLink> links) {
    for (auto& link : links) {
        LOG1("setup Link %s [%d:%d] ==> %s [%dx%d] enable %d.", link.srcEntityName.c_str(),
             link.srcEntity, link.srcPad, link.sinkEntityName.c_str(), link.sinkEntity,
             link.sinkPad, link.enable);

        int ret = setupLink(link);
        CheckAndLogError(ret < 0, ret, "setup Link %s [%d:%d] ==> %s [%dx%d] enable %d failed.",
                         link.srcEntityName.c_str(), link.srcEntity, link.srcPad,
                         link.sinkEntityName.c_str(), link.sinkEntity, link.sinkPad, link.enable);
    }

    return OK;
}

int MediaControl::setFormat(int cameraId, const McFormat* format, int targetWidth, int targetHeight,
                            int field) {
    PERF_CAMERA_ATRACE();
    int ret;
    v4l2_mbus_framefmt mbusfmt;
    libcamera::MediaEntity* entity = mIsysDev->getEntityByName(format->entityName);
    CheckAndLogError(!entity, BAD_VALUE, "Get entity fail for calling getEntityById");

    const libcamera::MediaPad* pad = entity->getPadByIndex(format->pad);
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->deviceNode());
    LOG1("SENSORCTRLINFO: width=%d, height=%d, code=0x%x", targetWidth, targetHeight,
         format->pixelCode);

    CLEAR(mbusfmt);
    if (format->width != 0 && format->height != 0) {
        mbusfmt.width = format->width;
        mbusfmt.height = format->height;
    } else if (format->type == RESOLUTION_TARGET) {
        mbusfmt.width = targetWidth;
        mbusfmt.height = targetHeight;
    }
    mbusfmt.field = field;

    if (format->pixelCode) {
        mbusfmt.code = format->pixelCode;
    } else {
        mbusfmt.code = CameraUtils::getMBusFormat(cameraId, PlatformData::getISysFormat(cameraId));
    }
    LOG1("set format %s [%d:%d/%d] [%dx%d] [%dx%d] %s ", format->entityName.c_str(), format->entity,
         format->pad, format->stream, mbusfmt.width, mbusfmt.height, targetWidth, targetHeight,
         CameraUtils::pixelCode2String(mbusfmt.code));

    struct v4l2_subdev_format fmt = {};
    fmt.pad = format->pad;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    fmt.format = mbusfmt;
    fmt.stream = format->stream;
    ret = subDev->SetFormat(fmt);
    CheckAndLogError(ret < 0, BAD_VALUE, "set format %s [%d:%d] [%dx%d] %s failed.",
                     format->entityName.c_str(), format->entity, format->pad, format->width,
                     format->height, CameraUtils::pixelCode2String(format->pixelCode));

    mbusfmt = fmt.format;

    /* If the pad is an output pad, automatically set the same format on
     * the remote subdev input pads, if any.
     */
    if (pad->flags() & MEDIA_PAD_FL_SOURCE) {
        const std::vector<libcamera::MediaLink *> links = pad->links();
        for (auto* link : links) {
            if (!(link->flags() & MEDIA_LNK_FL_ENABLED)) continue;

            if (link->source() == pad &&
                link->sink()->entity()->type() == libcamera::MediaEntity::Type::V4L2Subdevice) {
                auto subDev = V4l2DeviceFactory::getSubDev(cameraId,
                                                           link->sink()->entity()->deviceNode());

                struct v4l2_subdev_format tmt = {};
                tmt.format = mbusfmt;
                tmt.pad = link->sink()->index();
                tmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
                subDev->SetFormat(tmt);
            }
        }
    }

    return 0;
}

int MediaControl::setSelection(int cameraId, const McFormat* format, int targetWidth,
                               int targetHeight) {
    PERF_CAMERA_ATRACE();
    int ret = OK;

    libcamera::MediaEntity* entity = mIsysDev->getEntityByName(format->entityName);
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, entity->deviceNode());
    LOG1("<id%d> @%s, targetWidth:%d, targetHeight:%d", cameraId, __func__, targetWidth,
         targetHeight);

    if (format->top != -1 && format->left != -1 && format->width != 0 && format->height != 0) {
        struct v4l2_subdev_selection selection = {};
        selection.pad = format->pad;
        selection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        selection.target = format->selCmd;
        selection.flags = 0;
        selection.r.top = format->top;
        selection.r.left = format->left;
        selection.r.width = format->width;
        selection.r.height = format->height;

        ret = subDev->SetSelection(selection);
    } else if (format->selCmd == V4L2_SEL_TGT_CROP || format->selCmd == V4L2_SEL_TGT_COMPOSE) {
        struct v4l2_subdev_selection selection = {};
        selection.pad = format->pad;
        selection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        selection.target = format->selCmd;
        selection.flags = 0;
        selection.r.top = 0;
        selection.r.left = 0;
        selection.r.width = targetWidth;
        selection.r.height = targetHeight;

        ret = subDev->SetSelection(selection);
    } else {
        ret = BAD_VALUE;
    }

    CheckAndLogError(ret < 0, BAD_VALUE,
                     "set selection %s [%d:%d] selCmd: %d [%d, %d] [%dx%d] failed",
                     format->entityName.c_str(), format->entity, format->pad, format->selCmd,
                     format->top, format->left, format->width, format->height);

    return OK;
}

int MediaControl::setRoute(int cameraId, McRoute* route) {
    LOG1("<id%d> %s", cameraId, __func__);

    /* TODO: Need Google to implemment the API */
    return OK;
}

int MediaControl::mediaCtlSetup(int cameraId, MediaCtlConf* mc, int width, int height, int field) {
    LOG1("<id%d> %s", cameraId, __func__);
    /* Setup controls in format Configuration */
    setMediaMcCtl(cameraId, mc->ctls);

    int ret = OK;
    /* TODO: Set routing */

    /* Set format & selection in format Configuration */
    for (auto& fmt : mc->formats) {
        if (fmt.formatType == FC_FORMAT) {
            setFormat(cameraId, &fmt, width, height, field);
        } else if (fmt.formatType == FC_SELECTION) {
            setSelection(cameraId, &fmt, width, height);
        }
    }

    /* Set link in format Configuration */
    ret = setMediaMcLink(mc->links);
    CheckAndLogError(ret != OK, ret, "set MediaCtlConf McLink failed: ret = %d", ret);

    return OK;
}

int MediaControl::getVCMI2CAddr(const char* vcmName, string* vcmI2CAddr) {
    CheckAndLogError(!vcmI2CAddr, BAD_VALUE, "vcmI2CAddr is nullptr");
    CheckAndLogError(!vcmName, BAD_VALUE, "vcmName is nullptr");

    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();
    for (auto* entity : entities) {
        if (strncmp(entity->name().c_str(), vcmName, strlen(vcmName)) == 0) {
            *vcmI2CAddr = entity->name();
            LOG1("%s, vcm addr name %s", __func__, entity->name().c_str());
            return OK;
        }
    }

    return NAME_NOT_FOUND;
}

void MediaControl::mediaCtlClear(int cameraId, MediaCtlConf* mc) {
    LOG1("<id%d> %s", cameraId, __func__);

    /* TODO: Google doesn't have the API to set routing */
    /* Clear routing */
}

// This function must be called after enumEntities().
int MediaControl::getLensName(string* lensName) {
    CheckAndLogError(!lensName, UNKNOWN_ERROR, "lensName is nullptr");

    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();
    for (auto* entity : entities) {
        if (entity->function() == MEDIA_ENT_F_LENS) {
            *lensName = entity->name();
            return OK;
        }
    }

    return UNKNOWN_ERROR;
}

// This function must be called after enumEntities().
bool MediaControl::checkAvailableSensor(const std::string& sensorEntityName) {
    LOG1("@%s, sensorEntityName:%s", __func__, sensorEntityName.c_str());

    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();
    for (auto* entity : entities) {
        if (strncmp(sensorEntityName.c_str(), entity->name().c_str(), sensorEntityName.length())
            == 0) {
            return true;
        }
    }
    return false;
}

// This function must be called after enumEntities().
bool MediaControl::checkAvailableSensor(const std::string& sensorEntityName,
                                        const std::string& sinkEntityName) {
    LOG1("@%s, sensorEntityName:%s, sinkEntityName:%s", __func__, sensorEntityName.c_str(),
         sinkEntityName.c_str());

    // Check if any sensor starts with sensorEntityName connects to
    // sinkEntityName, which is IPU CSI port
    std::string sensorEntityNameTmp = sensorEntityName + " ";
    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();
    CheckAndLogError(entities.size() == 0, false, "entities size is 0");

    libcamera::MediaEntity *sinkEntity = nullptr;
    libcamera::MediaEntity *sensorEntity = nullptr;
    for (auto* entity : entities) {
        if (strcmp(sinkEntityName.c_str(), entity->name().c_str()) == 0) {
            sinkEntity = entity;
        } else if (strncmp(sensorEntityName.c_str(), entity->name().c_str(),
                           sensorEntityName.length()) == 0) {
            sensorEntity = entity;
        }

        if (sinkEntity && sensorEntity) {
            break;
        }
    }

    CheckAndLogError(!sinkEntity || !sensorEntity, false, "sinkEntity or sensorEntity is nullptr");
    /** TODO: Add I2C bus to find related sensor entity
    const std::vector<libcamera::MediaPad *> pads = sensorEntity->pads();
    for (auto* pad : pads) {
        const std::vector<libcamera::MediaLink *> links = pad->links();
        for (auto* link : links) {
            if (strcmp(link->sink()->entity()->name().c_str(), sinkEntity->name().c_str()) == 0) {
                return true;
            }
        }
    }

    return false;
    */
    return true;
}

// This function must be called after enumEntities().
int MediaControl::getI2CBusAddress(const string& sensorEntityName, const string& sinkEntityName,
                                   string* i2cBus) {
    LOG1("@%s, sensorEntityName:%s, sinkEntityName:%s", __func__, sensorEntityName.c_str(),
         sinkEntityName.c_str());
    CheckAndLogError(!i2cBus, UNKNOWN_ERROR, "i2cBus is nullptr");

    const std::vector<libcamera::MediaEntity *> &entities = mIsysDev->entities();

    libcamera::MediaEntity *sinkEntity = nullptr;
    for (auto* entity : entities) {
        if (strcmp(sinkEntityName.c_str(), entity->name().c_str()) == 0) {
            sinkEntity = entity;
            break;
        }
    }
    CheckAndLogError(!sinkEntity, UNKNOWN_ERROR, "%s, sinkEntity is nullptr", __func__);

    const std::vector<libcamera::MediaPad *> pads = sinkEntity->pads();
    size_t sensorEntityNameLen = sensorEntityName.length();
    for (auto* pad : pads) {
        const std::vector<libcamera::MediaLink *> links = pad->links();
        for (auto* link : links) {
            const char *entityName = link->source()->entity()->name().c_str();
            if (strncmp(sensorEntityName.c_str(), entityName, sensorEntityNameLen) == 0) {
                *i2cBus = entityName + sensorEntityNameLen + 1;
                LOG1("i2cBus is %s", i2cBus->c_str());
                return OK;
            }
        }
    }

    return UNKNOWN_ERROR;
}

void MediaControl::setSensorOrientation(int cameraId) {
    int orientation = icamera::PlatformData::getSensorOrientation(cameraId);

    if (orientation != ORIENTATION_0 && orientation != ORIENTATION_180) {
        LOGW("camera %d: sensor orientation %d cannot be applied with HFLIP/VFLIP",
             cameraId, orientation);
    }

    std::string subDevName;
    const int ret = PlatformData::getDevNameByType(cameraId, VIDEO_PIXEL_ARRAY, subDevName);
    if (ret != OK || subDevName.empty()) {
        LOGE("@%s, failed to get device name for VIDEO_PIXEL_ARRAY, cameraId: %d", __func__, cameraId);
        return;
    }
    LOG1("@%s, sub-dev name is %s", __func__, subDevName.c_str());
    V4L2Subdevice* subDev = V4l2DeviceFactory::getSubDev(cameraId, subDevName);
    if (!subDev) {
        LOGE("camera %d: sensor subdevice %s is unavailable", cameraId, subDevName.c_str());
        return;
    }
    const int flip = orientation == ORIENTATION_180 ? 1 : 0;
    const int hflipRet = subDev->SetControl(V4L2_CID_HFLIP, flip);
    const int vflipRet = subDev->SetControl(V4L2_CID_VFLIP, flip);
    if (hflipRet == OK && vflipRet == OK) {
        LOGI("camera %d: sensor.orientation=%d on %s (HFLIP=%d VFLIP=%d)",
             cameraId, orientation, subDevName.c_str(), flip, flip);
    } else {
        LOGE("camera %d: sensor.orientation=%d failed on %s (HFLIP=%d VFLIP=%d)",
             cameraId, orientation, subDevName.c_str(), hflipRet, vflipRet);
    }
}

void MediaControl::dumpPadDesc(media_pad_desc* pads, const int padsCount, const char* name) {
    for (int i = 0; i < padsCount; i++) {
        LOGI("Dump %s Pad desc %d", name == nullptr ? "" : name, i);
        LOGI("entity: %d", pads[i].entity);
        LOGI("index: %d", pads[i].index);
        LOGI("flags: %d", pads[i].flags);
        LOGI("reserved[0]: %d", pads[i].reserved[0]);
        LOGI("reserved[1]: %d", pads[i].reserved[1]);
    }
}

void MediaControl::dumpLinkDesc(media_link_desc* links, const int linksCount) {
    for (int i = 0; i < linksCount; i++) {
        LOGI("Dump Link desc %d", i);
        libcamera::MediaEntity* sourceEntity = getEntityById(links[i].source.entity);
        libcamera::MediaEntity* sinkEntity = getEntityById(links[i].sink.entity);

        dumpPadDesc(&links[i].source, 1, sourceEntity->name().c_str());
        dumpPadDesc(&links[i].sink, 1, sinkEntity->name().c_str());
        LOGI("flags: %d", links[i].flags);
        LOGI("reserved[0]: %d", links[i].reserved[0]);
        LOGI("reserved[1]: %d", links[i].reserved[1]);
    }
}
}  // namespace icamera
