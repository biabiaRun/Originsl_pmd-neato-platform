#include <iostream>
#include "stdlib.h"

#include <royale/ICameraDevice.hpp>

#include <CameraFactory.hpp>

using namespace royale;
using namespace platform;
#include "camera.h"
void Camera::onNewData (const royale::DepthData *data)
{
    // Do something with the data
}

Camera::CameraError Camera::RunInitializeTests()
{
    // Test if CameraDevice was created
    if(camera_ == nullptr)
    {
        std::cerr << "[ERROR] Camera device could not be created." << std::endl;
        return CAM_NOT_CREATED;
    }
    // Test Initialize()
    royale::CameraStatus status = camera_->initialize();
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Camera device could not be initialized. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return CAM_NOT_INITIALIZED;
    }

    std::clog << "[SUCCESS] All initialize tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunStreamTests()
{
    // Get camera streams
    royale::Vector<royale::StreamId> streamids;
    royale::CameraStatus status = camera_->getStreams(streamids);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get the camera streams. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return CAM_STREAM_ERROR;
    }
    if (streamids.size() > 1) 
    {
        std::clog << "[WARNING] More than one camera stream exists. Using only the FIRST stream." << std::endl;
    }

    // Set stream ID
    try 
    {
        stream_id_ = streamids.front();
    }
    catch (const std::out_of_range& oor)
    {
        std::cerr << "[ERROR] No camera streams found. Caught Exception: " 
                  << oor.what() << std::endl;
        return CAM_STREAM_ERROR;
    }

    std::clog << "[SUCCESS] All camera stream tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunAccessLevelTests(int user_level)
{
    // Get the access level
    royale::CameraAccessLevel level;
    royale::CameraStatus status = camera_->getAccessLevel(level);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not grab the access level. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return ACCESS_LEVEL_ERROR;
    }

    // Check if access level was set properly
    access_level_ = static_cast<int>(level);
    if (access_level_ != user_level)
    {
        std::cerr << "[ERROR] Access level mismatch. " << access_level_
                  << "!=" << user_level << std::endl;
        return ACCESS_LEVEL_ERROR;
    }

    std::clog << "[SUCCESS] All access level tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunUseCaseTests()
{
    // Test retrieval of use cases / current use case
    royale::Vector<royale::String> use_cases;
    royale::CameraStatus status = camera_->getUseCases(use_cases);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get use cases. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return USE_CASE_ERROR;
    }
    royale::String current_use_case;
    status = camera_->getCurrentUseCase(current_use_case);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get the current use case. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return USE_CASE_ERROR;
    }

    // Test setting the use case and 
    // check to see if it was set to the right use case
    royale::String next_use_case = current_use_case;
    for (auto& uc : use_cases)
    {
        if (uc != current_use_case)
        {
            next_use_case = uc;
            break;
        }
    }
    if (next_use_case == current_use_case)
    {
        std::clog << "[WARNING] Could not find another use case to test setUseCase." << std::endl;
    }
    else
    {
        status = camera_->setUseCase(next_use_case);
        if (status != royale::CameraStatus::SUCCESS)
        {
            std::cerr << "[ERROR] Could not set a new use case. " 
                      << royale::getStatusString(status).c_str() << std::endl;
            return USE_CASE_ERROR;
        }
        else
        {
            camera_->getCurrentUseCase(current_use_case);
            if (status != royale::CameraStatus::SUCCESS)
            {
                std::cerr << "[ERROR] Could not get the current use case. " 
                          << royale::getStatusString(status).c_str() << std::endl;
                return USE_CASE_ERROR;
            }
            if (current_use_case != next_use_case)
            {
                std::cerr << "[ERROR] Current use case does not match the set use case. " 
                      << royale::getStatusString(status).c_str() << std::endl;
                return USE_CASE_ERROR;
            }
        }
    }
    use_case_ = current_use_case.c_str();
    std::clog << "[SUCCESS] All use case tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunExposureTests()
{
    // Set Auto Exposure
    royale::CameraStatus status = camera_->setExposureMode(royale::ExposureMode::AUTOMATIC);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure mode to Automatic. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }
    // Set Manual Exposure
    status = camera_->setExposureMode(royale::ExposureMode::MANUAL);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure mode to Manual. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // Get Exposure Limits
    royale::Pair<std::uint32_t, std::uint32_t> limits;
    status = camera_->getExposureLimits(limits, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get exposure limits. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // Set Exposure Time (Manual ONLY!)
    std::uint32_t rand_exposure = rand() % limits.second + limits.first;
    status = camera_->setExposureTime(rand_exposure, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could set the exposure to " << rand_exposure << ". " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // NOTE: You can only check if the exposure time is set correctly 
    //       via exposure member of data in onNewData

    std::clog << "[SUCCESS] All exposure tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunProcessingParametersTests()
{
    // Must be level 2.
    if (access_level_ < 2)
    {
        std::clog << "[WARNING] Ignoring ProcessingParametersTests()" 
                  << " - requires L2 access. " << std::endl;
        return NONE;
    }

    // Get Processing Parameters
    royale::ProcessingParameterVector ppvec;
    royale::CameraStatus status = camera_->getProcessingParameters(ppvec, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get processing parameters. "
                  << royale::getStatusString(status).c_str() << std::endl;
        return PROCESSING_PARAMETER_ERROR;
    }

    // Change/Set the Processing Parameters
    const bool FLYING_PIXEL = false;
    const bool STRAY_LIGHT = true;
    const int ADAPTIVE_NOISE_FILTER = 2;
    const float NOISE_THRESH = 0.14f;
    const int GLOBAL_BINNING = 2;

    for (auto& flag_pair : ppvec)
    {
        royale::Variant var;
        switch(flag_pair.first)
        {
            case royale::ProcessingFlag::UseRemoveFlyingPixel_Bool:
                var.setBool(FLYING_PIXEL);
                flag_pair.second = var;
                break;
            case royale::ProcessingFlag::UseRemoveStrayLight_Bool:
                var.setBool(STRAY_LIGHT);
                flag_pair.second = var;
                break;
            case royale::ProcessingFlag::AdaptiveNoiseFilterType_Int:
                var.setInt(ADAPTIVE_NOISE_FILTER);
                flag_pair.second = var;
                break;
            case royale::ProcessingFlag::NoiseThreshold_Float:
                var.setFloat(NOISE_THRESH);
                flag_pair.second = var;
                break;
            case royale::ProcessingFlag::GlobalBinning_Int:
                var.setInt(GLOBAL_BINNING);
                flag_pair.second = var;
                break;
            default:
                break;
        }
    }
    status = camera_->setProcessingParameters(ppvec, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the processing parameters. "
                  << royale::getStatusString(status).c_str() << std::endl;
        return PROCESSING_PARAMETER_ERROR;
    }

    // Check to see if the processing parameters were set correctly.
    status = camera_->getProcessingParameters(ppvec, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get processing parameters. "
                  << royale::getStatusString(status).c_str() << std::endl;
        return PROCESSING_PARAMETER_ERROR;
    }
    CameraError err = NONE;
    for (auto& flag_pair : ppvec)
    {
        switch(flag_pair.first)
        {
            case royale::ProcessingFlag::UseRemoveFlyingPixel_Bool:
                if (flag_pair.second.getBool() != FLYING_PIXEL)
                {
                    std::cerr << "[ERROR] Flying pixel processing parameter mismatch. " << std::endl;
                    err = PROCESSING_PARAMETER_ERROR;
                }
                break;
            case royale::ProcessingFlag::UseRemoveStrayLight_Bool:
                if (flag_pair.second.getBool() != STRAY_LIGHT)
                {
                    std::cerr << "[ERROR] Stray light processing parameter mismatch. " << std::endl;
                    err = PROCESSING_PARAMETER_ERROR;
                }
                break;
            case royale::ProcessingFlag::AdaptiveNoiseFilterType_Int:
                if (flag_pair.second.getInt() != ADAPTIVE_NOISE_FILTER)
                {
                    std::cerr << "[ERROR] Adaptive noise processing parameter mismatch. " << std::endl;
                    err = PROCESSING_PARAMETER_ERROR;
                }
                break;
            case royale::ProcessingFlag::NoiseThreshold_Float:
                if (flag_pair.second.getFloat() != NOISE_THRESH)
                {
                    std::cerr << "[ERROR] Noise threshold processing parameter mismatch. " << std::endl;
                    err = PROCESSING_PARAMETER_ERROR;
                }
                break;
            case royale::ProcessingFlag::GlobalBinning_Int:
                if (flag_pair.second.getInt() != GLOBAL_BINNING)
                {
                    std::cerr << "[ERROR] Global binning processing parameter mismatch. " << std::endl;
                    err = PROCESSING_PARAMETER_ERROR;
                }
                break;
            default:
                break;
        }
    }

    std::clog << "[SUCCESS] All processing parameters tests passed. " << std::endl;
    return err;
}
