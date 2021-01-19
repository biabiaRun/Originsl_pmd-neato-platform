/*
 * pipeline_listener.h
 *
 * Class definition of a Royale listener object.
 */

#ifndef _pipeline_listener_h_
#define _pipeline_listener_h_

#include <royale.hpp>

class MyListener : public royale::IDepthDataListener {
 public:
  // MyListener(const bool quiet, const std::string pointcloud_dir);
  MyListener(const bool quiet);
  ~MyListener();
  void onNewData(const royale::DepthData* data) override;
  void DisplayTimeDeltaMS(const struct timespec& startTime, const struct timespec& endTime, const char* comment);

 private:
  // Should debug messages be suppressed?
  bool m_quiet;
  float m_atanf_norm;
  int64_t m_last_frame_timestamp_long;
  int64_t m_royale_data_timeStamp;
  int64_t m_depth_data_timeStamp;
  uint8_t m_invalid_depth;
};

#endif