/*
 * neural_network_params.h
 *
 * Settings for formatting the neural network input and Tensorflow Light
 */

#ifndef _NEURAL_NETWORK_PARAMS_H_
#define _NEURAL_NETWORK_PARAMS_H_

constexpr float GRAY_NORMAL = 4095.0f; // 1200;
constexpr float DEPTH_NORMAL = 7.5f;   // 1.1f;
constexpr float CLIP_DISTANCE_MAX_THRESHOLD = 1.0f;
constexpr float CLIP_DISTANCE_MIN_THRESHOLD = 0.11f;
constexpr uint8_t HIGH_CONFIDENCE_THRESHOLD = 200;
constexpr float DEFAULT_MIN_DIST = 0.1f;
constexpr float DEFAULT_MAX_DIST = 1.1f;
constexpr uint16_t DEFAULT_MIN_GRAY = 1;
constexpr uint16_t DEFAULT_MAX_GRAY = 500;
constexpr float CONFIDENCE_THRESHOLD = 0.6f;
constexpr float NMS_THRESHOLD = 0.3f;
constexpr int NNET_INPUT_WIDTH = 300;
constexpr int NNET_INPUT_HEIGHT = 300;
const std::string NNET_OUTPUT_LAYER = "detection_out";

#endif
