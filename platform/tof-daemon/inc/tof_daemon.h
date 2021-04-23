
#include <bits/stdc++.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <CameraFactory.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <thread>

#include "PracticalSocket.h"
#include "frame_queue.h"
#include "neural_network_params.h"
#include "otsu_threshold.h"
#include "pipeline_listener.h"
#include "tof_daemon_params.h"

#include "neatoipc/neatoipc.h"

#ifndef _TOF_DAEMON_H_
#define _TOF_DAEMON_H_

using namespace std;
using namespace cv;

// Global Frame Queue
FrameQueue<FrameDataStruct> gFramesQueue;

// Global flag to indicate if the TOF Daemon should continue running. By default
// it is set to true but the signal handlers can shut the daemon off.
bool gTOFDaemonRunning = true;

/**
 * @brief SIGHUP Handler. If the SIGHUP signal is received, the handler flips
 * the global boolean used to indicate if the TOF daemon should keep running,
 * essentially forcing the daemon to shut down gracefully.
 * @param signo the signal received.
 */
void sighup_handler(int signo) {
  syslog(LOG_INFO, "Received SIGHUP.  Shutting down TOFDaemon\n");
  gTOFDaemonRunning = false;
}

/**
 * @brief SIGINT Handler. If the SIGINT signal is received, the handler flips
 * the global boolean used to indicate if the TOF daemon should keep running,
 * essentially forcing the daemon to shut down gracefully.
 * @param signo the signal received.
 */
void sigint_handler(int signo) {
  syslog(LOG_INFO, "Received SIGINT.  Shutting down TOFDaemon\n");
  gTOFDaemonRunning = false;
}

/**
 * @brief Creates a directory given a path. The function also creates the parent
 * directories along the path if they also do not exist.
 * @param path The file path to be created
 * @return true if the path is successfully created, false otherwise
 */
bool CreateDirectory(std::string path);

/**
 * @brief Removes the following file path to create more space. This is used by
 * the nftw function
 * @param path File path to be removed
 * @param sbuf Not used but is required by the nftw function
 * @param type Not used but is required by the nftw function
 * @param ftwb Not used but is required by the nftw function
 * @return Returns a 0 if the file is removed succesfully, -1 otherwise
 */
static int RemoveFiles(const char *pathname, const struct stat *sbuf, int type,
                       struct FTW *ftwb);

/**
 * @brief Struct to hold the parameters for socket streaming. This struct
 * contains the socket IP address of the server as a string, the port on the
 * server as a string, and the port on the host (as defined by the
 * Socket::resolveService() call) as an unsigned short.
 */
struct SocketParams {
  std::string ip_address;
  std::string port_string;
  unsigned short socket_port;
};

struct TOFMessage {
  enum TOFCommand {
    // Driver Command/Response values
    RAW_COMMAND = 0,
    STREAM_STOP = 1,
    STREAM_START = 2,
    POWER_OFF = 3,
    POWER_ON = 4,
  };

  enum TOFStatus {
    // Driver command status
    TOF_OK = 0,
    CMD_ACK = 0,
    CMD_NAK = 1,
    TOF_STREAM_ERROR = 2,
    TOF_UNKNOWN = 3,
  };

  enum {
    TOF_STREAMING_DATA = 0x0001,
    // TODO(CodeCleanup): This number was chosen arbitrarily.
    kTOFObjectPointsPerImage = 500,
    RESPONSE_BUFFER_LEN = 1024,
  };

  struct TOFPoint3f {
    float x, y, z;
  };

  union {
    // Streaming data
    struct {
      uint32_t header;

      uint32_t status;
      size_t num_points;
      int64_t timestamp;

      TOFPoint3f points[kTOFObjectPointsPerImage];
    } pubsub;

    // Command response transactions
    struct {
      uint32_t command;
      uint32_t status;

      union {
        float data_float;
        int32_t data_integer;
        struct {
          uint8_t data_buffer[RESPONSE_BUFFER_LEN];
          size_t data_len;
        };
      };
    } transact;
  };
};

/**
 * @brief TOFServerInterface responsible for:
 *
 * 1. broadcasting TOF data after the processing of each frame
 * 2. Handling TOF CLI commands from clients
 */
