/*
 * pipeline_listener.cpp
 *
 * Class implementation of a Royale listener object to execute
 * the object avoidance pipeline.
 */

#include "pipeline_listener.h"
// #include "common.h"
#include "neural_network_params.h"

#include <memory>
#include <syslog.h>
#include <math.h>

/*
MyListener::MyListener(const bool quiet, const std::string pointcloud_dir, const std::string grayscale_dir,  std::vector<float>* vec_x, std::vector<float>* vec_y, std::vector<float>* vec_z,
                      std::vector<uint8_t>* vec_nnet_depth, std::vector<uint16_t>* vec_gray) :
    m_quiet(quiet), m_pointcloud_dir(pointcloud_dir), m_grayscale_dir(grayscale_dir), m_last_frame_timestamp_long(0), m_royale_data_timeStamp(0)
{
  m_point_cloud_x = vec_x;
  m_point_cloud_y = vec_y;
  m_point_cloud_z = vec_z;
  m_point_cloud_nnet = vec_nnet_depth;
  m_gray_image = vec_gray;
  m_colorHelper = new ColorHelper();
  m_colorHelper->setMinDist(0.1f);
  m_colorHelper->setMaxDist(1.8f);
  m_colorHelper->setMinVal(1);
  m_colorHelper->setMaxVal(500);
  m_currentDataset = nullptr;
}
*/

