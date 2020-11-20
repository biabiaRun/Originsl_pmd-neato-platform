#include <getopt.h>
#include <unistd.h>

#include <CameraFactory.hpp>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <royale/ICameraDevice.hpp>
#include <string>
#include <thread>

#include "parameters.hpp"

// using namespace std;
// using namespace royale;
// using namespace platform;

// Image is 172 x 224 pixels
#define NUM_IMAGE_ELEMENTS 38528

namespace {
// pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
// pcl::PointCloud<pcl::PointXYZ>::Ptr cloudFiltered;
std::vector<cv::Point3f> pt_cloud;
std::mutex cloudMutex;
std::condition_variable cloudCV;
bool newDataAvailable;
}  // namespace

std::string VERSION{"1.1"};
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

typedef struct {
  std::string version;
  royale::String test_mode;
} options_t;

struct plane_eq {
  float a, b, c, d;
};

class MyListener : public royale::IExtendedDataListener {
 public:
  MyListener() {}

  void onNewData(const royale::IExtendedData* data) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    pt_cloud.clear();
    if (data->hasDepthData()) {
      auto depth = data->getDepthData();
      for (size_t i = 0u; i < NUM_IMAGE_ELEMENTS; ++i) {
        if (depth->points[i].depthConfidence > 0) {
          pt_cloud.push_back(cv::Point3f(depth->points[i].x, depth->points[i].y, depth->points[i].z));
        }
      }
    }
    newDataAvailable = true;
    cloudCV.notify_all();
  }
};

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

