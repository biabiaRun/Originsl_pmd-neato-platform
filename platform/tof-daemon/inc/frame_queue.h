/*
 * frame_queue.h
 *
 * Class definition of data structure that handles camera frames
 * in a double-ended queue
 */

#ifndef _frame_queue_h_
#define _frame_queue_h_

#include <deque>
#include <mutex>
#include <opencv2/core/utility.hpp>

/*
 * Pointcloud X,Y,Z values
 *  vec_point_cloud_X, vec_point_cloud_Y, vec_point_cloud_Z
 * Euclidean distance to cloud points
 *  vec_point_cloud_distance
 * Cloud point gray values
 *  mat_nnet_input
 * Cloud point formulated neural net input values
 *  mat_nnet_input
 * Timestamp given by Royale API
 *  royale_data_timestamp
 * Timestamp given by the CLOCK_MONOTONIC clock in ms
 *  system_timestamp
 */
struct FrameDataStruct {
  static constexpr unsigned int sensor_num_columns{224};
  static constexpr unsigned int sensor_num_rows{172};
  static constexpr unsigned long buffer_size{38528};
  // Sensor Size is 224 x 172, requiring 38528 storage elements
  std::vector<float> vec_point_cloud_X = std::vector<float>(buffer_size);
  std::vector<float> vec_point_cloud_Y = std::vector<float>(buffer_size);
  std::vector<float> vec_point_cloud_Z = std::vector<float>(buffer_size);
  std::vector<float> vec_point_cloud_distance = std::vector<float>(buffer_size);
  cv::Mat mat_nnet_input =
      cv::Mat(sensor_num_rows, sensor_num_columns, CV_8UC3);
  cv::Mat mat_gray_image = cv::Mat(sensor_num_rows, sensor_num_columns, CV_8U);
  int64_t royale_data_timestamp;
  double system_timestamp;
};

template <typename T> class FrameQueue {
public:
  FrameQueue() : counter(0) {}

  template<class U>
  void push_front(U&& entry) {
    std::lock_guard<std::mutex> lock(mutex);

    deque.emplace_front(std::forward<U>(entry));
    counter += 1;
    if (counter == 1) {
      // Start counting from second frame to warmup.
      tm.reset();
      tm.start();
    }
  }

  T get_fresh() {
    std::lock_guard<std::mutex> lock(mutex);
    T entry = deque.front();
    deque.pop_back();
    return entry;
  }

  T get_fresh_and_pop() {
    std::lock_guard<std::mutex> lock(mutex);
    T entry = std::move(deque.front());
    deque.clear();
    return entry;
  }

  T get_back() {
    std::lock_guard<std::mutex> lock(mutex);
    T entry = deque.back();
    return entry;
  }

  float getFPS() {
    tm.stop();
    double fps = counter / tm.getTimeSec();
    tm.start();
    return static_cast<float>(fps);
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex);
    deque.clear();
    // Have seen odd behavior with clear, fallback is below
    // while (!this->empty()) this->pop_back();
  }

  bool empty() const noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    return deque.empty();
  }

  typename std::deque<T>::size_type size() const noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    return deque.size();
  }

  unsigned int counter;

private:
  cv::TickMeter tm;
  mutable std::mutex mutex;
  std::deque<T> deque;
};

#endif
