/*
 * pipeline_listener.cpp
 *
 * Class implementation of a Royale listener object to execute
 * the object avoidance pipeline.
 */

#include "pipeline_listener.h"

#include <math.h>
#include <syslog.h>

#include <memory>

#include "neural_network_params.h"

// False Coloring Colormap look-up table
static constexpr uint8_t gRGBLookupTable[256][3] = {
    {0, 0, 131},     {0, 0, 135},     {0, 0, 139},     {0, 0, 143},
    {0, 0, 147},     {0, 0, 151},     {0, 0, 155},     {0, 0, 159},
    {0, 0, 163},     {0, 0, 167},     {0, 0, 171},     {0, 0, 175},
    {0, 0, 179},     {0, 0, 183},     {0, 0, 187},     {0, 0, 191},
    {0, 0, 195},     {0, 0, 199},     {0, 0, 203},     {0, 0, 207},
    {0, 0, 211},     {0, 0, 215},     {0, 0, 219},     {0, 0, 223},
    {0, 0, 227},     {0, 0, 231},     {0, 0, 235},     {0, 0, 239},
    {0, 0, 243},     {0, 0, 247},     {0, 0, 251},     {0, 0, 255},
    {0, 3, 255},     {0, 7, 255},     {0, 11, 255},    {0, 15, 255},
    {0, 19, 255},    {0, 23, 255},    {0, 27, 255},    {0, 31, 255},
    {0, 35, 255},    {0, 39, 255},    {0, 43, 255},    {0, 47, 255},
    {0, 51, 255},    {0, 55, 255},    {0, 59, 255},    {0, 63, 255},
    {0, 67, 255},    {0, 71, 255},    {0, 75, 255},    {0, 79, 255},
    {0, 83, 255},    {0, 87, 255},    {0, 91, 255},    {0, 95, 255},
    {0, 99, 255},    {0, 103, 255},   {0, 107, 255},   {0, 111, 255},
    {0, 115, 255},   {0, 119, 255},   {0, 123, 255},   {0, 127, 255},
    {0, 131, 255},   {0, 135, 255},   {0, 139, 255},   {0, 143, 255},
    {0, 147, 255},   {0, 151, 255},   {0, 155, 255},   {0, 159, 255},
    {0, 163, 255},   {0, 167, 255},   {0, 171, 255},   {0, 175, 255},
    {0, 179, 255},   {0, 183, 255},   {0, 187, 255},   {0, 191, 255},
    {0, 195, 255},   {0, 199, 255},   {0, 203, 255},   {0, 207, 255},
    {0, 211, 255},   {0, 215, 255},   {0, 219, 255},   {0, 223, 255},
    {0, 227, 255},   {0, 231, 255},   {0, 235, 255},   {0, 239, 255},
    {0, 243, 255},   {0, 247, 255},   {0, 251, 255},   {0, 255, 255},
    {3, 255, 251},   {7, 255, 247},   {11, 255, 243},  {15, 255, 239},
    {19, 255, 235},  {23, 255, 231},  {27, 255, 227},  {31, 255, 223},
    {35, 255, 219},  {39, 255, 215},  {43, 255, 211},  {47, 255, 207},
    {51, 255, 203},  {55, 255, 199},  {59, 255, 195},  {63, 255, 191},
    {67, 255, 187},  {71, 255, 183},  {75, 255, 179},  {79, 255, 175},
    {83, 255, 171},  {87, 255, 167},  {91, 255, 163},  {95, 255, 159},
    {99, 255, 155},  {103, 255, 151}, {107, 255, 147}, {111, 255, 143},
    {115, 255, 139}, {119, 255, 135}, {123, 255, 131}, {127, 255, 127},
    {131, 255, 123}, {135, 255, 119}, {139, 255, 115}, {143, 255, 111},
    {147, 255, 107}, {151, 255, 103}, {155, 255, 99},  {159, 255, 95},
    {163, 255, 91},  {167, 255, 87},  {171, 255, 83},  {175, 255, 79},
    {179, 255, 75},  {183, 255, 71},  {187, 255, 67},  {191, 255, 63},
    {195, 255, 59},  {199, 255, 55},  {203, 255, 51},  {207, 255, 47},
    {211, 255, 43},  {215, 255, 39},  {219, 255, 35},  {223, 255, 31},
    {227, 255, 27},  {231, 255, 23},  {235, 255, 19},  {239, 255, 15},
    {243, 255, 11},  {247, 255, 7},   {251, 255, 3},   {255, 255, 0},
    {255, 251, 0},   {255, 247, 0},   {255, 243, 0},   {255, 239, 0},
    {255, 235, 0},   {255, 231, 0},   {255, 227, 0},   {255, 223, 0},
    {255, 219, 0},   {255, 215, 0},   {255, 211, 0},   {255, 207, 0},
    {255, 203, 0},   {255, 199, 0},   {255, 195, 0},   {255, 191, 0},
    {255, 187, 0},   {255, 183, 0},   {255, 179, 0},   {255, 175, 0},
    {255, 171, 0},   {255, 167, 0},   {255, 163, 0},   {255, 159, 0},
    {255, 155, 0},   {255, 151, 0},   {255, 147, 0},   {255, 143, 0},
    {255, 139, 0},   {255, 135, 0},   {255, 131, 0},   {255, 127, 0},
    {255, 123, 0},   {255, 119, 0},   {255, 115, 0},   {255, 111, 0},
    {255, 107, 0},   {255, 103, 0},   {255, 99, 0},    {255, 95, 0},
    {255, 91, 0},    {255, 87, 0},    {255, 83, 0},    {255, 79, 0},
    {255, 75, 0},    {255, 71, 0},    {255, 67, 0},    {255, 63, 0},
    {255, 59, 0},    {255, 55, 0},    {255, 51, 0},    {255, 47, 0},
    {255, 43, 0},    {255, 39, 0},    {255, 35, 0},    {255, 31, 0},
    {255, 27, 0},    {255, 23, 0},    {255, 19, 0},    {255, 15, 0},
    {255, 11, 0},    {255, 7, 0},     {255, 3, 0},     {255, 0, 0},
    {251, 0, 0},     {247, 0, 0},     {243, 0, 0},     {239, 0, 0},
    {235, 0, 0},     {231, 0, 0},     {227, 0, 0},     {223, 0, 0},
    {219, 0, 0},     {215, 0, 0},     {211, 0, 0},     {207, 0, 0},
    {203, 0, 0},     {199, 0, 0},     {195, 0, 0},     {191, 0, 0},
    {187, 0, 0},     {183, 0, 0},     {179, 0, 0},     {175, 0, 0},
    {171, 0, 0},     {167, 0, 0},     {163, 0, 0},     {159, 0, 0},
    {155, 0, 0},     {151, 0, 0},     {147, 0, 0},     {143, 0, 0},
    {139, 0, 0},     {135, 0, 0},     {131, 0, 0},     {127, 0, 0}};