class TOFServerInterface {
public:
  /**
   * @brief constructor for TOFServerInterface objects
   * @param handler call back function to call on the message when message is
   * received
   */
  explicit TOFServerInterface(neato_ipc::Server::transaction_handler_fn handler)
      : tof_streaming_broadcast_(neato_ipc::Node::IPC, "tof_stream_data"),
        tof_command_server_(neato_ipc::Node::IPC, "tof_command", handler) {}

  /**
   * @brief publish LDS Stream data
   * @param status status of the command
   * @param num_points the number of points stored in the data arrays
   * @param image_object_x_coords x coordinates of the object points
   * @param image_object_y_coords y coordinates of the object points
   * @param image_object_z_coords z coordinates of the object points
   * @return int -1 for bad arguement, -2 for ipc failure and 0 for success
   */
  int Publish(const int64_t timestamp, const size_t num_points,
              const vector<float> &image_object_x_coords,
              const vector<float> &image_object_y_coords,
              const vector<float> &image_object_z_coords) {
    const float kMeterToMMConversion = 1000.0f;
    TOFMessage msg;
    // TODO(CodeCleanup): Remove this forced instantiation of the message status
    // and return the actual status of the TOF sensor.
    msg.pubsub.header = TOFMessage::TOF_STREAMING_DATA;
    msg.pubsub.status = TOFMessage::TOF_OK;
    msg.pubsub.timestamp = timestamp;
    msg.pubsub.num_points = num_points;

    for (size_t i = 0; i < num_points; i++) {
      // Convert the points from coordinates in meters to millimeters since that
      // is what is expected on the robot side
      msg.pubsub.points[i].x = image_object_x_coords[i] * kMeterToMMConversion;
      msg.pubsub.points[i].y = image_object_y_coords[i] * kMeterToMMConversion;
      msg.pubsub.points[i].z = image_object_z_coords[i] * kMeterToMMConversion;
    }
    return tof_streaming_broadcast_.Send(msg);
  }

  /**
   * @brief Destructor. delete the objects created
   */
  ~TOFServerInterface() {}

private:
  neato_ipc::Bcast tof_streaming_broadcast_;
  neato_ipc::Server tof_command_server_;
};

/**
 * @brief TOFServerInterface responsible for:
 *
 * 1. broadcasting TOF data after the processing of each frame
 * 2. Handling TOF CLI commands from clients
 */
class TOFDaemon {
public:
  /**
   * @brief TOFDaemon constructor. Initializes the daemon.
   */
  TOFDaemon(){};

  /**
   * Run a single iteration of the TOF daemon.
   */
  int Run();

  /**
   * @brief Gracefully shuts down the TOF daemon by properly closing the file
   * descriptors.
   */
  void Shutdown();

private:
  /**
   * @brief Initialize the classes vector that stores all of the difference
   * classes that the neural network can classify objects into.
   */
  void InitializeClasses();

  /**
   * @brief Initialize the neural network and set the init_successful_ flag to
   * false if the neural network was not loaded properly
   */
  void InitializeNeuralNet();

  /**
   * @brief Initialize the sockets used to live stream video, localization
   * values, and plots. This is mainly used for debugging and can be toggled.
   */
  void InitializeStreamingSockets();

  /**
   * @brief Daemonize the TOF daemon code. This function creates a child, then
   * grandchild process. The parent and child are killed and the grandchild is
   * left as the daemon process. Once the grandchild is the only process
   * remaining, the lock file is created to ensure that only one daemon is
   * running and the stdin, stdout, stderr are set.
   * @param daemon_name The name of the daemon
   * @param daemon_path The file path in which the daemon operates and where the
   * lock file is going to be stored
   * @param outfile The path where stdout should be routed to
   * @param errfile The path where stderr should be routed to
   * @param infile The path there stdin should be routed to
   * @return 0 if the daemon was created successfully, 1 otherwise
   */
  int Daemonize(const char *daemon_name, const char *daemon_path,
                const char *outfile, const char *errfile, const char *infile);

  /**
   * @brief Initialize the folder structure for saving the TOF data. Deletes any
   * old data in the tof folder, if any exists. Then the function creates the
   * folders where the data will be stored for the daemon.
   */
  void InitializeTOFDataStorage();

