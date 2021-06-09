
#include "tof_daemon.h"
#include "frame_queue.h"
#include "pipeline_listener.h"

/*****************************************
 * Utility Functions
 *****************************************/
bool CreateDirectory(std::string path) {
  bool path_created = false;
  // Try and create the directory without any special read and write premissions
  int mkdir_status = mkdir(path.c_str(), 0775);
  if (mkdir_status == -1) {
    switch (errno) {
    case ENOENT:
      // In this case, the path prefix specified does not exist so we will try
      // to create it
      if (CreateDirectory(path.substr(0, path.find_last_of('/'))))
        // Now we try to create the full path again
        path_created = 0 == mkdir(path.c_str(), 0775);
      else
        path_created = false;
      break;
    case EEXIST:
      // In this case the path already exists so we can return true
      path_created = true;
      break;
    default:
      path_created = false;
      break;
    }
  } else {
    path_created = true;
  }
  return path_created;
}

static int RemoveFiles(const char *pathname, const struct stat *sbuf, int type,
                       struct FTW *ftwb) {
  // Since we are just removing all the files from the pathname, we can ignore
  // the other parameters
  (void)sbuf;
  (void)type;
  (void)ftwb;
  // Try to remove the given file path
  if (remove(pathname) < 0) {
    syslog(LOG_ERR, "Error: Failed to remove files\n");
    return -1;
  }
  return 0;
}

static void *TOFTransactionHandler(void *req_buf, size_t req_len,
                                   void **resp_buf, size_t *resp_len) {
  static TOFMessage tof_message;
  *resp_len = sizeof(tof_message);
  *resp_buf = &tof_message;
  TOFMessage *in_msg = (TOFMessage *)req_buf;
  TOFMessage *out_msg = (TOFMessage *)*resp_buf;

  if (req_len != sizeof(TOFMessage)) {
    return NULL;
  }

  memset(out_msg, 0, sizeof(tof_message));

  out_msg->transact.command = in_msg->transact.command;
  out_msg->transact.status = TOFMessage::CMD_NAK;

  switch (in_msg->transact.command) {
  case TOFMessage::STREAM_STOP: {
    syslog(LOG_NOTICE,
           "Received the STREAM_STOP command. Stopping the camera.\n");
    gTOFDaemonStreaming = false;
    out_msg->transact.status = TOFMessage::CMD_ACK;
    break;
  }
  case TOFMessage::STREAM_START: {
    syslog(LOG_NOTICE,
           "Received the STREAM_START command. Starting the camera.\n");
    gTOFDaemonStreaming = true;
    out_msg->transact.status = TOFMessage::CMD_ACK;
    break;
  }
  default:
    break;
  }
  return NULL;
}

/*****************************************
 * TOFDaemon Class Functions
 *****************************************/
void TOFDaemon::Shutdown() {
  // Unlock the lock file
  if (lock_fd_ >= 0) {
    if (lockf(lock_fd_, F_ULOCK, 0) < 0) {
      syslog(LOG_ERR, "Error %d: Failed to unlock lock file: %s\n", errno,
             strerror(errno));
    }

    // Close the file descriptor and remove the lock file
    close(lock_fd_);
    lock_fd_ = -1;
    unlink(LOCK_FILE);
  }
}

void TOFDaemon::Init() {
  if (save_data_) {
    InitializeTOFDataStorage();
  }

  // Initialize the NeatoIPC TOF server
  tof_server_.reset(new TOFServerInterface(TOFTransactionHandler));

  InitializeStreamingSockets();
  InitializeNeuralNet();

  if (ConnectTOFCamera() < 0) {
    syslog(LOG_ERR, "Error: Failed to start camera capture\n");
    init_successful_ = false;
  }

  ReadTOFToLDSTransformationConfig();
}

void TOFDaemon::InitializeNeuralNet() {
  // Initialize the classes that can be detected
  InitializeClasses();

  // Neural Network Setup - For Tensorflow
  /*
  class_names_.push_back("unlabeled");
  class_names_.push_back("sock");
  // Load Model
  Model model("/home/root/sdk/upload/frozen_inference_graph.pb");
  Tensor outNames1{model, "num_detections"};
  Tensor outNames2{model, "detection_scores"};
  Tensor outNames3{model, "detection_boxes"};
  Tensor outNames4{model, "detection_classes_"};
  Tensor inpName{model, "image_tensor"};
  */

  // Try to load the neural net for opencv
  try {
    net_ = dnn::readNetFromTensorflow(
        "/home/root/sdk/install/frozen_inference_graph.pb",
        "/home/root/sdk/install/MobileNetV2.pbtxt");
  } catch (cv::Exception &e) {
    const char *err_msg = e.what();
    syslog(LOG_ERR, "Error: %s\n", err_msg);
    // Mark the initialization as a fail
    init_successful_ = false;
  }

  // Check that the neural net is not empty
  if (net_.empty()) {
    syslog(LOG_ERR, "Error loading the network model.\n");
    // Mark the initialization as a fail
    init_successful_ = false;
  } else {
    syslog(LOG_NOTICE, "Loaded the network model.\n");
  }

  // TODO(CodeCleanup): Can this be removed?
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
}

