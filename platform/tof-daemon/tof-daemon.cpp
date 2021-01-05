#include "tof_module_params.h"
#include "pipeline_listener.h"
#include "otsu_threshold.h"
#include "localize.h"
#include "neural_network_params.h"
#include "PracticalSocket.h"
// #include "Model.h"
// #include "Tensor.h"
// #include "common.h"

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string>
#include <CameraFactory.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <ftw.h>
// #include <opencv2/opencv.hpp>
// #include <opencv2/dnn.hpp>
// #include <thread>
// #include <queue>

using namespace std;
using namespace cv;

// #include <royale/ICameraDevice.hpp>
// #include <CameraFactory.hpp>

// Name and version of the TOF Daemon
const char *DAEMON_NAME = "TOFDaemon";
const char *REVISION = "1.0.0";

// Local Data Write Folders
std::string LOCAL_DATA_PATH = "/home/root/tof-data-repo/";
std::string IR_FOLDER("/gray-img/");
std::string PTCLOUD_FOLDER("/dense-pc/");
std::string DEPTH_IMAGE_FOLDER("/depth-img/");
std::string CALIBRATION_FOLDER("/tof-to-lds-calibration/");

/*
 * File path to the "lock file", which when created indicates that the
 * TOF daemon already exists.
 */
const char *LOCK_FILE = "/tmp/tofdaemon.lock";

// File descriptor to the lock file
int lock_fd = -1;

// Should the TOFDaemon continue processing frames?
bool g_daemon_live = true;

// Neural Network Variables
std::vector<std::string> class_names;
dnn::Net net;

// SIGHUP Handler
void sighup_handler(int signo) {
  syslog(LOG_INFO, "Received SIGHUP.  Shutting down TOFDaemon\n");
  g_daemon_live = false;
}

// SIGINT Handler
void sigint_handler(int signo) {
  syslog(LOG_INFO, "Received SIGINT.  Shutting down TOFDaemon\n");
  g_daemon_live = false;
}

/**
 * Shutdown
 *
 * Properly closes file descriptors and gracefully shutsdown daemon.
 */
void Shutdown() {
  if(lock_fd >= 0) {
    if(lockf(lock_fd, F_ULOCK, 0) < 0) {
      syslog(LOG_ERR, "Error %d: Failed to unlock lock file: %s\n", errno, strerror(errno));
    }

    close(lock_fd);
    lock_fd = -1;
    unlink(LOCK_FILE);
  }
}

/**
 * Daemonize
 *
 * Creates a Linux Daemon
 */