// False Coloring Colormap look-up table
static constexpr uint8_t lookup_table_rgb [256][3] = {{0, 0, 131}, {0, 0, 135}, {0, 0, 139}, {0, 0, 143}, {0, 0, 147}, {0, 0, 151}, {0, 0, 155},
  {0, 0, 159}, {0, 0, 163}, {0, 0, 167}, {0, 0, 171}, {0, 0, 175}, {0, 0, 179}, {0, 0, 183}, {0, 0, 187}, {0, 0, 191}, {0, 0, 195}, {0, 0, 199},
  {0, 0, 203}, {0, 0, 207}, {0, 0, 211}, {0, 0, 215}, {0, 0, 219}, {0, 0, 223}, {0, 0, 227}, {0, 0, 231}, {0, 0, 235}, {0, 0, 239}, {0, 0, 243},
  {0, 0, 247}, {0, 0, 251}, {0, 0, 255}, {0, 3, 255}, {0, 7, 255}, {0, 11, 255}, {0, 15, 255}, {0, 19, 255}, {0, 23, 255}, {0, 27, 255},
  {0, 31, 255}, {0, 35, 255}, {0, 39, 255}, {0, 43, 255}, {0, 47, 255}, {0, 51, 255}, {0, 55, 255}, {0, 59, 255}, {0, 63, 255}, {0, 67, 255},
  {0, 71, 255}, {0, 75, 255}, {0, 79, 255}, {0, 83, 255}, {0, 87, 255}, {0, 91, 255}, {0, 95, 255}, {0, 99, 255}, {0, 103, 255}, {0, 107, 255},
  {0, 111, 255}, {0, 115, 255}, {0, 119, 255}, {0, 123, 255}, {0, 127, 255}, {0, 131, 255}, {0, 135, 255}, {0, 139, 255}, {0, 143, 255}, {0, 147, 255},
  {0, 151, 255}, {0, 155, 255}, {0, 159, 255}, {0, 163, 255}, {0, 167, 255}, {0, 171, 255}, {0, 175, 255}, {0, 179, 255}, {0, 183, 255}, {0, 187, 255},
  {0, 191, 255}, {0, 195, 255}, {0, 199, 255}, {0, 203, 255}, {0, 207, 255}, {0, 211, 255}, {0, 215, 255}, {0, 219, 255}, {0, 223, 255}, {0, 227, 255},
  {0, 231, 255}, {0, 235, 255}, {0, 239, 255}, {0, 243, 255}, {0, 247, 255}, {0, 251, 255}, {0, 255, 255}, {3, 255, 251}, {7, 255, 247}, {11, 255, 243},
  {15, 255, 239}, {19, 255, 235}, {23, 255, 231}, {27, 255, 227}, {31, 255, 223}, {35, 255, 219}, {39, 255, 215}, {43, 255, 211}, {47, 255, 207},
  {51, 255, 203}, {55, 255, 199}, {59, 255, 195}, {63, 255, 191}, {67, 255, 187}, {71, 255, 183}, {75, 255, 179}, {79, 255, 175}, {83, 255, 171},
  {87, 255, 167}, {91, 255, 163}, {95, 255, 159}, {99, 255, 155}, {103, 255, 151}, {107, 255, 147}, {111, 255, 143}, {115, 255, 139}, {119, 255, 135},
  {123, 255, 131}, {127, 255, 127}, {131, 255, 123}, {135, 255, 119}, {139, 255, 115}, {143, 255, 111}, {147, 255, 107}, {151, 255, 103}, {155, 255, 99},
  {159, 255, 95}, {163, 255, 91}, {167, 255, 87}, {171, 255, 83}, {175, 255, 79}, {179, 255, 75}, {183, 255, 71}, {187, 255, 67}, {191, 255, 63},
  {195, 255, 59}, {199, 255, 55}, {203, 255, 51}, {207, 255, 47}, {211, 255, 43}, {215, 255, 39}, {219, 255, 35}, {223, 255, 31}, {227, 255, 27},
  {231, 255, 23}, {235, 255, 19}, {239, 255, 15}, {243, 255, 11}, {247, 255, 7}, {251, 255, 3}, {255, 255, 0}, {255, 251, 0}, {255, 247, 0},
  {255, 243, 0}, {255, 239, 0}, {255, 235, 0}, {255, 231, 0}, {255, 227, 0}, {255, 223, 0}, {255, 219, 0}, {255, 215, 0}, {255, 211, 0}, {255, 207, 0},
  {255, 203, 0}, {255, 199, 0}, {255, 195, 0}, {255, 191, 0}, {255, 187, 0}, {255, 183, 0}, {255, 179, 0}, {255, 175, 0}, {255, 171, 0}, {255, 167, 0},
  {255, 163, 0}, {255, 159, 0}, {255, 155, 0}, {255, 151, 0}, {255, 147, 0}, {255, 143, 0}, {255, 139, 0}, {255, 135, 0}, {255, 131, 0}, {255, 127, 0},
  {255, 123, 0}, {255, 119, 0}, {255, 115, 0}, {255, 111, 0}, {255, 107, 0}, {255, 103, 0}, {255, 99, 0}, {255, 95, 0}, {255, 91, 0}, {255, 87, 0},
  {255, 83, 0}, {255, 79, 0}, {255, 75, 0}, {255, 71, 0}, {255, 67, 0}, {255, 63, 0}, {255, 59, 0}, {255, 55, 0}, {255, 51, 0}, {255, 47, 0},
  {255, 43, 0}, {255, 39, 0}, {255, 35, 0}, {255, 31, 0}, {255, 27, 0}, {255, 23, 0}, {255, 19, 0}, {255, 15, 0}, {255, 11, 0}, {255, 7, 0},
  {255, 3, 0}, {255, 0, 0}, {251, 0, 0}, {247, 0, 0}, {243, 0, 0}, {239, 0, 0}, {235, 0, 0}, {231, 0, 0}, {227, 0, 0}, {223, 0, 0}, {219, 0, 0},
  {215, 0, 0}, {211, 0, 0}, {207, 0, 0}, {203, 0, 0}, {199, 0, 0}, {195, 0, 0}, {191, 0, 0}, {187, 0, 0}, {183, 0, 0}, {179, 0, 0}, {175, 0, 0},
  {171, 0, 0}, {167, 0, 0}, {163, 0, 0}, {159, 0, 0}, {155, 0, 0}, {151, 0, 0}, {147, 0, 0}, {143, 0, 0}, {139, 0, 0}, {135, 0, 0}, {131, 0, 0}, {127, 0, 0}};

