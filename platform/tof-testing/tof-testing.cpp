#include <iostream>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include <royale/ICameraDevice.hpp>
#include <CameraFactory.hpp>

using namespace std;
using namespace royale;
using namespace platform;
#include "camera-testing.h"

std::string VERSION{"1.1"};
const bool EXIT_ON_ERROR = true;

void print_help() {
  std::cout <<
        "-v                   Show program version.\n"
        "-r <n>               Set number of seconds to record: -r 60\n"
        "-m <str>             Set ToF mode: -m MODE_9_5FPS\n"
        "-h                   Show help\n";
  exit(EXIT_FAILURE);
}

#define OPTSTR "vr:m:h"

typedef struct {
  string         version;
  royale::String test_mode;
} options_t;

int main(int argc, char **argv)
{
    int opt;
    // Default options
    options_t options = { VERSION, "MODE_9_5FPS" };
    std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";

    if (argc > 1) {
      while ((opt = getopt(argc, argv, OPTSTR)) != -1) {
        switch(opt) {
          case 'v':
            std::cout << "Version: " << options.version << std::endl;
            exit(EXIT_FAILURE);
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
    }else{
      std::cout << "Using Default Settings." << std::endl;
      std::cout << "Setting ToF Mode: " << options.test_mode << std::endl;
    }

    // CameraFactory factory;
    Camera cam;

    // [Setup] Camera Initialization Test
    Camera::CameraError error = cam.InitializeCamera(options.test_mode);
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    return 0;
}