void TOFDaemon::InitializeClasses() {
  // TODO(CodeCleanup): Maybe this should read from a config file to describe
  // all the classes possible.
  classes_.push_back(std::string("Fabric"));
  classes_.push_back(std::string("Cord"));
}

void TOFDaemon::InitializeStreamingSockets() {
  live_video_socket_params_ = {debug_ip_address_, "10000", 0};
  if (live_stream_video_) {
    live_video_socket_params_.socket_port =
        Socket::resolveService(live_video_socket_params_.port_string, "udp");
    live_video_socket_.reset(new UDPSocket());
  }
}

int TOFDaemon::Daemonize(const char *daemon_name, const char *daemon_path,
                         const char *outfile, const char *errfile,
                         const char *infile) {
  const char *DEFAULT_FILE = "/dev/null";

  if (!infile)
    infile = DEFAULT_FILE;
  if (!outfile)
    outfile = DEFAULT_FILE;
  if (!errfile)
    errfile = DEFAULT_FILE;

  // Open syslog
  openlog(daemon_name, LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO, "Entering ToF Daemon");

  // Fork the parent to create a child process
  pid_t child = fork();
  if (child < 0) {
    // Error: Failed to fork().
    fprintf(stderr, "Error %d: failed to fork: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to fork: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }
  if (child > 0) {
    // Parent process.  Parent exits.
    exit(EXIT_SUCCESS);
  }

  // Create a new Signature Id for the child
  if (setsid() < 0) {
    // Error: Failed to become session leader
    fprintf(stderr, "Error %d: failed to setsid: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to setsid: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  // Child initialize signal handlers
  signal(SIGCHLD, SIG_IGN); // Ignore SigChild
  signal(SIGINT, sigint_handler);
  signal(SIGHUP, sighup_handler);

  // Spawn a "grandchild" process; this guarantees that we removed the session
  // leading process
  child = fork();
  if (child < 0) {
    // Error: Creating grandchild process failed
    fprintf(stderr, "Error %d: failed to fork: %s", errno, strerror(errno));
    syslog(LOG_ERR, "Error %d: failed to fork: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }
  if (child > 0) {
    // Parent (child process) also exits, leaving only the grandchild process
    // active
    exit(EXIT_SUCCESS);
  }

  // New permissions
  umask(0);

  // Change to path directory
  if (chdir(daemon_path) < 0) {
    // Error: Failed to change grandchild's directory
    fprintf(stderr, "Error %d: failed to chdir: %s", errno, strerror(errno));
    return EXIT_FAILURE;
  }

  // Close all open file descriptors
  int fd;
  for (fd = static_cast<int>(sysconf(_SC_OPEN_MAX)); fd > 0; --fd) {
    close(fd);
  }

  // Reopen stdin/stdout/stderr, pointing to the specified files
  stdin = fopen(infile, "r");    // File descriptor 0
  stdout = fopen(outfile, "w+"); // File descriptor 1
  stderr = fopen(errfile, "w+"); // File descriptor 2

  // Create a "lock file" whose appearance indicates that the TOFDaemon is
  // already created

  // But first check if the file already exists
  lock_fd_ = open(LOCK_FILE, O_RDONLY);
  if (lock_fd_ >= 0) {
    // Error: The lock file already exists so another TOFDaemon must already been running
    syslog(LOG_ERR, "Error %d: Lock file already exists %s: %s", errno,
           LOCK_FILE, strerror(errno));
    return EXIT_FAILURE;
  }

  lock_fd_ = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
  if (lock_fd_ < 0) {
    // Error: failed to create lock file
    syslog(LOG_ERR, "Error %d: failed to open lock file %s: %s", errno,
           LOCK_FILE, strerror(errno));
    return EXIT_FAILURE;
  }

  // Check if the lock file has been locked
  if (lockf(lock_fd_, F_TLOCK, 0) < 0) {
    // Error: Lock already applied
    syslog(LOG_ERR, "Error: tried to run TOFDaemon twice");
    exit(0);
  }

  // Daemon created; write the PID to the lock file
  char str[16];
  snprintf(str, sizeof(str), "%d\n", getpid());
  if (write(lock_fd_, str, strlen(str)) < 0) {
    // Error: Failed to write PID to lock file
    syslog(LOG_ERR, "Error %d: Failed to write PID to lock file %s: %s", errno,
           LOCK_FILE, strerror(errno));
    return EXIT_FAILURE;
  }

  return 0;
}

void TOFDaemon::InitializeTOFDataStorage() {
  // Create unique folder identifier from timestamp
  char folder_id[100];
  time_t cur_time = time(NULL);
  const std::string FORMATSTRING("%Y-%m-%d_%H-%M-%S");
  strftime(folder_id, sizeof(folder_id), FORMATSTRING.c_str(),
           gmtime(&cur_time));

  IR_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + IR_FOLDER;
  PTCLOUD_FOLDER = LOCAL_DATA_PATH + std::string(folder_id) + PTCLOUD_FOLDER;
  DEPTH_IMAGE_FOLDER =
      LOCAL_DATA_PATH + std::string(folder_id) + DEPTH_IMAGE_FOLDER;
  CALIBRATION_FOLDER =
      LOCAL_DATA_PATH + std::string(folder_id) + CALIBRATION_FOLDER;

  // Clean out the LOCAL_DATA_PATH folder by removing it
  if (nftw(LOCAL_DATA_PATH.c_str(), RemoveFiles, 10,
           FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0) {
    syslog(LOG_ERR, "Failed to clean up TOF %s output directory",
           LOCAL_DATA_PATH.c_str());
  } else {
    syslog(LOG_ERR, "Cleaned up TOF %s output directory",
           LOCAL_DATA_PATH.c_str());
  }
  // Create the output directories
  if (!CreateDirectory(IR_FOLDER)) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory",
           IR_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory",
           IR_FOLDER.c_str());
  }
  if (!CreateDirectory(PTCLOUD_FOLDER)) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory",
           PTCLOUD_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory",
           PTCLOUD_FOLDER.c_str());
  }
  if (!CreateDirectory(DEPTH_IMAGE_FOLDER)) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory",
           DEPTH_IMAGE_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory",
           DEPTH_IMAGE_FOLDER.c_str());
  }
  if (!CreateDirectory(CALIBRATION_FOLDER)) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory",
           CALIBRATION_FOLDER.c_str());
  } else {
    syslog(LOG_NOTICE, "Created new TOF %s output directory",
           CALIBRATION_FOLDER.c_str());
  }
}

