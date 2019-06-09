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
#include <gtest/gtest.h>
#include <FileSystem.hpp>

#include <CameraDeviceL2Fixture.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;
using namespace royale::common;

TEST_F (CameraDeviceL2Fixture, TestSetParameters)
{
    // this test should test specifically boards which does not have calibration data
    initCamera();

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

    auto status = camera->setProcessingParameters (ProcessingParameterVector::fromStdMap (mappedFlags));
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    EXPECT_EQ (mappedFlags[ProcessingFlag::UseRemoveFlyingPixel_Bool].getBool(), false);
}

TEST_F (CameraDeviceL2Fixture, TestRawCallbackData)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    auto status = camera->registerDataListenerExtended (listener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    status = camera->setCallbackData (royale::CallbackData::Raw);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (2, std::chrono::seconds (3)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestDepthCallbackData)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    auto status = camera->registerDataListenerExtended (listener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    status = camera->setCallbackData (royale::CallbackData::Depth);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (2, std::chrono::seconds (3)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestIntermediateCallbackData)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    auto status = camera->registerDataListenerExtended (listener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    status = camera->setCallbackData (royale::CallbackData::Intermediate);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (2, std::chrono::seconds (3)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestGetCalibrationData)
{
    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    Vector<uint8_t> calibData;
    ASSERT_NO_THROW (ASSERT_EQ (CameraStatus::SUCCESS, camera->getCalibrationData (calibData)));

    ASSERT_NE (calibData.size(), 0u);

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestSetCalibrationData)
{
    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::seconds (1));

    Vector<uint8_t> calibData;
    ASSERT_NO_THROW (ASSERT_EQ (CameraStatus::SUCCESS, camera->getCalibrationData (calibData)));

    ASSERT_NE (calibData.size(), 0u);

    ASSERT_EQ (camera->setCalibrationData (calibData), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestSetCalibrationDataFromFile)
{
    initCamera();

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::seconds (1));

    Vector<uint8_t> calibData;
    ASSERT_NO_THROW (ASSERT_EQ (CameraStatus::SUCCESS, camera->getCalibrationData (calibData)));

    ASSERT_NE (calibData.size(), 0u);

    ASSERT_EQ (writeVectorToFile ("testCalib.cal", calibData), calibData.size());

    ASSERT_EQ (camera->setCalibrationData ("testCalib.cal"), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    remove ("testCalib.cal");
}

/**
 * This tests that the number of available modes in L2 is the number expected, and not one or two
 * extra.  The number of use cases depsend on the level, for example the UseCaseCalibration is only
 * available in L3/L4.
 */
TEST_F (CameraDeviceL2Fixture, TestUseCasesCallbackData)
{
    initCamera();

    String id;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (id));

    struct TestData
    {
        size_t rawUseCaseCount;
        size_t depthUseCaseCount;
        size_t intermediateUseCaseCount;
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { 10u, 10u, 10u } },
        { "PICOMAXX2", { 9u, 9u, 9u } },
        { "PICOMONSTAR2", { 9u, 9u, 9u } },
        { "Alea945nm", { 13u, 13u, 13u } },
        { "Salome940nm", { 14u, 14u, 14u } },
    };

    auto td_it = testDataMap.find (id);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &expected = td_it->second;

    {
        ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (CallbackData::Raw));

        Vector<String> useCases;
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getUseCases (useCases));
        ASSERT_EQ (useCases.size(), expected.rawUseCaseCount);
    }

    {
        ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (CallbackData::Depth));

        Vector<String> useCases;
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getUseCases (useCases));
        ASSERT_EQ (useCases.size(), expected.depthUseCaseCount);
    }

    {
        ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (CallbackData::Intermediate));

        Vector<String> useCases;
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getUseCases (useCases));
        ASSERT_EQ (useCases.size(), expected.intermediateUseCaseCount);
    }
}

