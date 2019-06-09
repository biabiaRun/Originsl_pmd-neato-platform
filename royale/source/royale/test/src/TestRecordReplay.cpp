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
#include <royale/IReplay.hpp>
#include <gtest/gtest.h>
#include <FileSystem.hpp>

#include <CameraDeviceL1Fixture.hpp>

#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>
#include <memory>

using namespace royale;
using namespace royale::common;

TEST_F (CameraDeviceL1Fixture, TestRecording)
{
    initCamera();

    EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->startRecording ("test.rrf"), CameraStatus::SUCCESS);

    std::this_thread::sleep_for (std::chrono::seconds (5));

    EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (fileexists ("test.rrf"), true);
    EXPECT_EQ (std::remove ("test.rrf"), 0);
}

TEST_F (CameraDeviceL1Fixture, TestRecordingNrFrames)
{
    const uint32_t captureFrames = 50u;
    initCamera();

    std::shared_ptr<RecordingListener> listener { new RecordingListener() };
    camera->registerRecordListener (listener.get());
    uint16_t frames = 1;
    EXPECT_EQ (camera->getFrameRate(frames), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (camera->startRecording ("test.rrf", captureFrames), CameraStatus::SUCCESS);

    // timeout ist calculated based on the framerate
    EXPECT_TRUE (listener->hasBeenCalled (std::chrono::seconds (75/frames)));
    EXPECT_EQ (listener->frames, captureFrames);

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (fileexists ("test.rrf"), true);

    // Unregister the listener before it goes out of scope
    EXPECT_EQ (CameraStatus::SUCCESS, camera->unregisterRecordListener());

    // try to replay
    {
        CameraManager manager;
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();
        ASSERT_EQ (status, CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        EXPECT_EQ (captureFrames, replay->frameCount());
    }

    EXPECT_EQ (std::remove ("test.rrf"), 0);
}

TEST_F (CameraDeviceL1Fixture, TestRecordingSkipFrames)
{
    const uint32_t skipFrames = 2u;
    initCamera();

    uint16_t framerate = 1;
    EXPECT_EQ (camera->getFrameRate(framerate), CameraStatus::SUCCESS);
    auto waitTime = 10000/framerate;

    EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->startRecording ("test_skipframes.rrf", 0, skipFrames), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (waitTime));
    EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->startRecording ("test.rrf"), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (waitTime));
    EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (fileexists ("test_skipframes.rrf"), true);
    EXPECT_EQ (fileexists ("test.rrf"), true);

    uint32_t capturedFramesWithSkip = 0;
    uint32_t capturedFramesWithoutSkip = 0;

    {
        CameraManager manager;
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();

        ASSERT_EQ (status, CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        capturedFramesWithoutSkip = replay->frameCount();
    }

    {
        CameraManager manager;
        auto camera = manager.createCamera ("test_skipframes.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();

        ASSERT_EQ (status, CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        capturedFramesWithSkip = replay->frameCount();
    }

    EXPECT_LE (capturedFramesWithoutSkip, capturedFramesWithSkip * (skipFrames + 1) + 1);
    EXPECT_GE (capturedFramesWithoutSkip, capturedFramesWithSkip * skipFrames + 1);

    EXPECT_EQ (std::remove ("test.rrf"), 0);
    EXPECT_EQ (std::remove ("test_skipframes.rrf"), 0);
}

TEST_F (CameraDeviceL1Fixture, TestRecordingSkipMs)
{
    initCamera();

    uint16_t framerate = 1;
    EXPECT_EQ (camera->getFrameRate(framerate), CameraStatus::SUCCESS);
    uint32_t captureTime = 10000/framerate;
    uint32_t skipMS = 2500u/framerate;

    EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->startRecording ("test_skipframes.rrf", 0, 0, skipMS), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (captureTime));
    EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->startRecording ("test.rrf"), CameraStatus::SUCCESS);
    std::this_thread::sleep_for (std::chrono::milliseconds (captureTime));
    EXPECT_EQ (camera->stopRecording(), CameraStatus::SUCCESS);

    EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);
    EXPECT_EQ (fileexists ("test_skipframes.rrf"), true);
    EXPECT_EQ (fileexists ("test.rrf"), true);

    uint32_t capturedFramesWithSkip = 0;
    uint32_t capturedFramesWithoutSkip = 0;

    {
        CameraManager manager;
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();

        ASSERT_EQ (status, CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        capturedFramesWithoutSkip = replay->frameCount();
    }

    {
        CameraManager manager;
        auto camera = manager.createCamera ("test_skipframes.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();

        ASSERT_EQ (status, CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        capturedFramesWithSkip = replay->frameCount();
    }

    EXPECT_LE (capturedFramesWithSkip, capturedFramesWithoutSkip / 2);

    EXPECT_EQ (std::remove ("test.rrf"), 0);
    EXPECT_EQ (std::remove ("test_skipframes.rrf"), 0);
}

TEST_F (CameraDeviceL1Fixture, TestReplay)
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
        CameraManager manager;
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_NE (camera, nullptr);

        auto status = camera->initialize();
        ASSERT_EQ (status, CameraStatus::SUCCESS);

        EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS);

        std::this_thread::sleep_for (std::chrono::seconds (10));

        EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS);

        auto replay = dynamic_cast<IReplay *> (camera.get());
        ASSERT_NE (replay, nullptr);

        EXPECT_EQ (replay->seek (2), CameraStatus::SUCCESS);
    }

    // try to replay non existing file
    {
        EXPECT_EQ (std::remove ("test.rrf"), 0);
        EXPECT_EQ (fileexists ("test.rrf"), false);

        CameraManager manager;
        auto camera = manager.createCamera ("test.rrf");
        ASSERT_EQ (camera, nullptr);
    }
}

#define ReplayTestVersion(name, filename) TEST_F (CameraDeviceL1Fixture, name) \
{ \
CameraManager manager; \
auto camera = manager.createCamera (ROYALE_TEST_FILE_PATH filename); \
ASSERT_NE (camera, nullptr); \
auto status = camera->initialize(); \
ASSERT_EQ (status, CameraStatus::SUCCESS); \
EXPECT_EQ (camera->startCapture(), CameraStatus::SUCCESS); \
std::this_thread::sleep_for (std::chrono::milliseconds (500)); \
EXPECT_EQ (camera->stopCapture(), CameraStatus::SUCCESS); \
}

ReplayTestVersion (TestReplayRRFv1, "/rrf_v1.rrf")
ReplayTestVersion (TestReplayRRFv2, "/rrf_v2.rrf")
ReplayTestVersion (TestReplayRRFv3, "/rrf_v3.rrf")