void TOFDaemon::SavePointCloud(const std::string &filename,
                               const std::string &filename_gray,
                               const std::vector<float> *vec_x,
                               const std::vector<float> *vec_y,
                               const std::vector<float> *vec_z,
                               const std::vector<uint16_t> *vec_gray) {
  std::ofstream outputFile;
  std::stringstream stringStream;

  std::ofstream outputFile_gray;
  std::stringstream stringStream_gray;
  uint16_t mask = 255;

  outputFile_gray.open(filename_gray, std::ofstream::out);
  if (outputFile_gray.fail()) {
    std::cerr << "Outputfile " << filename_gray << " could not be opened!"
              << std::endl;
    return;
  }

  outputFile.open(filename, std::ofstream::out);
  if (outputFile.fail()) {
    std::cerr << "Outputfile " << filename << " could not be opened!"
              << std::endl;
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
      stringStream << (*vec_x)[i] << " " << (*vec_y)[i] << " " << (*vec_z)[i]
                   << std::endl;
      stringStream_gray << ((*vec_gray)[i] & mask) << " "
                        << (((*vec_gray)[i] >> 8) & mask) << std::endl;
      // reconstruct like this: gray_val = byte_low | (byte_high << 8);
    }
    // output stringstream to file and close it
    outputFile << stringStream.str();
    outputFile.close();

    outputFile_gray << stringStream_gray.str();
    outputFile_gray.close();
  }
}

void TOFDaemon::ReadTOFToLDSTransformationConfig() {
  // Set the transform_loaded flag to false by default
  bool transform_loaded = false;

  // Indices for the matrix when loading from the file
  int row = 0, col = 0;

  std::ifstream transform_file;
  transform_file.open(kTOFToLDSTransformFile);
  if (transform_file.is_open()) {
    // Keep track of the number of elements being read from the transform
    // configuration file
    int num_transform_elems = 0;
    std::string transform_elem;
    while (getline(transform_file, transform_elem)) {
      // If we are reading more lines than required for the transformation
      // matrix then the configuration file is not properly formatted and is
      // therefore invalid
      if (num_transform_elems >= kNumTransformElements) {
        break;
      }

      // Add the element to the transform matrix
      row = static_cast<int>(num_transform_elems / kTransformMatrixDimension);
      col = num_transform_elems % kTransformMatrixDimension;
      tof_to_lds_transform_(row, col) = std::stof(transform_elem);

      // Increment the element counter to ensure we have the exact amount of
      // elements for the transformation matrix
      num_transform_elems++;
    }

    // We only set the transform loaded flag to true if the correct amount of
    // elements is read from the configuration file
    if (num_transform_elems == kNumTransformElements) {
      transform_loaded = true;
    }
  }

  if (transform_loaded) {
    syslog(LOG_NOTICE,
           "TOF to LDS transformation configuration loaded properly!\n");
  } else {
    syslog(LOG_ERR, "TOF to LDS transformation was not loaded properly! "
                    "TOFDaemon will not run. \n");
    init_successful_ = false;
  }
}

