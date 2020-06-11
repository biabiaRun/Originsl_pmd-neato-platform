#include <iostream>
#include <string>
#include <royale/ICameraDevice.hpp>
#include <CameraFactory.hpp>

using namespace std;
using namespace royale;
using namespace platform;
#include "camera.h"

const bool EXIT_ON_ERROR = true;

int main(int argc, char **argv)
{
    std::string ACCESS_CODE = "d79dab562f13ef8373e906d919aec323a2857388";

    //CameraFactory factory;
    Camera cam;

    royale::String test_mode = "MODE_5_15FPS";
    int test_fps = 15;
    int numSecondsToStream = 10;

    // [Setup] Camera Initialization Test
    Camera::CameraError error = cam.RunInitializeTests(test_mode, test_fps);
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
    error = cam.RunTestReceiveData(numSecondsToStream);
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    return 0;
}
