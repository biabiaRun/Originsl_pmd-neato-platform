/*
 * pipeline_listener.h
 *
 * Class definition of a Royale listener object.
 */

#ifndef _pipeline_listener_h_
#define _pipeline_listener_h_

// #include "cluster.h"

#include <fstream>
#include <unordered_map>
// #include <pcl/io/pcd_io.h>
// #include <pcl/point_types.h>
#include <royale.hpp>
#include <sys/time.h>
// #include <pcl/memory.h>


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

//private:

  // void FilterPointCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr &input, pcl::PointCloud<pcl::PointXYZ>::Ptr &output);
  // void StoreClustersForRobot(const std::unordered_map<int, Cluster> &clusterMap) const;
  // pcl::PointCloud<pcl::PointXYZ>::Ptr CreatePointCloud(const royale::DepthData *data) const;
  void DisplayTimeDeltaMS(const struct timeval &startTime, const struct timeval &endTime, const char *comment);

  // Should debug messages be suppressed?
  bool m_quiet;
  // Save 3D pointcloud as PLY
  std::string m_pointcloud_dir;
  std::string m_grayscale_dir;
  uint16_t m_sensor_width;
  uint16_t m_sensor_height;
  long long m_last_frame_timestamp_long;
  std::chrono::microseconds m_royale_data_timeStamp;
  // royale::Vector<royale::DepthPoint> m_depth_points;
  std::vector<float>* m_point_cloud_x;
  std::vector<float>* m_point_cloud_y;
  std::vector<float>* m_point_cloud_z;
  std::vector<uint16_t>* m_gray_image;

};

#endif