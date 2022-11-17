/*
 * Copyright (c) 2019, Neato Robotics, Inc.. All Rights Reserved.
 *
 * This file may contain contributions from others.
 *
 * This software is proprietary to Neato Robotics, Inc. and its transference
 * and use is to be strictly controlled.
 * Transference of this software to another party requires that all of the
 * following conditions be met:
 * 	A)	Neato has a copy of a signed NDA agreement with the receiving
 *      party
 * 	B)	Neato Software Engineering has explicitly authorized the
 *      receiving party to have a copy of this software
 * 	C)	When the work is completed or terminated by the receiving party,
 *      all copies of this software that the receiving party holds must be
 *      returned to Neato, or destroyed.
 * The receiving party is under legal obligation to not disclose or  transfer
 * this software.
 * The receiving party may not appropriate, transform or re-use this software
 * for any purpose other than a Neato Robotics authorized purpose.
 */

/**
 * Calibration application for aligning the ToF camera coordiante frame to the LDS coordinate frame.
 * The Prime or VRxx assembled robot is placed into the 270mm distance fixture.  ToF intensity images are
 * collected with the LDS spinning and the LDS signal reflecting off the walls is path is segmented.
 * A line segment is fit to the laser path and the 3D location of the segment endpoints are recorded.
 * The position of the LDS with respect to the ToF sensor as described in the design CAD is combined
 * with the two endpoint positions to form a plane which spans the LDS X,Y coordiante system.
 * The span of the X,Z axes of the ToF form a second plane and a basis transformation is calculated
 * from the 2 planes.  The result is a 4x4 transformation matrix which is saved in /user/tof_camera.conf
 * file.  The 16 coefficients are formated as one per line.
 */

#include <CameraFactory.hpp>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <royale/ICameraDevice.hpp>
#include <string>

#include "parameters.hpp"
#include "running_stat.hpp"

// Image is 172 x 224 pixels
#define NUM_IMAGE_ELEMENTS 38528

// Simplify memory handeling for testing
namespace {
// The camera device
std::unique_ptr<royale::ICameraDevice> camera_;
// Running Mean and Standard Deviation collections
// Intensity Data
std::vector<RunningStat> pt_intensity_running_stat;
// ToF 3D X/Y/Z values in meters
std::vector<RunningStat> pt_x_running_stat;
std::vector<RunningStat> pt_y_running_stat;
std::vector<RunningStat> pt_z_running_stat;
// ToF Depth Confidence Level, royale::DepthPoint.depthConfidence
// A value from 0 (invalid) to 255 (full confidence)
std::vector<RunningStat> confident_pixel;
std::mutex cloudMutex;
std::condition_variable cloudCV;
bool newDataAvailable;
}  // namespace

std::string VERSION{"1.1"};

//  When exiting application stop LDS and Camera.
void terminate() {
  const char *stop_lds_command = ToF_calibration_params::kStopLdsString.c_str();
  system(stop_lds_command);
  if (camera_) {
    bool camera_is_capturing = false;
    camera_->isCapturing(camera_is_capturing);
    if (camera_is_capturing) {
      if (camera_->stopCapture() != royale::CameraStatus::SUCCESS) {
        std::cout << "Error stopping camera" << std::endl;
      }
    }
  }
}

// A timer class to be used as a stopwatch
class Timer {
public:
  void start() {
    m_StartTime = std::chrono::steady_clock::now();
    m_bRunning = true;
  }

  void stop() {
    m_EndTime = std::chrono::steady_clock::now();
    m_bRunning = false;
  }

  double elapsedMilliseconds() {
    std::chrono::time_point<std::chrono::steady_clock> endTime;
    if (m_bRunning) {
      endTime = std::chrono::steady_clock::now();
    } else {
      endTime = m_EndTime;
    }
    return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count());
  }

  int elapsedSeconds() {
    return static_cast<int>(elapsedMilliseconds() / 1000.);
  }

private:
  std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
  std::chrono::time_point<std::chrono::steady_clock> m_EndTime;
  bool m_bRunning = false;

};

// A class for a standard 3D plane defined by a* x + b* y + c* z + d = 0
class Plane {
 public:
  Plane();
  Plane(const Plane &P);

  // Creates the plane from a point on the surface and a normal vector
  void ConstructFromPointNormal(const cv::Point3f &Pt, const cv::Point3f &Normal);

