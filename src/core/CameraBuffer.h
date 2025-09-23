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

#pragma once

#include <memory>
#include <queue>
#include <vector>

#include <linux/videodev2.h>
#include <v4l2_device.h>
#include "ParamDataType.h"
#include "iutils/Utils.h"

namespace icamera {

typedef struct v4l2_buffer v4l2_buffer_t;

class CameraBuffer {
 public:
    /* Construct an internal CameraBuffer, use memory to indicate the memory type.
     * memory: V4L2_MEMORY_USERPTR heap buffer
     *         V4L2_MEMORY_MMAP    mmap buffer
     *         V4L2_MEMORY_DMABUF  camera APP buffer
     */
    static std::shared_ptr<CameraBuffer> create(int memory, unsigned int size, int index,
                                                int srcFmt, int srcWidth, int srcHeight);
    // Construct a CameraBuffer from camera_buffer_t pointer.
    static std::shared_ptr<CameraBuffer> create(int memory, int size, int index,
                                                camera_buffer_t* ubuffer);
    // Construct a CameraBuffer from void* pointer.
    static std::shared_ptr<CameraBuffer> create(int srcWidth, int srcHeight, int size, int srcFmt,
                                                int index, void* buffer);

    CameraBuffer(int memory, uint32_t size, int index);
    virtual ~CameraBuffer();

    // user buffer information
    int getWidth() const { return mU->s.width; }
    int getHeight() const { return mU->s.height; }
    int getStride() const { return mU->s.stride; }
    int getFormat() const { return mU->s.format; }
    int getStreamType() const { return mU->s.streamType; }
    int getStreamUsage() const { return mU->s.usage; }
    int getStreamId() const { return mU->s.id; }
    int32_t getFlags() const { return mU->flags; }
    // v4l2 buffer information
    uint32_t getIndex(void) const { return mV.Index(); }

    uint32_t getSequence(void) const { return mV.Sequence(); }
    void setSequence(uint32_t sequence) { mV.SetSequence(sequence); }

    uint32_t getField() const { return mV.Field(); }
    void setField(uint32_t field) { mV.SetField(field); }

    struct timeval getTimestamp(void) const {
        return mV.Timestamp();
    }
    void setTimestamp(struct timeval timestamp) { mV.SetTimestamp(timestamp); }

    // Use for GFX/DMA/GBM buffer
    int getFd();

    uint32_t getMemory(void) const { return mV.Memory(); }

    // For debug only v4l2 buffer information
    int getCsi2Port(void) const { return (mV.RequestFd() >> 4) & 0xf; }
    int getVirtualChannel(void) const { return mV.RequestFd() & 0xf; }

    /* u buffer is used to attach user private structure pointer
     * in CameraBuffer.
     *
     * Now, one of this usage is linking camera_buffer_t to CameraBuffer
     * together, so that we can get each pointer by other.
     * Notes: Please don't abuse this. It is only used in CameraDevice for user buffer
     */
    camera_buffer_t* getUserBuffer() { return mU; }
    // update the user  buffer with latest v4l2 buffer info from driver
    void updateUserBuffer(void);
    // Update the v4l2 flags according to user buffer flag
    void updateFlags(void);

    void setUserBufferFlags(int flags) { mBufferflag = flags; }
    // Check if the specific flag in "mU->flags" is set or not
    bool isFlagsSet(uint32_t flag) const;
    // The ubuffer is from application
    void setUserBufferInfo(camera_buffer_t* ubuffer);
    void setUserBufferInfo(int format, int width, int height);

    uint32_t getBufferSize() { return mV.Length(0); }
    void setBufferSize(unsigned int size) { mV.SetLength(size, 0); }

    unsigned int getBytesused() { return mV.BytesUsed(0); }
    void setBytesused(unsigned int bytes) { mV.SetBytesUsed(bytes, 0); }
    // Addr is used by userptr and DMA buffer
    void* getBufferAddr();

    void updateV4l2Buffer(const v4l2_buffer_t& v4l2buf);

    V4L2Buffer& getV4L2Buffer() { return mV; }

    bool isExtDmaBuf() {
      return (mBufferflag & BUFFER_FLAG_DMA_EXPORT) != 0;
    }

    bool isNeedFlush() {
      return (mBufferflag & (BUFFER_FLAG_SW_READ | BUFFER_FLAG_SW_WRITE)) != 0;
    }

    bool isInternalBuffer() {
      return (mBufferflag & BUFFER_FLAG_INTERNAL) != 0;
    }

    void setSettingSequence(int64_t sequence) { mSettingSequence = sequence; }
    int64_t getSettingSequence() const { return mSettingSequence; }

    // Buffers are allocated the buffers by Camera
    int allocateMemory(V4L2VideoNode* vDevice = nullptr);

    static void* mapDmaBufferAddr(int fd, unsigned int bufferSize);
    static void unmapDmaBufferAddr(void* addr, unsigned int bufferSize);

 private:
    CameraBuffer(const CameraBuffer&);
    CameraBuffer& operator=(const CameraBuffer&);

    void setUserBufferInfo(int format, int width, int height, void* usrPtr);

    void freeMemory();
    int exportMmapDmabuf(V4L2VideoNode* vDevice);
    int allocateMmap(V4L2VideoNode* dev);
    void freeMmap();
    int allocateUserPtr();
    void freeUserPtr();

    // Use for GFX/DMA/GBM buffer
    void setFd(int val);

 protected:
    V4L2Buffer mV;

 private:
    // To tag whether the memory is allocated by CameraBuffer class. We need to free them
    bool mAllocatedMemory;

    uint32_t mBufferflag;
    camera_buffer_t* mU;
    int64_t mSettingSequence;

    void* mMmapAddrs;

#ifdef LIBDRM_SUPPORT_MMAP_OFFSET
    class DeviceRender {
     public:
        DeviceRender();
        explicit DeviceRender(const char* path_file);
        ~DeviceRender();
        void* mapDmaBufferAddr(int fd, unsigned int bufferSize);

     private:
        int m_handle;
    };

    static DeviceRender mDeviceRender;
#endif
};

typedef std::vector<std::shared_ptr<CameraBuffer> > CameraBufVector;
typedef std::queue<std::shared_ptr<CameraBuffer> > CameraBufQ;

class CameraBufferMapper {
 public:
    explicit CameraBufferMapper(std::shared_ptr<CameraBuffer> buffer);
    ~CameraBufferMapper();

    void* addr();
    int size();

 private:
    std::shared_ptr<CameraBuffer> mBuffer;
    bool mDMAMapped;
};

}  // namespace icamera