MyListener::MyListener(std::mutex& ptcloud_mutex,
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
                      bool* new_data_available)
: m_ptcloud_mutex(ptcloud_mutex),
  m_ptcloud_cv(ptcloud_cv),
  m_quiet(quiet),
  m_last_frame_timestamp_long(0)
{
  m_depth_data_timeStamp = royale_data_timeStamp;
  m_vec_point_cloud_X = vec_point_cloud_X;
  m_vec_point_cloud_Y = vec_point_cloud_Y;
  m_vec_point_cloud_Z = vec_point_cloud_Z;
  m_vec_point_cloud_distance = vec_point_cloud_distance;
  m_mat_nnet_input = mat_nnet_input;
  // m_vec_nnet_input = vec_nnet_input;
  // m_gray_image = vec_gray;
  m_mat_gray_image = mat_gray_image;
  m_new_data_available = new_data_available;
  m_atanf_norm = (255.0f/90.0f) * static_cast<float>(180. / M_PI);
  m_invalid_depth = 255;
  // m_currentDataset = nullptr;
}

MyListener::~MyListener()
{
}

/**
 * SaveDensePointcloud
 *
 * Streams the pointcloud data to a local file in a .ply format
 * For an explanation of the PLY file format please have a look at
 * https://en.wikipedia.org/wiki/PLY_(file_format)
*/
/*
void MyListener::SaveDensePointcloud(const std::string &filename, const std::string &filename_gray, const royale::DepthData *data) {
  std::ofstream outputFile;
  std::stringstream stringStream;

  std::ofstream outputFile_gray;
  std::stringstream stringStream_gray;
  uint16_t gray_val;
  uint16_t mask = 255;
  uint8_t byte_low, byte_high;

  outputFile_gray.open (filename_gray, std::ofstream::out);
  if (outputFile_gray.fail()) {
    std::cerr << "Outputfile " << filename_gray << " could not be opened!" << std::endl;
    return;
  }

  outputFile.open (filename, std::ofstream::out);
  if (outputFile.fail()) {
    std::cerr << "Outputfile " << filename << " could not be opened!" << std::endl;
    return;
  } else {
    // if the file was opened successfully write the PLY header
    stringStream << "ply" << std::endl;
    stringStream << "format ascii 1.0" << std::endl;
    stringStream << "comment Generated by tof-daemon" << std::endl;
    stringStream << "element vertex " << data->points.size() << std::endl;
    stringStream << "property float x" << std::endl;
    stringStream << "property float y" << std::endl;
    stringStream << "property float z" << std::endl;
    stringStream << "element face 0" << std::endl;
    stringStream << "property list uchar int vertex_index" << std::endl;
    stringStream << "end_header" << std::endl;

    // output XYZ coordinates into one line
    for (size_t i = 0; i < data->points.size(); ++i) {
        stringStream << data->points[i].x << " " << data->points[i].y << " " << data->points[i].z << std::endl;

        gray_val = data->points[i].grayValue;
        byte_low = static_cast<uint8_t>((gray_val) & mask);
        byte_high = static_cast<uint8_t>((gray_val >> 8) & mask);

        stringStream_gray << byte_low << " " << byte_high << std::endl;
        // reconstruct like this: gray_val = byte_low | (byte_high << 8);
    }
    // output stringstream to file and close it
    outputFile << stringStream.str();
    outputFile.close();

    outputFile_gray << stringStream_gray.str();
    outputFile_gray.close();
  }
}
*/

inline float MyListener::ClipValue(float min_value, float value, float max_value) {
  return value < min_value ? min_value : (value > max_value ? max_value : value);
}
// inline float clamp(float x, float a, float b){    return x < a ? a : (x > b ? b : x);}

