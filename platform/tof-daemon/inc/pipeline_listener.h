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

class MyListener : public royale::IDepthDataListener
{
public:
  // MyListener(const bool quiet, const std::string pointcloud_dir);
  MyListener(const bool quiet, const std::string pointcloud_dir, const std::string grayscale_dir,
            std::vector<float>* vec_x,
            std::vector<float>* vec_y,
            std::vector<float>* vec_z,
            std::vector<uint16_t>* vec_gray);
  ~MyListener();

  void SaveDensePointcloud(const std::string &filename, const std::string &filename_gray, const royale::DepthData *data);
  void onNewData(const royale::DepthData *data) override;

  std::vector<float>* m_point_cloud_x;
  std::vector<float>* m_point_cloud_y;
  std::vector<float>* m_point_cloud_z;
  std::vector<uint16_t>* m_gray_image;

private:

  void DisplayTimeDeltaMS(const struct timeval &startTime, const struct timeval &endTime, const char *comment);

  // Should debug messages be suppressed?
  bool m_quiet;
  // Save 3D pointcloud as PLY
  std::string m_pointcloud_dir;
  std::string m_grayscale_dir;
  // Timestamps
  int64_t m_last_frame_timestamp_long;
  int64_t m_royale_data_timeStamp;
  // royale::Vector<royale::DepthPoint> m_depth_points;

};

#endif