  /**
   * @brief Initialize the TOF sensor and connect to the camera. Create the
   * listener and camera objects to start retrieving data. This function also
   * modifies the camera parameters following predefined settings.
   * @return 0 if the camera was connected successfully, -1 otherwise
   */
  int ConnectTOFCamera();

  /**
   * @brief Saves the TOF point cloud data in PLY format to a specified file.
   * The x,y,z and the gray data are stored in separate files.
   * @param filename The file path to store the x,y,z data
   * @param filename_gray The file path to store the gray data
   * @param vec_x The x data
   * @param vec_y The y data
   * @param vec_z The z data
   * @param vec_gray The gray data
   */
  void SavePointCloud(const std::string &filename,
                      const std::string &filename_gray,
                      const std::vector<float> *vec_x,
                      const std::vector<float> *vec_y,
                      const std::vector<float> *vec_z,
                      const std::vector<uint16_t> *vec_gray);

  /**
   * @brief Take the output of the neural net and extract the bounding boxes,
   * classes, and confidence scores. After extraction, the bounding boxes are
   * run through non-maximum verbose_ion (NMS) to further filter the bounding
   * boxes to the best detections.
   * @param nn_outputs The output of the neural net
   * @param class_ids Vector of class ids to be filled
   * @param confidences Vector of confidences to be filled
   * @param boxes Vector of bounding boxes to be filled
   * @param indices Vector of indices for the best bounding boxes to be filled
   * after running NMS
   * @param sensor_size_float The shape of the input image as floats
   */
  void Postprocess(const std::vector<Mat> &nn_outputs,
                   std::vector<int> &class_ids, std::vector<float> &confidences,
                   std::vector<cv::Rect> &boxes, std::vector<int> &indices,
                   cv::Size2f &sensor_size_float);

  /**
   * @brief Process a region of interest for the given image. Extract the x and
   * z coordinates of the image so that they can be sent to the robot for object
   * avoidance.
   * @param gray_image The gray scale image from the TOF sensor
   * @param roi_rect The region of interest after post processing the neural net
   * output
   * @param ptcloud_depth The TOF depth values for the entire image
   * @param ptcloud_x The TOF x values for the entire image
   * @param ptcloud_y The TOF y values for the entire image
   * @param ptcloud_z The TOF z values for the entire image
   * @param roi_object_x_coords x coordinates of the object in the region of
   * interest
   * @param roi_object_y_coords y coordinates of the object in the region of
   * interest
   * @param roi_object_z_coords z coordinates of the object in the region of
   * interest
   * @return The number of points extracted from the regoin of interest and
   * filled into the roi_object_x_coords and roi_object_z_coords
   */
  size_t ProcessROI(cv::Mat &gray_image, const cv::Rect roi_rect,
                    const cv::Mat ptcloud_depth, const cv::Mat ptcloud_x,
                    const cv::Mat ptcloud_y, const cv::Mat ptcloud_z,
                    vector<float> &roi_object_x_coords,
                    vector<float> &roi_object_y_coords,
                    vector<float> &roi_object_z_coords);

  /**
   * @brief Exract the x,y,z coordinates in the TOF frame of the point closest
   * to the robot on the obstacle with the given ROI
   * @param img_roi The ROI of the whole image
   * @param ptcloud_depth_roi The depth point cloud data in the ROI
   * @param ptcloud_x_roi The TOF x values for the ROI
   * @param ptcloud_y_roi The TOF y values for the ROI
   * @param ptcloud_z_roi The TOF z values for the ROI
   * @param threshold The otsu threshold calculated on the image ROI
   * @param roi_object_x_coords x coordinates of the object in the region of
   * interest
   * @param roi_object_y_coords y coordinates of the object in the region of
   * interest
   * @param roi_object_z_coords z coordinates of the object in the region of
   * interest
   * @return The number of points extracted from the regoin of interest and
   * filled into the roi_object_x_coords and roi_object_z_coords
   */
  size_t LocalizeROI(const cv::Mat &img_roi, cv::Mat &ptcloud_depth_roi,
                     cv::Mat &ptcloud_x_roi, cv::Mat &ptcloud_y_roi,
                     cv::Mat &ptcloud_z_roi, const int &threshold,
                     std::vector<float> &x_vals, std::vector<float> &y_vals,
                     std::vector<float> &z_vals);

