/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

// This file contains platform tests that are not specific to the module that is used

#include "TestPlatform.hpp"

using namespace royale;
using namespace platform;

TEST_F (CameraDeviceFixture, Init)
{
    ASSERT_NE (camera, nullptr);
    ASSERT_EQ (camera->setCallbackData (royale::CallbackData::Raw), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->initialize(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceFixture, StartCaptureFailure)
{
    ASSERT_NE (camera, nullptr);

    ASSERT_EQ (camera->startCapture(), CameraStatus::DEVICE_NOT_INITIALIZED);
}

TEST_F (CameraDeviceFixture, Initialize)
{
    ASSERT_NE (camera, nullptr);

    // this sets up additional parameters and the processing pipeline
    auto status = camera->initialize();
    ASSERT_EQ (status, CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceFixture, SecondInitialize)
{
    initCamera();

    //2nd call was throwing an unhandled RuntimeError ("Lens offset cannot be changed twice")
    EXPECT_NO_THROW (EXPECT_NE (camera->initialize(), CameraStatus::SUCCESS));
}

TEST_F (CameraDeviceFixture, SetUseCase)
{
    initCamera();
    Vector<String> supportedUseCases;

    auto status = camera->getUseCases (supportedUseCases);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->setUseCase (supportedUseCases[supportedUseCases.size() - 1]), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceFixture, TestExposureOutOfRange)
{
    initCamera();

    Vector<String> supportedUseCases;
    Pair<uint32_t, uint32_t> exposureLimits;

    auto status = camera->getUseCases (supportedUseCases);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->setUseCase (supportedUseCases[0]), CameraStatus::SUCCESS);

    status = camera->getExposureLimits (exposureLimits);
    ASSERT_EQ (CameraStatus::SUCCESS, status);
    EXPECT_EQ (camera->setExposureTime (exposureLimits.second + 5), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED);
}

TEST_F (CameraDeviceFixture, TestLensParameters)
{
    initCamera();

    LensParameters params;
    camera->getLensParameters (params);

    using namespace std;
    cout << "px/py     " << params.principalPoint.first << " " << params.principalPoint.second << endl;
    cout << "cx/cy     " << params.focalLength.first << " " << params.focalLength.second << endl;
    cout << "tan coeff "
         << (params.distortionTangential.first)
         << " " << (params.distortionTangential.second) << endl;
    cout << "rad coeff "
         << params.distortionRadial.at (0)
         << " " << params.distortionRadial.at (1)
         << " " << params.distortionRadial.at (2) << endl;

    EXPECT_GT (params.principalPoint.first, 0.0f);
    EXPECT_GT (params.principalPoint.second, 0.0f);
    EXPECT_GT (params.focalLength.first, 0.0f);
    EXPECT_GT (params.focalLength.second, 0.0f);

    LensParameters params2;
    camera->getLensParameters (params2);

    LensParameters params3;
    camera->getLensParameters (params3);

    EXPECT_EQ (params.distortionRadial.size(), params2.distortionRadial.size());
    EXPECT_EQ (params.distortionRadial.size(), params3.distortionRadial.size());
}

TEST_F (CameraDeviceFixture, TestSweepExposure)
{
    const uint32_t MAX_RETRIES_PER_EXPOSURE_SET = 5;
    const uint32_t MS_TIME_TO_WAIT_BETWEEN_ATTEMPTS_FOR_ONE_FPS = 500;
    const uint32_t STEP_SIZE = 100;

    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    // sweep over all exposures which are valid for the connected camera module
    CameraStatus status;
    Pair<uint32_t, uint32_t> exposureLimits;

    status = camera->getExposureLimits (exposureLimits);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    uint32_t firstValue = exposureLimits.second;
    uint32_t secondValue = exposureLimits.first;

    if (firstValue > secondValue)
    {
        uint32_t tmp = firstValue;
        firstValue = secondValue;
        secondValue = tmp;
    }
    uint16_t frameRate = 1;
    status = camera->getFrameRate (frameRate);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    for (uint32_t i = firstValue; i <= secondValue; i += STEP_SIZE)
    {
        //attempt to setExposureTime in a loop, in case of DEVICE_IS_BUSY, retry with delay between attempts
        for (uint32_t currentTry = 0; currentTry < MAX_RETRIES_PER_EXPOSURE_SET; currentTry++)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (MS_TIME_TO_WAIT_BETWEEN_ATTEMPTS_FOR_ONE_FPS / frameRate));
            status = camera->setExposureTime (i);
            if (CameraStatus::DEVICE_IS_BUSY != status)
            {
                //stop the loop at SUCCESS or any other error than DEVICE_IS_BUSY
                break;
            }
        }
        ASSERT_EQ (status, CameraStatus::SUCCESS) << "Could not set exposure time to " << i << ".";
    }

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceFixture, TestReceiveData)
{
    initCamera();

    DepthDataListener depthListener;
    camera->registerDataListener (&depthListener);

    switchToFastUseCasePicoFlexx();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    ASSERT_GT (depthListener.m_count, 0);
}



