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
std::vector<cv::Point3f> pt_cloud;
std::vector<float> pt_amplitude;
std::vector<RunningStat> v_pt_cloud_running_stats;
std::vector<RunningStat> v_pt_amplitude_running_stats;
std::mutex cloudMutex;
std::condition_variable cloudCV;
bool newDataAvailable;
}  // namespace

std::string VERSION{"1.1"};

// Enumerated list of potential camera errors
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

// Command line options
struct options_t {
  std::string version;
  royale::String test_mode;
};

// The plane fitting parameters
class PlaneParams {
 public:
  PlaneParams(float v1, float v2, float v3, float v4, cv::Point3f c);
  float a, b, c, d;
  cv::Point3f centroid;
};

inline PlaneParams::PlaneParams(float v1, float v2, float v3, float v4, cv::Point3f c) : a(v1), b(v2), c(v3), d(v4), centroid(c) {}

// Class for organizing the test metrics
class TestMetrics {
 public:
  TestMetrics();
  float depth_precision_spatial;
  float depth_precision_temporal;
  float depth_precision_temporal_Q90;
  float single_shot_depth_error;
  float depth_accuracy;
  float depth_accuracy_percent;
  float depth_accuracy_percent_Q90;
  float amplitude_std_temporal;
  float amplitude_std;
  float amplitude_mean;
  float amplitude_min;
  float amplitude_max;
  float amplitude_min_of_mins;
  float amplitude_max_of_maxs;
  float plane_tilt_angle;
  float plane_pan_angle;
  float avg_plane_residual;
  float plane_rot_y_axis;
  float plane_rot_x_axis;
};

// Initialize the test metrics
inline TestMetrics::TestMetrics()
    : depth_precision_spatial(0.0f),
      depth_precision_temporal(0.0f),
      depth_precision_temporal_Q90(0.0f),
      single_shot_depth_error(0.0f),
      depth_accuracy(0.0f),
      depth_accuracy_percent(0.0f),
      depth_accuracy_percent_Q90(0.0f),
      amplitude_std_temporal(0.0f),
      amplitude_std(0.0f),
      amplitude_mean(0.0f),
      amplitude_min(10000.0f),
      amplitude_max(-10000.0f),
      amplitude_min_of_mins(10000.0f),
      amplitude_max_of_maxs(-10000.0f),
      plane_tilt_angle(0.0f),
      plane_pan_angle(0.0f),
      avg_plane_residual(0.0f) {}

// Listener for new ToF frames
class MyListener : public royale::IExtendedDataListener {
 public:
  MyListener() {}

  void onNewData(const royale::IExtendedData* data) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    pt_cloud.clear();
    pt_amplitude.clear();
    if (data->hasDepthData() && data->hasIntermediateData()) {
      auto depth = data->getDepthData();
      auto intermed = data->getIntermediateData();
      for (size_t i = 0u; i < NUM_IMAGE_ELEMENTS; ++i) {
        if (depth->points[i].depthConfidence > 0) {
          pt_cloud.push_back(cv::Point3f(depth->points[i].x, depth->points[i].y, depth->points[i].z));
          pt_amplitude.push_back(intermed->points[i].amplitude);
          v_pt_cloud_running_stats[i].Push(depth->points[i].z);
          v_pt_amplitude_running_stats[i].Push(intermed->points[i].amplitude);
        }
      }
    }
    newDataAvailable = true;
    cloudCV.notify_all();
  }
};

// Holds max and min exposure values
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

// Find the value at the 90'th Quartile
float calculate_Q90(const std::vector<float>& data_in) {
  std::vector<float> data_in_copy = data_in;
  std::nth_element(data_in_copy.begin(), data_in_copy.begin() + data_in_copy.size() / 10, data_in_copy.end(), std::greater<float>());
  return data_in_copy[data_in_copy.size() / 10];
}

// For testing if a value is inside a range
bool inRange(std::pair<float, float> range_vals, float val) { return ((val - range_vals.second) * (val - range_vals.first) <= 0); }

