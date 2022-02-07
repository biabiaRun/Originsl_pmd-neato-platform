/*
 * tof_daemon_params.h
 *
 * Settings for the TOF Sensor are defined here.
 */

#include <vector>

#ifndef _TOF_DAEMON_PARAMS_H_
#define _TOF_DAEMON_PARAMS_H_

// Name and version of the TOF Daemon
const char *DAEMON_NAME = "TOFDaemon";
const char *REVISION = "1.0.0";

const bool USE_FLYING_PIXEL = false;
const bool USE_STRAY_LIGHT = true;
const bool USE_ADAPTIVE_NOISE_FILTER = true;
const bool USE_VALIDATE_IMAGE = false;
const bool USE_SMOOTHING_FILTER = false;
const bool USE_SBI_FLAG = false;
const bool USE_HOLE_FILLING = false;

// Multipath interference filters
const bool USE_MPI_AVERAGE = true;
const bool USE_MPI_AMP = true;
const bool USE_MPI_DIST = true;
const bool USE_FILTER_2_FREQ = true;

const int ADAPTIVE_NOISE_FILTER_TYPE = 1;
const float NOISE_THRESHOLD = 0.07f;
const int GLOBAL_BINNING = 1;

// Exposure time in microseconds
const float AUTO_EXPOSURE_REF_VALUE =
    1000.0f; // 400.0f;// AutoExposureRefValue_Float

// File paths used when saving the TOF data for dedbugging
std::string LOCAL_DATA_PATH = "/home/root/tof-data-repo/";
std::string IR_FOLDER("/gray-img/");
std::string PTCLOUD_FOLDER("/dense-pc/");
std::string DEPTH_IMAGE_FOLDER("/depth-img/");
std::string CALIBRATION_FOLDER("/tof-to-lds-calibration/");

// File path to the "lock file", which when created indicates that the TOF
// daemon already exists.
const char *LOCK_FILE = "/run/tofdaemon.lock";
const char* PID_FILE = "/run/tof-daemon.pid";

// Packet size for streaming the live video through the socket
const int PACK_SIZE = 4096;

// Parameters for image compression when streaming live video
std::vector<int> COMPRESSION_PARAMS{cv::IMWRITE_JPEG_QUALITY, 80};

// Exposure settings for the TOF camera. 0 indicates automatic exposure
// otherwise the exposure is set to the user timing
const std::uint32_t EXPOSURE_TIME_MS = 0;

#endif