  cv::Point3f origin;   // Origin of the plane
  cv::Point3f vector1;  // Two orthogonal vectors that span the plane
  cv::Point3f vector2;
  cv::Point3f normal;  // The plane normal vector
};

Plane::Plane() {
  origin = cv::Point3f(0.0f, 0.0f, 0.0f);
  vector1 = cv::Point3f(1.0f, 0.0f, 0.0f);
  vector2 = cv::Point3f(0.0f, 1.0f, 0.0f);
  normal = cv::Point3f(0.0f, 0.0f, 1.0f);
}

Plane::Plane(const Plane &P) {
  origin = P.origin;
  vector1 = P.vector1;
  vector2 = P.vector2;
  normal = P.normal;
}

// Construct a plane given a point on the plane and vector normal to the plane
void Plane::ConstructFromPointNormal(const cv::Point3f &pt, const cv::Point3f &normal_vector) {
  Plane result;
  result.origin = pt;
  result.normal = normal_vector;
  result.normal = result.normal / cv::norm(result.normal);
  cv::Point3f V0(1.0f, 0.0f, 0.0f);
  result.vector1 = result.normal.cross(V0);
  result.vector1 = result.vector1 / cv::norm(result.vector1);
  if (cv::norm(result.vector1) < 1e-14) {
    V0 = cv::Point3f(0.0f, 1.0f, 0.0f);
    result.vector1 = result.normal.cross(V0);
    result.vector1 = result.vector1 / cv::norm(result.vector1);
  }
  result.vector2 = result.vector1.cross(result.normal);
  result.vector2 = result.vector2 / cv::norm(result.vector2);
  result.vector2 = -result.vector2;

  this->origin = result.origin;
  this->vector1 = result.vector1;
  this->vector2 = result.vector2;
  this->normal = result.normal;
}

// Return a 4x4 transformation matrix that takes you from the source basis to the target basis
cv::Mat CreateBasisTransfrom3D(const Plane &source, const Plane &target) {
  cv::Mat t2 = cv::Mat::eye(4, 4, CV_32F);
  cv::Mat_<float> I = (cv::Mat_<float>(1, 4) << 0.0f, 0.0f, 0.0f, 1.0f);

  // Put target plane into a t - form matrix.
  t2.at<float>(0, 0) = target.vector1.x;
  t2.at<float>(1, 0) = target.vector1.y;
  t2.at<float>(2, 0) = target.vector1.z;
  t2.at<float>(0, 1) = target.vector2.x;
  t2.at<float>(1, 1) = target.vector2.y;
  t2.at<float>(2, 1) = target.vector2.z;
  t2.at<float>(0, 2) = target.normal.x;
  t2.at<float>(1, 2) = target.normal.y;
  t2.at<float>(2, 2) = target.normal.z;
  t2.at<float>(0, 3) = target.origin.x;
  t2.at<float>(1, 3) = target.origin.y;
  t2.at<float>(2, 3) = target.origin.z;

  // Compute the transform matrix
  // Form coordinate of 4 reference points in the source basis
  cv::Point3f p0 = source.origin;
  cv::Point3f px = p0 + source.vector1;
  cv::Point3f py = p0 + source.vector2;
  cv::Point3f pz = p0 + source.normal;

  // Express coordinates of reference points in the new basis
  cv::Mat t2_inv = t2.inv();
  cv::Mat pot = t2_inv * cv::Mat(cv::Vec4f(p0.x, p0.y, p0.z, 1.0f));
  cv::Mat pxt = t2_inv * cv::Mat(cv::Vec4f(px.x, px.y, px.z, 1.0f));
  cv::Mat pyt = t2_inv * cv::Mat(cv::Vec4f(py.x, py.y, py.z, 1.0f));
  cv::Mat pzt = t2_inv * cv::Mat(cv::Vec4f(pz.x, pz.y, pz.z, 1.0f));

  // Compute direction vectors in new basis
  cv::Mat vx = pxt - pot;
  cv::Mat vy = pyt - pot;
  cv::Mat vz = pzt - pot;

  // Concatenate result into 4x4 affine transformation matrix
  cv::Mat transfo;
  cv::hconcat(vx, vy, transfo);
  cv::hconcat(transfo, vz, transfo);
  cv::hconcat(transfo, pot, transfo);
  cv::vconcat(transfo, I, transfo);

  return transfo;
}

