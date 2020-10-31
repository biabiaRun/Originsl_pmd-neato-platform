#ifndef _otsu_threshold_h_
#define _otsu_threshold_h_

#include <opencv2/core/mat.hpp>

class otsu_threshold {

  public:
    otsu_threshold();
    ~otsu_threshold();
    int get_threshold(const cv::Mat& img_roi);
  private:
    void init_buffer();
    uint8_t* hist_buff;
    int num_hist_bins;
};

#endif