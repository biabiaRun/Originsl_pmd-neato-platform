#ifndef _OTSU_THRESHOLD_H_
#define _OTSU_THRESHOLD_H_

#include <opencv2/core/mat.hpp>

/**
 * @brief Class implementation of Otsu Thresholding.
 */
class OtsuThresholding {

public:
  /**
   * @brief Otsu thresholding constructor that initializes the histogram buffer
   */
  OtsuThresholding();

  /**
   * @brief Otsu thresholding destructor that clears the histogram buffer
   */
  ~OtsuThresholding();

  /**
   * @brief Default getter function to retrieve the previously calculated otsu
   * threshold.
   * @return Previously calculated otsu threshold
   */
  int GetThreshold() { return otsu_threshold_; };

  /**
   * @brief Overloaded getter function to first calculate and then return the
   * otsu threshold of the input.
   * @param img_roi Image that the calculation should be performed on
   * @return Calculated otsu threshold
   */
  int GetThreshold(const cv::Mat &img_roi) {
    CalculateThreshold(img_roi);
    return GetThreshold();
  };

private:
  /**
   * @brief Initialize the histogram buffer with the correct number of bins
   */
  void InitializeBuffer();

  /**
   * @brief Calculate the otsu threshold on the given image and set the
   * otsu_threshold_ member variable to the calculated value
   * @param img_roi Image that the calculation should be performed on
   */
  void CalculateThreshold(const cv::Mat &img_roi);

  // Number of bins for the histogram
  const int kNumHistogramBins = 256;

  // The histogram buffer
  uint8_t *histogram_buffer_;

  // The calculated otsu threshold
  int otsu_threshold_;
};

#endif