TEST_F (CameraDeviceL2Fixture, TestSetExposureTime)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    ASSERT_EQ (CameraStatus::SUCCESS, camera->registerDataListenerExtended (listener.get()));

    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (royale::CallbackData::Intermediate));

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    uint32_t secondExposureTime;
    if (name == "PICOFLEXX")
    {
        // For the pico flexx, we created the test with a specific use case and
        // exposure time, so test using the old value.
        EXPECT_EQ (camera->setUseCase ("MODE_9_10FPS_1000"), CameraStatus::SUCCESS);
        secondExposureTime = 321;
    }
    else
    {
        // Run with whichever use case is the default on this device
        Pair<uint32_t, uint32_t> exposureLimits;
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getExposureLimits (exposureLimits));
        secondExposureTime = (exposureLimits.first + exposureLimits.second) / 2;
    }
    uint16_t framerate = 1;
    ASSERT_EQ (camera->getFrameRate (framerate), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (framerate, std::chrono::seconds (3)));
    EXPECT_EQ (camera->setExposureTime (secondExposureTime), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (framerate, std::chrono::seconds (3)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_GT (listener->depthDataExposureTimes.size(), 0u);

    EXPECT_EQ (listener->depthDataExposureTimes.size(), listener->rawDataExposureTimes.size());
    EXPECT_EQ (listener->depthDataExposureTimes.size(), listener->intermediateDataExposureTimes.size());

    for (size_t i = 0; i < listener->depthDataExposureTimes.size(); ++i)
    {
        EXPECT_EQ (listener->depthDataExposureTimes.at (i), listener->rawDataExposureTimes.at (i));
        EXPECT_EQ (listener->depthDataExposureTimes.at (i), listener->intermediateDataExposureTimes.at (i));
    }
}

TEST_F (CameraDeviceL2Fixture, TestSetExposureTimes)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener>();
    ASSERT_EQ (CameraStatus::SUCCESS, camera->registerDataListenerExtended (listener.get()));

    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (royale::CallbackData::Raw));

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        Vector<uint32_t> goodExposureTimes;
        Vector<uint32_t> badExposureTimes; // out of limits
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_10FPS_1000", { 500u, 500u, 500u }, { 500u, 500u, 1100u } } },
        { "PICOMAXX2", { "MODE_9_10FPS_900",  { 200u, 500u, 500u }, { 200u, 950u, 950u } } },
        { "PICOMONSTAR2", { "MODE_9_10FPS_900",  { 200u, 500u, 500u }, { 200u, 950u, 950u } } },
        { "Alea945nm", { "MODE_9_10FPS", { 100u, 400u, 400u }, { 900u, 900u, 900u } } },
        { "Salome940nm", { "MODE_9_10FPS", {100u, 400u, 400u }, {800u, 800u, 800u} } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    // first try an exposure time list which is too short.
    royale::Vector<uint32_t> expoTimes = testData.goodExposureTimes;
    ASSERT_FALSE (expoTimes.empty());
    expoTimes.pop_back();
    EXPECT_EQ (camera->setExposureTimes (expoTimes), CameraStatus::OUT_OF_BOUNDS);
    // and now an exposure time list which is too long.
    expoTimes = testData.goodExposureTimes;
    expoTimes.push_back (expoTimes.at (0));
    EXPECT_EQ (camera->setExposureTimes (expoTimes), CameraStatus::SUCCESS); // extra values are ignored
    EXPECT_TRUE (listener->waitForCallbacks (5, std::chrono::seconds (3)));

    // good case:
    EXPECT_EQ (camera->setExposureTimes (testData.goodExposureTimes), CameraStatus::SUCCESS);

    // error case:
    EXPECT_EQ (camera->setExposureTimes (testData.badExposureTimes), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED);

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestSetExposureForGroups)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener>();
    ASSERT_EQ (CameraStatus::SUCCESS, camera->registerDataListenerExtended (listener.get()));

    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (royale::CallbackData::Raw));

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        Vector<uint32_t> goodExposureTimes;
        Vector<uint32_t> badExposureTimes; // out of limits
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_10FPS_1000", { 500u, 500u, 500u }, { 500u, 500u, 1100u } } },
        { "PICOMAXX2", { "MODE_9_10FPS_900", { 200u, 500u, 500u }, { 200u, 950u, 950u } } },
        { "PICOMONSTAR2", { "MODE_9_10FPS_900", { 200u, 500u, 500u }, { 200u, 950u, 950u } } },
        { "Alea945nm", { "MODE_9_10FPS", { 100u, 400u, 400u }, { 900u, 900u, 900u } } },
        { "Salome940nm", { "MODE_9_10FPS", { 100u, 400u, 400u }, { 800u, 800u, 800u } } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    // first try an exposure time list which is too short.
    royale::Vector<uint32_t> expoTimes = testData.goodExposureTimes;
    ASSERT_FALSE (expoTimes.empty());
    expoTimes.pop_back();
    EXPECT_EQ (camera->setExposureTimes (expoTimes), CameraStatus::OUT_OF_BOUNDS);
    EXPECT_EQ (camera->setExposureForGroups (expoTimes), CameraStatus::OUT_OF_BOUNDS);
    // and now an exposure time list which is too long.
    expoTimes = testData.goodExposureTimes;
    expoTimes.push_back (expoTimes.at (0));
    EXPECT_EQ (camera->setExposureForGroups (expoTimes), CameraStatus::SUCCESS); // extra values are ignored
    EXPECT_TRUE (listener->waitForCallbacks (5, std::chrono::seconds (3)));

    // good case:
    EXPECT_EQ (camera->setExposureForGroups (testData.goodExposureTimes), CameraStatus::SUCCESS);

    // error case:
    EXPECT_EQ (camera->setExposureForGroups (testData.badExposureTimes), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED);
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL2Fixture, TestGetExposureGroups)
{
    initCamera();

    royale::Vector< royale::String > exposureGroups;

    ASSERT_EQ (camera->getExposureGroups (exposureGroups), CameraStatus::SUCCESS);
    ASSERT_GT (exposureGroups.count(), 0u);

    // Check for uniqueness
    std::set<std::string> exposureGroupSet;
    for (auto &group : exposureGroups)
    {
        auto groupAsStdString = group.toStdString();
        exposureGroupSet.emplace (groupAsStdString);
    }
    EXPECT_EQ (exposureGroups.count(), exposureGroupSet.size());
}

