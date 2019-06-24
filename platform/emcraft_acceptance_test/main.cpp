#include <iostream>
#include <string>

#include "camera.h"

const bool EXIT_ON_ERROR = true;

int main(int argc, char **argv)
{
    // Setup the Camera Manager
    std::string ACCESS_CODE;
    if (argc >= 2)
    {
        ACCESS_CODE = argv[1];
    }
    royale::CameraManager manager(ACCESS_CODE);

    // [Setup] Make sure at least one camera is connected
    // Grab the *FIRST* camera found and create a CameraDevice
    auto camlist = manager.getConnectedCameraList();
    if(camlist.empty())
    {
        std::cerr << "[ERROR] No camera connected" << std::endl;
        return Camera::CameraError::CAM_NOT_DETECTED;
    }
    Camera cam = Camera(manager.createCamera(camlist.at(0)), camlist.at(0).c_str());
    
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