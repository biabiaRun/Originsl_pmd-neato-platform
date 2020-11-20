#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <CameraFactory.hpp>
#include <chrono>
#include <royale/ICameraDevice.hpp>
#include <set>
#include <thread>

class Camera {
 public:
  // Camera() : camera_(nullptr) {}
  Camera() {
    platform::CameraFactory factory;
    camera_ = factory.createCamera();
  }

  std::unique_ptr<royale::ICameraDevice> camera_;  // The camera device
  std::unique_ptr<MyListener> listener_;
  std::string id_;              // Unique ID for the camera device
  royale::String use_case_;     // Camera use_case_
  royale::StreamId stream_id_;  // FIRST stream ID for the given use_case_
  uint16_t fps_;

  enum CameraError {
    NONE = 0,
    CAM_NOT_DETECTED,
    CAM_NOT_CREATED,
    CAM_NOT_INITIALIZED,
    CAM_STREAM_ERROR,
    ACCESS_LEVEL_ERROR,
    LISTENER_MODE_ERROR,
    EXPOSURE_MODE_ERROR,
    CAPTURE_START_ERROR,
    USE_CASE_ERROR,
    PROCESSING_PARAMETER_ERROR,
    LENS_PARAMETER_ERROR,
    RECEIVE_DATA_ERROR,
  };

  inline const std::string GetID() const { return id_; }

  CameraError InitializeCamera(royale::String useCase);
};

#endif  // __CAMERA_H__