void TOFDaemon::Postprocess(const std::vector<Mat> &nn_outputs,
                            std::vector<int> &class_ids,
                            std::vector<float> &confidences,
                            std::vector<cv::Rect> &boxes,
                            std::vector<int> &indices,
                            cv::Size2f &sensor_size_float) {
  // The number of parameters for every object found
  size_t kParamsPerNNOutput = 7;

  // The offsets to the current detected object to extract the desired
  // information
  int kConfidenceIdxOffset = 2;
  int kTopLeftXIdxOffset = 3;
  int kTopLeftYIdxOffset = 4;
  int kBottomRightXIdxOffset = 5;
  int kBottomRightYIdxOffset = 6;

  float detect_confidence;
  int top_left_x, top_left_y, bottom_right_x, bottom_right_y;
  int bbox_width, bbox_height;
  cv::Size2i sensor_size_int(static_cast<int>(sensor_size_float.width),
                             static_cast<int>(sensor_size_float.height));

  for (size_t k = 0; k < nn_outputs.size(); k++) {
    float *data = (float *)nn_outputs[k].data;
    for (size_t loop_index = 0; loop_index < nn_outputs[k].total();
         loop_index += kParamsPerNNOutput) {
      detect_confidence = data[loop_index + kConfidenceIdxOffset];

      // Only process the object if its confidence exceeds the threshold
      if (detect_confidence > CONFIDENCE_THRESHOLD) {
        top_left_x = (int)(data[loop_index + kTopLeftXIdxOffset] *
                           sensor_size_float.width);
        top_left_y = (int)(data[loop_index + kTopLeftYIdxOffset] *
                           sensor_size_float.height);
        bottom_right_x = (int)(data[loop_index + kBottomRightXIdxOffset] *
                               sensor_size_float.width);
        bottom_right_y = (int)(data[loop_index + kBottomRightYIdxOffset] *
                               sensor_size_float.height);

        // Ensure that the top left coordinates are 0 or positive
        if (top_left_x < 0)
          top_left_x = 0;
        if (top_left_y < 0)
          top_left_y = 0;

        // Calculate the bounding box width and height
        bbox_width = bottom_right_x - top_left_x + 1;
        bbox_height = bottom_right_y - top_left_y + 1;

        // Skip this detected object if the bounding boxes are invalid
        if (bbox_width < 1)
          continue;
        if (bbox_height < 1)
          continue;

        // Ensure that the bounding box width and height does not exceed the
        // dimensions of the sensor image
        if ((top_left_x + bbox_width) > sensor_size_int.width)
          bbox_width = sensor_size_int.width - top_left_x;
        if ((top_left_y + bbox_height) > sensor_size_int.height)
          bbox_height = sensor_size_int.height - top_left_y;

        // Save the confidence, the class, and the bounding box of the detected
        // object
        // TODO(CodeCleanup): Why do we substract one here??????
        class_ids.push_back((int)(data[loop_index + 1]) - 1);
        boxes.push_back(Rect(top_left_x, top_left_y, bbox_width, bbox_height));
        confidences.push_back(detect_confidence);
      }
    }
  }

  // Perform non-maximum supression on the detected bounding boxes to filter the
  // output of the NN
  cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD,
                    indices);
}

void TOFDaemon::DrawPredictions(int class_id, float confidence, int left,
                                int top, int right, int bottom, Mat &frame) {
  // Create the bounding box for the object
  rectangle(frame, Point(left, top), Point(right, bottom),
            Scalar(255, 255, 255));

  // Ensure that the class is valid and format the class with the confidence
  // score
  std::string label = format("%.2f", confidence);
  if (!classes_.empty()) {
    CV_Assert(class_id < (int)classes_.size());
    label = classes_[class_id] + ": " + label;
  }

  // Create the box for the class and confidence display
  int base_line;
  Size label_size =
      getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &base_line);
  top = max(top, label_size.height);
  rectangle(frame, Point(left, top - label_size.height),
            Point(left + label_size.width, top + base_line), Scalar::all(255),
            FILLED);
  putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar());
}