TEST_F (CameraDeviceL2Fixture, TestSetExposureByGroup)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    ASSERT_EQ (CameraStatus::SUCCESS, camera->registerDataListenerExtended (listener.get()));

    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (royale::CallbackData::Intermediate));

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        Vector<uint32_t> goodExposureTimes;
        Vector<uint32_t> badExposureTimes; // out of limits
    };

    /*
     * The usecases are expected to have 3 rawframe sets, each having its own exposure group.
     * These are named gray, mod1 and mod2 and should be in the same order as the rawframe sets.
     * Usecases created with the UseCaseEightPhase constructor should be suitable here.
     */
    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_10FPS_1000", { 123u, 234u, 345u }, { 1100u, 1100u, 1100u } } },
        { "PICOMAXX2", { "MODE_9_10FPS_900",  { 123u, 234u, 345u }, { 950u, 950u, 950u } } },
        { "PICOMONSTAR2", { "MODE_9_10FPS_900",  { 123u, 234u, 345u }, { 950u, 950u, 950u } } },
        { "Alea945nm", { "MODE_9_10FPS", { 100u, 400u, 400u }, { 900u, 900u, 900u } } },
        { "Salome940nm", { "MODE_9_10FPS", { 100u, 400u, 400u }, { 800u, 800u, 800u } } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);

    const auto gray_good = testData.goodExposureTimes.at (0);
    const auto mod1_good = testData.goodExposureTimes.at (1);
    const auto mod2_good = testData.goodExposureTimes.at (2);
    const auto gray_bad = testData.badExposureTimes.at (0);
    const auto mod1_bad = testData.badExposureTimes.at (1);
    const auto mod2_bad = testData.badExposureTimes.at (2);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->setExposureTime ("mod1", mod1_bad), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED); // out of limits
    EXPECT_EQ (camera->setExposureTime ("mod1", mod1_good), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));
    EXPECT_EQ (camera->setExposureTime ("mod2", mod2_bad), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED); // out of limits
    EXPECT_EQ (camera->setExposureTime ("mod2", mod2_good), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));
    EXPECT_EQ (camera->setExposureTime ("gray", gray_bad), CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED); // out of limits
    EXPECT_EQ (camera->setExposureTime ("gray", gray_good), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));
    EXPECT_EQ (camera->setExposureTime ("green", 555), CameraStatus::INVALID_VALUE); // exposure group doesn't exist

    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (4)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (listener->depthDataExposureTimes.size(), 3u);

    EXPECT_EQ (listener->depthDataExposureTimes.size(), listener->rawDataExposureTimes.size());
    EXPECT_EQ (listener->depthDataExposureTimes.size(), listener->intermediateDataExposureTimes.size());

    for (size_t i = 0; i < listener->depthDataExposureTimes.size(); ++i)
    {
        EXPECT_EQ (listener->depthDataExposureTimes.at (i), listener->rawDataExposureTimes.at (i));
        EXPECT_EQ (listener->depthDataExposureTimes.at (i), listener->intermediateDataExposureTimes.at (i));
    }
    EXPECT_EQ (gray_good, listener->depthDataExposureTimes.at (0));
    EXPECT_EQ (mod1_good, listener->depthDataExposureTimes.at (1));
    EXPECT_EQ (mod2_good, listener->depthDataExposureTimes.at (2));

}

