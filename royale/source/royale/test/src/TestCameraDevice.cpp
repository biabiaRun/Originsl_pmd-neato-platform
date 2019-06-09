/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/CameraManager.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/Status.hpp>
#include <gmock/gmock.h>
#include <FileSystem.hpp>

#include <CameraDeviceL1Fixture.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;
using ::testing::SizeIs;
using ::testing::ContainerEq;

TEST_F (CameraDeviceL1Fixture, StartCaptureFailure)
{
    ASSERT_NE (camera, nullptr);

    EXPECT_EQ (camera->startCapture(), CameraStatus::DEVICE_NOT_INITIALIZED);
}

TEST_F (CameraDeviceL1Fixture, TestProperId)
{
    ASSERT_NE (camera, nullptr);

    String id;
    auto status = camera->getId (id);
    EXPECT_EQ (CameraStatus::SUCCESS, status);
    EXPECT_EQ (cameraId, id);
}

TEST_F (CameraDeviceL1Fixture, Initialize)
{
    ASSERT_NE (camera, nullptr);

    // this sets up additional parameters and the processing pipeline
    auto status = camera->initialize();
    ASSERT_EQ (status, CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, SecondInitialize)
{
    initCamera();

    //2nd call was throwing an unhandled RuntimeError ("Lens offset cannot be changed twice")
    EXPECT_NO_THROW (EXPECT_NE (camera->initialize(), CameraStatus::SUCCESS));
}

TEST_F (CameraDeviceL1Fixture, ChangeCaptureModesDuringCapture)
{
    initCamera();

    String cameraName;
    auto status = camera->getCameraName (cameraName);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    std::shared_ptr<DepthDataListener> listener{ new DepthDataListener() };
    camera->registerDataListener (listener.get());

    std::map<String, std::vector<String>> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_5FPS_2000", "MODE_9_25FPS_450" } },
        { "EVALBOARD_INFINEON_IRS10X0C_EVALUATION_KIT", { "MODE_5_10FPS_2900", "MODE_5_20FPS_2900", "MODE_5_30FPS_2900", "MODE_5_45FPS_1000" } },
        { "PICOMAXX2",    { "MODE_9_5FPS_1900", "MODE_9_25FPS_300", "MODE_5_35FPS_500", "MODE_MIXED_50_5" } },
        { "PICOMONSTAR2", { "MODE_9_5FPS_1900", "MODE_9_25FPS_300", "MODE_5_35FPS_500", "MODE_MIXED_50_5" } },
        { "Alea945nm", { "MODE_9_5FPS", "MODE_9_25FPS", "MODE_5_45FPS" } },
        { "Salome940nm", { "MODE_9_5FPS", "MODE_9_25FPS", "MODE_5_45FPS" } },
    };

    auto td_it = testDataMap.find (cameraName);
    if (td_it == testDataMap.end())
    {
        // Unknown camera, no testcases known
        return;
    }

    const auto &testData = td_it->second;
    ASSERT_FALSE (testData.empty());
    auto uc_it = testData.begin();
    const auto uc_end = testData.end();

    ASSERT_EQ (camera->setUseCase (*uc_it++), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (2)));
    while (uc_it != uc_end)
    {
        ASSERT_EQ (camera->setUseCase (*uc_it++), CameraStatus::SUCCESS);
        EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (2)));
    }
    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    camera->unregisterDataListener();
}

TEST_F (CameraDeviceL1Fixture, ValidateUseCases)
{
    if (!validateUseCases)
    {
        std::cout << "Use case validation not possible or not feasible" << std::endl;
        SUCCEED();
        return;
    }
    initCamera();

    Vector<String> supportedUseCases;

    auto status = camera->getUseCases (supportedUseCases);
    ASSERT_EQ (status, CameraStatus::SUCCESS);
    ASSERT_EQ (expectedUseCases.size(), supportedUseCases.size());
    ASSERT_THAT (supportedUseCases, SizeIs (expectedUseCases.size()));
    ASSERT_THAT (supportedUseCases, ContainerEq (expectedUseCases));
}

TEST_F (CameraDeviceL1Fixture, SetUseCase)
{
    initCamera();
    Vector<String> supportedUseCases;

    auto status = camera->getUseCases (supportedUseCases);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->setUseCase (supportedUseCases[0]), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestExposureOutOfRange)
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