TEST_F (CameraDeviceFixture, TestAutoExposure)
{
    initCamera();

    DepthDataListener depthListener;
    camera->registerDataListener (&depthListener);

    switchToFastUseCasePicoFlexx();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    CameraStatus status;
    Pair<uint32_t, uint32_t> exposureLimits;
    status = camera->getExposureLimits (exposureLimits);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    bool lowChanged = false;
    bool highChanged = false;

    // Check if exposure time is adapted if we start with the lowest exposure
    {
        ASSERT_EQ (camera->setExposureMode (ExposureMode::MANUAL), CameraStatus::SUCCESS);
        status = camera->setExposureTime (exposureLimits.first);
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        ASSERT_GT (depthListener.m_count, 0);
        auto curCount = depthListener.m_count;
        ASSERT_EQ (depthListener.m_expoTimes.at (1), exposureLimits.first);

        ASSERT_EQ (camera->setExposureMode (ExposureMode::AUTOMATIC), CameraStatus::SUCCESS);
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        ASSERT_GT (depthListener.m_count, curCount);
        if (depthListener.m_expoTimes.at (1) != exposureLimits.first)
        {
            lowChanged = true;
        }
    }

    depthListener.m_count = 0u;

    // Check if exposure time is adapted if we start with the highest exposure
    {
        ASSERT_EQ (camera->setExposureMode (ExposureMode::MANUAL), CameraStatus::SUCCESS);
        status = camera->setExposureTime (exposureLimits.second);
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        ASSERT_GT (depthListener.m_count, 0);
        auto curCount = depthListener.m_count;
        ASSERT_EQ (depthListener.m_expoTimes.at (1), exposureLimits.second);

        ASSERT_EQ (camera->setExposureMode (ExposureMode::AUTOMATIC), CameraStatus::SUCCESS);
        std::this_thread::sleep_for (std::chrono::milliseconds (500));
        ASSERT_GT (depthListener.m_count, curCount);
        if (depthListener.m_expoTimes.at (1) != exposureLimits.second)
        {
            highChanged = true;
        }
    }

    ASSERT_TRUE (lowChanged || highChanged);

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceFixture, TestTemperature)
{
    initCamera();

    ExtendedDataListener extendedListener;
    camera->registerDataListenerExtended (&extendedListener);

    switchToFastUseCasePicoFlexx();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    ASSERT_GT (extendedListener.m_count, 0);

    auto curTemperature = extendedListener.m_temperature;
    extendedListener.m_count = 0u;

    std::this_thread::sleep_for (std::chrono::milliseconds (300));
    ASSERT_GT (extendedListener.m_count, 0);

    // Check if the temperature increased
    ASSERT_GE (extendedListener.m_temperature, curTemperature);
}

TEST_F (CameraDeviceFixture, TestParameters)
{
    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    ProcessingParameterVector flags;
    camera->getProcessingParameters (flags);

    ProcessingParameterMap mappedFlags;

    // read out the flags and check if the values are correctly set
    for (size_t i = 0; i < flags.size(); i++)
    {
        if (flags[i].first == ProcessingFlag::UseRemoveFlyingPixel_Bool)
        {
            EXPECT_EQ (flags[i].second, true);
        }
    }

    mappedFlags[ProcessingFlag::UseRemoveFlyingPixel_Bool].setBool (false);

    ASSERT_EQ (camera->setProcessingParameters (ProcessingParameterVector::fromStdMap (mappedFlags)), CameraStatus::SUCCESS);

    EXPECT_EQ (mappedFlags[ProcessingFlag::UseRemoveFlyingPixel_Bool].getBool(), false);

    camera->getProcessingParameters (flags);
    for (size_t i = 0; i < flags.size(); i++)
    {
        if (flags[i].first == ProcessingFlag::UseRemoveFlyingPixel_Bool)
        {
            EXPECT_EQ (flags[i].second, false);
        }
    }
}

TEST_F (CameraDeviceFixture, TestCalibData)
{
    initCamera();

    royale::Vector<uint8_t> calibData;
    ASSERT_EQ (camera->getCalibrationData (calibData), CameraStatus::SUCCESS);

    ASSERT_GT (calibData.size(), 0u);
}
