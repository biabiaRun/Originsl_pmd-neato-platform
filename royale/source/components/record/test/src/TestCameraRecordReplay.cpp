/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include <record/CameraRecord.hpp>
#include <record/CameraPlayback.hpp>

#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseMixedXHt.hpp>
#include <collector/IFrameCaptureReleaser.hpp>

#include <royale/IDepthDataListener.hpp>
#include <royale/IReplay.hpp>
#include <royale/IPlaybackStopListener.hpp>
#include <royale/IExtendedDataListener.hpp>
#include <royale/ProcessingFlag.hpp>

#include <NarrowCast.hpp>

#include "CommonTestRecord.hpp"

using namespace std;
using namespace royale;
using namespace royale;
using namespace royale::record;
using namespace royale::config;
using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;

namespace
{
    static const std::string testfilename = "testfile2.rrf";

    std::vector<UseCaseDefinition> ucVec;

    class PlaybackListener : public IExtendedDataListener, public IPlaybackStopListener
    {
    public:
        explicit PlaybackListener (bool checkTimestamps)
            : m_checkTimestamps (checkTimestamps),
              m_numFramesPlayed (0),
              m_stopCalled (0),
              m_expectedFrame (0),
              m_expectFrame (false),
              m_seekTest (false)
        {
        }

        void onNewData (const IExtendedData *data)
        {
            if (!data->hasRawData())
            {
                return;
            }

            if (m_numFramesPlayed  % ucVec.size()  == 0)
            {
                m_lastTimestampPlayback = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
                m_lastTimestampRecord = data->getRawData ()->timeStamp;
            }

            std::chrono::microseconds currentTimestamp = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());

            std::chrono::microseconds durationPlayback = currentTimestamp - m_lastTimestampPlayback;
            std::chrono::microseconds durationRecord = data->getRawData()->timeStamp - m_lastTimestampRecord;

            if (!m_seekTest)
            {
                // Check that the timestamps are correct (100 ms * number of frames)
                EXPECT_EQ (static_cast<int> ( (m_numFramesPlayed % ucVec.size()) * 100000),
                           static_cast<int> (data->getRawData()->timeStamp.count()));
            }

            if (m_checkTimestamps)
            {
                // Check that we waited the correct amount of time between frames
                EXPECT_GE (durationPlayback.count(), durationRecord.count() - 20000); // allow +- 20 ms
                EXPECT_LE (durationPlayback.count(), durationRecord.count() + 20000);
            }

            if (m_expectFrame)
            {
                // Check that we received the correct data
                const uint16_t *curFrame = data->getRawData()->rawData.at (0);
                for (size_t idx = static_cast<size_t> (data->getRawData()->width); idx < static_cast<size_t> (data->getRawData()->height * data->getRawData()->width); ++idx, ++curFrame)
                {
                    EXPECT_EQ (CommonTestRecord::expectedValue (static_cast<uint32_t> (idx),
                               static_cast<uint16_t> (m_expectedFrame),
                               0), *curFrame);
                }
                m_expectFrame = false;
                m_expectedFrame = 0;
            }

            m_numFramesPlayed++;
            m_lastTimestampPlayback = currentTimestamp;
            m_lastTimestampRecord = data->getRawData()->timeStamp;
        }

        void onPlaybackStopped() override
        {
            m_stopCalled++;
        }

