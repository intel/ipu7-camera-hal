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

#define LOG_TAG CameraBuffer

#include "CameraBuffer.h"

#ifdef LIBDRM_SUPPORT_MMAP_OFFSET
#include <xf86drm.h>
#include <drm/xe_drm.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <memory>
#include <vector>

#include "PlatformData.h"
#include "iutils/CameraLog.h"
#include "iutils/Utils.h"

namespace icamera {
CameraBuffer::CameraBuffer(int memory, uint32_t size, int index)
        : mAllocatedMemory(false),
          mU(nullptr),
          mSettingSequence(-1),
          mMmapAddrs(nullptr) {
    LOG2("%s: construct buffer with memory:%d, size:%d, index:%d",  __func__, memory, size, index);

    mU = new camera_buffer_t;
    CLEAR(*mU);
    mBufferflag = BUFFER_FLAG_INTERNAL;
    mU->flags = BUFFER_FLAG_INTERNAL;
    mU->sequence = -1;

    mV.SetMemory(memory);
    mV.SetIndex(index);
    mV.SetType(V4L2_BUF_TYPE_VIDEO_CAPTURE);
    mV.SetLength(size, 0);
    mV.SetFlags(mV.Flags() |\
                static_cast<uint32_t>(V4L2_BUF_FLAG_NO_CACHE_INVALIDATE) |\
                static_cast<uint32_t>(V4L2_BUF_FLAG_NO_CACHE_CLEAN));
}

CameraBuffer::~CameraBuffer() {
    freeMemory();

    if ((mBufferflag & BUFFER_FLAG_INTERNAL) != 0U) {
        delete mU;
    }
}

// Helper function to construct a internal CameraBuffer with memory type
std::shared_ptr<CameraBuffer> CameraBuffer::create(int memory, unsigned int size, int index,
                                                   int srcFmt, int srcWidth, int srcHeight) {
    LOG1("%s, width:%d, height:%d, memory type:%d, size:%d, format:%d, index:%d", __func__,
         srcWidth, srcHeight, memory, size, srcFmt, index);
    std::shared_ptr<CameraBuffer> camBuffer =
        std::make_shared<CameraBuffer>(memory, size, index);
    CheckAndLogError(camBuffer == nullptr, nullptr, "@%s: fail to alloc CameraBuffer", __func__);

    camBuffer->setUserBufferInfo(srcFmt, srcWidth, srcHeight);
    int ret = camBuffer->allocateMemory();
    CheckAndLogError(ret != OK, nullptr, "Allocate memory failed ret %d", ret);

    return camBuffer;
}

// Helper function to construct a CameraBuffer from camera_buffer_t pointer
std::shared_ptr<CameraBuffer> CameraBuffer::create(int memory, int size, int index,
                                                   camera_buffer_t* ubuffer) {
    CheckAndLogError(ubuffer == nullptr, nullptr, "ubuffer is nullptr");
    LOG1("%s, memory type:%d, size:%d, index:%d", __func__, memory, size, index);

    std::shared_ptr<CameraBuffer> camBuffer =
        std::make_shared<CameraBuffer>(memory, size, index);
    CheckAndLogError(camBuffer == nullptr, nullptr, "@%s: fail to alloc CameraBuffer", __func__);

    camBuffer->setUserBufferInfo(ubuffer);
    camBuffer->updateFlags();

    return camBuffer;
}

// Helper function to construct a CameraBuffer from void* pointer
std::shared_ptr<CameraBuffer> CameraBuffer::create(int srcWidth, int srcHeight, int size,
                                                   int srcFmt, int index, void* buffer) {
    CheckAndLogError(buffer == nullptr, nullptr, "buffer is nullptr");
    LOG1("%s, width:%d, height:%d, size:%d, format:%d, index:%d", __func__, srcWidth, srcHeight,
         size, srcFmt, index);

    std::shared_ptr<CameraBuffer> camBuffer =
        std::make_shared<CameraBuffer>(V4L2_MEMORY_USERPTR, size, index);
    CheckAndLogError(camBuffer == nullptr, nullptr, "Fail to alloc CameraBuffer");

    camBuffer->setUserBufferInfo(srcFmt, srcWidth, srcHeight, buffer);

    return camBuffer;
}

// Internal frame Buffer
void CameraBuffer::setUserBufferInfo(int format, int width, int height) {
    mU->s.width = width;
    mU->s.height = height;
    mU->s.format = format;
    if (format != -1) {
        mU->s.stride = CameraUtils::getStride(format, width);
    }
}

// Memory type is V4L2_MEMORY_USERPTR which can use it
void CameraBuffer::setUserBufferInfo(int format, int width, int height, void* usrPtr) {
    setUserBufferInfo(format, width, height);
    mV.SetUserptr(reinterpret_cast<uintptr_t>(usrPtr), 0);
}

// Called when a buffer is from the application
void CameraBuffer::setUserBufferInfo(camera_buffer_t* ubuffer) {
    CheckAndLogError(ubuffer == nullptr, VOID_VALUE, "%s: ubuffer is nullptr", __func__);

    if ((mU->flags & BUFFER_FLAG_INTERNAL) != 0U) {
        delete mU;
    }
    mU = ubuffer;
    mBufferflag = mU->flags;

    mV.SetSequence(0);
    const struct timeval ts = {0};
    mV.SetTimestamp(ts);

    // update the v4l2 buffer memory with user info
    switch (ubuffer->s.memType) {
        case V4L2_MEMORY_USERPTR:
            mV.SetUserptr(reinterpret_cast<uintptr_t>(ubuffer->addr), 0);
            break;
        case V4L2_MEMORY_DMABUF:
            mV.SetFd(mU->dmafd, 0);
            mV.SetLength(mU->s.size, 0);
            break;
        case V4L2_MEMORY_MMAP:
            /* do nothing */
            break;
        default:
            LOGE("iomode %d is not supported yet.", mV.Memory());
            break;
    }

    if ((mU->s.streamType == CAMERA_STREAM_INPUT) || (ubuffer->sequence >= 0)) {
        // update timestamp if raw buffer is selected by user and timestamp is set
        if (ubuffer->timestamp > 0U) {
            struct timeval timestamp = {0};
            timestamp.tv_sec = ubuffer->timestamp / 1000000000ULL;
            timestamp.tv_usec = (ubuffer->timestamp - timestamp.tv_sec * 1000000000ULL) / 1000ULL;
            mV.SetTimestamp(timestamp);
        }
        mV.SetSequence(ubuffer->sequence);
        LOG2("%s, input buffer sequence %ld, timestamp %ld", __func__, ubuffer->sequence,
             ubuffer->timestamp);
    }
}

void CameraBuffer::updateV4l2Buffer(const v4l2_buffer_t& v4l2buf) {
    mV.SetField(v4l2buf.field);
    mV.SetTimestamp(v4l2buf.timestamp);
    mV.SetSequence(v4l2buf.sequence);
    mV.SetRequestFd(v4l2buf.request_fd);
}

/*export mmap buffer as dma_buf fd stored in mV and mU*/
int CameraBuffer::exportMmapDmabuf(V4L2VideoNode* vDevice) {
    std::vector<int> fds;

    int ret = vDevice->ExportFrame(mV.Index(), &fds);
    CheckAndLogError((ret != OK) || (fds.size() != 1), -1,
                     "exportMmapDmabuf failed, ret %d, fds size:%zu", ret, fds.size());
    mU->dmafd = fds[0];

    return OK;
}

int CameraBuffer::allocateMemory(V4L2VideoNode* vDevice) {
    int ret = BAD_VALUE;
    switch (mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            ret = allocateUserPtr();
            break;
        case V4L2_MEMORY_MMAP:
            (void)exportMmapDmabuf(vDevice);
            ret = allocateMmap(vDevice);
            break;
        case V4L2_MEMORY_DMABUF:
        default:
            LOGE("memory type %d is incorrect for allocateMemory.", mV.Memory());
            break;
    }

    if (ret == OK) {
        mAllocatedMemory = true;
    }

    return ret;
}

void CameraBuffer::freeMemory() {
    if (!mAllocatedMemory) {
        return;
    }

    switch (mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            freeUserPtr();
            break;
        case V4L2_MEMORY_MMAP:
            freeMmap();
            break;
        case V4L2_MEMORY_DMABUF:
        default:
            LOGE("Free camera buffer failed, due to memory %d type is not implemented yet.",
                 mV.Memory());
            break;
    }
}

int CameraBuffer::allocateUserPtr() {
    void* buffer = nullptr;
    const int ret = posix_memalign(&buffer, getpagesize(), mV.Length(0));
    CheckAndLogError(ret != 0, -1, "%s, posix_memalign fails, ret:%d", __func__, ret);
    mV.SetUserptr(reinterpret_cast<uintptr_t>(buffer), 0);
    return OK;
}

void CameraBuffer::freeUserPtr() {
    void* ptr = reinterpret_cast<void*>(mV.Userptr(0));
    ::free(ptr);
    mV.SetUserptr(reinterpret_cast<uintptr_t>(nullptr), 0);
}

int CameraBuffer::allocateMmap(V4L2VideoNode* dev) {
    std::vector<void*> addrs;
    int ret = dev->MapMemory(mV.Index(), PROT_READ | PROT_WRITE, MAP_SHARED, &addrs);
    CheckAndLogError((ret != OK) || (addrs.size() != 1), -1,
                     "allocateMmap failed, ret %d, addr size:%zu", ret, addrs.size());
    mU->addr = addrs[0];

    return OK;
}

void CameraBuffer::freeMmap() {
    if (mU->dmafd != -1) {
        ::close(mU->dmafd);
        mU->dmafd = -1;
    }
    if (mU->addr != nullptr) {
        const int ret = ::munmap(mU->addr, mV.Length(0));
        CheckAndLogError(ret != 0, VOID_VALUE, "failed to munmap buffer");
    }
}

#ifdef LIBDRM_SUPPORT_MMAP_OFFSET
CameraBuffer::DeviceRender::DeviceRender() : m_handle(-1) {
    m_handle = open("/dev/dri/renderD128", O_RDWR);
}

CameraBuffer::DeviceRender::DeviceRender(const char* path_file) : m_handle(-1) {
    m_handle = open(path_file, O_RDWR);
}

CameraBuffer::DeviceRender::~DeviceRender() {
    close(m_handle);
}

CameraBuffer::DeviceRender CameraBuffer::mDeviceRender("/dev/dri/renderD128");

void* CameraBuffer::DeviceRender::mapDmaBufferAddr(int fd, unsigned int bufferSize) {
    if (m_handle == -1) {
        LOGE("open device /dev/dri/renderD128 failed!\n");
        return MAP_FAILED;
    }

    int ret = 0;
    struct drm_prime_handle prime_handle;
    memset(&prime_handle, 0, sizeof(prime_handle));
    prime_handle.fd = fd;
    ret = drmIoctl(m_handle, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_handle);
    if (ret != 0) {
        LOGE("DRM_IOCTL_PRIME_FD_TO_HANDLE failed (fd=%u)\n", prime_handle.fd);
        return MAP_FAILED;
    }

    struct drm_xe_gem_mmap_offset gem_map = {0};
    gem_map.handle = prime_handle.handle;
    /* Get the fake offset back */
    ret = drmIoctl(m_handle, DRM_IOCTL_XE_GEM_MMAP_OFFSET, &gem_map);
    void* addr = MAP_FAILED;
    if (ret != 0)
        LOGE("DRM_IOCTL_XE_GEM_MMAP_OFFSET failed with errno: %s", strerror(errno));
    else
        addr = ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_handle,
                      gem_map.offset);

