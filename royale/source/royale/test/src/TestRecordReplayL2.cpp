/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
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
#include <royale/IReplay.hpp>
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

TEST_F (CameraDeviceL2Fixture, TestReplayL2)
{
    // record first
    {
        initCamera();

        EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
        EXPECT_EQ (camera->startRecording ("test.rrf"), CameraStatus::SUCCESS);

        std::this_thread::sleep_for (std::chrono::seconds (4));

        EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);
        EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

        EXPECT_EQ (fileexists ("test.rrf"), true);
    }

    // try to replay
    {
        CameraManager manager (ROYALE_ACCESS_CODE_LEVEL2);
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_NE (camera, nullptr);

        // Try raw playback
        camera->setCallbackData (CallbackData::Raw);

        auto status = camera->initialize();
        ASSERT_EQ (status, CameraStatus::SUCCESS);

        royale::Vector<uint8_t> calibData;
        camera->getCalibrationData (calibData);

        EXPECT_GT (calibData.size(), 0u);

        EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

        std::this_thread::sleep_for (std::chrono::seconds (2));

        EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    }

    EXPECT_EQ (std::remove ("test.rrf"), 0);
}