        bool m_checkTimestamps;
        uint32_t m_numFramesPlayed;
        std::chrono::microseconds m_lastTimestampPlayback;
        std::chrono::microseconds m_lastTimestampRecord;
        uint32_t m_stopCalled;
        uint32_t m_expectedFrame;
        bool m_expectFrame;
        bool m_seekTest;
    };

    class TestListener : public IFrameCaptureListener, public IRecordStopListener, public IFrameCaptureReleaser
    {
    public:
        TestListener()
            :            m_callbackCalled (0),
                         m_stopCalled (0),
                         m_numFramesRecorded (0)
        {
        }

        void captureCallback (std::vector<ICapturedRawFrame *> &frames,
                              const UseCaseDefinition &definition,
                              royale::StreamId streamId,
                              std::unique_ptr<const CapturedUseCase> capturedCase) override
        {
            m_callbackCalled++;
        }

        void releaseAllFrames () override
        {
        }

        void releaseCapturedFrames (std::vector<ICapturedRawFrame *> frame) override
        {

        }

        void onRecordingStopped (const uint32_t numFrames) override
        {
            m_numFramesRecorded = numFrames;
            m_stopCalled++;
        }

        uint32_t m_callbackCalled;
        uint32_t m_stopCalled;
        uint32_t m_numFramesRecorded;
    };

    class TestDevice
    {
    public:

        explicit TestDevice (IFrameCaptureListener *listener)
        {
            m_listener = listener;
        }

        void writeFrames (std::vector<UseCaseDefinition> &ucVec)
        {
            for (size_t curFrame = 0; curFrame < ucVec.size(); ++curFrame)
            {
                UseCaseDefinition &uc = ucVec.at (curFrame);

                uint16_t numCols, numRows;
                uc.getImage (numCols, numRows);

                float illuTemp = 30.0f + static_cast<float> (curFrame);

                std::vector<uint32_t> capturedExposureTimes;

                // This will create frames that are 100 milliseconds apart
                std::chrono::milliseconds timestamp = std::chrono::milliseconds (curFrame * 100);

                for (size_t i = 0; i < uc.getRawFrameSets ().size (); i++)
                {
                    capturedExposureTimes.push_back (narrow_cast<uint32_t> (100 + curFrame));
                }

                std::vector<ICapturedRawFrame *> frames;

                CommonTestRecord::createFrames (frames, uc, narrow_cast<uint16_t> (curFrame));

                auto streamId = uc.getStreamIds().at (0);

                std::unique_ptr<CapturedUseCase> cuc { new CapturedUseCase {nullptr, illuTemp, timestamp, capturedExposureTimes} };

                m_listener->captureCallback (frames, uc, streamId, std::move (cuc));

                for (size_t i = 0; i < frames.size(); ++i)
                {
                    ICapturedRawFrame *frame = frames.at (i);
                    delete frame;
                }
                frames.clear();
            }
        }
    private:
        IFrameCaptureListener *m_listener;
    };

    TestListener listener;
    CameraRecord *recorder;
    CameraPlayback *player;

    void recordFrames (uint32_t numFrames)
    {
        remove (testfilename.c_str());

        recorder = new CameraRecord (&listener, &listener, &listener, "TestCam", static_cast<ImagerType> (0u));

        std::vector<uint8_t> calibArray;
        calibArray.push_back (1);
        calibArray.push_back (2);
        calibArray.push_back (3);
        calibArray.push_back (4);

        ProcessingParameterMap paramMap;

        paramMap[ProcessingFlag::AdaptiveNoiseFilterType_Int] = Variant (1);
        paramMap[ProcessingFlag::FlyingPixelsF0_Float] = Variant (2.345f);
        paramMap[ProcessingFlag::UseAdaptiveNoiseFilter_Bool] = Variant (true);
        paramMap[ProcessingFlag::UseValidateImage_Bool] = Variant (false);

        EXPECT_NO_THROW (recorder->startRecord (testfilename, calibArray, "1234", numFrames));

        TestDevice device (recorder);

        UseCaseFourPhase testUC (45u, 30000000, {50u, 1000u}, 1000u, 1000u);
        UseCaseEightPhase testUC2 (10u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);
        UseCaseMixedXHt testUC3 (5u, 4, 30000000, 30000000, 20200000, { 200u, 1000u }, { 100u, 1500u },
                                 1000u, 1500u, 1500u, 200u, 200u);

        recorder->setProcessingParameters (ProcessingParameterVector::fromStdMap (paramMap), testUC.getStreamIds().at (0));
        recorder->setProcessingParameters (ProcessingParameterVector::fromStdMap (paramMap), testUC2.getStreamIds().at (0));
        recorder->setProcessingParameters (ProcessingParameterVector::fromStdMap (paramMap), testUC3.getStreamIds().at (0));

        ucVec.clear();

        ucVec.push_back (testUC);
        ucVec.push_back (testUC2);
        ucVec.push_back (testUC);
        ucVec.push_back (testUC2);
        ucVec.push_back (testUC);
        ucVec.push_back (testUC2);
        ucVec.push_back (testUC2);
        ucVec.push_back (testUC3);
        ucVec.push_back (testUC3);
        ucVec.push_back (testUC2);

        device.writeFrames (ucVec);
    }

}


TEST (TestCameraRecordReplay, Record)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_EQ (static_cast<uint32_t> (ucVec.size()), listener.m_callbackCalled);
    EXPECT_EQ (0u, listener.m_stopCalled);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (static_cast<uint32_t> (ucVec.size()), listener.m_numFramesRecorded);
    EXPECT_EQ (1u, listener.m_stopCalled);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, RecordnFrames)
{
    listener = TestListener();

    recordFrames (5);

    EXPECT_EQ (static_cast<uint32_t> (ucVec.size()), listener.m_callbackCalled);
    EXPECT_EQ (5u, listener.m_numFramesRecorded);
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, StartCaptureCrash)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    EXPECT_EQ (CameraStatus::DEVICE_NOT_INITIALIZED, player->startCapture());

    // ROYAL-3769 Crashed during the sleep
    std::this_thread::sleep_for (std::chrono::milliseconds (10));

    delete (player);

    remove (testfilename.c_str());
}


