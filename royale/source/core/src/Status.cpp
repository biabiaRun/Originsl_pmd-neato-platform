/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/Status.hpp>

using namespace royale;

royale::String royale::getStatusString (CameraStatus status)
{
    switch (status)
    {
        case CameraStatus::SUCCESS:
            return "No error";

        case CameraStatus::RUNTIME_ERROR:
            return "A runtime error happened, something unexpected happened";
        case CameraStatus::DISCONNECTED:
            return "Camera device is disconnected";
        case CameraStatus::INVALID_VALUE:
            return "The provided value is invalid";
        case CameraStatus::TIMEOUT:
            return "The connection got a timeout";

        case CameraStatus::LOGIC_ERROR:
            return "A logic error happened, this does not make any sense here";
        case CameraStatus::NOT_IMPLEMENTED:
            return "This feature is not implemented yet";
        case CameraStatus::OUT_OF_BOUNDS:
            return "Setting/parameter is out of specified range";

        case CameraStatus::RESOURCE_ERROR:
            return "Cannot access resource";
        case CameraStatus::FILE_NOT_FOUND:
            return "Specified file was not found";
        case CameraStatus::COULD_NOT_OPEN:
            return "Cannot open resource";
        case CameraStatus::DATA_NOT_FOUND:
            return "The requested data could not be found";
        case CameraStatus::DEVICE_IS_BUSY:
            return "Another action is in progress";
        case CameraStatus::WRONG_DATA_FORMAT_FOUND:
            return "A resource was expected to be in one data format, but was in a different (recognisable) format";

        case CameraStatus::USECASE_NOT_SUPPORTED:
            return "This use case is not supported";
        case CameraStatus::FRAMERATE_NOT_SUPPORTED:
            return "The specified frame rate is not supported";
        case CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED:
            return "The exposure time is not supported";
        case CameraStatus::DEVICE_NOT_INITIALIZED:
            return "The device seems to be uninitialized";
        case CameraStatus::CALIBRATION_DATA_ERROR:
            return "The calibration data is not readable";
        case CameraStatus::INSUFFICIENT_PRIVILEGES:
            return "The camera access level does not allow to call this operation";
        case CameraStatus::DEVICE_ALREADY_INITIALIZED:
            return "The camera was already initialized";
        case CameraStatus::EXPOSURE_MODE_INVALID:
            return "The current set exposure mode does not support this operation";
        case CameraStatus::NO_CALIBRATION_DATA:
            return "The method cannot be called since no calibration data is available";
        case CameraStatus::INSUFFICIENT_BANDWIDTH:
            return "The interface to the camera module does not provide a sufficient bandwidth";
        case CameraStatus::DUTYCYCLE_NOT_SUPPORTED:
            return "The specified dutycycle is not supported";
        case CameraStatus::SPECTRE_NOT_INITIALIZED:
            return "Spectre was not initialized properly";
        case CameraStatus::NO_USE_CASES:
            return "The camera offers no use cases";
        case CameraStatus::NO_USE_CASES_FOR_LEVEL:
            return "The camera offers no use cases for the current access level";


        case CameraStatus::FSM_INVALID_TRANSITION:
            return "Camera module state machine does not support current transition";

        case CameraStatus::UNKNOWN:
            return "An unknown error occurred";
    }
    return "Status is not known";
}
