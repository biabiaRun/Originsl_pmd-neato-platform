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

#include <CameraDeviceL3Fixture.hpp>

#include <PlatformResources.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;

std::unique_ptr<sample_utils::PlatformResources> platformResources{ new sample_utils::PlatformResources() };

TEST_F (CameraDeviceL3Fixture, TestSetDutyCycle)
{
    initCamera();

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        struct DutyCycleData
        {
            double dutyCycle;
            uint16_t index;
        };
        Vector<DutyCycleData> dutyCycles;
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_5FPS_2000", { { 0., 0u }, { 0., 1u }, { 25., 1u }, { 0., 2u }, } } },
        { "EVALBOARD_INFINEON_IRS10X0C_EVALUATION_KIT", { "MODE_5_10FPS_2900", { { 0., 0u }, { 0., 1u }, { 25., 1u }, } } },
        { "PICOMAXX2", { "MODE_9_5FPS_1900", { { 0., 0u }, { 0., 1u }, { 25., 1u }, { 0., 2u }, } } },
        { "PICOMONSTAR2", { "MODE_9_5FPS_1900", { { 0., 0u }, { 0., 1u }, { 25., 1u }, { 0., 2u }, } } },
        { "Alea945nm", { "MODE_9_10FPS", { { 0., 0u }, { 0., 1u }, { 25., 1u }, { 0., 2u }, } } },
        { "Salome940nm", { "MODE_9_10FPS", { { 0., 0u }, { 0., 1u }, { 25., 1u }, { 0., 2u }, } } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    auto listener = std::make_shared<ExtendedDataListener>();
    camera->registerDataListenerExtended (listener.get());
    camera->setCallbackData (royale::CallbackData::Raw);
    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));

    for (const auto &dc : testData.dutyCycles)
    {
        EXPECT_EQ (camera->setDutyCycle (dc.dutyCycle, dc.index), CameraStatus::SUCCESS);
        EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));
    }

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL3Fixture, TestWriteRegister)
{
    initCamera();

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        String regAddr;
        uint64_t regVal;
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_5FPS_2000", "0xA830", 100 } }, // S12_EXPOTIME
        { "Alea945nm", { "MODE_9_10FPS", "0x9824", 100 } }, // S12_EXPOTIME
        // \todo ROYAL-2832 need to find a good register/value pair to test PICOMAXX2
        { "Salome940nm", { "MODE_9_10FPS", "0x9018", 100 } }, // S12_EXPOTIME
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    auto listener = std::make_shared<ExtendedDataListener>();
    camera->registerDataListenerExtended (listener.get());
    camera->setCallbackData (royale::CallbackData::Raw);
    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));

    Vector<Pair<String, uint64_t>> registers;
    registers.push_back (Pair<String, uint64_t> (testData.regAddr, testData.regVal));
    EXPECT_EQ (camera->writeRegisters (registers), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL3Fixture, TestReadRegister)
{
    initCamera();

    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        String regAddr;
        uint64_t regVal;
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_9_5FPS_2000", "0x0B0AD", 0x0A12u } }, // ANAIP_DESIGNSTEP
        { "Alea945nm", { "MODE_9_10FPS", "0xA0A5", 0x0B12u } }, // ANAIP_DESIGNSTEP
        // \todo ROYAL-2832 need to find a good register/value pair to test PICOMAXX2
        { "Salome940nm", { "MODE_9_10FPS", "0xA0A5", 0x0A11 } }, // ANAIP_DESIGNSTEP
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &testData = td_it->second;

    auto listener = std::make_shared<ExtendedDataListener>();
    camera->registerDataListenerExtended (listener.get());
    camera->setCallbackData (royale::CallbackData::Raw);
    EXPECT_EQ (camera->setUseCase (testData.useCase), CameraStatus::SUCCESS);
    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (3, std::chrono::seconds (1)));
    {
        Vector<Pair<String, uint64_t>> registers;
        registers.push_back (Pair<String, uint64_t> (testData.regAddr, 0));

        EXPECT_EQ (camera->readRegisters (registers), CameraStatus::SUCCESS);

        EXPECT_EQ (registers.at (0).second, testData.regVal);
    }

    {
        Vector<Pair<String, uint64_t>> registers;
        registers.push_back (Pair<String, uint64_t> (testData.regAddr, std::numeric_limits<uint64_t>::max()));

        EXPECT_EQ (camera->readRegisters (registers), CameraStatus::SUCCESS);

        EXPECT_EQ (registers.at (0).second, testData.regVal);
    }
}

/**
 * The UseCaseCalibration is only available in L3/L4.
 * This tests that the number of available modes in L3 is the number expected.
 */
TEST_F (CameraDeviceL3Fixture, TestUseCasesCallbackData)
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
        { "PICOFLEXX", { 11u, 10u, 10u } },
        { "PICOMAXX2", { 10u, 9u, 9u } },
        { "PICOMONSTAR2", { 10u, 9u, 9u } },
        { "Alea945nm", { 31u, 13u, 13u } },
        { "Salome940nm", { 15u, 14u, 14u } },
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