// Scan down the columns of the intensity image and calculate the intensity weighted centroid
// taking 3 columns at a time.  The centroids are then used for line fiitting
cv::Point2f FindLdsReturn(const cv::Size &camera_size, std::vector<cv::Point2d> &xy_vals) {
  int cam_width = camera_size.width;
  int cam_height = camera_size.height;
  // 1-D point index into 2-D image data
  int pt_index = 0;
  float sum_region = 0.0f;
  float pix_val = 0.0f;
  // x (column) & y (row) accumulator or intensity weighted centroid
  float x_bar = 0.0f;
  float y_bar = 0.0f;
  float min_col = std::numeric_limits<float>::max();
  float max_col = std::numeric_limits<float>::min();
  cv::Point2f min_max_col;
  for (int col = 0; col < cam_width - 2; col += 3) {
    sum_region = 0.0f;
    x_bar = 0.0f;
    y_bar = 0.0f;
    for (int row = 0; row < cam_height; row++) {
      pt_index = row * cam_width + col;
      pix_val = pt_intensity_running_stat[pt_index].Mean();
      if (pix_val > 0.0f) {
        y_bar += static_cast<float>(row) * pix_val;
        x_bar += static_cast<float>(col) * pix_val;
        sum_region += pix_val;
      }
      pt_index = row * cam_width + col + 1;
      pix_val = pt_intensity_running_stat[pt_index].Mean();
      if (pix_val > 0.0f) {
        y_bar += static_cast<float>(row) * pix_val;
        x_bar += static_cast<float>(col + 1) * pix_val;
        sum_region += pix_val;
      }
      pt_index = row * cam_width + col + 2;
      pix_val = pt_intensity_running_stat[pt_index].Mean();
      if (pix_val > 0.0f) {
        y_bar += static_cast<float>(row) * pix_val;
        x_bar += static_cast<float>(col + 2) * pix_val;
        sum_region += pix_val;
      }
    }
    if (sum_region > 0.0f) {
      y_bar /= sum_region;
      x_bar /= sum_region;
      xy_vals.push_back(cv::Point2d(static_cast<double>(x_bar), static_cast<double>(y_bar)));
      if (x_bar > max_col) max_col = x_bar;
      if (x_bar < min_col) min_col = x_bar;
    }
  }
  min_max_col.x = min_col;
  min_max_col.y = max_col;
  return min_max_col;
}

// Listener for new ToF frames
class MyListener : public royale::IExtendedDataListener {
 public:
  enum DataCollectionMode {
    NONE = 0,
    LDS_DATA,
    TOF_DATA,
  };

  MyListener() : m_counter(0), current_mode(DataCollectionMode::NONE) {
    spin_chars[0] = '|';
    spin_chars[1] = '/';
    spin_chars[2] = '-';
    spin_chars[3] = '\\';
  }

  void onNewData(const royale::IExtendedData *data) {
    if (current_mode != DataCollectionMode::NONE) {
      putchar(spin_chars[m_counter % sizeof(spin_chars)]);
      putchar('\r');
      fflush(stdout);
      m_counter++;
      std::unique_lock<std::mutex> lock(cloudMutex);
      auto depth = data->getDepthData();
      auto intermed = data->getIntermediateData();
      float intensity_val = 0.0f;
      int row;
      int col;
      for (size_t i = 0u; i < NUM_IMAGE_ELEMENTS; ++i) {
        if (current_mode == DataCollectionMode::LDS_DATA) {
          // Temporary ignore region at top and bottom of image due to incorrect calibration changes
          row = static_cast<int>(static_cast<int>(i) / static_cast<int>(intermed->width));
          col = static_cast<int>(i) % static_cast<int>(intermed->width);
          // Skip if outside of detection boundary
          if (row < ToF_calibration_params::kRowDetectionBounds.first || row > ToF_calibration_params::kRowDetectionBounds.second) continue;
          if (col < ToF_calibration_params::kColDetectionBounds.first || col > ToF_calibration_params::kColDetectionBounds.second) continue;

          intensity_val = intermed->points[i].intensity;
          if (intensity_val > ToF_calibration_params::kLdsDetectionThreshold) {
            pt_intensity_running_stat[i].Push(intensity_val);
          }
        } else if (current_mode == DataCollectionMode::TOF_DATA) {
          confident_pixel[i].Push(depth->points[i].depthConfidence);
          if (depth->points[i].depthConfidence > 0) {
            pt_x_running_stat[i].Push(depth->points[i].x);
            pt_y_running_stat[i].Push(depth->points[i].y);
            pt_z_running_stat[i].Push(depth->points[i].z);
          }
        }
      }
      newDataAvailable = true;
      cloudCV.notify_all();
    }
  }