TEST (TestCameraRecordReplay, Replay)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    PlaybackListener pbListener (true);

    player->setCallbackData ( (uint16_t) CallbackData::Raw);
    EXPECT_NO_THROW (player->initialize());

    player->registerDataListenerExtended (&pbListener);
    player->registerStopListener (&pbListener);

    UseCaseFourPhase testUC (45u, 30000000, { 50u, 1000u }, 1000u, 1000u);

    uint16_t ucHeight, ucWidth;
    testUC.getImage (ucWidth, ucHeight);

    uint16_t maxHeight, maxWidth;
    player->getMaxSensorHeight (maxHeight);
    player->getMaxSensorWidth (maxWidth);

    EXPECT_EQ (ucWidth, maxWidth);
    EXPECT_EQ (ucHeight, maxHeight);

    player->startCapture();

    std::this_thread::sleep_for (std::chrono::milliseconds (500));

    EXPECT_EQ (0u, pbListener.m_stopCalled);

    delete (player);

    EXPECT_EQ (1u, pbListener.m_stopCalled);

    // Check how many frames were received. Depending on the timing
    // and the startup time of the test this should be between 4 and 6
    EXPECT_GE (pbListener.m_numFramesPlayed, 4u);
    EXPECT_LE (pbListener.m_numFramesPlayed, 6u);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, ReplayWithoutTimestamps)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    PlaybackListener pbListener (false);

    player->setCallbackData ( (uint16_t) CallbackData::Raw);
    EXPECT_NO_THROW (player->initialize());

    player->registerDataListenerExtended (&pbListener);
    player->registerStopListener (&pbListener);

    IReplay *replay = dynamic_cast<IReplay *> (player);

    replay->useTimestamps (false);


    player->startCapture();

    std::this_thread::sleep_for (std::chrono::milliseconds (200));

    EXPECT_EQ (0u, pbListener.m_stopCalled);

    delete (player);

    EXPECT_EQ (1u, pbListener.m_stopCalled);
    EXPECT_GT (pbListener.m_numFramesPlayed, 10u);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, ReplayWithoutLoop)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    PlaybackListener pbListener (false);

    player->setCallbackData ( (uint16_t) CallbackData::Raw);
    EXPECT_NO_THROW (player->initialize());

    player->registerDataListenerExtended (&pbListener);
    player->registerStopListener (&pbListener);

    IReplay *replay = dynamic_cast<IReplay *> (player);

    replay->loop (false);
    replay->useTimestamps (false);

    player->startCapture();

    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    EXPECT_EQ (1u, pbListener.m_stopCalled);

    delete (player);

    EXPECT_EQ (1u, pbListener.m_stopCalled);

    EXPECT_EQ (ucVec.size (), pbListener.m_numFramesPlayed);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, ReplaySeek)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    PlaybackListener pbListener (false);

    pbListener.m_seekTest = true;

    player->setCallbackData ( (uint16_t) CallbackData::Raw);
    EXPECT_NO_THROW (player->initialize());

    player->registerDataListenerExtended (&pbListener);
    player->registerStopListener (&pbListener);

    IReplay *replay = dynamic_cast<IReplay *> (player);

    EXPECT_EQ (CameraStatus::DATA_NOT_FOUND, replay->seek (static_cast<uint32_t> (ucVec.size())));
    EXPECT_EQ (CameraStatus::DATA_NOT_FOUND, replay->seek (static_cast<uint32_t> (ucVec.size() + 1)));
    EXPECT_EQ (CameraStatus::SUCCESS, replay->seek (1));

    delete (player);

    remove (testfilename.c_str());
}

TEST (TestCameraRecordReplay, ReplaySeek2)
{
    listener = TestListener();

    recordFrames (0);

    EXPECT_NO_THROW (recorder->stopRecord());
    EXPECT_EQ (1u, listener.m_stopCalled);

    delete (recorder);

    player = new CameraPlayback (CameraAccessLevel::L2, testfilename);
    PlaybackListener pbListener (false);

    pbListener.m_seekTest = true;

    player->setCallbackData ( (uint16_t) CallbackData::Raw);
    EXPECT_NO_THROW (player->initialize());

    player->registerDataListenerExtended (&pbListener);
    player->registerStopListener (&pbListener);

    IReplay *replay = dynamic_cast<IReplay *> (player);

    pbListener.m_expectFrame = true;
    pbListener.m_expectedFrame = 1;
    EXPECT_EQ (CameraStatus::SUCCESS, replay->seek (1));

    player->startCapture();

    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    player->stopCapture();

    pbListener.m_expectFrame = true;
    pbListener.m_expectedFrame = 2;
    EXPECT_EQ (CameraStatus::SUCCESS, replay->seek (2));

    player->startCapture();

    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    delete (player);

    remove (testfilename.c_str());
}
