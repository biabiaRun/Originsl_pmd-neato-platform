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
    std::string ACCESS_CODE;
    if (argc >= 2)
    {
        ACCESS_CODE = argv[1];
    }

    CameraFactory factory;
    Camera cam = Camera(factory.createCamera());

    // [Setup] Camera Initialization Test
    Camera::CameraError error = cam.RunInitializeTests();
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    // [Setup] Camera Stream Tests
    error = cam.RunStreamTests();
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    // [Optional] Access Level Test
    if (!ACCESS_CODE.empty()) { error = cam.RunAccessLevelTests(2); }
    else { error = cam.RunAccessLevelTests(1); }
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    // [Optional] Use Case Tests
    error = cam.RunUseCaseTests();
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    // [Optional] Exposure Tests
    error = cam.RunExposureTests();
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    // [Optional] Processing Parameters Test
    error = cam.RunProcessingParametersTests();
    if (EXIT_ON_ERROR && error != Camera::CameraError::NONE) { return error; }

    return 0;
}