    return addr;
}
#endif

void* CameraBuffer::mapDmaBufferAddr(int fd, unsigned int bufferSize) {
    CheckAndLogError((fd < 0) || (bufferSize == 0U), nullptr, "%s, fd:0x%x, bufferSize:%u",
                     __func__, fd, bufferSize);

#ifdef LIBDRM_SUPPORT_MMAP_OFFSET
    return mDeviceRender.mapDmaBufferAddr(fd, bufferSize);
#else
    return ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
#endif
}

void CameraBuffer::unmapDmaBufferAddr(void* addr, unsigned int bufferSize) {
    CheckAndLogError((addr == nullptr) || (bufferSize == 0U), VOID_VALUE,
                     "%s, addr:%p, bufferSize:%u", __func__, addr, bufferSize);
    munmap(addr, bufferSize);
}

void CameraBuffer::updateUserBuffer(void) {
    mU->timestamp = TIMEVAL2NSECS(getTimestamp());
    mU->s.field = getField();

    // Use valid setting sequence to align shutter/parameter with buffer
    mU->sequence = (mSettingSequence < 0) ? getSequence() : static_cast<uint32_t>(mSettingSequence);
}

void CameraBuffer::updateFlags(void) {
    const uint32_t flag = static_cast<uint32_t>(V4L2_BUF_FLAG_NO_CACHE_INVALIDATE) | \
                          static_cast<uint32_t>(V4L2_BUF_FLAG_NO_CACHE_CLEAN);
    bool set = true;

    // clear the flags if the buffers is accessed by the SW
    if (((mU->flags & BUFFER_FLAG_SW_READ) != 0U) || ((mU->flags & BUFFER_FLAG_SW_WRITE) != 0U)) {
        set = false;
    }
    const auto f = static_cast<uint32_t>(mV.Flags());
    mV.SetFlags(set ? (f | flag) : (f & (~flag)));
}