TOFDataListener::TOFDataListener(const bool verbose)
    : verbose_(verbose), last_frame_timestamp_(0) {}

TOFDataListener::~TOFDataListener() {}

// Helper to output time deltas for time benchmaking
void TOFDataListener::DisplayTimeDeltaMS(const struct timespec &start_time,
                                         const struct timespec &end_time,
                                         const char *comment) {
  const double TIME_DELTA =
      static_cast<double>(end_time.tv_sec - start_time.tv_sec) * 1000. +
      static_cast<double>(end_time.tv_nsec - start_time.tv_nsec) / 1000000.;
  if (verbose_) {
    syslog(LOG_NOTICE, "TOFDaemon %s: time delta took %g milliseconds\n",
    comment, TIME_DELTA);
  }
}

/**
 * onNewData
 *
 * Callback to execute the object avoidance pipeline.
 */
void TOFDataListener::onNewData(const royale::DepthData *data) {
  struct timespec start, stop;
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    syslog(LOG_ERR, "Error: Failed to get clock start time in onNewData.\n");
  }

  static long int counter = 0;
  counter++;
  if (verbose_)
    std::cout << "New Data count : " << counter << std::endl;

  FrameDataStruct frame_data;

  m_royale_data_timeStamp = data->timeStamp.count();
  frame_data.royale_data_timeStamp = m_royale_data_timeStamp / 1000L;

  float x_val, y_val, z_val, depth_ratio, depth_norm, gray_norm;
  int row, column;
  size_t img_width = data->width;
  size_t img_height = data->height;
  size_t numPoints = img_height * img_width;
  size_t idx;
  uint8_t depth_uint;

  const royale::DepthPoint *currentPoint = &data->points[0];
  for (idx = 0u; idx < numPoints; ++idx, ++currentPoint) {
    row = (int)(idx / img_width);
    column = (int)(idx % img_width);

    gray_norm = static_cast<float>(currentPoint->grayValue);
    x_val = currentPoint->x;
    y_val = currentPoint->y;
    z_val = currentPoint->z;
    frame_data.vec_point_cloud_X[idx] = x_val;
    frame_data.vec_point_cloud_Y[idx] = y_val;
    frame_data.vec_point_cloud_Z[idx] = z_val;

    // Depth normaliztion - normalize depth values by demodulating with the
    // returned laser intensiy The concept is desribed here S. Zhang, B. Chen,
    // L. Yan, and Z. Xu, "Real-time normalization
    //  and nonlinearity evaluation methods of the PGC-arctan demodulation in an
    // EOM-based sinusoidal phase modulating interferometer," Opt. Express  26,
    // 605-616 (2018).
    // https://www.osapublishing.org/oe/fulltext.cfm?uri=oe-26-2-605&id=380607

    // If cloud point is further than threshold or near 0 skip it.
    if (z_val > CLIP_DISTANCE_MAX_THRESHOLD || z_val < 1e-6) {
      depth_uint = invalid_depth_;
    } else {
      frame_data.vec_point_cloud_distance[idx] =
          sqrtf(x_val * x_val + y_val * y_val + z_val * z_val);
      depth_norm = frame_data.vec_point_cloud_distance[idx] / DEPTH_NORMAL;
      gray_norm /= GRAY_NORMAL;
      // If gray value is near zero do not normalize
      if (gray_norm < 1e-6) {
        depth_ratio = atanf(depth_norm);
      } else {
        depth_ratio = atanf(depth_norm / gray_norm);
      }
      depth_uint = static_cast<uint8_t>(atanf_norm_ * depth_ratio);
      if (depth_uint < 1) {
        depth_uint = invalid_depth_;
      }
    }

    // Scaled noise image used for locating an object within the bounding box
    frame_data.mat_gray_image.at<uint8_t>(row, column) =
        static_cast<uint8_t>(255.0f / (1300.0f * currentPoint->noise));

    // Apply false coloring lookup table to generate neural network input
    // feature image
    cv::Vec3b &bgr = frame_data.mat_nnet_input.at<cv::Vec3b>(row, column);
    bgr[0] = gRGBLookupTable[depth_uint][0];
    bgr[1] = gRGBLookupTable[depth_uint][1];
    bgr[2] = gRGBLookupTable[depth_uint][2];
  }

  // Push the frame data structure to the front of the shared Queue
  gFramesQueue.push_front(frame_data);

  if (verbose_)
    std::cout << "Listener gFramesQueue Size : " << gFramesQueue.size()
              << std::endl;

  if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
    syslog(LOG_ERR, "Error: Failed to get clock stop time onNewData.\n");
  }

  // update timestamp for next iteration
  last_frame_timestamp_ = m_royale_data_timeStamp;
}
