/*
 * Copyright (C) 2016-2022 Intel Corporation
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

#include <cstdint>

namespace icamera {

// Temporary solution
enum PipeUseCase : uint32_t {
    USE_CASE_COMMON = 0,
    USE_CASE_VIDEO = 1 << 0,
    USE_CASE_STILL = 1 << 1,
    USE_CASE_VIDEO_RECORDING = 1 << 9,  // has special setting for video encoder
};

struct streamProps {
    uint32_t width;
    uint32_t height;
    int format;
    int streamId;  // user stream id
    PipeUseCase useCase;
};

class HalStream
{
 public:
    HalStream() : mWidth(0), mHeight(0), mFormat(0), mStreamId(0), mUseCase(USE_CASE_VIDEO), maxBuffers(0), mPrivate(nullptr) {}
    HalStream(struct streamProps &props, void *priv) :
        mWidth(props.width),
        mHeight(props.height),
        mFormat(props.format),
        mStreamId(props.streamId),
        mUseCase(props.useCase)
    {
        maxBuffers = 0;
        mPrivate = priv;
    }

    ~HalStream() { }

    uint32_t width() const { return mWidth; }
    uint32_t height() const { return mHeight; }
    int format() const { return mFormat; }
    int streamId() const { return mStreamId; }
    PipeUseCase useCase() const { return mUseCase; }
    void *priv() { return mPrivate; }

 public:
    uint32_t mWidth;
    uint32_t mHeight;
    int mFormat;
    int mStreamId;  // user stream id
    PipeUseCase mUseCase;

    int maxBuffers;
    void *mPrivate;
};

} /* namespace icamera */
