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

#define LOG_TAG FaceSSD
#include "src/fd/facessd/CameraFaceDetection.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <optional>
#include <string>

#include <desktop/face_detector_c_abi.h>
#include <desktop/face_detector_types.h>
#include <libyuv.h>

#include "iutils/CameraLog.h"
#include "src/fd/facessd/CommonTypes.h"

namespace icamera {

namespace {

constexpr int kImageSizeForDetection = 160;

}  // namespace

// static
std::unique_ptr<FaceDetector> FaceDetector::Create() {
  auto face_detector_handle = face_detector_create();

  // store the model and anchor path
  if (!face_detector_initialize(face_detector_handle)) {
    LOGE("Failed to initialize FaceSSD detector");
    return nullptr;
  }
  return std::unique_ptr<FaceDetector>(new FaceDetector(face_detector_handle));
}

FaceDetector::~FaceDetector() {
  face_detector_delete(face_detector_handle_);
}

FaceDetector::FaceDetector(FaceDetectorHandle face_detector_handle)
    : face_detector_handle_(face_detector_handle) {};

FaceDetectResult FaceDetector::Detect(
    const uint8_t* buffer_addr, int input_stride, Size input_size,
    std::optional<Size> active_sensor_array_size,
    std::vector<DetectedFace>& faces) {
  Size scaled_size =
      (input_size.width > input_size.height)
          ? Size(kImageSizeForDetection,
                 kImageSizeForDetection * input_size.height / input_size.width)
          : Size(kImageSizeForDetection * input_size.width / input_size.height,
                 kImageSizeForDetection);

  PrepareBuffer(scaled_size);

  libyuv::ScalePlane(buffer_addr, input_stride, input_size.width,
                     input_size.height, scaled_buffer_.data(),
                     scaled_size.width, scaled_size.width, scaled_size.height,
                     libyuv::FilterMode::kFilterNone);

  {
    size_t num_faces = 0;
    faces.clear();
    faces.resize(MAX_NUM_FACES);
    if (!face_detector_detect(face_detector_handle_, scaled_buffer_.data(),
                              scaled_size.width, scaled_size.height,
                              faces.data(), &num_faces)) {
      return FaceDetectResult::kDetectError;
    }
    faces.resize(num_faces);
  }

  if (!faces.empty()) {
    float ratio = static_cast<float>(input_size.width) /
                  static_cast<float>(scaled_size.width);
    for (auto& f : faces) {
      f.bounding_box.x1 *= ratio;
      f.bounding_box.y1 *= ratio;
      f.bounding_box.x2 *= ratio;
      f.bounding_box.y2 *= ratio;
      for (auto& l : f.landmarks) {
        l.x *= ratio;
        l.y *= ratio;
      }
    }
  }

  if (active_sensor_array_size) {
    std::optional<std::tuple<float, float, float>> transform =
        GetCoordinateTransform(input_size, *active_sensor_array_size);
    if (!transform) {
      return FaceDetectResult::kTransformError;
    }
    const float scale = std::get<0>(*transform);
    const float offset_x = std::get<1>(*transform);
    const float offset_y = std::get<2>(*transform);
    for (auto& f : faces) {
      f.bounding_box.x1 = scale * f.bounding_box.x1 + offset_x;
      f.bounding_box.y1 = scale * f.bounding_box.y1 + offset_y;
      f.bounding_box.x2 = scale * f.bounding_box.x2 + offset_x;
      f.bounding_box.y2 = scale * f.bounding_box.y2 + offset_y;
      for (auto& l : f.landmarks) {
        l.x = scale * l.x + offset_x;
        l.y = scale * l.y + offset_y;
      }
    }
  }
  return FaceDetectResult::kDetectOk;
}

// static
std::optional<std::tuple<float, float, float>>
FaceDetector::GetCoordinateTransform(const Size src, const Size dst) {
  if (src.width > dst.width || src.height > dst.height) {
    return std::nullopt;
  }
  const float width_ratio = static_cast<float>(dst.width) / src.width;
  const float height_ratio = static_cast<float>(dst.height) / src.height;
  const float scaling = std::min(width_ratio, height_ratio);
  float offset_x = 0.0f, offset_y = 0.0f;
  if (width_ratio < height_ratio) {
    // |dst| has larger height than |src| * scaling.
    offset_y = (dst.height - (src.height * scaling)) / 2;
  } else {
    // |dst| has larger width than |src| * scaling.
    offset_x = (dst.width - (src.width * scaling)) / 2;
  }
  return std::make_tuple(scaling, offset_x, offset_y);
}

void FaceDetector::PrepareBuffer(Size img_size) {
  size_t new_size = img_size.width * img_size.height;
  if (new_size > scaled_buffer_.size()) {
    scaled_buffer_.resize(new_size);
  }
}

std::string LandmarkTypeToString(LandmarkType type) {
  switch (type) {
    case LANDMARK_LEFT_EYE:
      return "LeftEye";
    case LANDMARK_RIGHT_EYE:
      return "RightEye";
    case LANDMARK_NOSE_TIP:
      return "NoseTip";
    case LANDMARK_MOUTH_CENTER:
      return "MouthCenter";
    case LANDMARK_LEFT_EAR_TRAGION:
      return "LeftEarTragion";
    case LANDMARK_RIGHT_EAR_TRAGION:
      return "RightEarTragion";
    default:
      return "Unknown";
  }
}

}  // namespace icamera
