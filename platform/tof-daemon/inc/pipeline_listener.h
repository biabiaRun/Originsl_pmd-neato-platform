/*
 * pipeline_listener.h
 *
 * Class definition of a Royale listener object.
 */

#ifndef _pipeline_listener_h_
#define _pipeline_listener_h_

#include "frame_queue.h"
#include <royale.hpp>

extern FrameQueue<FrameDataStruct> gFramesQueue;

class TOFDataListener : public royale::IDepthDataListener {
public:
  TOFDataListener(const bool verbose);
  ~TOFDataListener();
  void onNewData(const royale::DepthData *data) override;
  void DisplayTimeDeltaMS(const struct timespec &start_time,
                          const struct timespec &end_time, const char *comment);

private:
  // Toggle to display debug messages
  bool verbose_;
  const float atanf_norm_ = (255.0f / 90.0f) * static_cast<float>(180. / M_PI);
  int64_t last_frame_timestamp_;
  int64_t m_royale_data_timeStamp;
  int64_t m_depth_data_timeStamp;
  const uint8_t invalid_depth_ = 255;
};

#endif