int main(int argc, char** argv) {
  int opt;
  // Default options
  options_t options = {VERSION, "MODE_9_5FPS"};
  std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";
  royale::String useCase;
  std::vector<plane_eq> plane_coeffs;

  if (argc > 1) {
    while ((opt = getopt(argc, argv, "m")) != -1) {
      switch (opt) {
        case 'm':
          //  "-m <str>             Set ToF mode: -m MODE_9_5FPS\n"
          options.test_mode = royale::String(optarg);
          break;
        default:
          break;
      }
    }
  }
  useCase = options.test_mode;

  pt_cloud.reserve(NUM_IMAGE_ELEMENTS);

  // [Setup] Camera Initialization
  std::unique_ptr<MyListener> listener_;
  newDataAvailable = false;
  std::unique_ptr<royale::ICameraDevice> camera_;  // The camera device
  platform::CameraFactory factory;
  camera_ = factory.createCamera();

  // Test if CameraDevice was created
  if (camera_ == nullptr) {
    std::cerr << "[ERROR] Camera device could not be created." << std::endl;
    return CAM_NOT_CREATED;
  }
  // Initialize()
  royale::CameraStatus status = camera_->initialize();
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Camera device could not be initialized. " << royale::getStatusString(status).c_str()
              << std::endl;
    return CAM_NOT_INITIALIZED;
  }

  royale::String current_use_case;
  camera_->getCurrentUseCase(current_use_case);
  if (current_use_case != useCase) {
    royale::Vector<royale::String> useCaseList;
    status = camera_->getUseCases(useCaseList);
    if (status != royale::CameraStatus::SUCCESS || useCaseList.empty()) {
      std::cerr << "[ERROR] Could not get use cases. " << royale::getStatusString(status).c_str() << std::endl;
      return USE_CASE_ERROR;
    }
    for (auto i = 0u; i < useCaseList.size(); ++i) {
      if (useCaseList.at(i) == useCase) {
        status = camera_->setUseCase(useCaseList.at(i));
        if (status != royale::CameraStatus::SUCCESS) {
          std::cerr << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   "
                    << royale::getStatusString(status).c_str() << std::endl;
          return USE_CASE_ERROR;
        } else {
          std::cout << "SETTING USE CASE :" << useCaseList[i].c_str() << std::endl;
        }
      }
    }
  }

  listener_.reset(new MyListener());
  status = camera_->registerDataListenerExtended(listener_.get());
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Could not register the extended data listener" << royale::getStatusString(status).c_str()
              << std::endl;
    return LISTENER_MODE_ERROR;
  }

  status = camera_->setExposureMode(royale::ExposureMode::MANUAL);
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Could not set exposure to manual" << royale::getStatusString(status).c_str() << std::endl;
    return EXPOSURE_MODE_ERROR;
  }

  std::pair<int, int> min_max_exposures = getExposureMinMax(useCase);
  status = camera_->setExposureTime(min_max_exposures.second);
  if (status != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set Manual Exposure to: " << min_max_exposures.second << std::endl;
    return EXPOSURE_MODE_ERROR;
  }

  // Start Capture
  status = camera_->startCapture();
  if (status != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not start video capture." << std::endl;
    return CAPTURE_START_ERROR;
  }
  // capture for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(5));

  if (camera_->setProcessingParameters({ToF_test_params::USE_ADAPTIVE_NOISE_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_ADAPTIVE_NOISE_FILTER." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_FLYING_PIXEL}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FLYING_PIXEL." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_MPI_AVERAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AVERAGE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_MPI_AMP}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AMP." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_MPI_DIST}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_DIST." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_VALIDATE_IMAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_VALIDATE_IMAGE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_STRAY_LIGHT}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_STRAY_LIGHT." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_FILTER_2_FREQ}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FILTER_2_FREQ." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_SBI_FLAG}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SBI_FLAG." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_SMOOTHING_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SMOOTHING_FILTER." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::USE_HOLE_FILLING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_HOLE_FILLING." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::NOISE_THRESHOLD}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set NOISE_THRESHOLD." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::AUTO_EXPOSURE_REF_VALUE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set AUTO_EXPOSURE_REF_VALUE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::ADAPTIVE_NOISE_FILTER_TYPE}) !=
      royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set ADAPTIVE_NOISE_FILTER_TYPE." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  if (camera_->setProcessingParameters({ToF_test_params::GLOBAL_BINNING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set GLOBAL_BINNING." << std::endl;
    return PROCESSING_PARAMETER_ERROR;
  }
  // warm up camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::vector<cv::Mat> vec_pt_cloud;
  int num_frames_collected = 0;
  int test_sample_size = 20;

  while (num_frames_collected < test_sample_size) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      vec_pt_cloud.push_back(cv::Mat(pt_cloud));
      num_frames_collected++;
      newDataAvailable = false;
    }
  }

  for (size_t frame_index = 0u; frame_index < vec_pt_cloud.size(); ++frame_index) {
    cv::Mat& frame_pt_cloud = vec_pt_cloud[frame_index];

    cv::Scalar ux = cv::mean(frame_pt_cloud.col(0));
    cv::Scalar uy = cv::mean(frame_pt_cloud.col(1));
    cv::Scalar uz = cv::mean(frame_pt_cloud.col(2));
    float fux = static_cast<float>(ux.val[0]);
    float fuy = static_cast<float>(uy.val[0]);
    float fuz = static_cast<float>(uz.val[0]);
    cv::Mat uxyz(1, 3, CV_32F);
    uxyz.at<float>(0, 0) = fux;
    uxyz.at<float>(0, 1) = fuy;
    uxyz.at<float>(0, 2) = fuz;

    cv::Mat uMat = cv::repeat(uxyz, frame_pt_cloud.rows, 1);

    cv::Mat frame_pt_cloud_bar = frame_pt_cloud - uMat;
    cv::Mat CC = frame_pt_cloud_bar.t() * frame_pt_cloud_bar;
    CC = CC / (static_cast<float>(frame_pt_cloud.rows) - 1.0f);
    cv::Mat e_values, e_vectors;
    cv::eigen(CC, e_values, e_vectors);
    float a = e_vectors.at<float>(2, 0);
    float b = e_vectors.at<float>(2, 1);
    float c = e_vectors.at<float>(2, 2);
    float d = a * fux + b * fuy + c * fuz;

    plane_eq cur_plane{a, b, c, d};
    plane_coeffs.push_back(cur_plane);
    // cv::SVD svdCC(CC, cv::SVD::MODIFY_A | cv::SVD::FULL_UV);
  }

  if (camera_->stopCapture() != royale::CameraStatus::SUCCESS) {
    std::cout << "Error stopping camera" << std::endl;
  }

  // Reset the frame count
  // int current_frame_count = cam.listener_->m_count;

  return 0;
}
