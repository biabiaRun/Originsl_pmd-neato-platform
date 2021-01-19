/*
 * localize.cpp
 *
 * Class implementation for determining the position of an object
 * in the coordinate frame of the Time of Flight sensor.
 */

#include "localize.h"

localize::localize() {}

template <typename T>
std::vector<std::size_t> localize::compute_order(const std::vector<T>& v) {
  std::vector<std::size_t> indices(v.size());
  std::iota(indices.begin(), indices.end(), 0u);
  std::sort(indices.begin(), indices.end(), [&](int lhs, int rhs) { return v[lhs] < v[rhs]; });
  std::vector<std::size_t> res(v.size());
  for (std::size_t i = 0; i != indices.size(); ++i) {
    res[indices[i]] = i;
  }
  return res;
}

size_t localize::localize_roi(const cv::Mat& img_roi, cv::Mat& ptcloud_depth_roi, cv::Mat& ptcloud_x_roi, cv::Mat& ptcloud_z_roi, const int& threshold, std::vector<float>& x_vals,
                              std::vector<float>& z_vals) {
  int i, j;
  float min_dist;
  int min_idx_i, min_idx_j;
  size_t num_points = 0;

  for (j = 0; j < img_roi.cols; j++) {
    min_dist = 100.0f;
    min_idx_i = -1;
    min_idx_j = -1;
    for (i = 0; i < img_roi.rows; i++) {
      if ((img_roi.at<uint8_t>(i, j) > threshold) && (ptcloud_depth_roi.at<float>(i, j) < min_dist) && (ptcloud_depth_roi.at<float>(i, j) > 0.1f)) {
        min_dist = ptcloud_depth_roi.at<float>(i, j);
        min_idx_i = i;
        min_idx_j = j;
      }
    }

    if (min_idx_i >= 0) {
      x_vals[num_points] = ptcloud_x_roi.at<float>(min_idx_i, min_idx_j);
      z_vals[num_points] = ptcloud_z_roi.at<float>(min_idx_i, min_idx_j);
      num_points++;
    }
  }
  // const auto order = compute_order<float>(x_vals);
  // for (i = 0; i < static_cast<int>(x_vals.size()); i++) {
  // }
  return num_points;
}