TEST_F (CameraDeviceL2Fixture, TestSetExposureByStream)
{
    initCamera();

    auto listener = std::make_shared<ExtendedDataListener> ();
    ASSERT_EQ (CameraStatus::SUCCESS, camera->registerDataListenerExtended (listener.get()));

    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (royale::CallbackData::Intermediate));

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        Vector<uint32_t> initExposureTimes;
        uint32_t         newModExposureTime;
    };

    /*
     * The test assumes 3 rawframe sets, gray first.
     * Usecases created with the UseCaseEightPhase constructor should be suitable here.
     */
    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_10FPS_1000", { 201u, 202u, 203u }, 123u } },
        { "PICOMAXX2", { "MODE_9_10FPS_900", { 201u, 202u, 203u }, 123u } },
        { "PICOMONSTAR2", { "MODE_9_10FPS_900", { 201u, 202u, 203u }, 123u } },
        { "Alea945nm", { "MODE_9_10FPS", { 201u, 202u, 203u }, 123u } },
        { "Salome940nm", { "MODE_9_10FPS", { 201u, 202u, 203u }, 123u } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);

    /*
     * This usecase has 3 rawframe sets; the gray exposure is first.
     * It has 3 corresponding exposure groups (named gray, mod1, mod2)
     * in the same order as the rawframe sets.
     */

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    // need to set exposure times first (they may have been changed by other tests)
    EXPECT_EQ (camera->setExposureTimes (testData.initExposureTimes), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));

    // now for the actual test:
    EXPECT_EQ (camera->setExposureTime (testData.newModExposureTime), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (4)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (listener->depthDataExposureTimes.size(), 3u);
    EXPECT_EQ (testData.initExposureTimes.at (0), listener->depthDataExposureTimes.at (0)); // gray (unchanged)
    EXPECT_EQ (testData.newModExposureTime, listener->depthDataExposureTimes.at (1)); // mod1
    EXPECT_EQ (testData.newModExposureTime, listener->depthDataExposureTimes.at (2)); // mod2
}

TEST_F (CameraDeviceL2Fixture, TestSetCallbackDataBeforeInit)
{
    EXPECT_EQ (camera->setCallbackData (CallbackData::Raw), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->setCallbackData (CallbackData::Depth), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->setCallbackData (CallbackData::Intermediate), CameraStatus::SUCCESS);

    initCamera();
}

TEST_F (CameraDeviceL2Fixture, TestFilterLevelL2)
{
    // Although setFilterLevel is a Level 1 function we need Level 2 functionality
    // to check if it really does what it is supposed to do

    initCamera();

    {
        EXPECT_EQ (camera->setFilterLevel (FilterLevel::Full), CameraStatus::SUCCESS);

        FilterLevel curFilterLevel;
        EXPECT_EQ (camera->getFilterLevel (curFilterLevel), CameraStatus::SUCCESS);
        EXPECT_EQ (curFilterLevel, FilterLevel::Full);

        ProcessingParameterVector flags;
        EXPECT_EQ (camera->getProcessingParameters (flags), CameraStatus::SUCCESS);

        auto stdflags = ProcessingParameterVector::toStdMap (flags);
        auto it = stdflags.find (ProcessingFlag::UseAdaptiveNoiseFilter_Bool);
        EXPECT_NE (it, stdflags.end());

        EXPECT_EQ (it->second.getBool(), true);
    }

    {
        EXPECT_EQ (camera->setFilterLevel (FilterLevel::Off), CameraStatus::SUCCESS);

        FilterLevel curFilterLevel;
        EXPECT_EQ (camera->getFilterLevel (curFilterLevel), CameraStatus::SUCCESS);
        EXPECT_EQ (curFilterLevel, FilterLevel::Off);

        ProcessingParameterVector flags;
        EXPECT_EQ (camera->getProcessingParameters (flags), CameraStatus::SUCCESS);

        auto stdflags = ProcessingParameterVector::toStdMap (flags);
        auto it = stdflags.find (ProcessingFlag::UseAdaptiveNoiseFilter_Bool);
        EXPECT_NE (it, stdflags.end());

        EXPECT_EQ (it->second.getBool(), false);
    }
}