// Helper to output time deltas for time benchmaking
void MyListener::DisplayTimeDeltaMS(const struct timeval &startTime, const struct timeval &endTime, const char *comment) {
  const double TIME_DELTA = static_cast<double>(endTime.tv_sec - startTime.tv_sec) * 1000. + static_cast<double>(endTime.tv_usec - startTime.tv_usec) / 1000.;
  if(!m_quiet) {
    syslog(LOG_NOTICE, "TOFDaemon %s: time delta took %g milliseconds\n", comment, TIME_DELTA);
  }
}

/*
// Find the detph and grayscale ranges of DepthData points with a high confidence
// Update the color helper with the new range(s)
void MyListener::FindDepthDataConfidentRange() {
  unsigned int numPoints = m_currentDataset->height * m_currentDataset->width;
  const royale::DepthPoint *currentPoint = &m_currentDataset->points[0];
  float minDist = std::numeric_limits<float>::max();
  float maxDist = std::numeric_limits<float>::min();
  uint16_t minGray = std::numeric_limits<uint16_t>::max();
  uint16_t maxGray = std::numeric_limits<uint16_t>::min();

  for (auto idx = 0u; idx < numPoints; ++idx, ++currentPoint) {

    if (currentPoint->z > CLIP_DISTANCE_MAX_THRESHOLD || currentPoint->z < CLIP_DISTANCE_MIN_THRESHOLD) {
      continue;
    }

    if (currentPoint->depthConfidence > HIGH_CONFIDENCE_THRESHOLD) {
      if (currentPoint->z < minDist) {
        minDist = currentPoint->z;
      } else if (currentPoint->z > maxDist) {
        maxDist = currentPoint->z;
      }

      if (currentPoint->grayValue < minGray) {
        minGray = currentPoint->grayValue;
      } else if (currentPoint->grayValue > maxGray) {
        maxGray = currentPoint->grayValue;
      }
    }
  }

  if (minDist <  std::numeric_limits<float>::max()) {
    m_colorHelper->setMinDist(minDist);
  }
  if (maxDist > std::numeric_limits<float>::min()) {
    m_colorHelper->setMaxDist(maxDist);
  }
  if (minGray < std::numeric_limits<uint16_t>::max()) {
    m_colorHelper->setMinVal(minGray);
  }
  if (maxGray > std::numeric_limits<uint16_t>::min()) {
    m_colorHelper->setMaxVal(maxGray);
  }
}


void MyListener::CreateNeuralInputData() {

  if (!m_currentDataset) {
    return;
  }

  size_t rgb_index = 0;
  uint8_t invalid_depth_blue = static_cast<uint8_t>(0);  // static_cast<uint8_t>(255);
  uint8_t invalid_depth_red_green = static_cast<uint8_t>(0);

  float grayVal, scaleTmp, scale, distance;
  unsigned int numPoints = m_currentDataset->height * m_currentDataset->width;
  const royale::DepthPoint *currentPoint = &m_currentDataset->points[0];

  for (auto idx = 0u; idx < numPoints; ++idx, ++currentPoint) {
    if (currentPoint->z > CLIP_DISTANCE_MAX_THRESHOLD) {
      (*m_tensorflow_input)[rgb_index] = invalid_depth_red_green;
      rgb_index++;
      (*m_tensorflow_input)[rgb_index] = invalid_depth_red_green;
      rgb_index++;
      (*m_tensorflow_input)[rgb_index] = invalid_depth_blue;
      rgb_index++;
    } else {
      distance = sqrtf( currentPoint->x * currentPoint->x +
                 currentPoint->y * currentPoint->y +
                 currentPoint->z * currentPoint->z);
      const RgbColor &curColor = m_colorHelper->getColor(distance);
      grayVal = (float) m_colorHelper->getGrayColor(currentPoint->grayValue).r;
      scaleTmp = 8.0f * std::pow(distance, 1.5f) * grayVal / 255.f;
      scale = ClipValue(0.f, scaleTmp, 1.f);
      (*m_tensorflow_input)[rgb_index] = static_cast<uint8_t> (scale * curColor.r);
      rgb_index++;
      (*m_tensorflow_input)[rgb_index] = static_cast<uint8_t> (scale * curColor.g);
      rgb_index++;
      (*m_tensorflow_input)[rgb_index] = static_cast<uint8_t> (scale * curColor.b);
      rgb_index++;
    }
  }
}
*/

