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
#include "running_stat.hpp"

// Image is 172 x 224 pixels
#define NUM_IMAGE_ELEMENTS 38528

// Simplify memory handeling for testing
namespace {
std::vector<RunningStat> pt_intensity_running_stat;
std::vector<RunningStat> pt_x_running_stat;
std::vector<RunningStat> pt_y_running_stat;
std::vector<RunningStat> pt_z_running_stat;
std::mutex cloudMutex;
std::condition_variable cloudCV;
bool newDataAvailable;
}  // namespace

std::string VERSION{"1.1"};

// The plane fitting parameters
class PlaneParams {
 public:
  PlaneParams(float v1, float v2, float v3, float v4, cv::Point3f c);
  // a*x + b*y + c*z = d
  float a, b, c, d;
  cv::Point3f centroid;
};

inline PlaneParams::PlaneParams(float v1, float v2, float v3, float v4, cv::Point3f c) : a(v1), b(v2), c(v3), d(v4), centroid(c) {}

// A class for a standard 3D plane defined by a* x + b* y + c* z + d = 0
class Plane {
 public:
  Plane();
  Plane(const Plane &P);

  // Creates the plane from a point on the surface and a normal vector
  void ConstructFromPointNormal(const cv::Point3f &Pt, const cv::Point3f &Normal);
  // Creates the plane from a point on the surface and two vectors in the plane
  // Plane ConstructFromPointVectors(const cv::Point3f &Pt, const cv::Point3f &V1, const cv::Point3f &V2);

  // Transform the plane with a 4x4 transformation matrix
  Plane TransformPlane3D(const cv::Mat &Tform);

  // Translate the plane origin to the point closest to the Basis origin [0,0,0]
  void CenterOnBasisOrigin();

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

Plane Plane::TransformPlane3D(const cv::Mat &Tform) {
  Plane result;
  cv::Mat origin_trans = Tform * cv::Mat(cv::Vec4f(this->origin.x, this->origin.y, this->origin.z, 1.0f));
  // Null out translation for direction vectors1 and 2
  cv::Mat Tform2 = Tform.clone();
  Tform2.at<float>(0, 3) = 0.0f;
  Tform2.at<float>(1, 3) = 0.0f;
  Tform2.at<float>(2, 3) = 0.0f;

  cv::Mat vector1_trans = Tform2 * cv::Mat(cv::Vec4f(this->vector1.x, this->vector1.y, this->vector1.z, 1.0f));
  cv::Mat vector2_trans = Tform2 * cv::Mat(cv::Vec4f(this->vector2.x, this->vector2.y, this->vector2.z, 1.0f));
  cv::Mat normal_trans = Tform2 * cv::Mat(cv::Vec4f(this->normal.x, this->normal.y, this->normal.z, 1.0f));

  result.origin = cv::Point3f(origin_trans.at<float>(0), origin_trans.at<float>(1), origin_trans.at<float>(2));
  result.vector1 = cv::Point3f(vector1_trans.at<float>(0), vector1_trans.at<float>(1), vector1_trans.at<float>(2));
  result.vector2 = cv::Point3f(vector2_trans.at<float>(0), vector2_trans.at<float>(1), vector2_trans.at<float>(2));
  result.normal = cv::Point3f(normal_trans.at<float>(0), normal_trans.at<float>(1), normal_trans.at<float>(2));

  return result;
}

void Plane::CenterOnBasisOrigin() { this->origin = this->normal * this->origin.dot(this->normal); }

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

Plane FitPlane(const std::vector<cv::Point3f> &pt_cloud_vec) {
  cv::Mat pt_cloud = cv::Mat(pt_cloud_vec).reshape(1).t();

  cv::Scalar ux = cv::mean(pt_cloud.row(0));
  cv::Scalar uy = cv::mean(pt_cloud.row(1));
  cv::Scalar uz = cv::mean(pt_cloud.row(2));

  float fux, fuy, fuz;
  fux = static_cast<float>(ux.val[0]);
  fuy = static_cast<float>(uy.val[0]);
  fuz = static_cast<float>(uz.val[0]);

  cv::Mat uxyz(3, 1, CV_32F);
  uxyz.at<float>(0, 0) = fux;
  uxyz.at<float>(1, 0) = fuy;
  uxyz.at<float>(2, 0) = fuz;

  cv::Mat uMat;
  cv::repeat(uxyz, 1, pt_cloud.cols, uMat);
  cv::Mat pt_cloud_bar = pt_cloud - uMat;
  cv::Mat CC = pt_cloud_bar * pt_cloud_bar.t();
  CC = CC / (static_cast<float>(pt_cloud.cols) - 1.0f);

  cv::Mat e_values, e_vectors;
  cv::eigen(CC, e_values, e_vectors);

  // Format to ensure first axis points in positive x direction
  if (e_vectors.at<float>(0, 0) < 0.0f) {
    e_vectors = -e_vectors;
    // keep matrix determinant positive
    e_vectors.at<float>(2, 0) *= -1.0f;
    e_vectors.at<float>(2, 1) *= -1.0f;
    e_vectors.at<float>(2, 2) *= -1.0f;
  }

  Plane fitted_plane;
  fitted_plane.origin = cv::Point3f(fux, fuy, fuz);
  fitted_plane.vector1 = cv::Point3f(e_vectors.at<float>(0, 0), e_vectors.at<float>(0, 1), e_vectors.at<float>(0, 2));
  fitted_plane.vector2 = cv::Point3f(e_vectors.at<float>(1, 0), e_vectors.at<float>(1, 1), e_vectors.at<float>(1, 2));
  fitted_plane.normal = cv::Point3f(e_vectors.at<float>(2, 0), e_vectors.at<float>(2, 1), e_vectors.at<float>(2, 2));

  return fitted_plane;
}

/*
Plane Plane::ConstructFromPointVectors(const cv::Point3f &pt, const cv::Point3f &v1, const cv::Point3f &v2) {
  cv::Point3f normal = v1.cross(v2);
  return ConstructFromPointNormal(pt, normal);
}*/

// Listener for new ToF frames
class MyListener : public royale::IExtendedDataListener {
 public:
  enum DataCollectionMode {
    NONE = 0,
    LDS_DATA,
    TOF_DATA,
  };