TEST_F (CameraDeviceL1Fixture, TestLensParameters)
{
#ifndef USE_SPECTRE
    // Simple processing doesn't support calibration data
    return;
#endif

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

TEST_F (CameraDeviceL1Fixture, TestSweepExposure)
{
    const uint32_t MAX_RETRIES_PER_EXPOSURE_SET = 5;
    const uint32_t MS_TIME_TO_WAIT_BETWEEN_ATTEMPTS_FOR_ONE_FPS = 500;
    const uint32_t STEP_SIZE = 10;

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
            // Replacing this sleep_for with a MonitorableListener->waitForCallback doesn't change
            // the speed of this test.  The call to setExposureTime will succeed after Royale has
            // received a frame with pseudodata confirming that the imager has acted on the previous
            // setExposureTime, giving a theoretical limit (assuming 200 steps and a 5FPS use case)
            // of around 40 seconds.
            //
            // With a 5FPS use case, MS_TIME_TO_WAIT_BETWEEN_ATTEMPTS_FOR_ONE_FPS is lower than the time between
            // frames, and the call to setExposureTime() is pushed to the imager early enough for it
            // to be acknowledged in the first possible frame.
            //
            // Changing the use case to a faster one would speed the test up.
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

TEST_F (CameraDeviceL1Fixture, TestAutoExposure)
{
    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->setExposureMode (ExposureMode::MANUAL), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->setExposureMode (ExposureMode::AUTOMATIC), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestIsCalibrated)
{
    initCamera();
    bool isCalibrated;
    auto status = camera->isCalibrated (isCalibrated);

    ASSERT_EQ (CameraStatus::SUCCESS, status);
    ASSERT_TRUE (isCalibrated);
}

TEST_F (CameraDeviceL1Fixture, TestIRImageCallback)
{
    initCamera();

    std::shared_ptr<IRImageListener> listener{ new IRImageListener() };
    camera->registerIRImageListener ( (IIRImageListener *) listener.get());

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_GT (listener->m_count, 0);
}

TEST_F (CameraDeviceL1Fixture, TestSparsePointCloudCallback)
{
    initCamera();

    std::shared_ptr<SparsePointCloudListener> listener{ new SparsePointCloudListener() };
    camera->registerSparsePointCloudListener ( (ISparsePointCloudListener *) listener.get());

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_GT (listener->m_count, 0);
}

TEST_F (CameraDeviceL1Fixture, TestDepthImageCallback)
{
    initCamera();

    std::shared_ptr<DepthImageListener> listener{ new DepthImageListener() };
    camera->registerDepthImageListener ( (IDepthImageListener *) listener.get());

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->hasBeenCalled (std::chrono::seconds (1)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_GT (listener->m_count, 0);
}

TEST_F (CameraDeviceL1Fixture, TestSetFramerateFail)
{
    initCamera();
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->setFrameRate (0u), CameraStatus::FRAMERATE_NOT_SUPPORTED);
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestSetFramerateSuccess)
{
    if (cameraNameIs ("Salome940nm"))
    {
        // \todo : ROYAL-3513 This test should be skipped for all M2453_A11 devices, not just Salome940nm
        std::cout << "Changing the frame rate currently not possible for M2453_A11" << std::endl;
        SUCCEED();
        return;
    }

    initCamera();
    uint16_t maxFr = 0;
    ASSERT_EQ (camera->getMaxFrameRate (maxFr), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    for (uint16_t i = 2u; i < maxFr; ++i)
    {
        ASSERT_EQ (camera->setFrameRate (i), CameraStatus::SUCCESS) << "framerate was " << i << ".";
        std::this_thread::sleep_for (std::chrono::milliseconds (1000u));
    }
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestSetFrameRateMixedMode)
{
    if (cameraNameIs ("Salome940nm"))
    {
        // \todo : ROYAL-3513 This test should be skipped for all M2453_A11 devices, not just Salome940nm
        std::cout << "Changing the frame rate currently not possible for M2453_A11" << std::endl;
        SUCCEED();
        return;
    }

    initCamera();
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    royale::Vector<royale::String> useCases;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getUseCases (useCases));
    for (auto useCase : useCases)
    {
        ASSERT_EQ (camera->setUseCase (useCase), CameraStatus::SUCCESS);

        royale::Vector<royale::StreamId> streams;
        camera->getStreams (streams);
        ASSERT_GE (streams.count(), 1u);
        bool isMixedMode = (streams.count() > 1u);

        if (isMixedMode)
        {
            // For mixed mode usecases, setting the frame rate is not supported.
            EXPECT_EQ (camera->setFrameRate (1), CameraStatus::NOT_IMPLEMENTED);
        }
        else
        {
            uint16_t maxFrameRate;
            ASSERT_EQ (camera->getMaxFrameRate (maxFrameRate), CameraStatus::SUCCESS);
            EXPECT_EQ (camera->setFrameRate (maxFrameRate), CameraStatus::SUCCESS);
        }
    }
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestGetStreams)
{
    initCamera();
    royale::Vector<royale::String> useCases;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getUseCases (useCases));
    for (auto useCase : useCases)
    {
        ASSERT_EQ (camera->setUseCase (useCase), CameraStatus::SUCCESS);

        royale::Vector<royale::StreamId> streams;
        ASSERT_EQ (camera->getStreams (streams), CameraStatus::SUCCESS);
        ASSERT_GE (streams.count(), 1u);
        std::set<royale::StreamId> streamsAsASet;
        for (auto stream : streams)
        {
            EXPECT_NE (stream, 0u);
            streamsAsASet.insert (stream);
        }
        auto nUniqueStreamIds = streamsAsASet.size();
        EXPECT_EQ (streams.count(), nUniqueStreamIds); // check stream ids are unique
    }
}

TEST_F (CameraDeviceL1Fixture, TestAutoExposureMixedMode)
{
    initCamera();

    if (!setMixedModeUseCase())
    {
        std::cout << "No mixed-mode usecase available" << std::endl;
        return;
    }

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    String cameraName;
    auto status = camera->getCameraName (cameraName);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    royale::Vector<royale::StreamId> streams;
    ASSERT_EQ (camera->getStreams (streams), CameraStatus::SUCCESS);
    ASSERT_GT (streams.count(), 1u);

    for (auto stream : streams)
    {
        ExposureMode mode;
        ASSERT_EQ (camera->getExposureMode (mode, stream), CameraStatus::SUCCESS);
        ASSERT_EQ (mode, ExposureMode::MANUAL); // Default setting should be "manual"

        ASSERT_EQ (camera->setExposureMode (ExposureMode::AUTOMATIC, stream), CameraStatus::SUCCESS);

        for (auto other_stream : streams)
        {
            ExposureMode other_mode;
            ASSERT_EQ (camera->getExposureMode (other_mode, other_stream), CameraStatus::SUCCESS);

            if (stream == other_stream)
            {
                ASSERT_EQ (other_mode, ExposureMode::AUTOMATIC);
            }
            else
            {
                ASSERT_EQ (other_mode, ExposureMode::MANUAL);
            }
        }

        ASSERT_EQ (camera->setExposureMode (ExposureMode::MANUAL, stream), CameraStatus::SUCCESS);
    }

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

}

TEST_F (CameraDeviceL1Fixture, TestManualExposureMixedMode)
{
    initCamera();

    if (!setMixedModeUseCase())
    {
        std::cout << "No mixed-mode usecase available" << std::endl;
        return;
    }

    const uint32_t TIME_FOR_SETTING_EXPOSURE_IN_MS = 500;


    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    royale::Vector<royale::StreamId> streams;
    ASSERT_EQ (camera->getStreams (streams), CameraStatus::SUCCESS);
    ASSERT_GT (streams.count(), 1u);

    auto firstStream = streams.front();
    auto lastStream = streams.back();
    ASSERT_NE (firstStream, lastStream);

    royale::Pair<uint32_t, uint32_t> firstLimits, lastLimits;
    ASSERT_EQ (camera->getExposureLimits (firstLimits, firstStream), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->getExposureLimits (lastLimits, lastStream), CameraStatus::SUCCESS);

    // Sanity check
    ASSERT_LT (firstLimits.first, firstLimits.second);
    ASSERT_LT (lastLimits.first, lastLimits.second);

    // Set exposures
    ASSERT_EQ (camera->setExposureTime (firstLimits.first, firstStream), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (TIME_FOR_SETTING_EXPOSURE_IN_MS));
    ASSERT_EQ (camera->setExposureTime (lastLimits.first, lastStream), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (TIME_FOR_SETTING_EXPOSURE_IN_MS));

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL1Fixture, TestMixedModeCallback)
{
    initCamera();

    if (!setMixedModeUseCase())
    {
        std::cout << "No mixed-mode usecase available" << std::endl;
        return;
    }

    std::shared_ptr<DepthDataListener> listener{ new DepthDataListener() };
    camera->registerDataListener ( (IDepthDataListener *) listener.get());

    royale::Vector<royale::StreamId> streams;
    ASSERT_EQ (camera->getStreams (streams), CameraStatus::SUCCESS);
    ASSERT_EQ (streams.count(), 2u);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (10, std::chrono::milliseconds (1500)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_GT (listener->m_count, 0);

    EXPECT_EQ (listener->m_streamIds.size (), streams.size());

    for (auto curStream : streams)
    {
        EXPECT_NE (listener->m_streamIds.find (curStream), listener->m_streamIds.end());
    }
}

TEST_F (CameraDeviceL1Fixture, TestGetNumberOfStreams)
{
    initCamera();

    royale::Vector<royale::String> useCases;
    ASSERT_EQ (camera->getUseCases (useCases), CameraStatus::SUCCESS);

    // Try to check number of streams for invalid use case
    uint32_t nr;
    ASSERT_EQ (camera->getNumberOfStreams ("bla", nr), CameraStatus::INVALID_VALUE);

    for (auto i = 0u; i < useCases.size(); ++i)
    {
        // Check number of streams for all available use cases
        ASSERT_EQ (camera->setUseCase (useCases.at (i)), CameraStatus::SUCCESS) << "Could not set use case " << useCases.at (i) << ".";
        royale::Vector<royale::StreamId> streams;
        ASSERT_EQ (camera->getStreams (streams), CameraStatus::SUCCESS);

        uint32_t nr1;
        ASSERT_EQ (camera->getNumberOfStreams (useCases.at (i), nr1), CameraStatus::SUCCESS);

        EXPECT_EQ (nr1, static_cast<uint32_t> (streams.size()));
    }
}

TEST_F (CameraDeviceL1Fixture, TestExternalTriggerEnabledButNotApplied)
{
    // \todo ROYAL-2461 add the Maxx and Monstar to this list
    if (cameraNameIs ("EVALBOARD_INFINEON_IRS10X0C_EVALUATION_KIT"))
    {
        ASSERT_EQ (camera->setExternalTrigger (true), CameraStatus::SUCCESS);

        initCamera();

        ASSERT_EQ (camera->setExternalTrigger (true), CameraStatus::DEVICE_ALREADY_INITIALIZED);

        std::shared_ptr<DepthImageListener> listener{ new DepthImageListener() };
        camera->registerDepthImageListener ( (IDepthImageListener *) listener.get());

        ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
        EXPECT_FALSE (listener->waitForCallback (std::chrono::seconds (2)));
        EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

        EXPECT_EQ (listener->m_count, 0);
    }
    else
    {
        ASSERT_EQ (camera->setExternalTrigger (true), CameraStatus::LOGIC_ERROR);
    }
}

TEST_F (CameraDeviceL1Fixture, TestSetExposureTimeBeforeStartCapture)
{
    initCamera();

    royale::Vector<royale::String> useCases;
    ASSERT_EQ (camera->getUseCases (useCases), CameraStatus::SUCCESS);

    bool nonMixedModeFound = false;
    for (auto i = 0u; i < useCases.size(); ++i)
    {
        // set a non mixed mode use case
        uint32_t nrStreams;
        ASSERT_EQ (camera->getNumberOfStreams (useCases.at (i), nrStreams), CameraStatus::SUCCESS);
        if (nrStreams == 1)
        {
            ASSERT_EQ (camera->setUseCase (useCases.at (i)), CameraStatus::SUCCESS);
            nonMixedModeFound = true;
            break;
        }
    }

    if (!nonMixedModeFound)
    {
        return;
    }

    royale::Pair<uint32_t, uint32_t> limits;
    ASSERT_EQ (camera->getExposureLimits (limits), CameraStatus::SUCCESS);

    uint32_t newTime = limits.first + (limits.second - limits.first) / 2;

    ASSERT_EQ (camera->setExposureTime (newTime), CameraStatus::SUCCESS);

    std::shared_ptr<DepthDataListener> listener{ new DepthDataListener() };
    camera->registerDataListener ( (IDepthDataListener *) listener.get());

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallback (std::chrono::seconds (2)));
    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    bool expoFound = false;
    for (auto curExpo : listener->m_expoTimes)
    {
        if (curExpo == newTime)
        {
            expoFound = true;
        }
    }

    EXPECT_TRUE (expoFound);
}

TEST_F (CameraDeviceL1Fixture, TestFilterLevelL1)
{
    initCamera();

    {
        EXPECT_EQ (camera->setFilterLevel (FilterLevel::Full), CameraStatus::SUCCESS);

        FilterLevel curFilterLevel;
        EXPECT_EQ (camera->getFilterLevel (curFilterLevel), CameraStatus::SUCCESS);
        EXPECT_EQ (curFilterLevel, FilterLevel::Full);
    }

    {
        EXPECT_EQ (camera->setFilterLevel (FilterLevel::Off), CameraStatus::SUCCESS);

        FilterLevel curFilterLevel;
        EXPECT_EQ (camera->getFilterLevel (curFilterLevel), CameraStatus::SUCCESS);
        EXPECT_EQ (curFilterLevel, FilterLevel::Off);
    }

    {
        EXPECT_EQ (camera->setFilterLevel (FilterLevel::Custom), CameraStatus::INVALID_VALUE);
    }
}
