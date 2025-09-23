/*
 * Copyright (C) 2024 Intel Corporation
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
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <desktop/face_detector_c_abi.h>
#include <desktop/face_detector_types.h>

#include "src/fd/facessd/CommonTypes.h"

namespace icamera {

enum class FaceDetectResult {
  kDetectOk,
  kDetectError,
  kBufferError,
  kTransformError,
  kTimeoutError,
};

// This class encapsulates Google3 FaceSSD library. Only support gray type.
class FaceDetector {
 public:
  static std::unique_ptr<FaceDetector> Create();

  ~FaceDetector();

  FaceDetectResult Detect(const uint8_t* buffer_addr, int input_stride, Size input_size,
              std::optional<Size> active_sensor_array_size,
              std::vector<DetectedFace>& faces);

  static std::optional<std::tuple<float, float, float>> GetCoordinateTransform(
      const Size src, const Size dst);

 private:
  explicit FaceDetector(FaceDetectorHandle face_detector_handle);

  void PrepareBuffer(Size img_size);

  std::vector<uint8_t> scaled_buffer_;

  FaceDetectorHandle face_detector_handle_;
};

std::string LandmarkTypeToString(LandmarkType type);

}  // namespace icamera
