#include <getopt.h>
#include <unistd.h>

#include <CameraFactory.hpp>
#include <iostream>
#include <royale/ICameraDevice.hpp>
#include <string>

using namespace std;
using namespace royale;
using namespace platform;
#include "camera.h"

std::string VERSION{"1.3"};
const bool EXIT_ON_ERROR = true;

void print_help() {
  std::cout << "-v                   Show program version.\n"
               "-r <n>               Set number of seconds to record: -r 60\n"
               "-m <str>             Set ToF mode: -m MODE_9_5FPS\n"
               "-h                   Show help\n";
  exit(EXIT_FAILURE);
}

#define OPTSTR "vr:m:h"

typedef struct {
  string version;
  int numSecondsToStream;
  royale::String test_mode;
} options_t;

int main(int argc, char **argv) {
  int opt;
  // Default options
  options_t options = {VERSION, 15, "MODE_9_5FPS"};
  std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";

  // CameraFactory factory;
  Camera cam;

  if (argc > 1) {
    while ((opt = getopt(argc, argv, OPTSTR)) != -1) {
      switch (opt) {
        case 'v':
          std::cout << "Version: " << options.version << std::endl;
          exit(EXIT_FAILURE);
          break;
        case 'r':
          options.numSecondsToStream = std::stoi(optarg);
          if (options.numSecondsToStream < 10) {
            options.numSecondsToStream = 10;
            std::cout << "Minimum Streaming Time is 10 Seconds." << std::endl;
          }
          std::cout << "Streaming Time [seconds]: " << options.numSecondsToStream << std::endl;
          if (options.numSecondsToStream > 300) {
            std::cout << "USING STREAM TEST-MODE : Output will be stream to /dev/null" << std::endl;
          }
          break;
        case 'm':
          options.test_mode = royale::String(optarg);
          std::cout << "Setting ToF Mode: " << options.test_mode << std::endl;
          break;
        case 'h':
        default:
          print_help();
          break;
      }
    }
  } else {
    std::cout << "Using Default Settings." << std::endl;
    std::cout << "Streaming Time [seconds]: " << options.numSecondsToStream << std::endl;
    std::cout << "Setting ToF Mode: " << options.test_mode << std::endl;
  }

  // [Setup] Camera Initialization Test
  Camera::CameraError error = cam.RunInitializeTests(options.test_mode);
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Setup] Camera Stream Tests
  error = cam.RunStreamTests();
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Setup] Access Level Test
  if (!ACCESS_CODE.empty()) { error = cam.RunAccessLevelTests(2); }
  else { error = cam.RunAccessLevelTests(1); }
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Optional] Use Case Tests  Waiting on Emcraft to get all use cases working
  // error = cam.RunUseCaseTests();
  // if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Setup] Exposure Tests
  error = cam.RunExposureTests();
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Setup] Processing Parameters Test
  error = cam.RunProcessingParametersTests();
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Setup] Lens Parameters Test
  error = cam.RunLensParametersTest();
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  // [Streaming] Test Receive Data
  error = cam.RunTestReceiveData(options.numSecondsToStream);
  if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

  return 0;
}