int Daemonize(const char *name, const char *path, const char *outfile,
                const char *errfile, const char *infile) {
  const char *DEFAULT_FILE = "/dev/null";

  if(!infile)
      infile = DEFAULT_FILE;
  if(!outfile)
      outfile = DEFAULT_FILE;
  if(!errfile)
      errfile = DEFAULT_FILE;

  // Open syslog
  openlog(name, LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO, "Entering ToF Daemon");

  // Fork the parent to create a child process
  pid_t child = fork();
  if(child < 0) {
    // ERROR: Failed to fork().
    fprintf(stderr, "Error %d: failed to fork: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to fork: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }
  if(child > 0) {
    // Parent process.  Parent exits.
    exit(EXIT_SUCCESS);
  }

  // Create a new Signature Id for the child
  if(setsid() < 0) {
    // ERROR: Failed to become session leader
    fprintf(stderr, "Error %d: failed to setsid: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to setsid: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  // Child initialize signal handlers
  signal(SIGCHLD, SIG_IGN); // Ignore SigChild
  signal(SIGINT, sigint_handler);
  signal(SIGHUP, sighup_handler);

  // Spawn a "grandchild" process; this guarantees that we removed the session leading process
  child = fork();
  if(child < 0) {
    // ERROR: Creating grandchild process failed
    fprintf(stderr, "Error %d: failed to fork: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to fork: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }
  if(child > 0) {
    // Parent (child process) also exits, leaving only the grandchild process active
    exit(EXIT_SUCCESS);
  }

  // New permissions
  umask(0);

  // Change to path directory
  if(chdir(path) < 0) {
    // ERROR: Failed to change grandchild's directory
    fprintf(stderr, "Error %d: failed to chdir: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  // Close all open file descriptors
  int fd;
  for(fd = static_cast<int>(sysconf(_SC_OPEN_MAX)); fd > 0; --fd) {
    close(fd);
  }

  // Reopen stdin/stdout/stderr, pointing to the specified files
  stdin = fopen(infile, "r"); // File descriptor 0
  stdout = fopen(outfile, "w+"); // File descriptor 1
  stderr = fopen(errfile, "w+"); // File descriptor 2

  // Create a "lock file" whose appearance indicates that the TOFDaemon is already created
  lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
  if(lock_fd < 0) {
    // ERROR: failed to create lock file
    syslog(LOG_ERR, "Error %d: failed to open lock file %s: %s", errno, LOCK_FILE,
            strerror(errno));
    return EXIT_FAILURE;
  }

  // Check if the lock file has been locked
  if(lockf(lock_fd, F_TLOCK, 0) < 0) {
    // ERROR: Lock already applied
    syslog(LOG_ERR, "Error: tried to run TOFDaemon twice");
    exit(0);
  }

  // Daemon created; write the PID to the lock file
  char str[16];
  snprintf(str, sizeof(str), "%d\n", getpid());
  if(write(lock_fd, str, strlen(str)) < 0) {
    // ERROR: Failed to write PID to lock file
    syslog(LOG_ERR, "Error %d: Failed to write PID to lock file %s: %s",
      errno, LOCK_FILE, strerror(errno));
    return EXIT_FAILURE;
  }

  return 0;
}


// Create a directory
bool mkpath( std::string path ) {
  bool bSuccess = false;
  int nRC = ::mkdir( path.c_str(), 0775 );
  if ( nRC == -1 ) {
    switch( errno ) {
      case ENOENT:
        //parent didn't exist, try to create it
        if( mkpath( path.substr(0, path.find_last_of('/')) ) )
            //Now, try to create again.
            bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
        else
            bSuccess = false;
        break;
      case EEXIST:
        //Done!
        bSuccess = true;
        break;
      default:
        bSuccess = false;
        break;
    }
  } else {
      bSuccess = true;
  }
  return bSuccess;
}


// Remove files to save space
static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb) {
  if(remove(pathname) < 0) {
    syslog(LOG_ERR, "Error: Failed to remove files\n");
    return -1;
  }
  return 0;
}


/**
 * Init_TOF_Streaming
 * Initialize the folder structure for saving TOF data
 *
 */
void Init_TOF_Streaming() {
  char folder_id[100];
  // Create unique folder identifier from timestamp
  time_t cur_time = time(NULL);
  const std::string FORMATSTRING( "%Y-%m-%d_%H-%M-%S" );
	strftime( folder_id, sizeof(folder_id), FORMATSTRING.c_str(), gmtime( &cur_time ) );

  IR_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + IR_FOLDER;
  PTCLOUD_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + PTCLOUD_FOLDER;
  DEPTH_IMAGE_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + DEPTH_IMAGE_FOLDER;
  CALIBRATION_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + CALIBRATION_FOLDER;

  // Clean out the LOCAL_DATA_PATH folder by removing it
  if (nftw(LOCAL_DATA_PATH.c_str(), rmFiles,10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0) {
    syslog(LOG_ERR, "Failed to clean up TOF %s output directory", LOCAL_DATA_PATH.c_str());
  } else {
    syslog(LOG_ERR, "Cleaned up TOF %s output directory", LOCAL_DATA_PATH.c_str());
  }
  // Create the output directories
  if ( !mkpath( IR_FOLDER ) ) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory", IR_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory", IR_FOLDER.c_str());
  }
  if ( !mkpath( PTCLOUD_FOLDER ) ) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory", PTCLOUD_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory", PTCLOUD_FOLDER.c_str());
  }
  if ( !mkpath( DEPTH_IMAGE_FOLDER ) ) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory", DEPTH_IMAGE_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory", DEPTH_IMAGE_FOLDER.c_str());
  }
  if ( !mkpath( CALIBRATION_FOLDER ) ) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory", CALIBRATION_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory", CALIBRATION_FOLDER.c_str());
  }
}

void SavePointcloud(const std::string &filename, const std::string &filename_gray, const std::vector<float>* vec_x, const std::vector<float>* vec_y, const std::vector<float>* vec_z,
                    const std::vector<uint16_t>* vec_gray) {
  std::ofstream outputFile;
  std::stringstream stringStream;

  std::ofstream outputFile_gray;
  std::stringstream stringStream_gray;
  uint16_t mask = 255;

  outputFile_gray.open (filename_gray, std::ofstream::out);
  if (outputFile_gray.fail()) {
    std::cerr << "Outputfile " << filename_gray << " could not be opened!" << std::endl;
    return;
  }

  outputFile.open (filename, std::ofstream::out);
  if (outputFile.fail()) {
    std::cerr << "Outputfile " << filename << " could not be opened!" << std::endl;
    return;
  } else {
    // if the file was opened successfully write the PLY header
    stringStream << "ply" << std::endl;
    stringStream << "format ascii 1.0" << std::endl;
    stringStream << "comment Generated by tof-daemon" << std::endl;
    stringStream << "element vertex " << vec_x->size() << std::endl;
    stringStream << "property float x" << std::endl;
    stringStream << "property float y" << std::endl;
    stringStream << "property float z" << std::endl;
    stringStream << "element face 0" << std::endl;
    stringStream << "property list uchar int vertex_index" << std::endl;
    stringStream << "end_header" << std::endl;

    // output XYZ coordinates into one line
    for (size_t i = 0; i < vec_x->size(); ++i) {
        stringStream << (*vec_x)[i] << " " << (*vec_y)[i] << " " << (*vec_z)[i] << std::endl;
        stringStream_gray << ((*vec_gray)[i] & mask) << " " << (((*vec_gray)[i] >> 8) & mask) << std::endl;
        // reconstruct like this: gray_val = byte_low | (byte_high << 8);
    }
    // output stringstream to file and close it
    outputFile << stringStream.str();
    outputFile.close();

    outputFile_gray << stringStream_gray.str();
    outputFile_gray.close();
  }
}