  void setDataCollectionMode(DataCollectionMode enum_mode) {
    const std::string enum_name[] = {"NONE", "LDS_DATA", "TOF_DATA"};
    current_mode = enum_mode;
  }

  char spin_chars[4];
  unsigned long m_counter;

 private:
  DataCollectionMode current_mode;
};

int setCameraProperties(std::unique_ptr<royale::ICameraDevice> &camera_) {
  // set ToF module processing parameters
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_ADAPTIVE_NOISE_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_ADAPTIVE_NOISE_FILTER." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_FLYING_PIXEL}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FLYING_PIXEL." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_AVERAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AVERAGE." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_AMP}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AMP." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_DIST}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_DIST." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_VALIDATE_IMAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_VALIDATE_IMAGE." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_STRAY_LIGHT}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_STRAY_LIGHT." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_FILTER_2_FREQ}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FILTER_2_FREQ." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_SBI_FLAG}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SBI_FLAG." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_SMOOTHING_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SMOOTHING_FILTER." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_HOLE_FILLING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_HOLE_FILLING." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::NOISE_THRESHOLD}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set NOISE_THRESHOLD." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::AUTO_EXPOSURE_REF_VALUE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set AUTO_EXPOSURE_REF_VALUE." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::ADAPTIVE_NOISE_FILTER_TYPE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set ADAPTIVE_NOISE_FILTER_TYPE." << std::endl;
    terminate();
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::GLOBAL_BINNING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set GLOBAL_BINNING." << std::endl;
    terminate();
    return -1;
  }
  return 0;
}

// For testing if a value is inside a range
bool inRange(royale::Pair<std::uint32_t, std::uint32_t> range_vals, std::uint32_t val) {
  return ((static_cast<float>(val) - static_cast<float>(range_vals.second)) * (static_cast<float>(val) - static_cast<float>(range_vals.first)) <= 0);
}

// Removes duplicate points from a vector
bool RemoveDuplicates(std::vector<cv::Point2d> &xy_vals) {
  unsigned nb_removed = 0;  // count number of removed from back
  double threshold_distance = 0.1;
  for (size_t i = 0; i < xy_vals.size() - nb_removed; ++i) {
    for (size_t j = i + 1; j < xy_vals.size() - nb_removed; ++j) {
      if (cv::norm(xy_vals[i] - xy_vals[j]) < threshold_distance) {
        std::iter_swap(xy_vals.begin() + i, xy_vals.end() - nb_removed - 1);
        nb_removed += 1;
        --i;
        break;
      }
    }
  }
  xy_vals.erase(xy_vals.end() - nb_removed, xy_vals.end());
  if (xy_vals.size() < 2) {
    return false;
  } else {
    return true;
  }
}

