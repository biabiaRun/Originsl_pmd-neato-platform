#include "otsu_threshold.h"

#include <float.h>
#include <math.h>

OtsuThresholding::OtsuThresholding() {
  otsu_threshold_ = 0;
  histogram_buffer_ = new uint8_t[kNumHistogramBins]();
}

OtsuThresholding::~OtsuThresholding() {
  if (histogram_buffer_)
    delete[] histogram_buffer_;
}

void OtsuThresholding::InitializeBuffer() {
  memset(histogram_buffer_, 0, kNumHistogramBins * sizeof(uint8_t));
}

void OtsuThresholding::CalculateThreshold(const cv::Mat &img_roi) {
  int i, j;
  int num_pixels = img_roi.cols * img_roi.rows;
  InitializeBuffer();

  for (i = 0; i < img_roi.rows; i++) {
    const uint8_t *src = img_roi.ptr<uint8_t>(i, 0);
    j = 0;
    for (; j < img_roi.cols; j++) {
      if (src[j] < 255) {
        histogram_buffer_[src[j]]++;
      } else {
        num_pixels--;
      }
    }
  }

  double mu = 0.;
  double scale = 1. / num_pixels;
  for (i = 0; i < kNumHistogramBins; i++) {
    mu += i * (double)histogram_buffer_[i];
  }
  mu *= scale;

  double mu1 = 0.;
  double q1 = 0.;
  double max_sigma = 0.;
  int max_val = 0.;

  for (i = 0; i < kNumHistogramBins; i++) {
    double p_i, q2, mu2, sigma;

    p_i = histogram_buffer_[i] * scale;
    mu1 *= q1;
    q1 += p_i;
    q2 = 1. - q1;

    if (std::min(q1, q2) < FLT_EPSILON || std::max(q1, q2) > 1. - FLT_EPSILON)
      continue;

    mu1 = (mu1 + i * p_i) / q1;
    mu2 = (mu - q1 * mu1) / q2;
    sigma = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);
    if (sigma > max_sigma) {
      max_sigma = sigma;
      max_val = i;
    }
  }

  // Set the threshold member variable to the calculated value
  otsu_threshold_ = max_val;
}
