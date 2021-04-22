/*
 * Copyright (c) 2019, Neato Robotics, Inc.. All Rights Reserved.
 *
 * This file may contain contributions from others.
 *
 * This software is proprietary to Neato Robotics, Inc. and its transference
 * and use is to be strictly controlled.
 * Transference of this software to another party requires that all of the
 * following conditions be met:
 * 	A)	Neato has a copy of a signed NDA agreement with the receiving
 *      party
 * 	B)	Neato Software Engineering has explicitly authorized the
 *      receiving party to have a copy of this software
 * 	C)	When the work is completed or terminated by the receiving party,
 *      all copies of this software that the receiving party holds must be
 *      returned to Neato, or destroyed.
 * The receiving party is under legal obligation to not disclose or  transfer
 * this software.
 * The receiving party may not appropriate, transform or re-use this software
 * for any purpose other than a Neato Robotics authorized purpose.
 */

/**
 * @brief Camera processing parameters and application constants grouped by namespace
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <royale.hpp>

namespace ToF_test_params {
// Ground Truth Distance in meters
constexpr float kGroundTruthDistance = 0.27f;
// Confidence threshold for pixel samples (value from 0 (invalid) to 255 (full confidence))
constexpr float kMinConfidenceThreshold = 200.0f;
// Sensor Parameter settings
static const royale::ProcessingParameterPair NOISE_THRESHOLD({royale::ProcessingFlag::NoiseThreshold_Float, 0.07f});
static const royale::ProcessingParameterPair AUTO_EXPOSURE_REF_VALUE({royale::ProcessingFlag::AutoExposureRefValue_Float, 1000.0f});
static const royale::ProcessingParameterPair ADAPTIVE_NOISE_FILTER_TYPE({royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, 2});
static const royale::ProcessingParameterPair GLOBAL_BINNING({royale::ProcessingFlag::GlobalBinning_Int, 1});
static const royale::ProcessingParameterPair USE_ADAPTIVE_NOISE_FILTER({royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, true});
static const royale::ProcessingParameterPair USE_FLYING_PIXEL({royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, false});
static const royale::ProcessingParameterPair USE_MPI_AVERAGE({royale::ProcessingFlag::UseMPIFlagAverage_Bool, true});
static const royale::ProcessingParameterPair USE_MPI_AMP({royale::ProcessingFlag::UseMPIFlag_Amp_Bool, true});
static const royale::ProcessingParameterPair USE_MPI_DIST({royale::ProcessingFlag::UseMPIFlag_Dist_Bool, true});
static const royale::ProcessingParameterPair USE_VALIDATE_IMAGE({royale::ProcessingFlag::UseValidateImage_Bool, false});
static const royale::ProcessingParameterPair USE_STRAY_LIGHT({royale::ProcessingFlag::UseRemoveStrayLight_Bool, true});
static const royale::ProcessingParameterPair USE_FILTER_2_FREQ({royale::ProcessingFlag::UseFilter2Freq_Bool, true});
static const royale::ProcessingParameterPair USE_SBI_FLAG({royale::ProcessingFlag::UseFlagSBI_Bool, false});
static const royale::ProcessingParameterPair USE_SMOOTHING_FILTER({royale::ProcessingFlag::UseSmoothingFilter_Bool, false});
static const royale::ProcessingParameterPair USE_HOLE_FILLING({royale::ProcessingFlag::UseHoleFilling_Bool, false});
}  // namespace ToF_test_params

namespace ToF_testing_limits {
// See documentation an-se-3-7-validation_test_points.pdf in /docs folder for explanation of testing limits below.
// Depth metrics were specified by PMD and Amplitude metrics are specific to our custom diffuser and values were
// determined by statistical sampling.
// A pair defines the range (min,max) for the specific test to pass.
static const int confident_pixel_count(34000);
static const float ground_truth_distance_check(0.003f);
static const std::pair<float, float> depth_precision_spatial(-0.013, 0.013);
static const std::pair<float, float> depth_precision_temporal(-0.01, 0.01);
static const std::pair<float, float> depth_precision_temporal_Q90(-0.01, 0.01);
static const std::pair<float, float> single_shot_depth_error(-0.013, 0.013);
static const std::pair<float, float> depth_accuracy(-0.013, 0.013);
static const std::pair<float, float> depth_accuracy_percent(-10.0, 10.0);
static const std::pair<float, float> depth_accuracy_percent_Q90(-10.0, 10.0);
static const std::pair<float, float> amplitude_std_temporal(0.0, 150.0);
static const std::pair<float, float> amplitude_std(0.0, 350.0);
static const std::pair<float, float> amplitude_mean(0.0, 600.0);
static const std::pair<float, float> amplitude_max(0.0, 2000.0);
static const std::pair<float, float> amplitude_min(0.0, 1000.0);
static const std::pair<float, float> amplitude_max_max(0.0, 2000.0);
static const std::pair<float, float> amplitude_min_min(0.0, 1000.0);
}  // namespace ToF_testing_limits

namespace ToF_calibration_params {

/*

ToF Coordinate Frame

      ToF Camera
       _________
      //       /`\
      K    +--|--------------> Z-axis
      \\__/|___\./
_________/_|______________________________
        /  |                             /
       /   |                            /
      /    |                           /
    \/     V Y-axis                   /
 X-axis                              /
                             FLOOR  /
___________________________________/

Robot LDS Coordinate Frame
,----------------------*
|                       **
|     ^                  **
|     | Y-axis            *
|     |                   **
|    _|_                  **
|   / | \                 /|
|  |  +--|---------->     [+
|   \___/     X-axis      \|
|                         **
|                         **
|                         *
|                        **
|                       **
|______________________*

*/
// Default calibration timeout in seconds
static const int kCalibrationTtimeoutSeconds(60);
// Roboctrl command to Start LDS
static const std::string kStartLdsString = "roboctrl -d b";
// Roboctrl command to Stop LDS
static const std::string kStopLdsString = "roboctrl -d s";
// Laser offset from the LDS center
static const double kLdsCenterToLaserAngle = 57.;  // Degrees
static const double kLdsCenterToLaserDistance = 22.768;  // Millimeters
// ToF to LDS position offset as taken from Mechanical drawings
// R
// ToF sensor origin is a focal point of lens
// LDS origin is at center of rotation
// X offset
static const float kLdsOffsetX(0.0f);
// Y offset
static const float kLdsOffsetY(-15.0f);
// Z offset
static const float kLdsOffsetZ(-250.683f);
// Exposure update retry threshold
static const int kNumExposureRetries(5);
// LDS Detection Threshold - Intensity values > threshold are considered signal from the LDS laser
static const float kLdsDetectionThreshold(30.0f);
// LDS Sample Size - Required sample size of detected LDS intensity points for line fitting.
static const int kLdsSampleSize(1000);
// TOF pixel high confidence threshold
static const float kConfidentPixelThreshold(200.0f);
// TOF Sample Size - Number of ToF depth frames to average over and reduce temporal noise
static const int kTofSampleSize(100);
// Row & Column Detection Bounds - Clip noisy areas and look between row:(top, bottom), col:(left,right) for LDS intensity signal.
static const std::pair<int, int> kRowDetectionBounds(20, 149);
static const std::pair<int, int> kColDetectionBounds(20, 200);
//  Threshold the span of the detected LDS signal to maximize resolution, set to 85% of kColDetectionBounds
static const float kLdsPtsPlaneFitMinRange = 153.0f;
// Sensor Parameter settings
static const royale::ProcessingParameterPair NOISE_THRESHOLD({royale::ProcessingFlag::NoiseThreshold_Float, 0.01f});
static const royale::ProcessingParameterPair AUTO_EXPOSURE_REF_VALUE({royale::ProcessingFlag::AutoExposureRefValue_Float, 1000.0f});
static const royale::ProcessingParameterPair ADAPTIVE_NOISE_FILTER_TYPE({royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, 2});
static const royale::ProcessingParameterPair GLOBAL_BINNING({royale::ProcessingFlag::GlobalBinning_Int, 1});
static const royale::ProcessingParameterPair USE_ADAPTIVE_NOISE_FILTER({royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, true});
static const royale::ProcessingParameterPair USE_FLYING_PIXEL({royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, false});
static const royale::ProcessingParameterPair USE_MPI_AVERAGE({royale::ProcessingFlag::UseMPIFlagAverage_Bool, true});
static const royale::ProcessingParameterPair USE_MPI_AMP({royale::ProcessingFlag::UseMPIFlag_Amp_Bool, true});
static const royale::ProcessingParameterPair USE_MPI_DIST({royale::ProcessingFlag::UseMPIFlag_Dist_Bool, true});
static const royale::ProcessingParameterPair USE_VALIDATE_IMAGE({royale::ProcessingFlag::UseValidateImage_Bool, true});
static const royale::ProcessingParameterPair USE_STRAY_LIGHT({royale::ProcessingFlag::UseRemoveStrayLight_Bool, true});
static const royale::ProcessingParameterPair USE_FILTER_2_FREQ({royale::ProcessingFlag::UseFilter2Freq_Bool, true});
static const royale::ProcessingParameterPair USE_SBI_FLAG({royale::ProcessingFlag::UseFlagSBI_Bool, false});
static const royale::ProcessingParameterPair USE_SMOOTHING_FILTER({royale::ProcessingFlag::UseSmoothingFilter_Bool, false});
static const royale::ProcessingParameterPair USE_HOLE_FILLING({royale::ProcessingFlag::UseHoleFilling_Bool, false});
}  // namespace ToF_calibration_params

#endif