  MyListener() : current_mode(DataCollectionMode::NONE) {}

  void onNewData(const royale::IExtendedData *data) {
    if (current_mode != DataCollectionMode::NONE) {
      std::unique_lock<std::mutex> lock(cloudMutex);
      auto depth = data->getDepthData();
      auto intermed = data->getIntermediateData();
      float intensity_val = 0.0f;
      int row;
      for (size_t i = 0u; i < NUM_IMAGE_ELEMENTS; ++i) {
        if (current_mode == DataCollectionMode::LDS_DATA) {
          // Temporary ignore region at top and bottom of image due to incorrect calibration changes
          row = (int)((int)i / (int)intermed->width);
          if (row < 20 || row > 149) continue;
          intensity_val = intermed->points[i].intensity;
          if (intensity_val > ToF_calibration_params::lds_detection_threshold) {
            pt_intensity_running_stat[i].Push(intensity_val);
          }

        } else if (current_mode == DataCollectionMode::TOF_DATA) {
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
    std::cout << "Current mode set to: " << enum_name[current_mode] << std::endl;
    current_mode = enum_mode;
    std::cout << "New mode set to: " << enum_name[enum_mode] << std::endl;
  }

 private:
  DataCollectionMode current_mode;
};

int setCameraProperties(std::unique_ptr<royale::ICameraDevice> &camera_) {
  // set ToF module processing parameters
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_ADAPTIVE_NOISE_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_ADAPTIVE_NOISE_FILTER." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_FLYING_PIXEL}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FLYING_PIXEL." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_AVERAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AVERAGE." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_AMP}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_AMP." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_MPI_DIST}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_MPI_DIST." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_VALIDATE_IMAGE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_VALIDATE_IMAGE." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_STRAY_LIGHT}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_STRAY_LIGHT." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_FILTER_2_FREQ}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_FILTER_2_FREQ." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_SBI_FLAG}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SBI_FLAG." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_SMOOTHING_FILTER}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_SMOOTHING_FILTER." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::USE_HOLE_FILLING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set USE_HOLE_FILLING." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::NOISE_THRESHOLD}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set NOISE_THRESHOLD." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::AUTO_EXPOSURE_REF_VALUE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set AUTO_EXPOSURE_REF_VALUE." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::ADAPTIVE_NOISE_FILTER_TYPE}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set ADAPTIVE_NOISE_FILTER_TYPE." << std::endl;
    return -1;
  }
  if (camera_->setProcessingParameters({ToF_calibration_params::GLOBAL_BINNING}) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set GLOBAL_BINNING." << std::endl;
    return -1;
  }
  return 0;
}