int TOFDaemon::ConnectTOFCamera() {
  // Connect to the TOF sensor
  camera_device_ = camera_factory_.createCamera();

  if (camera_device_ == nullptr) {
    // Error: Failed to connect to TOF sensor
    syslog(LOG_NOTICE, "Error: Failed to connect to TOF sensor\n");
    return -1;
  } else {
    if (verbose_)
      syslog(LOG_NOTICE,
             "%s:%d: TOFDaemon successfully connected to TOF sensor\n",
             __FUNCTION__, __LINE__);
  }

  // Initialize the camera
  if (camera_device_->initialize() != royale::CameraStatus::SUCCESS) {
    // Error: Failed to initialize the camera
    syslog(LOG_ERR, "Error: Could not initialize camera\n");
    return -1;
  } else {
    if (verbose_)
      syslog(LOG_NOTICE, "%s:%d: TOFDaemon successfully initialized camera\n",
             __FUNCTION__, __LINE__);
  }

  // retrieve available use cases
  royale::Vector<royale::String> use_cases;
  auto status = camera_device_->getUseCases(use_cases);
  if (status != royale::CameraStatus::SUCCESS || use_cases.empty()) {
    syslog(LOG_ERR, "Error retrieving use cases for the slave\n");
    return -1;
  }

  // choose use case "MODE_9_5FPS"
  size_t selected_use_case_idx = 0;
  bool use_case_found = false;
  royale::String use_mode("MODE_9_5FPS");
  for (size_t i = 0; i < use_cases.size(); ++i) {
    if (verbose_)
      syslog(LOG_NOTICE, "USE CASE : %s\n", use_cases[i].c_str());
    if (use_cases[i] == use_mode) {
      // we found the use case
      selected_use_case_idx = i;
      use_case_found = true;
      break;
    }
  }
  // check if we found a suitable use case
  if (!use_case_found) {
    syslog(LOG_ERR, "Error: Did not find MODE_9_5FPS\n");
    return -1;
  }
  // set use case
  if (camera_device_->setUseCase(use_cases.at(selected_use_case_idx)) !=
      royale::CameraStatus::SUCCESS) {
    syslog(LOG_ERR, "Error: Use case %s not set for the slave\n",
           use_mode.c_str());
    return -1;
  }

  // Modify the camera parameters and settings
  royale::Vector<royale::StreamId> stream_ids;
  camera_device_->getStreams(stream_ids);
  royale::StreamId stream_id = stream_ids.front();

  // Set camera exposure time. 0 indicates automatic exposure setting
  if (EXPOSURE_TIME_MS == 0) {
    // If the exposure time is 0, set the exposure automatically
    if (camera_device_->setExposureMode(royale::ExposureMode::AUTOMATIC) !=
        royale::CameraStatus::SUCCESS) {
      // Error: Failed to set camera in automatic exposure mode
      syslog(LOG_ERR, "Error: Failed to set automatic exposure\n");
      return -1;
    }
  } else {
    // Otherwise set the exposure to the user specified time
    if (camera_device_->setExposureTime(EXPOSURE_TIME_MS) !=
        royale::CameraStatus::SUCCESS) {
      // Error: Failed to set camera exposure time
      syslog(LOG_ERR, "Error: Failed to set manual exposure time\n");
      return -1;
    }
  }

  // Modifying camera parameters
  royale::ProcessingParameterVector ppvec;
  if (camera_device_->getProcessingParameters(ppvec, stream_id) !=
      royale::CameraStatus::SUCCESS) {
    // Error: Failed to grab camera parameters
    syslog(LOG_ERR, "Error: Failed to get processing parameters\n");
    return -1;
  }

  // Set the parameters
  for (auto &flagPair : ppvec) {
    royale::Variant var;
    switch (flagPair.first) {
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

  if (camera_device_->setProcessingParameters(ppvec, stream_id) !=
      royale::CameraStatus::SUCCESS) {
    syslog(LOG_ERR, "Error: Failed to set processing parameters\n");
    return -1;
  } else {
    if (verbose_)
      syslog(LOG_NOTICE, "Successfully set processing parameters\n");
  }

  tof_listener_.reset(new TOFDataListener(verbose_));
  if (camera_device_->registerDataListener(tof_listener_.get()) !=
      royale::CameraStatus::SUCCESS) {
    // Error: Failed to register data listener
    syslog(LOG_ERR, "Error: Failed to register data listener\n");
  } else {
    if (verbose_)
      syslog(LOG_NOTICE, "Successfully registered data listener\n");
  }
  return 0;
}

std::vector<cv::Mat> TOFDaemon::PerformForwardPass(cv::Mat &image) {
  std::vector<cv::Mat> nn_outputs;
  cv::Mat blob;
  cv::Mat nnet_input(NNET_INPUT_WIDTH, NNET_INPUT_HEIGHT, CV_8UC3);

  // First resize the image to the expected input size for the neural net
  cv::resize(image, nnet_input, cv::Size(NNET_INPUT_WIDTH, NNET_INPUT_HEIGHT));

  // TODO(CodeCleanup): Does this blob action here actually do anything since
  // there is no resizing and the mean for normalization is 0?
  cv::dnn::blobFromImage(nnet_input, blob, 1.0f,
                         cv::Size(NNET_INPUT_WIDTH, NNET_INPUT_HEIGHT), 0.0,
                         false, false);

  // Set the neural net input and perform the forward pass
  net_.setInput(blob);
  net_.forward(nn_outputs, NNET_OUTPUT_LAYER);
  return nn_outputs;
}

size_t TOFDaemon::ProcessROI(cv::Mat &gray_image, const cv::Rect roi_rect,
                             const cv::Mat ptcloud_depth,
                             const cv::Mat ptcloud_x, const cv::Mat ptcloud_y,
                             const cv::Mat ptcloud_z,
                             std::vector<float> &roi_object_x_coords,
                             std::vector<float> &roi_object_y_coords,
                             std::vector<float> &roi_object_z_coords) {
  OtsuThresholding otsu;

  // Extract the region of interest from the data
  cv::Mat img_roi(gray_image, roi_rect);
  cv::Mat ptcloud_depth_roi(ptcloud_depth, roi_rect);
  cv::Mat ptcloud_x_roi(ptcloud_x, roi_rect);
  cv::Mat ptcloud_y_roi(ptcloud_y, roi_rect);
  cv::Mat ptcloud_z_roi(ptcloud_z, roi_rect);

  // Extract the x and z coordinates from the region of interest
  int gray_threshold = otsu.GetThreshold(img_roi);
  size_t num_points =
      LocalizeROI(img_roi, ptcloud_depth_roi, ptcloud_x_roi, ptcloud_y_roi,
                  ptcloud_z_roi, gray_threshold, roi_object_x_coords,
                  roi_object_y_coords, roi_object_z_coords);
  return num_points;
}

size_t TOFDaemon::LocalizeROI(const cv::Mat &img_roi,
                              cv::Mat &ptcloud_depth_roi,
                              cv::Mat &ptcloud_x_roi, cv::Mat &ptcloud_y_roi,
                              cv::Mat &ptcloud_z_roi, const int &threshold,
                              std::vector<float> &roi_object_x_coords,
                              std::vector<float> &roi_object_y_coords,
                              std::vector<float> &roi_object_z_coords) {
  int i, j;
  float min_dist;
  int min_idx_i, min_idx_j;
  size_t num_points = 0;

  for (j = 0; j < img_roi.cols; j++) {
    min_dist = 100.0f;
    min_idx_i = -1;
    min_idx_j = -1;
    for (i = 0; i < img_roi.rows; i++) {
      if ((img_roi.at<uint8_t>(i, j) > threshold) &&
          (ptcloud_depth_roi.at<float>(i, j) < min_dist) &&
          (ptcloud_depth_roi.at<float>(i, j) > 0.1f)) {
        min_dist = ptcloud_depth_roi.at<float>(i, j);
        min_idx_i = i;
        min_idx_j = j;
      }
    }

    if (min_idx_i >= 0) {
      roi_object_x_coords[num_points] =
          ptcloud_x_roi.at<float>(min_idx_i, min_idx_j);
      roi_object_y_coords[num_points] =
          ptcloud_y_roi.at<float>(min_idx_i, min_idx_j);
      roi_object_z_coords[num_points] =
          ptcloud_z_roi.at<float>(min_idx_i, min_idx_j);
      num_points++;
    }
  }
  return num_points;
}

std::vector<TOFMessage::Point2D> TOFDaemon::ConvertTOFPointsToLDSPoints(
    const size_t num_points, const std::vector<float> &image_object_x_coords,
    const std::vector<float> &image_object_y_coords,
    const std::vector<float> &image_object_z_coords) {

  // Conversion from meter to millimeters since the TOF point units are in
  // meters and the transformation matrix + robot code are in millimeters
  const float kMeterToMMConversion = 1000.0f;

  std::vector<TOFMessage::Point2D> object_points;
  for (size_t i = 0; i < num_points; i++) {
    // Create the 4x1 vector with the object point coordinates in the TOF frame
    // with a 1 appended for the homogeneous transformation
    float x = image_object_x_coords[i] * kMeterToMMConversion;
    float y = image_object_y_coords[i] * kMeterToMMConversion;
    float z = image_object_z_coords[i] * kMeterToMMConversion;
    Eigen::Vector4f object_point(x, y, z, 1.0f);

    // Transform the point with the given transformation matrix
    Eigen::Vector4f transformed_object_point =
        tof_to_lds_transform_ * object_point;

    // We only need the transformed x and y coordinates which are the first and
    // second element of of the vector, respectively
    TOFMessage::Point2D transformed_point;
    transformed_point.x = static_cast<int32_t>(transformed_object_point(0));
    transformed_point.y = static_cast<int32_t>(transformed_object_point(1));
    object_points.push_back(transformed_point);
  }

  return object_points;
}

void TOFDaemon::SendLiveVideoStream(cv::Mat &image) {
  std::vector<unsigned char> encoded;
  int total_pack = 0;

  // Encode the image and calculate the number of packets required to send the
  // image
  cv::imencode(".jpg", image, encoded, COMPRESSION_PARAMS);
  total_pack = 1 + (static_cast<int>(encoded.size()) - 1) / PACK_SIZE;

  // Send the first header packet with the number of data packets for this
  // particular image
  int ibuf[1];
  ibuf[0] = total_pack;
  live_video_socket_->sendTo(ibuf, sizeof(int),
                             live_video_socket_params_.ip_address,
                             live_video_socket_params_.socket_port);

  // Send the image itself over the socket
  for (int buf_idx = 0; buf_idx < total_pack; buf_idx++) {
    live_video_socket_->sendTo(&encoded[buf_idx * PACK_SIZE], PACK_SIZE,
                               live_video_socket_params_.ip_address,
                               live_video_socket_params_.socket_port);
  }
}

int TOFDaemon::Run() {
  // Spawn this process as a daemon
  int ret = Daemonize(DAEMON_NAME, "/tmp", NULL, NULL, NULL);
  if (ret != 0) {
    // Error: Failed to create a daemon
    fprintf(stderr, "Error: Failed to create daemon\n");
    syslog(LOG_ERR, "Error: Failed to create daemon\n");
    exit(ret);
  } else {
    // Successfully spawned daemon
    syslog(LOG_NOTICE, "%s v%s running as daemon", DAEMON_NAME, REVISION);

    // Set the new data flag to false by default
    bool new_data_available = false;

    // Store the start time of the daemon
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Initialize the member variables in the daemon process so that everything
    // accessible in the grandchild process
    Init();

    // Check to make sure that initialization was successful, otherwise do not
    // run
    if (!init_successful_) {
      Shutdown();
      fprintf(stderr, "Error: TOFDaemon initialization failed.\n");
      syslog(LOG_ERR, "Error: TOFDaemon initialization failed.\n");
      return -1;
    }

    // Get the dimensions of the TOF sensor
    uint16_t tof_num_columns, tof_num_rows;
    camera_device_->getMaxSensorWidth(tof_num_columns);
    camera_device_->getMaxSensorHeight(tof_num_rows);

    // Initialize the buffer size and the opencv matrix to the correct
    // dimensions
    size_t buffer_size = static_cast<size_t>(tof_num_columns) *
                         static_cast<size_t>(tof_num_rows);
    cv::Size2f sensor_size_float(static_cast<float>(tof_num_columns),
                                 static_cast<float>(tof_num_rows));

    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    // DEBUG ONLY: Display the time in milliseconds needed to setup the daemon
    double setup_time_ms =
        static_cast<double>(end_time.tv_sec - start_time.tv_sec) * 1000. +
        static_cast<double>(end_time.tv_usec - start_time.tv_usec) / 1000.;
    if (verbose_)
      syslog(LOG_NOTICE, "setup time took %g milliseconds\n", setup_time_ms);

    std::thread processingThread([&]() {
      // We'll need the signal handler(s) to stop the video capture;
      while (gTOFDaemonRunning) {

        bool is_camera_capturing;
        camera_device_->isCapturing(is_camera_capturing);
        if (!gTOFDaemonStreaming && is_camera_capturing) {
          // If the command is given to stop streaming and the camera is still
          // capturing, turn the camera off since it is not being used
          if (camera_device_->stopCapture() != royale::CameraStatus::SUCCESS) {
            syslog(LOG_ERR, "Error: Failed to stop camera capture\n");
          } else {
            syslog(LOG_NOTICE, "Stopping video capture\n");

            // Need to reinitialize the camera when it is stopped so that when
            // the command to restart the camera is given again, the camera can
            // start immediately. Without reinitialization, the camera can take
            // ~20 seconds to perform the full initialization + start capture
            // process. By performing the initialization pre-emptively when the
            // stop command is given, this saves time later on.
            syslog(LOG_NOTICE, "Reinitializing camera for next usage\n");
            if (ConnectTOFCamera() < 0) {
              syslog(LOG_ERR, "Error: Failed to start camera capture\n");
            }
          }
        } else if (gTOFDaemonStreaming && !is_camera_capturing) {
          // If the daemon is supposed to be streaming and the camera is off,
          // turn the camera on.
          if (camera_device_->startCapture() != royale::CameraStatus::SUCCESS) {
            // Error: Failed to start capture
            // TODO(CodeCleanup): Should the TOFDaemon exit completely if the
            // camera cannot reconnect?
            syslog(LOG_ERR, "Error: Failed to start video capture\n");
          } else {
            syslog(LOG_NOTICE, "Starting video capture\n");
          }
        }

        struct timespec start, stop;
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
          syslog(LOG_ERR, "Error: Failed to get clock start time\n");
        }

        new_data_available = false;
        FrameDataStruct frame_data;
        if (!gFramesQueue.empty()) {
          if (verbose_) {
            syslog(LOG_NOTICE, "New Data Available\n");
            syslog(LOG_NOTICE, "Processing-Thread gFramesQueue Size : %zu\n",
                   gFramesQueue.size());
          }
          frame_data = gFramesQueue.get_fresh_and_pop();
          new_data_available = true;
        }

        // Only process data when new data is available AND the daemon should
        // stream to the robot process. When gTOFDaemonStreaming is false, we
        // still want to pop data from gFramesQueue so that the queue does not
        // fill up and stale data is not process when finally enabled
        if (new_data_available && gTOFDaemonStreaming) {
          struct timespec start_infer, stop_infer;
          if (clock_gettime(CLOCK_MONOTONIC, &start_infer) == -1) {
            syslog(LOG_ERR, "Error: Failed to get clock start time\n");
          }

          // Perform a forward pass on the image data from the TOF sensor
          std::vector<cv::Mat> nn_outputs =
              PerformForwardPass(frame_data.mat_nnet_input);

          //  Initialize the opencv matrices for the point cloud data
          cv::Mat ptcloud_depth(tof_num_rows, tof_num_columns, CV_32FC1,
                                frame_data.vec_point_cloud_distance.data());
          cv::Mat ptcloud_x(tof_num_rows, tof_num_columns, CV_32FC1,
                            frame_data.vec_point_cloud_X.data());
          cv::Mat ptcloud_y(tof_num_rows, tof_num_columns, CV_32FC1,
                            frame_data.vec_point_cloud_Y.data());
          cv::Mat ptcloud_z(tof_num_rows, tof_num_columns, CV_32FC1,
                            frame_data.vec_point_cloud_Z.data());

          // Postprocess the neural net output to extract the bounding boxes,
          // classes, and confidence scores
          std::vector<int> class_ids;
          std::vector<float> confidences;
          std::vector<Rect> boxes;
          std::vector<int> indices;
          Postprocess(nn_outputs, class_ids, confidences, boxes, indices,
                      sensor_size_float);

          // Instantiate the value buffers that will store all of the object
          // coordinates for the entire image
          std::vector<float> image_object_x_coords;
          std::vector<float> image_object_y_coords;
          std::vector<float> image_object_z_coords;

          // Keep track of the total number of object points there are in this
          // image
          size_t num_image_object_points = 0;

          // Process each region of interest in the image
          for (size_t i = 0; i < indices.size(); ++i) {
            int idx = indices[i];
            Rect roi_rect = boxes[idx];

            // Instantiate the value buffers since they will be modified by the
            // ProcessROI function
            std::vector<float> roi_object_x_coords(buffer_size, 0.0f);
            std::vector<float> roi_object_y_coords(buffer_size, 0.0f);
            std::vector<float> roi_object_z_coords(buffer_size, 0.0f);

            // Extract the x,y,z coordinates from the region of interest
            size_t num_object_points =
                ProcessROI(frame_data.mat_gray_image, boxes[idx], ptcloud_depth,
                           ptcloud_x, ptcloud_y, ptcloud_z, roi_object_x_coords,
                           roi_object_y_coords, roi_object_z_coords);

            // Copy the roi object points into the whole image object points
            if (num_object_points > 0) {
              image_object_x_coords.reserve(
                  image_object_x_coords.size() +
                  distance(roi_object_x_coords.begin(),
                           roi_object_x_coords.end()));
              image_object_x_coords.insert(
                  image_object_x_coords.end(), roi_object_x_coords.begin(),
                  roi_object_x_coords.begin() + num_object_points);
              image_object_y_coords.reserve(
                  image_object_y_coords.size() +
                  distance(roi_object_y_coords.begin(),
                           roi_object_y_coords.end()));
              image_object_y_coords.insert(
                  image_object_y_coords.end(), roi_object_y_coords.begin(),
                  roi_object_y_coords.begin() + num_object_points);
              image_object_z_coords.reserve(
                  image_object_z_coords.size() +
                  distance(roi_object_z_coords.begin(),
                           roi_object_z_coords.end()));
              image_object_z_coords.insert(
                  image_object_z_coords.end(), roi_object_z_coords.begin(),
                  roi_object_z_coords.begin() + num_object_points);
              num_image_object_points += num_object_points;
              if (verbose_)
                syslog(LOG_NOTICE,
                       "This ROI provides this many points %zu/%zu\n",
                       num_object_points, image_object_x_coords.size());
            }

            if (live_stream_video_) {
              DrawPredictions(class_ids[idx], confidences[idx], roi_rect.x,
                              roi_rect.y, roi_rect.x + roi_rect.width,
                              roi_rect.y + roi_rect.height,
                              frame_data.mat_nnet_input);
            }
          }

          // Send the object points over to the robot
          std::vector<TOFMessage::Point2D> object_points;
          uint32_t status = TOFMessage::TOF_OK;
          if (num_image_object_points > 0) {
            // If objects have been detected, transform the points from the TOF
            // frame to the LDS frame
            object_points = ConvertTOFPointsToLDSPoints(
                num_image_object_points, image_object_x_coords,
                image_object_y_coords, image_object_z_coords);

            // If there are more than the maximum number of points, only return
            // the maximum number but change the TOFMessage status to indicate
            // points were left out
            if (num_image_object_points >
                TOFMessage::kMaxTOFObjectPointsPerImage) {
              syslog(
                  LOG_NOTICE,
                  "WARNING: There are more object points detected than can "
                  "be published to the robot process. Only %d/%zu points are "
                  "contained in the message! \n",
                  TOFMessage::kMaxTOFObjectPointsPerImage,
                  num_image_object_points);
              num_image_object_points = TOFMessage::kMaxTOFObjectPointsPerImage;
              object_points.resize(num_image_object_points);
              status = TOFMessage::TOF_OBJECT_OVERFLOW;
            } else {
              syslog(LOG_NOTICE, "Publishing %zu points! \n",
                     num_image_object_points);
            }
          }
          // Send the points or an empty point vector over NeatoIPC to the robot
          // app
          tof_server_->Publish(status, frame_data.system_timestamp,
                               num_image_object_points, object_points);

          if (clock_gettime(CLOCK_MONOTONIC, &stop_infer) == -1) {
            syslog(LOG_ERR, "Error: Failed to get clock stop time\n");
          }

          if (live_stream_video_) {
            SendLiveVideoStream(frame_data.mat_nnet_input);
          }
        } else {
          if (gTOFDaemonStreaming) {
            // If the camera is turned on and the TOFDaemon should be streaming
            // but there is no data available, then print an error message
            syslog(LOG_ERR, "Error: TOFDaemon should be streaming but no "
                            "frames are entering the queue!\n");
          }

          // Sleep is necessary when the TOFDaemon is not processing frames so
          // that it doesn't take over all the processing power on the robot
          usleep(100000);
        }

        if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
          syslog(LOG_ERR, "Error: Failed to get clock stop time\n");
        }
      }
    });

    processingThread.join();

    // We're done processing frames.  Shutdown the camera.
    if (camera_device_->stopCapture() != royale::CameraStatus::SUCCESS) {
      syslog(LOG_ERR, "Error: Failed to stop camera capture\n");
      Shutdown();
      return -1;
    }
  }

  Shutdown();
  return 0;
}

int main(int argc, char **argv) {
  TOFDaemon tof_daemon;
  tof_daemon.Run();
  return 0;
}