// Main Program Entry
int main(int argc, char** argv) {
  // The test results reside in validation_test_metrics
  TestMetrics validation_test_metrics;
  // Default options
  options_t options = {VERSION, "MODE_9_5FPS"};
  std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";
  royale::String useCase;
  std::vector<PlaneParams> plane_coeffs;
  v_pt_cloud_running_stats.resize(NUM_IMAGE_ELEMENTS);
  v_pt_amplitude_running_stats.resize(NUM_IMAGE_ELEMENTS);

  useCase = options.test_mode;

  pt_cloud.reserve(NUM_IMAGE_ELEMENTS);
  pt_amplitude.reserve(NUM_IMAGE_ELEMENTS);

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
    std::cerr << "[ERROR] Camera device could not be initialized. " << royale::getStatusString(status).c_str() << std::endl;
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
          std::cerr << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " << royale::getStatusString(status).c_str() << std::endl;
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
    std::cerr << "[ERROR] Could not register the extended data listener" << royale::getStatusString(status).c_str() << std::endl;
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

  // set ToF module processing parameters
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
  if (camera_->setProcessingParameters({ToF_test_params::ADAPTIVE_NOISE_FILTER_TYPE}) != royale::CameraStatus::SUCCESS) {
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
  std::vector<cv::Mat> vec_pt_amplitude;
  int num_frames_collected = 0;
  int test_sample_size = 20;

  while (num_frames_collected < test_sample_size) {
    std::unique_lock<std::mutex> lock(cloudMutex);
    auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    if (cloudCV.wait_until(lock, timeOut, [&] { return newDataAvailable; })) {
      cv::Mat A = cv::Mat(pt_cloud).reshape(1).t();
      cv::Mat B = cv::Mat(pt_amplitude);
      vec_pt_cloud.push_back(A);
      vec_pt_amplitude.push_back(B);
      num_frames_collected++;
      newDataAvailable = false;
    }
  }

  RunningStat single_shot_depth_error_j;
  RunningStat amplitude_uniformity_j;

  // Fit a plane to each frame saving the plane equation coefficients into a
  // vector, plane_coeffs Using Least Squares
  float fux, fuy, fuz;
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  float d = 0.0f;
  float a_avg = 0.0f;
  float b_avg = 0.0f;
  float c_avg = 0.0f;
  float d_avg = 0.0f;
  float ave_dist_to_plane;
  for (size_t frame_index = 0u; frame_index < vec_pt_cloud.size(); ++frame_index) {
    cv::Mat& frame_pt_cloud = vec_pt_cloud[frame_index];
    cv::Scalar ux = cv::mean(frame_pt_cloud.row(0));
    cv::Scalar uy = cv::mean(frame_pt_cloud.row(1));
    cv::Scalar uz = cv::mean(frame_pt_cloud.row(2));

    fux = static_cast<float>(ux.val[0]);
    fuy = static_cast<float>(uy.val[0]);
    fuz = static_cast<float>(uz.val[0]);

    cv::Mat uxyz(3, 1, CV_32F);
    uxyz.at<float>(0, 0) = fux;
    uxyz.at<float>(1, 0) = fuy;
    uxyz.at<float>(2, 0) = fuz;

    cv::Mat uMat;
    cv::repeat(uxyz, 1, frame_pt_cloud.cols, uMat);
    cv::Mat frame_pt_cloud_bar = frame_pt_cloud - uMat;
    cv::Mat CC = frame_pt_cloud_bar * frame_pt_cloud_bar.t();
    CC = CC / (static_cast<float>(frame_pt_cloud.cols) - 1.0f);
    cv::Mat e_values, e_vectors;
    cv::eigen(CC, e_values, e_vectors);
    a = e_vectors.at<float>(2, 0);
    b = e_vectors.at<float>(2, 1);
    c = e_vectors.at<float>(2, 2);
    d = a * fux + b * fuy + c * fuz;
    PlaneParams cur_plane(a, b, c, d, cv::Point3f(fux, fuy, fuz));
    plane_coeffs.push_back(cur_plane);
    a_avg += a;
    b_avg += b;
    c_avg += c;
    d_avg += d;
  }

  // Average plane fit normal
  a_avg /= static_cast<float>(test_sample_size);
  b_avg /= static_cast<float>(test_sample_size);
  c_avg /= static_cast<float>(test_sample_size);
  d_avg /= static_cast<float>(test_sample_size);
  float norm_val = sqrt(a_avg * a_avg + b_avg * b_avg + c_avg * c_avg);
  a_avg /= norm_val;
  b_avg /= norm_val;
  c_avg /= norm_val;

  validation_test_metrics.plane_rot_x_axis = atan2f(a_avg, c_avg) * (180.0f / M_PIf32);
  validation_test_metrics.plane_rot_y_axis = asinf(b) * (180.0f / M_PIf32);
  ave_dist_to_plane = d_avg;

  // clear out running stats objects
  single_shot_depth_error_j.Clear();
  amplitude_uniformity_j.Clear();
  float shot_sum_val = 0.0f;
  float x_, y_, z_;
  for (size_t frame_index = 0u; frame_index < vec_pt_cloud.size(); ++frame_index) {
    cv::Mat& frame_amplitude = vec_pt_amplitude[frame_index];
    cv::Mat& frame_pt_cloud = vec_pt_cloud[frame_index];
    PlaneParams cur_plane = plane_coeffs[frame_index];
    std::vector<float> z_diff(frame_pt_cloud.cols, 0.0f);
    std::vector<float> vec_plane_residuals(frame_pt_cloud.cols, 0.0f);
    shot_sum_val = 0.0f;
    for (int pt_index = 0; pt_index < frame_pt_cloud.cols; ++pt_index) {
      // calculate spatial depth precision
      x_ = frame_pt_cloud.at<float>(0, pt_index);
      y_ = frame_pt_cloud.at<float>(1, pt_index);
      z_ = frame_pt_cloud.at<float>(2, pt_index);

      // subtract plane fit from z_ coordinate
      z_diff[pt_index] = z_ - (cur_plane.d - cur_plane.a * x_ - cur_plane.b * y_) / cur_plane.c;

      // shot distance
      shot_sum_val += (z_ - ToF_test_params::kGroundTruthDistance);

      // running stats for amplitude
      amplitude_uniformity_j.Push(frame_amplitude.at<float>(pt_index));

      // test for amplitude max of max's
      if (frame_amplitude.at<float>(pt_index) > validation_test_metrics.amplitude_max_of_maxs) {
        validation_test_metrics.amplitude_max_of_maxs = frame_amplitude.at<float>(pt_index);
      }
      // test for amplitude min of min's
      if (frame_amplitude.at<float>(pt_index) < validation_test_metrics.amplitude_min_of_mins) {
        validation_test_metrics.amplitude_min_of_mins = frame_amplitude.at<float>(pt_index);
      }
      // Calcualte plane residuals
      cv::Point3f diff = cv::Point3f(x_, y_, z_) - cur_plane.centroid;
      float sqrlen = diff.dot(diff);
      float dot_ = diff.dot(cv::Point3f(a, b, c));
      vec_plane_residuals[pt_index] = std::fabs(sqrlen - dot_ * dot_);
    }
    cv::Scalar avg, std_dev;
    cv::meanStdDev(cv::Mat(z_diff), cv::noArray(), std_dev, cv::Mat());
    float f_std_dev = static_cast<float>(std_dev.val[0]);
    // spatial depth precision
    validation_test_metrics.depth_precision_spatial += f_std_dev;

    // running stats for single shot
    single_shot_depth_error_j.Push(shot_sum_val / static_cast<float>(frame_pt_cloud.cols));

    // amplitude standard deviation
    validation_test_metrics.amplitude_std += amplitude_uniformity_j.StandardDeviation();

    // average plane residuals
    cv::meanStdDev(cv::Mat(vec_plane_residuals), avg, cv::noArray(), cv::Mat());
    validation_test_metrics.avg_plane_residual += static_cast<float>(avg.val[0]);
  }

  // Finalize Averages
  validation_test_metrics.depth_precision_spatial /= static_cast<float>(test_sample_size);
  validation_test_metrics.avg_plane_residual /= static_cast<float>(test_sample_size);
  validation_test_metrics.amplitude_std /= static_cast<float>(test_sample_size);

  float std_dev = 0.0f;
  float mean_val = 0.0f;
  // The min # frames a pixel must be valid for temporal stats.
  float sample_threshold_size = static_cast<float>(test_sample_size) / 2.0f;
  // The number of pixels passing the sample_threshold_size
  float pixel_count = 0.0f;
  // Holds std(z)_i
  std::vector<float> v_depth_precision_temporal_std;
  // Holds (mean(z)_i - ground_truth) / ground_truth
  std::vector<float> v_depth_accuracy_percent;
  for (size_t pt_index = 0u; pt_index < NUM_IMAGE_ELEMENTS; ++pt_index) {
    // only use pixels seen in this many "sample_threshold_size" frames
    if (v_pt_cloud_running_stats[pt_index].NumDataValues() >= sample_threshold_size) {
      // running stat for temporal depth precision
      std_dev = v_pt_cloud_running_stats[pt_index].StandardDeviation();
      v_depth_precision_temporal_std.push_back(std_dev);
      validation_test_metrics.depth_precision_temporal += std_dev;

      // depth accuracy and percent
      validation_test_metrics.depth_accuracy += (v_pt_cloud_running_stats[pt_index].Mean() - ToF_test_params::kGroundTruthDistance);
      v_depth_accuracy_percent.push_back((v_pt_cloud_running_stats[pt_index].Mean() - ToF_test_params::kGroundTruthDistance) / ToF_test_params::kGroundTruthDistance);

      // amplitude temporal standard deviation
      validation_test_metrics.amplitude_std_temporal += v_pt_amplitude_running_stats[pt_index].StandardDeviation();

      // amplitude mean
      mean_val = v_pt_amplitude_running_stats[pt_index].Mean();
      validation_test_metrics.amplitude_mean += mean_val;

      // amplitude min
      if (mean_val < validation_test_metrics.amplitude_min) {
        validation_test_metrics.amplitude_min = mean_val;
      }
      // amplitude max
      if (mean_val > validation_test_metrics.amplitude_max) {
        validation_test_metrics.amplitude_max = mean_val;
      }
      pixel_count++;
    }
  }

  // finalize avarages and statistics
  validation_test_metrics.depth_precision_temporal /= pixel_count;
  validation_test_metrics.depth_accuracy /= pixel_count;
  validation_test_metrics.amplitude_std_temporal /= pixel_count;
  validation_test_metrics.amplitude_mean /= pixel_count;
  validation_test_metrics.depth_accuracy_percent = 100.0f * (validation_test_metrics.depth_accuracy / ToF_test_params::kGroundTruthDistance);
  validation_test_metrics.single_shot_depth_error = single_shot_depth_error_j.Mean();
  // Q90
  validation_test_metrics.depth_precision_temporal_Q90 = calculate_Q90(v_depth_precision_temporal_std);
  validation_test_metrics.depth_accuracy_percent_Q90 = 100.0f * calculate_Q90(v_depth_accuracy_percent);

  // plane distance, rotation, equation, and residual parameters
  if (std::fabs(ave_dist_to_plane - ToF_test_params::kGroundTruthDistance) > ToF_testing_limits::ground_truth_distance_check) {
    std::cout << "[WARNING]:";
  } else {
    std::cout << "[PASS]:";
  }
  std::cout << "Ground_Truth_Distance_Check:" << ToF_test_params::kGroundTruthDistance << "/" << ave_dist_to_plane << std::endl;
  std::cout << "[INFO]:A:" << a_avg << ":B:" << b_avg << ":C:" << c_avg << ":D:" << d_avg << std::endl;
  std::cout << "[INFO]:Fit_Residual:" << validation_test_metrics.avg_plane_residual << ":Percent_Error:" << 100.0f * ((validation_test_metrics.avg_plane_residual - 0.01) / 0.01)
            << std::endl;
  std::cout << "[INFO]:Plane_Rotation_X:" << validation_test_metrics.plane_rot_x_axis << ":Y:" << validation_test_metrics.plane_rot_y_axis << std::endl;

  // test if test metrics are within limits
  inRange(ToF_testing_limits::depth_precision_spatial, validation_test_metrics.depth_precision_spatial) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Spatial_Depth_Precision:" << validation_test_metrics.depth_precision_spatial << std::endl;

  inRange(ToF_testing_limits::depth_precision_temporal, validation_test_metrics.depth_precision_temporal) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Temporal_Depth_Precision:" << validation_test_metrics.depth_precision_temporal << std::endl;

  inRange(ToF_testing_limits::depth_precision_temporal_Q90, validation_test_metrics.depth_precision_temporal_Q90) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Temporal_Depth_Precision_Q90:" << validation_test_metrics.depth_precision_temporal_Q90 << std::endl;

  inRange(ToF_testing_limits::single_shot_depth_error, validation_test_metrics.single_shot_depth_error) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Single_Shot_Depth_Error:" << validation_test_metrics.single_shot_depth_error << std::endl;

  inRange(ToF_testing_limits::depth_accuracy, validation_test_metrics.depth_accuracy) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Depth_Accuracy:" << validation_test_metrics.depth_accuracy << std::endl;

  inRange(ToF_testing_limits::depth_accuracy_percent, validation_test_metrics.depth_accuracy_percent) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Depth_Accuracy_Percent:" << validation_test_metrics.depth_accuracy_percent << std::endl;

  inRange(ToF_testing_limits::depth_accuracy_percent_Q90, validation_test_metrics.depth_accuracy_percent_Q90) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Depth_Accuracy_Percent_Q90:" << validation_test_metrics.depth_accuracy_percent_Q90 << std::endl;

  inRange(ToF_testing_limits::amplitude_std, validation_test_metrics.amplitude_std) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Std_Dev:" << validation_test_metrics.amplitude_std << std::endl;

  inRange(ToF_testing_limits::amplitude_std_temporal, validation_test_metrics.amplitude_std_temporal) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Temporal_Std_Dev:" << validation_test_metrics.amplitude_std_temporal << std::endl;

  inRange(ToF_testing_limits::amplitude_mean, validation_test_metrics.amplitude_mean) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Mean:" << validation_test_metrics.amplitude_mean << std::endl;

  inRange(ToF_testing_limits::amplitude_max, validation_test_metrics.amplitude_max) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Max:" << validation_test_metrics.amplitude_max << std::endl;

  inRange(ToF_testing_limits::amplitude_min, validation_test_metrics.amplitude_min) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Min:" << validation_test_metrics.amplitude_min << std::endl;

  inRange(ToF_testing_limits::amplitude_max_max, validation_test_metrics.amplitude_max_of_maxs) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Max_Of_Maxs:" << validation_test_metrics.amplitude_max_of_maxs << std::endl;

  inRange(ToF_testing_limits::amplitude_min_min, validation_test_metrics.amplitude_min_of_mins) ? std::cout << "[PASS]:" : std::cout << "[FAIL]:";
  std::cout << "Amplitude_Min_Of_Mins:" << validation_test_metrics.amplitude_min_of_mins << std::endl;

  if (camera_->stopCapture() != royale::CameraStatus::SUCCESS) {
    std::cout << "Error stopping camera" << std::endl;
  }
  return 0;
}