// Main Program Entry
int main(int argc, char **argv) {

  int calibration_timeout_seconds = ToF_calibration_params::kCalibrationTtimeoutSeconds;
  if (argc == 2) {
    calibration_timeout_seconds = atoi(argv[1]);
  }

  // Default options
  //std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";  //Running.G Edit
  std::string ACCESS_CODE = "c715e2ca31e816b1ef17ba487e2a5e9efc6bbd7b";

  royale::CameraStatus status;
  std::unique_ptr<MyListener> listener_;
  newDataAvailable = false;
  platform::CameraFactory factory;
  Timer timer;
  // MODE_5 allows longer exposure times allowing more opportunity to see the LDS signal
  royale::String useCase_LDS = "MODE_5_5FPS";
  royale::String useCase_TOF = "MODE_9_5FPS";

  // Use a constant exposure rate for collecting data with the LDS turned off
  std::uint32_t tof_exposure_time = 900;
  std::uint32_t current_exposure;

  pt_intensity_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_x_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_y_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_z_running_stat.resize(NUM_IMAGE_ELEMENTS);
  confident_pixel.resize(NUM_IMAGE_ELEMENTS);

  const char *start_lds_command = ToF_calibration_params::kStartLdsString.c_str();
  const char *stop_lds_command = ToF_calibration_params::kStopLdsString.c_str();

  // Starting LDS
  std::cout << "Starting LDS Closed Loop" << std::endl;
  system(start_lds_command);

  // [Setup] Camera Initialization
  camera_ = factory.createCamera();

  // Test if CameraDevice was created
  if (camera_ == nullptr) {
    std::cout << "[ERROR] Camera device could not be created." << std::endl;
    terminate();
    return -1;
  }
  // Initialize()
  if ((status = camera_->initialize()) != royale::CameraStatus::SUCCESS) {
    std::cout << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  uint16_t cam_height, cam_width;

  camera_->getMaxSensorHeight(cam_height);
  camera_->getMaxSensorWidth(cam_width);
  cv::Size camera_size(cam_width, cam_height);

  royale::String current_use_case;
  camera_->getCurrentUseCase(current_use_case);
  royale::Vector<royale::String> useCaseList;
  if (current_use_case != useCase_LDS) {
    if ((status = camera_->getUseCases(useCaseList)) != royale::CameraStatus::SUCCESS || useCaseList.empty()) {
      std::cout << "[ERROR] Could not get use cases. " << royale::getStatusString(status).c_str() << std::endl;
      terminate();
      return -1;
    }
    for (auto i = 0u; i < useCaseList.size(); ++i) {
      if (useCaseList.at(i) == useCase_LDS) {
        if ((status = camera_->setUseCase(useCaseList.at(i))) != royale::CameraStatus::SUCCESS) {
          std::cout << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " << royale::getStatusString(status).c_str() << std::endl;
          terminate();
          return -1;
        }
      }
    }
  }

  camera_->setCallbackData(royale::CallbackData::Intermediate);

  listener_.reset(new MyListener());
  if ((status = camera_->registerDataListenerExtended(listener_.get())) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not register the extended data listener" << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  if ((status = camera_->setExposureMode(royale::ExposureMode::MANUAL)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set exposure to manual" << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  // Get Exposure Limits
  royale::Pair<std::uint32_t, std::uint32_t> exposure_limits;
  if ((status = camera_->getExposureLimits(exposure_limits)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not get exposure limits. " << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  if ((status = camera_->setExposureTime(exposure_limits.second)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] setting exposure time to: " << exposure_limits.second << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }
  current_exposure = exposure_limits.second;

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Start Capture
  if ((status = camera_->startCapture()) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not start video capture." << std::endl;
    terminate();
    return -1;
  }

  // capture for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // Set camera properties
  if (setCameraProperties(camera_) == -1) {
    terminate();
    return -1;
  }

  // warm up camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::vector<cv::Mat> vec_pt_cloud;
  std::vector<cv::Mat> vec_pt_intensity;
  int num_frames_collected = 0;
  int kLdsSampleSize = ToF_calibration_params::kLdsSampleSize;
  int kTofSampleSize = ToF_calibration_params::kTofSampleSize;
  float col_range = 0.0f;
  bool exposure_going_down = true;

  // Set collect data mode to LDS_DATA
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::LDS_DATA);

  std::vector<cv::Point2d> xy_vals;
  int num_last_xy_pts = 0;
  int num_new_collected = 0;
  cv::Point2f min_max_col;

  // Continue to look for LDS laser signal locations until the number of collected samples exceeds the kLdsSampleSize and
  // the points are not sufficiently spread across the width of the sensor.  A robust solution for an accurate plane increases
  // as both of these criteria are met.  The chances of catching the LDS signal increase by incrementally varying the exposure.
  // In case a system cannot be calibrated the application will exit after a user specified timeout is reached.  If not specified
  // the default timeout is 90 seconds.
  timer.start();
  while (timer.elapsedSeconds() < calibration_timeout_seconds &&
        ((xy_vals.size() < static_cast<size_t>(kLdsSampleSize) || col_range < ToF_calibration_params::kLdsPtsPlaneFitMinRange))) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      min_max_col = FindLdsReturn(camera_size, xy_vals);
      col_range = min_max_col.y - min_max_col.x;

      if (num_last_xy_pts < static_cast<int>(xy_vals.size())) {
        num_new_collected = static_cast<int>(xy_vals.size()) - num_last_xy_pts;
        num_last_xy_pts = static_cast<int>(xy_vals.size());
      }
      newDataAvailable = false;
    }
    // Update the exposure if points were detected
    if (num_new_collected > 0) {
      listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);
      num_new_collected = 0;
      // Adjust the exposure
      if (exposure_going_down) {
        current_exposure -= 10;
      } else {
        current_exposure += 10;
      }
      if (current_exposure <= exposure_limits.first) {
        exposure_going_down = false;
        current_exposure = exposure_limits.first + 10;
      }
      if (current_exposure >= exposure_limits.second) {
        exposure_going_down = true;
        current_exposure = exposure_limits.second - 10;
      }

      if (inRange(exposure_limits, current_exposure)) {
        int exposure_retries = 0;
        // Update exposure setting and retry several times if camera is busy
        while ((status = camera_->setExposureTime(current_exposure)) != royale::CameraStatus::SUCCESS && exposure_retries <= ToF_calibration_params::kNumExposureRetries) {
          std::this_thread::sleep_for(std::chrono::milliseconds(300));
          exposure_retries++;
        }

        if (exposure_retries == ToF_calibration_params::kNumExposureRetries) {
          std::cout << "[ERROR] can't set exposure time to: " << current_exposure << royale::getStatusString(status).c_str() << std::endl;
          terminate();
          return -1;
        }
      }
      listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::LDS_DATA);
    }
  }

  if (timer.elapsedSeconds() > calibration_timeout_seconds) {
    std::cout << "[WARNING] Timer Has Expired at " << timer.elapsedSeconds() << " seconds." << std::endl;
  }

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Check if a sufficient number of LDS points were detected
  if (xy_vals.size() < 2) {
    std::cout << "[ERROR] No finding enough LDS points, increase the timeout" << std::endl;
    terminate();
    return -1;
  }

  // Remove duplicate points
  if (!RemoveDuplicates(xy_vals)) {
    std::cout << "[ERROR] No finding enough LDS points, increase the timeout" << std::endl;
    terminate();
    return -1;
  }

  // Stopping LDS
  std::cout << "Stopping LDS Closed Loop" << std::endl;
  system(stop_lds_command);

  // Set use case for collecting the point cloud
  for (auto i = 0u; i < useCaseList.size(); ++i) {
    if (useCaseList.at(i) == useCase_TOF) {
      if ((status = camera_->setUseCase(useCaseList.at(i))) != royale::CameraStatus::SUCCESS) {
        std::cout << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " << royale::getStatusString(status).c_str() << std::endl;
        terminate();
        return -1;
      }
    }
  }
  // wait on camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  if ((status = camera_->setExposureMode(royale::ExposureMode::MANUAL)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set exposure to manual" << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  // Get Exposure Limits
  if ((status = camera_->getExposureLimits(exposure_limits)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not get exposure limits. " << royale::getStatusString(status).c_str() << std::endl;
    terminate();
    return -1;
  }

  if (inRange(exposure_limits, tof_exposure_time)) {
    if ((status = camera_->setExposureTime(tof_exposure_time)) != royale::CameraStatus::SUCCESS) {
      std::cout << "[ERROR] setting exposure time to: " << tof_exposure_time << royale::getStatusString(status).c_str() << std::endl;
      terminate();
      return -1;
    }
  } else {
    std::cout << "Requested manual exposure time: " << tof_exposure_time << " outside of safe limits[" << exposure_limits.first << "," << exposure_limits.second << "]"
              << std::endl;
    std::cout << "Keeping current exposure." << std::endl;
  }

  // Set camera properties
  if (setCameraProperties(camera_) == -1) {
    terminate();
    return -1;
  }


  // wait for camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Set collect data mode to TOF_DATA
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::TOF_DATA);

  num_frames_collected = 0;
  while (num_frames_collected < kTofSampleSize) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      num_frames_collected++;
      newDataAvailable = false;
    }
  }

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Fit line
  cv::Mat lds_pts = cv::Mat::zeros(static_cast<int>(xy_vals.size()), 2, CV_64F);  //, xy_vals.data());
  int i_row = 0;
  for (size_t idx = 0; idx < xy_vals.size(); ++idx) {
    lds_pts.at<double>(i_row, 0) = xy_vals[idx].x;
    lds_pts.at<double>(i_row, 1) = xy_vals[idx].y;
    i_row++;
  }

  cv::PCA pca_analysis(lds_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);
  double beta = -pca_analysis.eigenvectors.at<double>(1, 0) / pca_analysis.eigenvectors.at<double>(1, 1);
  double b_zero = pca_analysis.mean.at<double>(0, 1) - beta * pca_analysis.mean.at<double>(0, 0);

  // Endpoints of the line fit to the LDS signal, for first and last columns
  cv::Point2f lds_pt_0;
  cv::Point2f lds_pt_1;

  lds_pt_0.x = 1.0f;
  lds_pt_0.y = static_cast<float>(beta + b_zero);
  lds_pt_1.x = static_cast<float>(cam_width) - 1.0f;
  lds_pt_1.y = static_cast<float>(beta * (static_cast<double>(cam_width) - 1.) + b_zero);

  // Laser offset from the LDS center
  double z_trans_lds = ToF_calibration_params::kLdsCenterToLaserDistance * cos(ToF_calibration_params::kLdsCenterToLaserAngle * (M_PI / 180.));
  double x_trans_lds = ToF_calibration_params::kLdsCenterToLaserDistance * sin(ToF_calibration_params::kLdsCenterToLaserAngle * (M_PI / 180.));
  cv::Mat lds_offset = cv::Mat::eye(4, 4, CV_32F);
  lds_offset.at<float>(0, 3) = static_cast<float>(x_trans_lds);
  lds_offset.at<float>(2, 3) = static_cast<float>(z_trans_lds);

  cv::Mat line_image(cam_height, cam_width, CV_8UC1, cv::Scalar(0));
  cv::line(line_image, lds_pt_0, lds_pt_1, cv::Scalar(255), 1, 8);

  int confident_image_index = 0;
  // Use a hightly confident pixel threshold
  std::vector<cv::Point3f> pt_3d_vector;
  for (int row = 0; row < line_image.rows; ++row) {
    uchar *line_image_ptr = line_image.ptr<uchar>(row);
    for (int col = 0; col < line_image.cols; ++col) {
      if ((line_image_ptr[col] > 0) && (confident_pixel[confident_image_index].Mean() > ToF_calibration_params::kConfidentPixelThreshold)) {
        cv::Point3f pt_3d(pt_x_running_stat[confident_image_index].Mean() * 1000.0f, pt_y_running_stat[confident_image_index].Mean() * 1000.0f,
                          pt_z_running_stat[confident_image_index].Mean() * 1000.0f);
        pt_3d_vector.push_back(pt_3d);
      }
      confident_image_index++;
    }
  }

  // Add in LDS position
  cv::Point3f P3(ToF_calibration_params::kLdsOffsetX, ToF_calibration_params::kLdsOffsetY, ToF_calibration_params::kLdsOffsetZ);
  P3.x = P3.x + static_cast<float>(x_trans_lds);
  P3.z = P3.z + static_cast<float>(z_trans_lds);
  pt_3d_vector.push_back(P3);

  // Use line points from detected LDS signal along with the LDS position from the mechanical drawings to form a plane
  cv::Mat plane_pts_3d = cv::Mat(pt_3d_vector).reshape(1).t();
  cv::Scalar ux = cv::mean(plane_pts_3d.row(0));
  cv::Scalar uy = cv::mean(plane_pts_3d.row(1));
  cv::Scalar uz = cv::mean(plane_pts_3d.row(2));

  float fux, fuy, fuz;
  fux = static_cast<float>(ux.val[0]);
  fuy = static_cast<float>(uy.val[0]);
  fuz = static_cast<float>(uz.val[0]);

  cv::Mat uxyz(3, 1, CV_32F);
  uxyz.at<float>(0, 0) = fux;
  uxyz.at<float>(1, 0) = fuy;
  uxyz.at<float>(2, 0) = fuz;
  cv::Mat uMat;
  cv::repeat(uxyz, 1, plane_pts_3d.cols, uMat);
  cv::Mat plane_pts_3d_bar = plane_pts_3d - uMat;

  cv::Mat CC = plane_pts_3d_bar * plane_pts_3d_bar.t();
  CC = CC / (static_cast<float>(plane_pts_3d.cols) - 1.0f);
  cv::Mat e_values, e_vectors;
  cv::eigen(CC, e_values, e_vectors);
  cv::Point3f n(e_vectors.at<float>(2, 0), e_vectors.at<float>(2, 1), e_vectors.at<float>(2, 2));

  // The ToF sensor y-axis is directed towards the floor so if the plane normal y component is positive
  // we need to flip the normal to be oriented away from the floor.
  if (n.y > 0.0f) {
    n.x *= -1.0f;
    n.y *= -1.0f;
    n.z *= -1.0f;
  }

  Plane LDS_in_TOF;
  LDS_in_TOF.ConstructFromPointNormal(P3, n);

  Plane TOF_in_TOF;
  cv::Mat Tform_TOF_to_LDS = CreateBasisTransfrom3D(TOF_in_TOF, LDS_in_TOF);
  // Apply the LDS laser offset to transform to the center of spinning LDS
  Tform_TOF_to_LDS = Tform_TOF_to_LDS * lds_offset;

  // To go the opposite way just invert the matrix, commenting for posterity.
  // cv::Mat Tform_LDS_to_TOF = Tform_TOF_to_LDS.inv();

  // Test transformation is within 2X of the tolerance of 1.63 mm
  // Transform the LDS's position in the ToF coordinate system
  // into the LDS coordinate system with Tform_TOF_to_LDS
  // and the distance of the result to [0,0,0] is the error.
  // This can be found by taking the norm of the result.
  cv::Mat_<float> lds_pos_in_tof = (cv::Mat_<float>(4, 1) << ToF_calibration_params::kLdsOffsetX, ToF_calibration_params::kLdsOffsetY, ToF_calibration_params::kLdsOffsetZ, 1.0f);
  cv::Mat trans_check = Tform_TOF_to_LDS * lds_pos_in_tof;
  cv::Mat_<float> trans_check2 = (cv::Mat_<float>(3, 1) << trans_check.at<float>(0, 0), trans_check.at<float>(1, 0), trans_check.at<float>(2, 0));
  double distance_check = cv::norm(trans_check2);

  std::cout << "Transformation Matrix: ToF -> LDS" << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(0, 0) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(0, 1) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(0, 2) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(0, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(1, 0) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(1, 1) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(1, 2) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(1, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(2, 0) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(2, 1) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(2, 2) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(2, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(3, 0) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(3, 1) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(3, 2) << "  ";
  std::cout << Tform_TOF_to_LDS.at<float>(3, 3) << std::endl;

  if (distance_check > 3.) {
    std::cout << "[FAIL]: Distance Check " << distance_check << " is over limit of 3 mm." << std::endl;
    std::cout << "Retry Calibration, Set Aside If Continues to Fail." << std::endl;
    terminate();
    return -1;
  } else {
    std::cout << "[PASS]: Distance Check " << distance_check*1000. << " mm < 3mm" << std::endl;
  }

  std::string filename = "/user/transformation_matrix_tof_into_lds.conf";
  std::ofstream calibration_file;
  std::stringstream stringStream;
  calibration_file.open(filename, std::ofstream::out);
  if (calibration_file.fail()) {
    std::cout << "[ERROR] Failed to create TOF CAMERA Calibration File" << std::endl;
  } else {
    stringStream << Tform_TOF_to_LDS.at<float>(0, 0) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(0, 1) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(0, 2) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(0, 3) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(1, 0) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(1, 1) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(1, 2) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(1, 3) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(2, 0) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(2, 1) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(2, 2) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(2, 3) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(3, 0) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(3, 1) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(3, 2) << std::endl;
    stringStream << Tform_TOF_to_LDS.at<float>(3, 3) << std::endl;
    calibration_file << stringStream.str();
    calibration_file.close();
  }

  // Useful for debugging LDS signal detection with the captured images.
  /*
    std::string filename1 = "/home/root/data/ntensity_data.dat";
    std::string filename2 = "/home/root/data/pointcloud_data.dat";
    std::ofstream intensityFile;
    std::ofstream pointFile;
    std::stringstream stringStream3;
    std::stringstream stringStream2;
    intensityFile.open(filename1, std::ofstream::out);
    pointFile.open(filename2, std::ofstream::out);
    if (intensityFile.fail() || pointFile.fail()) {
      std::cout << "Couldn't open intensity or point output file" << std::endl;
    } else {
      stringStream3 << pt_intensity_running_stat[0].Mean();
      stringStream2 << pt_x_running_stat[0].Mean() << " " << pt_y_running_stat[0].Mean() << " " << pt_z_running_stat[0].Mean() << std::endl;
      for (size_t index = 1; index < pt_intensity_running_stat.size(); ++index) {
        stringStream3 << " " << pt_intensity_running_stat[index].Mean();
        stringStream2 << pt_x_running_stat[index].Mean() << " " << pt_y_running_stat[index].Mean() << " " << pt_z_running_stat[index].Mean() << std::endl;
      }
      stringStream3 << std::endl;
      intensityFile << stringStream3.str();
      intensityFile.close();
      pointFile << stringStream2.str();
      pointFile.close();
    }
  */
  terminate();
  return 0;
}