/*
int otsu_threshold(cv::Mat& img_roi) {
  int hist_size = 256;
  float range[] = { 0, 256 }; // upper boundary is exclusive
  const float* hist_range = range;
  cv::Mat img_hist;
  // mask out the invalid pixels equal to 255
  cv::Mat img_mask = img_roi != 255;
  double min_val, max_val;
  cv::minMaxLoc(img_roi, &min_val, &max_val, NULL, NULL, img_mask);
  cv::calcHist(&img_roi, 1, 0, img_mask, img_hist, 1, &hist_size, &hist_range, true, false);
}
*/

int main(int argc, char **argv) {

  // Parse command line arguments
  int c;
  // Suppress debug messages?
  bool suppress = false;
  // Save ToF Data?oONo1

  bool save_data = false; //true; //false;

  std::mutex ptcloud_mutex;
  std::condition_variable ptcloud_cv;
  bool new_data_available = false;

  while((c = getopt(argc, argv, "qs")) != -1) {
    switch(c) {
      case 's':
        save_data = true;
        break;
      case 'q':
        // Suppress the debug statements
        suppress = true;
        break;
      default:
        std::cout << "Usage: " << DAEMON_NAME << " [OPTION]..." << std::endl;
        std::cout << "  -q\tsuppress debug statements " << std::endl;
        std::cout << "  -s\tsave TOF data to server " << std::endl;
        exit(EXIT_FAILURE);
        break;
    }
  }

  struct timeval startTime;
  gettimeofday(&startTime, NULL);

  // Spawn this process as a daemon
  int ret = Daemonize(DAEMON_NAME, "/tmp", NULL, NULL, NULL);
  if(ret != 0) {
    // ERROR: Failed to create a daemon
    fprintf(stderr, "Error: Failed to create daemon\n");
    syslog(LOG_ERR, "Error: Failed to create daemon\n");
    exit(ret);
  } else {
    // Successfully spawned daemon
    syslog(LOG_NOTICE, "%s v%s running as daemon", DAEMON_NAME, REVISION);

    if(save_data) {
      std::cout << "Creating TOF output folders" << std::endl << std::flush;
      Init_TOF_Streaming();
    } else {
      IR_FOLDER = "";
      PTCLOUD_FOLDER = "";
      DEPTH_IMAGE_FOLDER = "";
      CALIBRATION_FOLDER = "";
    }

    bool live_stream_video = true;
    // Live Stream Server Address and Port
    // std::string servAddress("192.168.1.137");
    std::string servAddress("127.0.0.1");
    // std::string servAddress("10.0.0.9");
    std::string servPortValue("10000");
    int PACK_SIZE = 4096;
    unsigned short servPort;
    UDPSocket sock;
    std::vector<int> compression_params;
    compression_params.push_back(IMWRITE_JPEG_QUALITY);
    compression_params.push_back(80);
    if (live_stream_video){
      servPort = Socket::resolveService(servPortValue, "udp");
    }

    bool live_localization = true;
    // std::string servAddress_local("192.168.1.137");
    std::string servAddress_local("127.0.0.1");
    // std::string servAddress_local("10.0.0.9");
    std::string servPortValue_local("10010");
    unsigned short servPort_local;
    UDPSocket sock_local;
    if (live_localization) {
      servPort_local = Socket::resolveService(servPortValue_local, "udp");
    }

    bool live_plot = true;
    // std::string servAddress_plot("192.168.1.137");
    std::string servAddress_plot("127.0.0.1");
    // std::string servAddress_plot("10.0.0.9");
    std::string servPortValue_plot("10020");
    unsigned short servPort_plot;
    UDPSocket sock_plot;
    if (live_plot) {
      servPort_plot = Socket::resolveService(servPortValue_plot, "udp");
    }

    // Neural Network Setup
    /*
    class_names.push_back("unlabeled");
    class_names.push_back("sock");
    // Load Model
    Model model("/home/root/sdk/upload/frozen_inference_graph.pb");
    Tensor outNames1{model, "num_detections"};
    Tensor outNames2{model, "detection_scores"};
    Tensor outNames3{model, "detection_boxes"};
    Tensor outNames4{model, "detection_classes"};
    Tensor inpName{model, "image_tensor"};
    */

    try {
      net = dnn::readNetFromTensorflow("/home/root/sdk/upload/frozen_inference_graph.pb", "/home/root/sdk/upload/MobileNetV2.pbtxt");
      // net.setPreferableTarget(1);
    } catch (cv::Exception& e) {
      const char* err_msg = e.what();
      std::cout << "Error: " << err_msg << std::endl;
      syslog(LOG_ERR, "Error: %s\n", err_msg);
      Shutdown();
      return -1;
    }

    if (net.empty()) {
      std::cout << "Error loading the network model." << std::endl;
      syslog(LOG_ERR, "Error loading the network model.\n");
      Shutdown();
      return -1;
    } else {
      std::cout << "Loaded the network model." << std::endl;
      syslog(LOG_NOTICE, "Loaded the network model.\n");
    }

    // OpencCV uses the center of pixel as index origin
    // While tensorflow uses the top/left corner of the pixel
    // So in order to match Tensorflow we need to shift by 1/2 pixel
    // https://towardsdatascience.com/image-read-and-resize-with-opencv-tensorflow-and-pil-3e0f29b992be
    // https://github.com/opencv/opencv/issues/9096
    /*
    cv::Mat H1 = cv::Mat::eye(3,3, CV_32F);
    H1.at<float>(0,2) = -0.5f;
    H1.at<float>(1,2) = -0.5f;
    cv::Mat H2 = cv::Mat::eye(3,3, CV_32F);
    H2.at<float>(0,0) = 224.0f / 300.0f;
    H2.at<float>(1,1) = 172.0f / 300.0f;
    cv::Mat H3 = cv::Mat::eye(3,3, CV_32F);
    H3.at<float>(0,2) = 0.5f;
    H3.at<float>(1,2) = 0.5f;

    cv::Mat H123 = H1*H2*H3;
    cv::Mat M = H123 / H123.at<float>(2,2);
    cv::Mat Hfinal = M(cv::Range(0,2), cv::Range(0,3));
    */

    // Connect to the TOF sensor
    std::unique_ptr<MyListener> listener;
    platform::CameraFactory factory;
    std::unique_ptr<royale::ICameraDevice> cameraDevice = factory.createCamera();

    if(cameraDevice == nullptr) {
      // ERROR: Failed to connect to TOF sensor
      syslog(LOG_NOTICE, "TOFDaemon: Error: Failed to connect to TOF sensor\n");
      Shutdown();
      return -1;
    } else {
      syslog(LOG_NOTICE, "%s:%d: TOFDaemon successfully connected to TOF sensor\n", __FUNCTION__, __LINE__);
    }

    // Initialize the camera
    if(cameraDevice->initialize() != royale::CameraStatus::SUCCESS) {
      // ERROR: Failed to initialize the camera
      syslog(LOG_ERR, "Error: Could not initialize camera\n");
      Shutdown();
      return -1;
    } else {
      syslog(LOG_NOTICE, "%s:%d: TOFDaemon successfully initialzed camera\n", __FUNCTION__, __LINE__);
    }

    // retrieve available use cases
    royale::Vector<royale::String> use_cases;
    auto status = cameraDevice->getUseCases(use_cases);
    if (status != royale::CameraStatus::SUCCESS || use_cases.empty()) {
      syslog(LOG_ERR, "Error retrieving use cases for the slave\n");
      Shutdown();
      return -1;
    }

    // choose use case "MODE_9_5FPS"
    auto selectedUseCaseIdx = 0u;
    auto useCaseFound = false;
    royale::String use_mode("MODE_9_5FPS");
    for (auto i = 0u; i < use_cases.size(); ++i) {
      syslog(LOG_ERR, "USE CASE : %s\n", use_cases[i].c_str());
      if (use_cases[i] == use_mode) {
        // we found the use case
        selectedUseCaseIdx = i;
        useCaseFound = true;
        break;
      }
    }
    // check if we found a suitable use case
    if (!useCaseFound) {
      syslog(LOG_ERR, "Error : Did not find MODE_9_5FPS\n");
      Shutdown();
      return -1;
    }
    // set use case
    if (cameraDevice->setUseCase(use_cases.at(selectedUseCaseIdx)) != royale::CameraStatus::SUCCESS) {
      syslog(LOG_ERR, "Error setting use case %s for the slave\n", use_mode.c_str());
      Shutdown();
      return -1;
    }

    // Modify the camera parameters and settings
    royale::Vector<royale::StreamId> streamids;
    cameraDevice->getStreams(streamids);
    royale::StreamId streamId = streamids.front();

    // Set camera exposure time
    cameraDevice->setExposureMode(royale::ExposureMode::AUTOMATIC, streamId);
    //cameraDevice->setExposureTime(EXPOSURE_TIME_MS, streamId);  // For manual exposure

    // Modifying camera parameters
    royale::ProcessingParameterVector ppvec;
    if(cameraDevice->getProcessingParameters(ppvec, streamId) != royale::CameraStatus::SUCCESS) {
      // Error: Failed to grab camera parameters
      syslog(LOG_ERR, "Error: Failed to get processing parameters\n");
      Shutdown();
      return -1;
    }

    // Set the parameters
    for (auto& flagPair : ppvec) {
      royale::Variant var;
      switch(flagPair.first) {
        case royale::ProcessingFlag::UseRemoveFlyingPixel_Bool:
          var.setBool(USE_FLYING_PIXEL);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseRemoveStrayLight_Bool:
          var.setBool(USE_STRAY_LIGHT);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::AdaptiveNoiseFilterType_Int:
          var.setInt(ADAPTIVE_NOISE_FILTER_TYPE);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::NoiseThreshold_Float:
          var.setFloat(NOISE_THRESHOLD);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::GlobalBinning_Int:
          var.setInt(GLOBAL_BINNING);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseValidateImage_Bool:
          var.setBool(USE_VALIDATE_IMAGE);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool:
          var.setBool(USE_ADAPTIVE_NOISE_FILTER);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseMPIFlagAverage_Bool:
          var.setBool(USE_MPI_AVERAGE);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseMPIFlag_Amp_Bool:
          var.setBool(USE_MPI_AMP);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseMPIFlag_Dist_Bool:
          var.setBool(USE_MPI_DIST);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseFilter2Freq_Bool:
          var.setBool(USE_FILTER_2_FREQ);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseSmoothingFilter_Bool:
          var.setBool(USE_SMOOTHING_FILTER);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseFlagSBI_Bool:
          var.setBool(USE_SBI_FLAG);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::UseHoleFilling_Bool:
          var.setBool(USE_HOLE_FILLING);
          flagPair.second = var;
          break;
        case royale::ProcessingFlag::AutoExposureRefValue_Float:
          var.setFloat(AUTO_EXPOSURE_REF_VALUE);
          flagPair.second = var;
          break;
        default:
          break;
      }
    }

    if(cameraDevice->setProcessingParameters(ppvec, streamId) != royale::CameraStatus::SUCCESS) {
      syslog(LOG_ERR, "TOFDaemon Error: Failed to set processing parameters\n");
      Shutdown();
      return -1;
    } else {
      syslog(LOG_NOTICE, "TOFDaemon successfully set processing parameters\n");
    }

    uint16_t sensor_num_columns, sensor_num_rows;
    status = cameraDevice->getMaxSensorWidth(sensor_num_columns);
    status = cameraDevice->getMaxSensorHeight(sensor_num_rows);

    size_t buffer_size = static_cast<size_t>(sensor_num_columns) * static_cast<size_t>(sensor_num_rows);

    // packed as r,g,b r,g,b  ...
    std::vector<float> vec_point_cloud_X(buffer_size);
    std::vector<float> vec_point_cloud_Y(buffer_size);
    std::vector<float> vec_point_cloud_Z(buffer_size);
    std::vector<float> vec_point_cloud_distance(buffer_size);
    // std::vector<uint8_t> gray_image(buffer_size);
    cv::Mat gray_image(sensor_num_rows, sensor_num_columns, CV_8U);

    // std::vector<uint8_t> vec_nnet_input(buffer_size);
    cv::Mat img_stream(sensor_num_rows, sensor_num_columns, CV_8UC3);
    cv::Mat blob;
    std::vector<unsigned char> encoded;
    // compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    // compression_params.push_back(9);

    std::vector<uint8_t > img_data;
    otsu_threshold otsu;
    localize object_localize;

    // uint8_t r_val, g_val, b_val;
    // float x_val, y_val, z_val;
    // int row, column;
    int total_pack;
    double accum;
    //int loop_index = 0;
    size_t loop_index, location_index;
    cv::Mat nnet_input(300, 300, CV_8UC3);
    const float confidence_threshold = 0.1f;
    float detect_confidence;
    // size_t det_index;
    int x1, y1, x2, y2;
    // float xx1, yy1, xx2, yy2;
    // int total_num_objects;
    int gray_threshold;
    size_t num_points;
    std::vector<float> obj_x_vals(buffer_size, 0.0f);
    std::vector<float> obj_z_vals(buffer_size, 0.0f);
    std::vector<int64_t> depth_data_timestamp(1);

    std::string s_object_timestamp = "TIMESTAMP : ";
    std::string s_object_id = "OBJECT_ID : SOCK-";
    std::string s_object_loc = "LOCATION : ";
    std::string delim = "|";
    std::string comma = ",";
    size_t stream_out_length;
    size_t stream_out_length_plot;

    // uint8_t depth_int = 0;
    // uint8_t invalid_depth = static_cast<uint8_t>(255);
    // uint8_t zero_depth = static_cast<uint8_t>(0);

    // Register a data listener
    // listener.reset(new MyListener(std::ref(ptcloud_mutex), ptcloud_cv, suppress, &vec_point_cloud, &vec_nnet_input, &gray_image, &new_data_available));
    listener.reset(new MyListener(std::ref(ptcloud_mutex), ptcloud_cv, suppress, &depth_data_timestamp, &vec_point_cloud_X, &vec_point_cloud_Y, &vec_point_cloud_Z, &vec_point_cloud_distance, img_stream, gray_image, &new_data_available));
    if(cameraDevice->registerDataListener(listener.get()) != royale::CameraStatus::SUCCESS) {
      // ERROR: Failed to register data listener
      syslog(LOG_ERR, "Error: Failed to register data listener\n");
      Shutdown();
      return -1;
    }
    syslog(LOG_NOTICE, "TOFDaemon: Successfully registered data listener\n");

    // Begin video capture
    if(cameraDevice->startCapture() != royale::CameraStatus::SUCCESS) {
      // ERROR: Failed to start capture
      syslog(LOG_ERR, "Error: Failed to start video capture\n");
      Shutdown();
      return -1;
    }
    syslog(LOG_NOTICE, "TOFDaemon: Successfully started video capture\n");

    // Mark that the daemon should continue processing frames
    g_daemon_live = true;

    syslog(LOG_NOTICE, "TOFDaemon: Starting video capture\n");

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    // DEBUG ONLY: Display the time in milliseconds needed to setup the daemon
    double setupTimeMS = static_cast<double>(endTime.tv_sec - startTime.tv_sec) * 1000. + static_cast<double>(endTime.tv_usec - startTime.tv_usec) / 1000.;
    syslog(LOG_NOTICE, "TOFDaemon: setup time took %g milliseconds\n", setupTimeMS);

    // We'll need the signal handler(s) to stop the video capture;
    while(g_daemon_live) {
      struct timespec start, stop;
      accum = 0.;
      if( clock_gettime( CLOCK_MONOTONIC, &start) == -1 ) {
        syslog(LOG_ERR, "Error: Failed to get clock start time\n");
      }
      std::unique_lock<std::mutex> lock (ptcloud_mutex);
      auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds (500));
      if (ptcloud_cv.wait_until (lock, timeOut, [&] {return new_data_available; })) {

        // std::vector<int> classIds;
        struct timespec start_infer, stop_infer;
        double accum_infer = 0.;
        if( clock_gettime( CLOCK_MONOTONIC, &start_infer) == -1 ) {
          syslog(LOG_ERR, "Error: Failed to get clock start time\n");
        }
        /* if (new_data_available) {
          syslog(LOG_NOTICE, "New Data Available in WAIT:::  TRUE");
        } else {
          syslog(LOG_NOTICE, "New Data Available in WAIT :::  FALSE");
        } */

        // std::string timestamp = std::to_string(static_cast<double>(start.tv_sec)*1000.0 + static_cast<double>(start.tv_nsec)/1000000.0);
        // std::string filename = "nnet_input_" + timestamp + ".raw";
        // filename = DEPTH_IMAGE_FOLDER + filename;

        // std::ofstream outputFile;
        // std::stringstream stringStream;
        // r_val = 0;
        // g_val = 0;
        // b_val = 0;
        // row = 0;
        // column = 0;
        total_pack = 0;

        // outputFile.open (filename, std::ofstream::out);
        // if (outputFile.fail()) {
        //   syslog(LOG_NOTICE, "Outputfile %s could not be opened!\n", filename.c_str());
        // } else {
          /*
          for (loop_index = 0; loop_index < vec_nnet_input.size(); ++loop_index) {
            row = (int)(loop_index / sensor_num_columns);
            column = loop_index % sensor_num_columns;

            // x_val = vec_point_cloud[xyz_index++];
            // y_val = vec_point_cloud[xyz_index++];
            // z_val = vec_point_cloud[xyz_index++];

            depth_int = vec_nnet_input[loop_index];
            r_val = lookup_table_rgb[depth_int][0];
            g_val = lookup_table_rgb[depth_int][1];
            b_val = lookup_table_rgb[depth_int][2];

            cv::Vec3b& bgr = img_stream.at<cv::Vec3b>(row, column);
            bgr[0] = b_val;
            bgr[1] = g_val;
            bgr[2] = r_val;
          }
          */
          //  No copy made here just initializes cv::Mat header
          cv::Mat ptcloud_depth(sensor_num_rows, sensor_num_columns, CV_32FC1, vec_point_cloud_distance.data());
          cv::Mat ptcloud_x(sensor_num_rows, sensor_num_columns, CV_32FC1, vec_point_cloud_X.data());
          cv::Mat ptcloud_z(sensor_num_rows, sensor_num_columns, CV_32FC1, vec_point_cloud_Z.data());

          // cv::warpAffine(img_stream, nnet_input, Hfinal, cv::Size(300, 300), cv::INTER_LINEAR + cv::WARP_INVERSE_MAP);
          cv::resize(img_stream, nnet_input, cv::Size(300, 300));

          /*
          img_data.assign(nnet_input.data, nnet_input.data + nnet_input.total() * nnet_input.channels());
          inpName.set_data(img_data, {1, 300, 300, 3});
          model.run(inpName, {&outNames1, &outNames2, &outNames3, &outNames4});

          // Visualize detected bounding boxes.
          int num_detections = (int)outNames1.get_data<float>()[0];
          for (int i=0; i<num_detections; i++) {
              int classId = (int)outNames4.get_data<float>()[i];
              float score = outNames2.get_data<float>()[i];
              auto bbox_data = outNames3.get_data<float>();
              std::vector<float> bbox = {bbox_data[i*4], bbox_data[i*4+1], bbox_data[i*4+2], bbox_data[i*4+3]};
              if (score > 0.3) {
                  float x = bbox[1] * sensor_num_columns;
                  float y = bbox[0] * sensor_num_rows;
                  float right = bbox[3] * sensor_num_columns;
                  float bottom = bbox[2] * sensor_num_rows;

                  // cv::rectangle(gray_image, {(int)x, (int)y}, {(int)right, (int)bottom}, {125, 255, 51}, 2);
                  cv::rectangle(gray_image, {(int)x, (int)y}, {(int)right, (int)bottom}, cv::Scalar(255), 2);
              }
          }
          */

          //cv::dnn::blobFromImage(nnet_input, blob, 1.0f, cv::Size(300, 300), cv::Scalar(0, 0, 0), false, false);
          cv::dnn::blobFromImage(nnet_input, blob, 1.0f, cv::Size(300, 300), 0.0, false, false);

          net.setInput(blob);

          std::vector<cv::Mat> outs;
          //cv::Mat detection = net.forward("detection_out");
          net.forward(outs, "detection_out");

          std::string stream_out;
          std::string stream_out_plot;
          int object_id = 0;
          float* data = (float*)outs[0].data;
          for (loop_index = 0; loop_index < outs[0].total(); loop_index += 7) {
            detect_confidence = data[loop_index + 2];
            if (detect_confidence > confidence_threshold) {
              x1 = static_cast<int>(data[loop_index + 3] * sensor_num_columns);
              y1 = static_cast<int>(data[loop_index + 4] * sensor_num_rows);
              x2 = static_cast<int>(data[loop_index + 5] * sensor_num_columns);
              y2 = static_cast<int>(data[loop_index + 6] * sensor_num_rows);
              cv::Rect roi_rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
              cv::rectangle(img_stream, roi_rect, cv::Scalar(255, 255, 255));
              // classIds.push_back(static_cast<int>(data[loop_index + 1]) - 1);  //  Need to skip  0th background class id.

              cv::Mat img_roi(gray_image, roi_rect);
              cv::Mat ptcloud_depth_roi(ptcloud_depth, roi_rect);
              cv::Mat ptcloud_x_roi(ptcloud_x, roi_rect);
              cv::Mat ptcloud_z_roi(ptcloud_z, roi_rect);

              gray_threshold = otsu.get_threshold(img_roi);
              num_points = object_localize.localize_roi(img_roi, ptcloud_depth_roi, ptcloud_x_roi, ptcloud_z_roi, gray_threshold, obj_x_vals, obj_z_vals);

              if (object_id == 0) {
                stream_out = s_object_timestamp + std::to_string(depth_data_timestamp[0]);
              }
              stream_out += delim + s_object_id + std::to_string(object_id) + delim;
              stream_out += s_object_loc;
              stream_out_plot += "NEWOBJ ";
              for (location_index = 0; location_index < num_points; ++location_index) {
                stream_out += " [" + std::to_string(obj_x_vals[location_index]) + "," + std::to_string(obj_z_vals[location_index]) + "]";
                stream_out_plot += std::to_string(obj_x_vals[location_index]) + " " + std::to_string(obj_z_vals[location_index]) + " ";
              }
              object_id++;
            }
          }

          if (!stream_out.empty()) {
            syslog(LOG_NOTICE, "%s\n", stream_out.c_str());
            if (live_localization) {
              stream_out_length = stream_out.size();
              sock_local.sendTo(&stream_out_length, sizeof(size_t), servAddress_local, servPort_local);
              sock_local.sendTo(stream_out.c_str(), stream_out.size(), servAddress_local, servPort_local);
            }
          }
          if (!stream_out_plot.empty()) {
            if (live_plot) {
              stream_out_length_plot = stream_out_plot.size();
              sock_plot.sendTo(&stream_out_length_plot, sizeof(size_t), servAddress_plot, servPort_plot);
              sock_plot.sendTo(stream_out_plot.c_str(), stream_out_plot.size(), servAddress_plot, servPort_plot);
            }
          }

          if( clock_gettime( CLOCK_MONOTONIC, &stop_infer) == -1 ) {
            syslog(LOG_ERR, "Error: Failed to get clock stop time\n");
          }
          accum_infer = static_cast<double>( stop_infer.tv_sec - start_infer.tv_sec ) * 1000.0 + static_cast<double>( stop_infer.tv_nsec - start_infer.tv_nsec ) / 1000000.0;
          syslog(LOG_NOTICE, "TIME DIFF CV INFERENCE *:::* %lf\n", accum_infer);

          /*
          cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
          // std::vector<int> classIds;  //TBD
          total_num_objects = detectionMat.rows;
          for (loop_index = 0; loop_index < total_num_objects; loop_index++) {
            detect_confidence = detectionMat.at<float>(loop_index, 2);
            if (detect_confidence > confidence_threshold) {
              // det_index = static_cast<size_t>(detectionMat.at<float>(loop_index, 1));
              x1 = static_cast<int>(detectionMat.at<float>(loop_index, 3) * sensor_num_columns); //  300.0f; // img_stream.cols;
              y1 = static_cast<int>(detectionMat.at<float>(loop_index, 4) * sensor_num_rows); // 300.0f; // img_stream.rows;
              x2 = static_cast<int>(detectionMat.at<float>(loop_index, 5) * sensor_num_columns); // 300.0f; //img_stream.cols;
              y2 = static_cast<int>(detectionMat.at<float>(loop_index, 6) * sensor_num_rows); // 300.0f; // img_stream.rows;
              cv::Rect roi_rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
              cv::rectangle(gray_image, cv::Point(x1, y1), cv::Point(x2 + 1, y2 + 1), cv::Scalar(255), 1, 8, 0);
              // cv::rectangle(gray_image, rec, cv::Scalar(255), 1, 8, 0);
              // cv::rectangle(img_stream, rec, cv::Scalar(255, 255, 255), 1, 8, 0);
              // cv::putText(img_stream, format("%s", class_names[det_index].c_str()), cv::Point(x1, y1-5) , FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255), 1, 8, 0);
            }
          }
          */

         if (live_stream_video) {

            cv::imencode(".jpg", img_stream, encoded, compression_params);
            // cv::imencode(".png", gray_image, encoded, compression_params);
            total_pack = 1 + (static_cast<int>(encoded.size()) - 1) / PACK_SIZE;

            int ibuf[1];
            ibuf[0] = total_pack;
            sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

            for (int buf_idx = 0; buf_idx < total_pack; buf_idx++) {
              sock.sendTo( & encoded[buf_idx * PACK_SIZE], PACK_SIZE, servAddress, servPort);
            }
         }


           // stringStream << (int)r_val << " " << (int)g_val << " " << (int)b_val << std::endl;
          // }
          // outputFile << stringStream.str();
          // outputFile.close();

        /* Tensorflow - Lite typed_input_tensor   (1, 300, 300, 3)
        for (loop_index = 0; size_t < cvimg.size(); ++loop_index) {
          const auto& rgb = cvimg[loop_index];
          interpreter->typed_input_tensor<uchar>(0)[3*loop_index + 0] = rgb[0];
          interpreter->typed_input_tensor<uchar>(0)[3*loop_index + 1] = rgb[1];
          interpreter->typed_input_tensor<uchar>(0)[3*loop_index + 2] = rgb[2];
        }
        */

        /*
        std::string filename = "densePointCloud_" + timestamp + ".ply";
        filename = PTCLOUD_FOLDER + filename;

        std::string filename_gray = "grayscale_" + timestamp + ".raw16";
        filename_gray = IR_FOLDER + filename_gray;

        SavePointcloud(filename, filename_gray, &point_cloud_x, &point_cloud_y, &point_cloud_z, &gray_image);
        */

        new_data_available = false;
      }// else {
      //  // we ran into a timeout so sleep for  100 milliseconds
      //  std::this_thread::sleep_for (std::chrono::milliseconds (100));
      //}
      if( clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ) {
        syslog(LOG_ERR, "Error: Failed to get clock stop time\n");
      }
      if (new_data_available) {
        syslog(LOG_NOTICE, "New Data Available 2 :::  TRUE");
      } else {
        syslog(LOG_NOTICE, "New Data Available 2 :::  FALSE");
      }
      accum = static_cast<double>( stop.tv_sec - start.tv_sec ) * 1000.0 + static_cast<double>( stop.tv_nsec - start.tv_nsec ) / 1000000.0;
      syslog(LOG_NOTICE, "TIME DIFF DAEMON *:::* %lf\n", accum);

    }

    // We're done processing frames.  Shutdown the camera.
    if(cameraDevice->stopCapture() != royale::CameraStatus::SUCCESS) {
      syslog(LOG_ERR, "Error: Failed to stop camera capture\n");
      Shutdown();
      return -1;
    }
  }

  Shutdown();
  return 0;

}
