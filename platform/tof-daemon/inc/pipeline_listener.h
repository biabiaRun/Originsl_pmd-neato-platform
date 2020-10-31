/*
 * pipeline_listener.h
 *
 * Class definition of a Royale listener object.
 */

#ifndef _pipeline_listener_h_
#define _pipeline_listener_h_

#include <fstream>
#include <unordered_map>
#include <royale.hpp>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>
#include <opencv2/core/mat.hpp>

class MyListener : public royale::IDepthDataListener
{
public:
  // MyListener(const bool quiet, const std::string pointcloud_dir);
  MyListener(std::mutex& ptcloud_mutex,
             std::condition_variable& ptcloud_cv,
             const bool quiet,
             std::vector<int64_t>* royale_data_timeStamp,
             std::vector<float>* vec_point_cloud_X,
             std::vector<float>* vec_point_cloud_Y,
             std::vector<float>* vec_point_cloud_Z,
             std::vector<float>* vec_point_cloud_distance,
             cv::Mat& mat_nnet_input,
             // std::vector<uint8_t>* vec_nnet_input,
             cv::Mat& mat_gray_image,
             // std::vector<uint8_t>* vec_gray,
             bool* new_data_available);

  ~MyListener();

  void SaveDensePointcloud(const std::string &filename, const std::string &filename_gray, const royale::DepthData *data);
  void onNewData(const royale::DepthData *data) override;
  float ClipValue(float min_value, float value, float max_value);

private:

  void DisplayTimeDeltaMS(const struct timeval &startTime, const struct timeval &endTime, const char *comment);
  // void FindDepthDataConfidentRange();
  // void CreateNeuralInputData();

  // Save 3D pointcloud as PLY
  // std::string m_pointcloud_dir;
  // std::string m_grayscale_dir;
  // Timestamps
  std::mutex& m_ptcloud_mutex;
  std::condition_variable& m_ptcloud_cv;

  // Should debug messages be suppressed?
  bool m_quiet;
  int64_t m_last_frame_timestamp_long;
  int64_t m_royale_data_timeStamp;
  std::vector<int64_t>* m_depth_data_timeStamp;
  std::vector<float>* m_vec_point_cloud_X;
  std::vector<float>* m_vec_point_cloud_Y;
  std::vector<float>* m_vec_point_cloud_Z;
  std::vector<float>* m_vec_point_cloud_distance;
  cv::Mat m_mat_nnet_input;
  //std::vector<uint8_t>* m_vec_nnet_input;
  cv::Mat m_mat_gray_image;
  // std::vector<uint8_t>* m_gray_image;
  bool* m_new_data_available;
  float m_atanf_norm;
  uint8_t m_invalid_depth;

  // const royale::DepthData  *m_currentDataset;
  // royale::Vector<royale::DepthPoint> m_depth_points;
};

#endif