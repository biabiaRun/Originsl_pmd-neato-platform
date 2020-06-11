#include <iostream>
#include "stdlib.h"

#include "camera.h"

using namespace royale;
using namespace platform;

Camera::CameraError Camera::RunInitializeTests(royale::String useCase, int fps)
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

    use_case_ = useCase;
    fps_ = fps;
    royale::Vector<royale::String> useCaseList;
    status = camera_->getUseCases (useCaseList);
    if (status != royale::CameraStatus::SUCCESS || useCaseList.empty())
    {
        std::cerr << "[ERROR] Could not get use cases. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return USE_CASE_ERROR;
    }

    for (auto i = 0u; i < useCaseList.size(); ++i) {
        std::cout << "USE CASE :" << useCaseList[i].c_str() << std::endl;
        if (useCaseList.at(i) == use_case_) {
            status = camera_->setUseCase(useCaseList.at(i));
            if (status != royale::CameraStatus::SUCCESS) {
                std::cerr << "[ERROR] Could not set a new use case. " << useCaseList[i].c_str() << "   " 
                      << royale::getStatusString(status).c_str() << std::endl;
                return USE_CASE_ERROR;
            }
        }
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

    // Get stream ID
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
        std::clog << "use case: " << next_use_case.c_str() << std::endl;
        status = camera_->setUseCase(next_use_case);
        if (status != royale::CameraStatus::SUCCESS)
        {
            std::cerr << "[ERROR] Could not set a new use case. " << next_use_case.c_str() << "   " 
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
    royale::CameraStatus status = camera_->registerDataListenerExtended(&rawListener_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not register the extended data listener" 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    status = camera_->setCallbackData (royale::CallbackData::Intermediate);
    if (status != royale::CameraStatus::SUCCESS) {
      std::cerr << "[ERROR] Could not set the callbackData" 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // Start capture mode
    status = camera_->startCapture();
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not start capturing" 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    std::this_thread::sleep_for (std::chrono::seconds (1));

    // Set Manual Exposure
    status = camera_->setExposureMode(royale::ExposureMode::MANUAL);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure mode to Manual. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    std::this_thread::sleep_for (std::chrono::seconds (1));

    // Get Exposure Limits
    royale::Pair<std::uint32_t, std::uint32_t> limits;
    status = camera_->getExposureLimits(limits, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not get exposure limits. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    std::this_thread::sleep_for (std::chrono::seconds (5));

    // Set Exposure Time (Manual ONLY!)
    // std::uint32_t rand_exposure = rand() % limits.second + limits.first;
    std::uint32_t man_exposure_low = limits.first;
    status = camera_->setExposureTime(man_exposure_low, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure to " << man_exposure_low << ". " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // Wait for 1 second before changing exposure
    std::this_thread::sleep_for (std::chrono::seconds (1));

    std::uint32_t man_exposure_high = limits.second;
    status = camera_->setExposureTime(man_exposure_high, stream_id_);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure to " << man_exposure_high << ". " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    // NOTE: You can only check if the exposure time is set correctly 
    //       via exposure member of data in onNewData

    // Wait for 1 second before changing exposure
    std::this_thread::sleep_for (std::chrono::seconds (1));

    // Set Auto Exposure
    status = camera_->setExposureMode(royale::ExposureMode::AUTOMATIC);
    if (status != royale::CameraStatus::SUCCESS)
    {
        std::cerr << "[ERROR] Could not set the exposure mode to Automatic. " 
                  << royale::getStatusString(status).c_str() << std::endl;
        return EXPOSURE_MODE_ERROR;
    }

    std::clog << "[SUCCESS] All exposure tests passed. " << std::endl;
    return NONE;
}

Camera::CameraError Camera::RunLensParametersTest() {
  LensParameters params;
  camera_->getLensParameters (params);

  if (params.principalPoint.first <= 0.0f) {
    std::cerr << "[ERROR] Principal point X is invalid" << std::endl;
    return LENS_PARAMETER_ERROR;
  }

  if (params.principalPoint.second <= 0.0f) {
    std::cerr << "[ERROR] Principal point Y is invalid" << std::endl;
    return LENS_PARAMETER_ERROR;
  }

  if (params.focalLength.first <= 0.0f) {
    std::cerr << "[ERROR] Focal length X is invalid" << std::endl;
    return LENS_PARAMETER_ERROR;
  }

  if (params.focalLength.second <= 0.0f) {
    std::cerr << "[ERROR] Focal length Y is invalid" << std::endl;
    return LENS_PARAMETER_ERROR;
  }

  std::clog << "[SUCCESS] All lens parameters tests passed. " << std::endl;
  return NONE;
}

Camera::CameraError Camera::RunTestReceiveData(int secondsToStream) {
  // Let the camera run for a second
  std::this_thread::sleep_for (std::chrono::seconds (1));

  if (rawListener_.m_count == 0) {
    std::cerr << "[ERROR] Not receiving new depth data" << std::endl;
    return RECEIVE_DATA_ERROR;
  }

  // Record to output file
  camera_->startRecording ("test.rrf");

  rawListener_.m_count = 0u;
  std::this_thread::sleep_for (std::chrono::seconds(secondsToStream));

  // Stop the recording
  camera_->stopRecording();

  std::clog << "Temperature Sensor Value" << std::endl;
  for (auto& tmp : rawListener_.m_cur_temp) {
    if (tmp <= 0) {
        std::cerr << "[ERROR] Temperature reading of " << tmp << " <= 0." << std::endl;
            return RECEIVE_DATA_ERROR;
    }
  }

  // Stop the capturing mode
  royale::CameraStatus status = camera_->stopCapture();
  if (status != royale::CameraStatus::SUCCESS) {
    std::cerr << "[ERROR] Could not stop the camera capture. " 
              << royale::getStatusString(status).c_str() << std::endl;
    return RECEIVE_DATA_ERROR;
  }
  float measuredFPS = static_cast<float>(rawListener_.m_count) / static_cast<float>(secondsToStream);
  float fps_lower_limit = static_cast<float>(fps_) - 0.5f;
  float fps_upper_limit = static_cast<float>(fps_) + 0.5f;

  if (measuredFPS < fps_lower_limit || measuredFPS > fps_upper_limit) {
    std::cerr << "[ERROR] FPS is outside of limits at " << measuredFPS << std::endl;
    return RECEIVE_DATA_ERROR;
  }
  std::clog << "[SUCCESS] All receive data tests passed. " << std::endl;
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
    const int GLOBAL_BINNING = 1;

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
