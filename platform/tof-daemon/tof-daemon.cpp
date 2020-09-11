#include "tof_module_params.h"
#include "pipeline_listener.h"
#include "common.h"

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
#include <sys/types.h>
#include <CameraFactory.hpp>

using namespace std;

// #include <royale/ICameraDevice.hpp>
// #include <CameraFactory.hpp>

// Name and version of the TOF Daemon
const char *DAEMON_NAME = "TOFDaemon";
const char *REVISION = "1.0.0";

std::mutex g_ptcloud_mutex;
std::condition_variable g_ptcloud_cv;
bool g_newDataAvailable;

// Local Data Write Folders
std::string LOCAL_DATA_PATH = "/home/root/tof-data-repo/";
std::string IR_FOLDER("/gray-img/");
std::string PTCLOUD_FOLDER("/dense-pc/");
std::string DEPTH_IMAGE_FOLDER("/depth-img/");
std::string CALIBRATION_FOLDER("/tof-to-lsd-calibration/");

/*
 * File path to the "lock file", which when created indicates that the
 * TOF daemon already exists.
 */
const char *LOCK_FILE = "/tmp/tofdaemon.lock";

// File descriptor to the lock file
int lock_fd = -1;

// Should the TOFDaemon continue processing frames?
bool g_daemon_live = true;

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
bool CreateDirectory(std::string &dir_path) {
  int status;
  std::string command = "mkdir -p ";
  command += dir_path;
  status = system(command.c_str());
  if(status == -1) {
    syslog(LOG_ERR, "Failed to make new TOF %s output directory", dir_path.c_str());
    std::cout << "Failed to create new Direcotry " << dir_path.c_str() << std::endl << std::flush;
    return false;
  } else {
    return true;
  }
}

// Remove a directory
bool RemoveDirectory(std::string &dir_path) {
  int status;
  std::string command = "rm -rf ";
  command += dir_path;
  status = system(command.c_str());
  if(status == -1) {
    syslog(LOG_ERR, "Failed to remove directory %s", dir_path.c_str());
    std::cout << "Failed to remove the Direcotry " << dir_path.c_str() << std::endl << std::flush;
    return false;
  } else {
    return true;
  }
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
  //RemoveDirectory(LOCAL_DATA_PATH);

  // Create the output directories
  CreateDirectory(IR_FOLDER);
  CreateDirectory(PTCLOUD_FOLDER);
  CreateDirectory(DEPTH_IMAGE_FOLDER);
  CreateDirectory(CALIBRATION_FOLDER);
}


int main(int argc, char **argv) {

  // Parse command line arguments
  int c;
  // Suppress debug messages?
  bool suppress = false;
  // Save ToF Data?
  bool save_data = true; //false;  
  g_newDataAvailable = false;

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
      return 1;
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
      return 1;
    }
    // set use case
    if (cameraDevice->setUseCase(use_cases.at(selectedUseCaseIdx)) != royale::CameraStatus::SUCCESS) {
      syslog(LOG_ERR, "Error setting use case %s for the slave\n", use_mode.c_str());
      return 1;
    }


    // Modify the camera parameters and settings
    royale::Vector<royale::StreamId> streamids;
    cameraDevice->getStreams(streamids);
    royale::StreamId streamId = streamids.front();

    // Set camera exposure time
    cameraDevice->setExposureMode(royale::ExposureMode::AUTOMATIC, streamId);
    //cameraDevice->setExposureTime(EXPOSURE_TIME_MS, streamId);

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
    } else {
      syslog(LOG_NOTICE, "TOFDaemon successfully set processing parameters\n");
    }


      // Register depth image listener for saving data
      /*std::unique_ptr<royale::IDepthImageListener> myDepthImageListener;
      if(save_data)
      {
          myDepthImageListener.reset(new DepthImageListener(DEPTH_IMAGE_FOLDER));
          if(cameraDevice->registerDepthImageListener(myDepthImageListener.get()) != royale::CameraStatus::SUCCESS)
          {
              // ERROR: Failed to register data listener
              syslog(LOG_ERR, "Error: Failed to register depth image data listener\n");
              Shutdown();
              return -1;
          }
      }

      // Register IR intensity image listener for saving data
      std::unique_ptr<royale::IIRImageListener> myIRImageListener;
      if(save_data)
      {
          myIRImageListener.reset(new IRImageListener(IR_FOLDER));
          if(cameraDevice->registerIRImageListener(myIRImageListener.get()) != royale::CameraStatus::SUCCESS)
          {
              // ERROR: Failed to register data listener
              syslog(LOG_ERR, "Error: Failed to register IR image data listener\n");
              Shutdown();
              return -1;
          }
      } */

    uint16_t maxSensorWidth, maxSensorHeight;
    status = cameraDevice->getMaxSensorWidth(maxSensorWidth);
    status = cameraDevice->getMaxSensorHeight(maxSensorHeight);

    size_t buffer_size = static_cast<size_t>(maxSensorWidth) * static_cast<size_t>(maxSensorHeight);

    std::vector<float> point_cloud_x(buffer_size);
    std::vector<float> point_cloud_y(buffer_size);
    std::vector<float> point_cloud_z(buffer_size);
    std::vector<uint16_t> gray_image(buffer_size);

    // Register a data listener
    // listener.reset(new MyListener(suppress, PTCLOUD_FOLDER));
    listener.reset(new MyListener(suppress, PTCLOUD_FOLDER, IR_FOLDER, &point_cloud_x, &point_cloud_y, &point_cloud_z, &gray_image));
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
      std::unique_lock<std::mutex> lock (g_ptcloud_mutex);
      auto timeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds (2000));
      if (g_ptcloud_cv.wait_until (lock, timeOut, [&] {return g_newDataAvailable; })) {
        g_newDataAvailable = false;
      } else {
        // we ran into a timeout so sleep for  100 milliseconds
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
      }
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
/*
using namespace std;

#define DAEMON_NAME "tof-daemon"

void process(){

    syslog (LOG_NOTICE, "Writing to my Syslog");
}

int main(int argc, char *argv[]) {

    //Set our Logging Mask and open the Log
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

    syslog(LOG_INFO, "Entering Daemon");

    pid_t pid, sid;

   //Fork the Parent Process
    pid = fork();

    if (pid < 0) { exit(EXIT_FAILURE); }

    //We got a good pid, Close the Parent Process
    if (pid > 0) { exit(EXIT_SUCCESS); }

    //Change File Mask
    umask(0);

    //Create a new Signature Id for our child
    sid = setsid();
    if (sid < 0) { exit(EXIT_FAILURE); }

    //Change Directory
    //If we cant find the directory we exit with failure.
    if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }

    //Close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //----------------
    //Main Process
    //----------------
    while(true){
        process();    //Run our Process
        sleep(60);    //Sleep for 60 seconds
    }

    //Close the log
    closelog ();
}

*/
