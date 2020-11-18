#include <iostream>
#include "stdlib.h"

#include "camera-testing.h"
#include "parameters.hpp"

using namespace royale;
using namespace platform;

std::pair<int, int> getExposureMinMax(const royale::String& tof_mode) {
    if (tof_mode == "MODE_9_5FPS") {
        return std::pair<int, int>(8, 1880);
    } else if (tof_mode == "MODE_9_10FPS") {
        return std::pair<int, int>(8, 1070);
    } else if (tof_mode == "MODE_9_15FPS") {
        return std::pair<int, int>(8, 710);
    } else if (tof_mode == "MODE_9_20FPS") {
        return std::pair<int, int>(8, 530);
    } else if (tof_mode == "MODE_9_30FPS") {
        return std::pair<int, int>(8, 350);
    } else if (tof_mode == "MODE_5_5FPS") {
        return std::pair<int, int>(8, 3500);
    } else if (tof_mode == "MODE_5_10FPS") {
        return std::pair<int, int>(8, 1750);
    } else if (tof_mode == "MODE_5_15FPS") {
        return std::pair<int, int>(8, 1430);
    } else if (tof_mode == "MODE_5_30FPS") {
        return std::pair<int, int>(8, 710);
    } else if (tof_mode == "MODE_5_45FPS") {
        return std::pair<int, int>(8, 470);
    } else if (tof_mode == "MODE_5_60FPS") {
        return std::pair<int, int>(8, 350);
    } else {
        return std::pair<int, int>(8, 350);
    }
}

Camera::CameraError Camera::InitializeCamera(royale::String useCase) {
  // Test if CameraDevice was created
  if(camera_ == nullptr) {
      std::cerr << "[ERROR] Camera device could not be created." << std::endl;
      return CAM_NOT_CREATED;
  }
  // Initialize()
  royale::CameraStatus status = camera_->initialize();
  if (status != royale::CameraStatus::SUCCESS) {
      std::cerr << "[ERROR] Camera device could not be initialized. "
                << royale::getStatusString(status).c_str() << std::endl;
      return CAM_NOT_INITIALIZED;
  }
  use_case_ = useCase;
  royale::String current_use_case;
  camera_->getCurrentUseCase(current_use_case);
  if ( current_use_case != useCase) {
    royale::Vector<royale::String> useCaseList;
    status = camera_->getUseCases (useCaseList);
    if (status != royale::CameraStatus::SUCCESS || useCaseList.empty()) {
        std::cerr << "[ERROR] Could not get use cases. "
                  << royale::getStatusString(status).c_str() << std::endl;
        return USE_CASE_ERROR;
    }

    for (auto i = 0u; i < useCaseList.size(); ++i) {
      if (useCaseList.at(i) == use_case_) {
        status = camera_->setUseCase(useCaseList.at(i));
        if (status != royale::CameraStatus::SUCCESS) {
            std::cerr << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   "
                  << royale::getStatusString(status).c_str() << std::endl;
            return USE_CASE_ERROR;
        }else{
          std::cout << "SETTING USE CASE :" << useCaseList[i].c_str() << std::endl;
          status = camera_->getFrameRate(fps_);
          if (status != royale::CameraStatus::SUCCESS) {
              std::cerr << "[ERROR] Could not get camera frame rate. "
                        << royale::getStatusString(status).c_str() << std::endl;
              return USE_CASE_ERROR;
          }
        }
      }
    }
  } else {
    status = camera_->getFrameRate(fps_);
    if (status != royale::CameraStatus::SUCCESS) {
      std::cerr << "[ERROR] Could not get camera frame rate. "
                << royale::getStatusString(status).c_str() << std::endl;
      return USE_CASE_ERROR;
    }
  }

  listener_.reset(new MyListener());
  status = camera_->registerDataListenerExtended(listener_.get());
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Could not register the extended data listener"
                << royale::getStatusString(status).c_str() << std::endl;
    return LISTENER_MODE_ERROR;
  }

  status = camera_->setExposureMode(royale::ExposureMode::MANUAL);
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Could not set exposure to manual"
                << royale::getStatusString(status).c_str() << std::endl;
    return EXPOSURE_MODE_ERROR;
  }

  std::pair<int, int> min_max_exposures =  getExposureMinMax(useCase);
  status = camera_->setExposureTime(min_max_exposures.second);
  if ( status != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set Manual Exposure to: " << min_max_exposures.second << std::endl;
    return EXPOSURE_MODE_ERROR;
  }

  // Start Capture
  status = camera_->startCapture();
  if ( status != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not satrt video capture." << std::endl;
    return CAPTURE_START_ERROR;
  }
  // capture for a few seconds
  std::this_thread::sleep_for (std::chrono::seconds (5));

  if (camera_->setProcessingParameters({ ToFparams::USE_ADAPTIVE_NOISE_FILTER }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_ADAPTIVE_NOISE_FILTER." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_FLYING_PIXEL }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FLYING_PIXEL." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_MPI_AVERAGE }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AVERAGE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_MPI_AMP }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AMP." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_MPI_DIST }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_DIST." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_VALIDATE_IMAGE }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_VALIDATE_IMAGE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_STRAY_LIGHT }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_STRAY_LIGHT." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_FILTER_2_FREQ }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FILTER_2_FREQ." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_SBI_FLAG }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SBI_FLAG." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_SMOOTHING_FILTER }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SMOOTHING_FILTER." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::USE_HOLE_FILLING }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_HOLE_FILLING." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::NOISE_THRESHOLD }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set NOISE_THRESHOLD." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::AUTO_EXPOSURE_REF_VALUE }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set AUTO_EXPOSURE_REF_VALUE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::ADAPTIVE_NOISE_FILTER_TYPE }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set ADAPTIVE_NOISE_FILTER_TYPE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ ToFparams::GLOBAL_BINNING }) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set GLOBAL_BINNING." << std::endl;
    return PROCESSING_PARAMETER_ERROR;

  // capture for a few seconds
  std::this_thread::sleep_for (std::chrono::seconds (5));
}