/**
 * onNewData
 *
 * Callback to execute the object avoidance pipeline.
 */
void MyListener::onNewData(const royale::DepthData *data) {
  static int counter = 0;
  counter++;

  struct timespec start, stop;
  double accum;

  if( clock_gettime( CLOCK_MONOTONIC, &start) == -1 ) {
    syslog(LOG_ERR, "Error: Failed to get clock start time\n");
  }
  // struct timeval startTime;
  // gettimeofday(&startTime, NULL);

  // m_currentDataset = data;

  /* long long cur_timestamp_long = (long long) static_cast<long long>(startTime.tv_sec) * 1000L + static_cast<long long>(startTime.tv_usec) / 1000L;
  if(m_last_frame_timestamp_long > 0) {
      if( (cur_timestamp_long - m_last_frame_timestamp_long) < 120 ) {
          std::string timestamp_diff = std::to_string(cur_timestamp_long - m_last_frame_timestamp_long);
          syslog(LOG_NOTICE, "Skipping Frame : Timestamp Diff = %s\n", timestamp_diff.c_str());
          return;
      }else{
          m_last_frame_timestamp_long = cur_timestamp_long;
      }
  }else{
      m_last_frame_timestamp_long = cur_timestamp_long;
      m_royale_data_timeStamp = data->timeStamp;
  }*/

  // m_depth_points =  data->points;

  std::unique_lock<std::mutex> lock (m_ptcloud_mutex);

  m_royale_data_timeStamp = data->timeStamp.count();
  (*m_depth_data_timeStamp)[0] = m_royale_data_timeStamp / 1000L;

  int64_t time_diff =(m_royale_data_timeStamp - m_last_frame_timestamp_long) / 1000L;
  // syslog(LOG_NOTICE, "timestamp %ld  :::  previous %ld\n", m_royale_data_timeStamp, m_last_frame_timestamp_long);
  syslog(LOG_NOTICE, "TIME DIFF TOF ::: %ld\n", time_diff);
  m_last_frame_timestamp_long = m_royale_data_timeStamp;

  struct timespec start_lock;
  double accum_lock;
  if( clock_gettime( CLOCK_MONOTONIC, &start_lock) == -1 ) {
    syslog(LOG_ERR, "Error: Failed to get clock start_lock time\n");
  }

  float x_val, y_val, z_val, depth_ratio, depth_norm, gray_norm;
  size_t img_width = data->width;
  size_t img_height = data->height;
  size_t numPoints = img_height * img_width;
  uint8_t depth_uint;
  int row, column;
  size_t idx;

  const royale::DepthPoint *currentPoint = &data->points[0];

  // m_royale_data_timeStamp = data->timeStamp;
  for (idx = 0u; idx < numPoints; ++idx, ++currentPoint) {
    row = (int)(idx / img_width);
    column = idx % img_width;

    gray_norm = static_cast<float>(currentPoint->grayValue);
    x_val = currentPoint->x;
    y_val = currentPoint->y;
    z_val = currentPoint->z;
    (*m_vec_point_cloud_X)[idx] = x_val;
    (*m_vec_point_cloud_Y)[idx] = y_val;
    (*m_vec_point_cloud_Z)[idx] = z_val;

    // m_mat_gray_image.at<uchar>(row, column) = static_cast<uchar>(cv::pow(gray_norm / 4095.0f, 0.4) * 255.0f);
    // (*m_gray_image)[idx] = static_cast<uint8_t>(255.0f * (gray_norm / 600.0f));

    if (z_val > CLIP_DISTANCE_MAX_THRESHOLD || z_val < 1e-6) {
      depth_uint = m_invalid_depth;
    } else {
      (*m_vec_point_cloud_distance)[idx] = sqrtf(x_val*x_val + y_val*y_val + z_val*z_val);
      depth_norm = (*m_vec_point_cloud_distance)[idx] / DEPTH_NORMAL;
      gray_norm /= GRAY_NORMAL;
      if (gray_norm < 1e-6) {
        depth_ratio = atanf(depth_norm);
      } else {
        depth_ratio = atanf(depth_norm / gray_norm);
      }
      depth_uint = static_cast<uint8_t>(m_atanf_norm * depth_ratio);
      if (depth_uint < 1) {
        depth_uint = m_invalid_depth;
      }
    }
    // (*m_gray_image)[idx] = depth_uint;
    // m_mat_gray_image.at<uchar>(row, column) = depth_uint;
    m_mat_gray_image.at<uint8_t>(row, column) = static_cast<uint8_t>(255.0f/(1300.0f * currentPoint->noise));
    // (*m_vec_nnet_input)[idx] = depth_uint;
    cv::Vec3b& bgr = m_mat_nnet_input.at<cv::Vec3b>(row, column);
    bgr[0] = lookup_table_rgb[depth_uint][0];
    bgr[1] = lookup_table_rgb[depth_uint][1];
    bgr[2] = lookup_table_rgb[depth_uint][2];
    // bgr[0] = lookup_table_rgb[depth_uint][2];
    // bgr[1] = lookup_table_rgb[depth_uint][1];
    // bgr[2] = lookup_table_rgb[depth_uint][0];
  }


// Fast calculation of square root
// http://bits.stephan-brumme.com/squareRoot.html
// i = *(unsigned int*) &dist_sq;
// adjust bias
// i  += 127 << 23;
// approximation of square root
// i >>= 1;
// (*m_point_cloud_d)[index] = (*(float*) &i) / DEPTH_NORM;


  /*
  // Test if m_pointcloud_dir is not empty
  if ((counter < 11) && !m_pointcloud_dir.empty()) {
    // If a directory is defined then save pointcloud
    //struct timeval tp;
    //gettimeofday(&tp, NULL);
    // Must cast to long long to avoid overflows
    // long long msec_long = (long long) startTime.tv_sec * 1000L + startTime.tv_usec / 1000;
    // std::string timestamp = std::to_string(msec_long);
    std::string timestamp = std::to_string(m_royale_data_timeStamp);
    std::string filename = "densePointCloud_" + timestamp + ".ply";
    filename = m_pointcloud_dir + filename;
    // std::string lclpth = "/home/root/tof-data-repo/";
    // filename = lclpth + filename;
    std::string filename_gray = "grayscale_" + timestamp + ".raw16";
    filename_gray = m_grayscale_dir + filename_gray;
    // filename_gray = lclpth + filename_gray;

    SaveDensePointcloud(filename, filename_gray, data);
  }
  */

  if( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
    syslog(LOG_ERR, "Error: Failed to get clock stop time\n");
  }

  accum = static_cast<double>( stop.tv_sec - start.tv_sec ) * 1000.0 + static_cast<double>( stop.tv_nsec - start.tv_nsec ) / 1000000.0;
  syslog(LOG_NOTICE, "TIME DIFF SYSTEM *:::* %lf\n", accum);

  accum_lock = static_cast<double>( stop.tv_sec - start_lock.tv_sec ) * 1000.0 + static_cast<double>( stop.tv_nsec - start_lock.tv_nsec ) / 1000000.0;
  syslog(LOG_NOTICE, "TIME DIFF SYSTEM_LOCK *:::* %lf\n", accum_lock);
  // struct timeval finalTime;
  // gettimeofday(&finalTime, NULL);
  // DisplayTimeDeltaMS(startTime, finalTime, "Total Elapsed Time: ");

  *m_new_data_available = true;
  m_ptcloud_cv.notify_all();
}