bool CameraBuffer::isFlagsSet(uint32_t flag) const {
    return ((mU->flags & flag) ? true : false);
}

int CameraBuffer::getFd() {
    int ret = -1;
    switch (mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            ret = mV.Fd(0);
            break;
        case V4L2_MEMORY_DMABUF:
        case V4L2_MEMORY_MMAP:
            ret = mU->dmafd;
            break;
        default:
            LOGE("iomode %d is not supported yet.", mV.Memory());
            break;
    }
    return ret;
}

void* CameraBuffer::getBufferAddr() {
    void* ret = nullptr;
    switch (mV.Memory()) {
        case V4L2_MEMORY_USERPTR:
            ret =  reinterpret_cast<void*>(mV.Userptr(0));
            break;
        case V4L2_MEMORY_DMABUF:
        case V4L2_MEMORY_MMAP:
            ret =  mU->addr;
            break;
        default:
            LOGE("%s: Not supported memory type %u", __func__, mV.Memory());
            break;
    }
    return ret;
}

CameraBufferMapper::CameraBufferMapper(std::shared_ptr<CameraBuffer> buffer)
        : mBuffer(buffer),
          mDMAMapped(false) {
    if ((buffer->getMemory() == V4L2_MEMORY_DMABUF) &&
        (mBuffer->getUserBuffer()->addr == nullptr)) {
        void* addr = CameraBuffer::mapDmaBufferAddr(mBuffer->getFd(), mBuffer->getBufferSize());

        mBuffer->getUserBuffer()->addr = addr;
        mDMAMapped = true;
    }
}

CameraBufferMapper::~CameraBufferMapper() {
    if (mDMAMapped) {
        CameraBuffer::unmapDmaBufferAddr(mBuffer->getBufferAddr(), mBuffer->getBufferSize());

        mBuffer->getUserBuffer()->addr = nullptr;
    }
}

void* CameraBufferMapper::addr() {
    return mBuffer->getBufferAddr();
}

int CameraBufferMapper::size() {
    return mBuffer->getBufferSize();
}

}  // namespace icamera