  /**
   * @brief Take the object coordinates from the ROI and add them to the text
   * stream output that will be sent over the socket.
   * TODO(CodeCleanup): This whole function can be deprecated once the NeatoIPC
   * implementation is finished.
   * @param num_points The number of points that the ROI processing found
   * @param roi_object_x_coords vector containing the x coordinates
   * @param roi_object_z_coords vector containing the z coordinates
   * @param stream_out The text string output to be concatenated with the object
   * coordinates
   * @param stream_out_plot The text string output to be concatenated with the
   * object coordinates for plotting
   */
  void SendTextStringOutput(const size_t num_points,
                            vector<float> &roi_object_x_coords,
                            vector<float> &roi_object_z_coords,
                            std::string &stream_out,
                            std::string &stream_out_plot);

  /**
   * @brief Draw the bounding box along with the detected class and the
   * confidence score onto the image for live video streaming.
   * @param class_id The class of the object
   * @param confidence Confidence score of the object
   * @param left The x coordinate of the top left corner of the bounding box
   * @param top The y coordinate of the top left corner of the bounding box
   * @param right The x coordinate of the bottom right corner of the bounding
   * box
   * @param bottom The y coordinate of the bottom right corner of the bounding
   * box
   * @param frame The input image passed into the neural net so the bounding
   * boxes and labels are overlayed on the original image
   */
  void DrawPredictions(int class_id, float confidence, int left, int top,
                       int right, int bottom, Mat &frame);

  /**
   * @brief Take the raw image from the listener, preprocess the image, then
   * run the preprocessed image through the neural network for a forward pass.
   * Return the neural net output.
   * @param image The raw image from the TOF sensor
   * @return The output of the neural network on the preprocessed image
   */
  std::vector<cv::Mat> PerformForwardPass(cv::Mat &image);

  /**
   * @brief Take the postprocessed image with the predictions overlayed
   * and send the image over the live video socket for display.
   * @param image The postprocessed and labeled image to be sent over the socket
   */
  void SendLiveVideoStream(cv::Mat &image);

  // All of the object classes that are identifiable by the neural network
  std::vector<std::string> classes_;

  // File descriptor to the lock file
  int lock_fd_ = -1;

  // Instantiate the vector of all possible class names that can be identified
  std::vector<std::string> class_names_;

  // Instantiate the neural network
  dnn::Net net_;

  // Turn on or off live video streaming
  bool live_stream_video_ = true;

  // Toggle the streaming to the localization server
  bool live_localization_ = true;

  // Toggle the streaming to the plotting server
  bool live_plot_ = false;

  // Used for debugging. Set this to true to save the TOF data locally
  bool save_data_ = false;

  // Set this to false to stop the logging statements
  bool verbose_ = true;

  // Flag that indicates if the initialization of the member variables was
  // successful or not
  bool init_successful_ = true;

  // Sockets and socket parameters. Used primarily for debugging.
  SocketParams live_video_socket_params_;
  SocketParams live_localization_socket_params_;
  SocketParams live_plot_socket_params_;
  // NOTE: These sockets have to be instantiated in the grandchild daemon
  // process otherwise they will not send data correctly. Instantiating them as
  // pointers here so that they can be initialized by the daemon process
  // directly later.
  std::unique_ptr<UDPSocket> live_video_socket_;
  std::unique_ptr<UDPSocket> live_localization_socket_;
  std::unique_ptr<UDPSocket> live_plot_socket_;

  // Connections to the TOF sensor
  std::unique_ptr<TOFDataListener> tof_listener_;
  platform::CameraFactory camera_factory_;
  std::unique_ptr<royale::ICameraDevice> camera_device_;

  // TODO(CodeCleanup): Remove these string constants when we transition to
  // using NeatoIPC
  const std::string kObjectTimeStampString = "TIMESTAMP : ";
  const std::string kObjectIDString = "OBJECT_ID : SOCK-";
  const std::string kObjectLocationString = "LOCATION : ";
  const std::string kObjectDelimeterString = "|";

  std::unique_ptr<TOFServerInterface> tof_server_;
};

#endif
