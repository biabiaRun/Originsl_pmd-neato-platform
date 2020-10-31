/*
 * otsu_threshold.cpp
 *
 * Class implementation of Otsu Thresholding.
 */

#include "otsu_threshold.h"
#include <float.h>
#include <math.h>


otsu_threshold::otsu_threshold() {
  num_hist_bins = 256;
  hist_buff = new uint8_t[256]();
}

otsu_threshold::~otsu_threshold() {
  if (hist_buff)
    delete[] hist_buff;
}

void otsu_threshold::init_buffer() {
  memset(hist_buff, 0, num_hist_bins * sizeof(uint8_t));
}

int otsu_threshold::get_threshold(const cv::Mat& img_roi) {
  const int N = num_hist_bins;
  int i, j;
  int num_pixels = img_roi.cols * img_roi.rows;
  init_buffer();

  for (i = 0; i < img_roi.rows; i++) {
    const uint8_t* src = img_roi.ptr<uint8_t>(i, 0);
    j = 0;
    for ( ; j < img_roi.cols; j++) {
      if (src[j] < 255) {
        hist_buff[src[j]]++;
      } else {
        num_pixels--;
      }
    }
  }

  double mu = 0.;
  double scale = 1./num_pixels;
  for (i = 0; i < N; i++) {
    mu += i*(double)hist_buff[i];
  }
  mu *= scale;

  double mu1 = 0.;
  double q1 = 0.;
  double max_sigma = 0.;
  int max_val = 0.;

  for (i = 0; i < N; i++) {
    double p_i, q2, mu2, sigma;

    p_i = hist_buff[i] * scale;
    mu1 *= q1;
    q1 += p_i;
    q2 = 1. - q1;

    if ( std::min(q1, q2) < FLT_EPSILON || std::max(q1,q2) > 1. - FLT_EPSILON )
      continue;

    mu1 = (mu1 + i*p_i)/q1;
    mu2 = (mu - q1*mu1)/q2;
    sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
    if (sigma > max_sigma) {
      max_sigma = sigma;
      max_val = i;
    }
  }
  return max_val;
}