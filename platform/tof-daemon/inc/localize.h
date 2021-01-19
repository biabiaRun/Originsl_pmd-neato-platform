#ifndef _localize_h_
#define _localize_h_

#include <numeric>
#include <opencv2/core/mat.hpp>

class localize {
 public:
  localize();
  size_t localize_roi(const cv::Mat& img_roi, cv::Mat& ptcloud_depth_roi, cv::Mat& ptcloud_x_roi, cv::Mat& ptcloud_z_roi, const int& threshold, std::vector<float>& x_vals,
                      std::vector<float>& z_vals);

 private:
  template <typename T>
  std::vector<std::size_t> compute_order(const std::vector<T>& v);
};

#endif