// For testing if a value is inside a range
bool inRange(royale::Pair<std::uint32_t, std::uint32_t> range_vals, std::uint32_t val) {
  return ((static_cast<float>(val) - static_cast<float>(range_vals.second)) * (static_cast<float>(val) - static_cast<float>(range_vals.first)) <= 0);
}

// Main Program Entry
int main(int argc, char **argv) {
  // Default options
  std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";
  royale::CameraStatus status;
  std::unique_ptr<MyListener> listener_;
  newDataAvailable = false;
  std::unique_ptr<royale::ICameraDevice> camera_;  // The camera device
  platform::CameraFactory factory;
  royale::String useCase_LDS = "MODE_5_5FPS";
  royale::String useCase_TOF = "MODE_9_5FPS";

  std::vector<std::uint32_t> lds_exposure_times{1000, 1500, 2000, 2500, 3000};
  std::uint32_t tof_exposure_time = 1200;

  std::vector<PlaneParams> plane_coeffs;
  pt_intensity_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_x_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_y_running_stat.resize(NUM_IMAGE_ELEMENTS);
  pt_z_running_stat.resize(NUM_IMAGE_ELEMENTS);

  std::string start_lds_string = "roboctrl -d b";
  const char *start_lds_command = start_lds_string.c_str();
  std::string stop_lds_string = "roboctrl -d s";
  const char *stop_lds_command = stop_lds_string.c_str();

  // Starting LDS
  std::cout << "Starting LDS Closed Loop" << std::endl;
  system(start_lds_command);

  // [Setup] Camera Initialization
  camera_ = factory.createCamera();

  // Test if CameraDevice was created
  if (camera_ == nullptr) {
    std::cout << "[ERROR] Camera device could not be created." << std::endl;
    return -1;
  }
  // Initialize()
  if ((status = camera_->initialize()) != royale::CameraStatus::SUCCESS) {
    std::cout << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  uint16_t cam_height, cam_width;
  camera_->getMaxSensorHeight(cam_height);
  camera_->getMaxSensorWidth(cam_width);

  royale::String current_use_case;
  camera_->getCurrentUseCase(current_use_case);
  royale::Vector<royale::String> useCaseList;
  if (current_use_case != useCase_LDS) {
    if ((status = camera_->getUseCases(useCaseList)) != royale::CameraStatus::SUCCESS || useCaseList.empty()) {
      std::cout << "[ERROR] Could not get use cases. " << royale::getStatusString(status).c_str() << std::endl;
      return -1;
    }
    for (auto i = 0u; i < useCaseList.size(); ++i) {
      if (useCaseList.at(i) == useCase_LDS) {
        if ((status = camera_->setUseCase(useCaseList.at(i))) != royale::CameraStatus::SUCCESS) {
          std::cout << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " << royale::getStatusString(status).c_str() << std::endl;
          return -1;
        }
      }
    }
  }

  camera_->setCallbackData(royale::CallbackData::Intermediate);

  listener_.reset(new MyListener());
  if ((status = camera_->registerDataListenerExtended(listener_.get())) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not register the extended data listener" << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  if ((status = camera_->setExposureMode(royale::ExposureMode::MANUAL)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set exposure to manual" << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  // Get Exposure Limits
  royale::Pair<std::uint32_t, std::uint32_t> exposure_limits;
  if ((status = camera_->getExposureLimits(exposure_limits)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not get exposure limits. " << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  if (inRange(exposure_limits, lds_exposure_times[0])) {
    if ((status = camera_->setExposureTime(lds_exposure_times[0])) != royale::CameraStatus::SUCCESS) {
      std::cout << "[ERROR] setting exposure time to: " << lds_exposure_times[0] << royale::getStatusString(status).c_str() << std::endl;
      return -1;
    }
  } else {
    std::cout << "Requested manual exposure time: " << lds_exposure_times[0] << " outside of safe limits[" << exposure_limits.first << "," << exposure_limits.second << "]"
              << std::endl;
    std::cout << "Keeping current exposure." << std::endl;
  }

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Start Capture
  if ((status = camera_->startCapture()) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not start video capture." << std::endl;
    return -1;
  }
  // capture for a few seconds
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // Set camera properties
  if (setCameraProperties(camera_) == -1) return -1;

  // warm up camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::vector<cv::Mat> vec_pt_cloud;
  std::vector<cv::Mat> vec_pt_intensity;
  int num_frames_collected = 0;
  int lds_sample_size = ToF_calibration_params::lds_sample_size;
  int tof_sample_size = ToF_calibration_params::tof_sample_size;
  int exposure_frame_threshold = 100;
  size_t curr_lds_exposure_index = 0;

  // Set collect data mode to LDS_DATA
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::LDS_DATA);

  while (num_frames_collected < lds_sample_size) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      num_frames_collected++;
      newDataAvailable = false;
    }
    // Update the exposure every exposure_frame_threshold number of frames collected
    if ((num_frames_collected % exposure_frame_threshold) == 0) {
      // Adjust the exposure
      curr_lds_exposure_index++;
      if (curr_lds_exposure_index < lds_exposure_times.size()) {
        if (inRange(exposure_limits, lds_exposure_times[curr_lds_exposure_index])) {
          if ((status = camera_->setExposureTime(lds_exposure_times[curr_lds_exposure_index])) != royale::CameraStatus::SUCCESS) {
            std::cout << "[ERROR] setting exposure time to: " << lds_exposure_times[curr_lds_exposure_index] << royale::getStatusString(status).c_str() << std::endl;
            return -1;
          }
        } else {
          std::cout << "Requested manual exposure time: " << lds_exposure_times[curr_lds_exposure_index] << " outside of safe limits[" << exposure_limits.first << ","
                    << exposure_limits.second << "]" << std::endl;
          std::cout << "Keeping current exposure." << std::endl;
        }
      }
    }
  }

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Stopping LDS
  std::cout << "Stopping LDS Closed Loop" << std::endl;
  system(stop_lds_command);

  // Set use case for collecting the point cloud
  for (auto i = 0u; i < useCaseList.size(); ++i) {
    if (useCaseList.at(i) == useCase_TOF) {
      if ((status = camera_->setUseCase(useCaseList.at(i))) != royale::CameraStatus::SUCCESS) {
        std::cout << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " << royale::getStatusString(status).c_str() << std::endl;
        return -1;
      }
    }
  }
  // wait on camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  if ((status = camera_->setExposureMode(royale::ExposureMode::MANUAL)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not set exposure to manual" << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  // Get Exposure Limits
  if ((status = camera_->getExposureLimits(exposure_limits)) != royale::CameraStatus::SUCCESS) {
    std::cout << "[ERROR] Could not get exposure limits. " << royale::getStatusString(status).c_str() << std::endl;
    return -1;
  }

  if (inRange(exposure_limits, tof_exposure_time)) {
    if ((status = camera_->setExposureTime(tof_exposure_time)) != royale::CameraStatus::SUCCESS) {
      std::cout << "[ERROR] setting exposure time to: " << tof_exposure_time << royale::getStatusString(status).c_str() << std::endl;
      return -1;
    }
  } else {
    std::cout << "Requested manual exposure time: " << tof_exposure_time << " outside of safe limits[" << exposure_limits.first << "," << exposure_limits.second << "]"
              << std::endl;
    std::cout << "Keeping current exposure." << std::endl;
  }

  // Set camera properties
  if (setCameraProperties(camera_) == -1) return -1;

  // wait for camera
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Set collect data mode to TOF_DATA
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::TOF_DATA);

  num_frames_collected = 0;
  while (num_frames_collected < tof_sample_size) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      num_frames_collected++;
      newDataAvailable = false;
    }
  }

  // Set collect data mode to NONE
  listener_.get()->setDataCollectionMode(MyListener::DataCollectionMode::NONE);

  // Scan down the columns of the intensity image and calculate the intensity weighted centroid
  // taking 3 columns at a time.  The centroids are then used for line fiitting
  std::vector<double> xy_vals;
  int pt_index = 0;
  float sum_region = 0.0f;
  float pix_val = 0.0f;
  float x_bar = 0.0f;
  float y_bar = 0.0f;
  int number_of_points = 0;
  for (int col = 1; col < cam_width - 3; col += 3) {
    sum_region = 0.0f;
    x_bar = 0.0f;
    y_bar = 0.0f;
    for (int row = 1; row < cam_height - 3; row++) {
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
      xy_vals.push_back(static_cast<double>(x_bar));
      xy_vals.push_back(static_cast<double>(y_bar));
      number_of_points++;
    }
  }

  // Fit line
  cv::Mat lds_pts = cv::Mat(number_of_points, 2, CV_64F, xy_vals.data());
  int i_row = 0;
  for (size_t idx = 0; idx < xy_vals.size(); idx += 2) {
    lds_pts.at<double>(i_row, 0) = xy_vals[idx];
    lds_pts.at<double>(i_row, 1) = xy_vals[idx + 1];
    i_row++;
  }

  cv::PCA pca_analysis(lds_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);
  double beta = -pca_analysis.eigenvectors.at<double>(1, 0) / pca_analysis.eigenvectors.at<double>(1, 1);
  double b_zero = pca_analysis.mean.at<double>(0, 1) - beta * pca_analysis.mean.at<double>(0, 0);

  // Endpoints of the line fit to the LDS signal, for first and last columns
  cv::Point2d lds_pt_0;
  cv::Point2d lds_pt_1;

  lds_pt_0.x = 1;
  lds_pt_0.y = beta + b_zero;
  lds_pt_1.x = cam_width - 1;
  lds_pt_1.y = beta * (cam_width - 1) + b_zero;

  double lds_center_to_laser_angle = 57.;        // Degrees
  double lds_center_to_laser_distance = 22.768;  // Millimeters
  double z_trans_lds = lds_center_to_laser_distance * cos(lds_center_to_laser_angle * (M_PI / 180.));
  double x_trans_lds = lds_center_to_laser_distance * sin(lds_center_to_laser_angle * (M_PI / 180.));

  int pt_index_0 = (int)(lds_pt_0.y * (double)(cam_width) + lds_pt_0.x);
  int pt_index_1 = (int)(lds_pt_1.y * (double)(cam_width) + lds_pt_1.x);

  cv::Point3f P1(pt_x_running_stat[pt_index_0].Mean() * 1000.0f, pt_y_running_stat[pt_index_0].Mean() * 1000.0f, pt_z_running_stat[pt_index_0].Mean() * 1000.0f);
  cv::Point3f P2(pt_x_running_stat[pt_index_1].Mean() * 1000.0f, pt_y_running_stat[pt_index_1].Mean() * 1000.0f, pt_z_running_stat[pt_index_1].Mean() * 1000.0f);
  cv::Point3f P3(0.0f, -15.0f, -250.683f);
  P3.x = P3.x + (float)x_trans_lds;
  P3.z = P3.z + (float)z_trans_lds;

  // Form the plane normal
  cv::Point3f V1 = P2 - P1;
  cv::Point3f V2 = P3 - P1;
  cv::Point3f n = V1.cross(V2);
  n = n / cv::norm(n);
  // flip the normal if y component is positive, want the normal pointing towards LDS, TOF +y points towards ground.
  // and
  if (n.y > 0.0f) {
    n.x *= -1.0f;
    n.y *= -1.0f;
    n.z *= -1.0f;
  }

  Plane LDS_in_TOF;
  LDS_in_TOF.ConstructFromPointNormal(P3, n);

  Plane TOF_in_TOF;
  cv::Mat Tform_TOF_to_LDS = CreateBasisTransfrom3D(TOF_in_TOF, LDS_in_TOF);
  cv::Mat Tform_LDS_to_TOF = CreateBasisTransfrom3D(LDS_in_TOF, TOF_in_TOF);

  std::cout << "RESULT" << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(0, 0) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(0, 1) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(0, 2) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(0, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(1, 0) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(1, 1) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(1, 2) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(1, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(2, 0) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(2, 1) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(2, 2) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(2, 3) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(3, 0) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(3, 1) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(3, 2) << std::endl;
  std::cout << Tform_TOF_to_LDS.at<float>(3, 3) << std::endl;

  std::string filename = "/user/tof_camera.conf";
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

  //  Very little accuracy gained if incorporating the floor plane, left in comments in case needed in the future
  /*
  // FIND THE FLOOR PLANE
  //
  // Sort out the points which are within the boundary of the fixture floor
  std::vector<cv::Point3f> pt_cloud_vec;
  for (int index = 0; index < NUM_IMAGE_ELEMENTS; ++index) {
    if (fabs(pt_x_running_stat[index].Mean()) < 300.0f && pt_z_running_stat[index].Mean() > 250.0f && pt_z_running_stat[index].Mean() < 450.0f &&
        pt_y_running_stat[index].Mean() > 50.0f) {
      pt_cloud_vec.push_back(cv::Point3f(pt_x_running_stat[index].Mean(), pt_y_running_stat[index].Mean(), pt_z_running_stat[index].Mean()));
    }
  }

  Plane floor_plane = FitPlane(pt_cloud_vec);
  floor_plane.CenterOnBasisOrigin();

  Plane FLOOR_IN_LDS = floor_plane.TransformPlane3D(Tform_TOF_to_LDS);
  FLOOR_IN_LDS.CenterOnBasisOrigin();

  Plane LDS_IN_LDS;
  // Adjust between TOF and LDS Axis by -pi/2 around Z axis
  cv::Mat_<float> adjust_axis = (cv::Mat_<float>(4, 4) << 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  cv::Mat Tform_LDS_to_FLOOR = adjust_axis * CreateBasisTransfrom3D(LDS_IN_LDS, FLOOR_IN_LDS);
  cv::Mat Tform_FLOOR_to_LDS = Tform_LDS_to_FLOOR.inv();

  // Plane FLOOR_IN_TOF = FLOOR_IN_LDS.TransformPlane3D(Tform_LDS_to_TOF);
  // cv::Mat Tform_FLOOR_to_LDS = CreateBasisTransfrom3D(FLOOR_IN_TOF, LDS_in_TOF);
*/
  // Useful for debugging with the captured images.
  /*
    std::string filename1 = "/home/root/data/ntensity_data.dat";
    std::string filename2 = "/home/root/data/pointcloud_data.dat";
    std::ofstream intensityFile;
    std::ofstream pointFile;
    std::stringstream stringStream;
    std::stringstream stringStream2;
    intensityFile.open(filename1, std::ofstream::out);
    pointFile.open(filename2, std::ofstream::out);
    if (intensityFile.fail() || pointFile.fail()) {
      std::cout << "Couldn't open intensity or point output file" << std::endl;
    } else {
      stringStream << pt_intensity_running_stat[0].Mean();
      stringStream2 << pt_x_running_stat[0].Mean() << " " << pt_y_running_stat[0].Mean() << " " << pt_z_running_stat[0].Mean() << std::endl;
      for (size_t index = 1; index < pt_intensity_running_stat.size(); ++index) {
        stringStream << " " << pt_intensity_running_stat[index].Mean();
        stringStream2 << pt_x_running_stat[index].Mean() << " " << pt_y_running_stat[index].Mean() << " " << pt_z_running_stat[index].Mean() << std::endl;
      }
      stringStream << std::endl;
      intensityFile << stringStream.str();
      intensityFile.close();
      pointFile << stringStream2.str();
      pointFile.close();
    }
  */
  if (camera_->stopCapture() != royale::CameraStatus::SUCCESS) {
    std::cout << "Error stopping camera" << std::endl;
  }
  return 0;
}