TEST_F (CameraDeviceL3Fixture, TestShiftLensCenter)
{
    ASSERT_EQ (CameraStatus::SUCCESS, camera->setCallbackData (CallbackData::Raw));

    initCamera();

    String id;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (id));

    // There is no module-specific data for this test,
    // but we still want to limit this to known modules
    std::set<String> testDataMap
    {
        { "PICOFLEXX" },
        // \todo ROYAL-2833 This doesn't work for Maxx...
    };

    auto td_it = testDataMap.find (id);
    if (td_it == testDataMap.end())
    {
        // Unknown module, test does not apply
        return;
    }

    uint16_t cx, cy;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getLensCenter (cx, cy));

    uint16_t cx2, cy2;
    {
        ASSERT_EQ (CameraStatus::INVALID_VALUE, camera->shiftLensCenter (-1, 1));
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getLensCenter (cx2, cy2));

        ASSERT_EQ (cx, cx2);
        ASSERT_EQ (cy, cy2);
    }

    {
        ASSERT_EQ (CameraStatus::SUCCESS, camera->shiftLensCenter (-16, 1));
        ASSERT_EQ (CameraStatus::SUCCESS, camera->getLensCenter (cx2, cy2));

        ASSERT_EQ (cx - 16, cx2);
        ASSERT_EQ (cy + 1, cy2);
    }
}

TEST_F (CameraDeviceL3Fixture, TestExposureTimes)
{
    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        royale::Vector<uint32_t> expect1;  // default exposure times of the usecase
        royale::Vector<uint32_t> expect2;  // different (but legal) values
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_10_5FPS_2000", { 200, 2000, 2000, 200, 200 }, { 100, 200, 300, 400, 500 } } },
        { "Alea945nm", { "MODE_10_5FPS", { 200, 1200, 1200, 200, 200 }, { 100, 200, 300, 400, 500 } } },
        { "Salome940nm", { "MODE_CALIBRATION", { 100, 1200, 1200, 100, 100 }, { 100, 200, 300, 400, 500 } } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &expected = td_it->second;

    initCamera();

    EXPECT_EQ (camera->setUseCase (expected.useCase), CameraStatus::SUCCESS);

    auto listener = std::make_shared<ExtendedDataListener>();
    auto status = camera->registerDataListenerExtended (listener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    status = camera->setCallbackData (royale::CallbackData::Raw);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_TRUE (listener->waitForCallbacks (1, std::chrono::seconds (2)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (listener->rawDataExposureTimes.size(), expected.expect1.size());
    for (auto i = 0u; i < expected.expect1.size(); ++i)
    {
        EXPECT_EQ (expected.expect1[i], listener->rawDataExposureTimes[i]);
    }


    ASSERT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    ASSERT_EQ (camera->setExposureTimes (expected.expect2), CameraStatus::SUCCESS);

    EXPECT_TRUE (listener->waitForCallbacks (2, std::chrono::seconds (2)));
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (listener->rawDataExposureTimes.size(), expected.expect2.size());
    for (auto i = 0u; i < expected.expect2.size(); ++i)
    {
        EXPECT_EQ (expected.expect2[i], listener->rawDataExposureTimes[i]);
    }
}

TEST_F (CameraDeviceL3Fixture, TestSetExposureTimesWithoutCapturing)
{
    String name;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (name));

    struct TestData
    {
        String useCase;
        royale::Vector<uint32_t> expdata;
    };

    std::map<String, TestData> testDataMap
    {
        { "PICOFLEXX", { "MODE_10_5FPS_2000", { 100, 200, 300, 400, 500 } } },
        { "Alea945nm", { "MODE_10_5FPS", { 100, 200, 300, 400, 500 } } },
        { "Salome940nm", { "MODE_CALIBRATION", { 100, 200, 300, 400, 500 } } },
    };

    auto td_it = testDataMap.find (name);
    if (td_it == testDataMap.end())
    {
        // Unknown module, no test data available
        return;
    }
    const auto &expected = td_it->second;

    initCamera();

    auto status = camera->setCallbackData (royale::CallbackData::Raw);
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    EXPECT_EQ (camera->setUseCase (expected.useCase), CameraStatus::SUCCESS);

    auto listener = std::make_shared<ExtendedDataListener>();
    status = camera->registerDataListenerExtended (listener.get());
    ASSERT_EQ (CameraStatus::SUCCESS, status);

    ASSERT_EQ (camera->setExposureTimes (expected.expdata), CameraStatus::SUCCESS);
}

TEST_F (CameraDeviceL3Fixture, TestLensCenterRaw)
{
    // Check if the lens center is correctly loaded even
    // when callback data is set to raw

    String id;
    ASSERT_EQ (CameraStatus::SUCCESS, camera->getCameraName (id));

    // There is no module-specific data for this test,
    // but we still want to limit this to known modules
    std::set<String> testDataMap
    {
        { "PICOFLEXX" },
        { "Alea945nm" },
        { "Salome940nm" },
    };

    auto td_it = testDataMap.find (id);
    if (td_it == testDataMap.end())
    {
        // Unknown module, test does not apply
        return;
    }

    initCamera();

    uint16_t xBefore, yBefore;
    camera->getLensCenter (xBefore, yBefore);

    camera.reset (nullptr);

    // Reopen the camera and check the lens center with raw mode
#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = cameraManager->getConnectedCameraList (0);
#else
    auto connectedCameras = cameraManager->getConnectedCameraList();
#endif
    ASSERT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    camera = cameraManager->createCamera (connectedCameras[0]);

    auto status = camera->setCallbackData (royale::CallbackData::Raw);

    initCamera();

    uint16_t xAfter, yAfter;
    camera->getLensCenter (xAfter, yAfter);

    EXPECT_EQ (xBefore, xAfter);
    EXPECT_EQ (yBefore, yAfter);

    ASSERT_EQ (CameraStatus::SUCCESS, status);
}
