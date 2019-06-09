#pragma once

#include <royale/ICameraDevice.hpp>
#include <royale/IExtendedDataListener.hpp>
#include <royale/IDepthDataListener.hpp>
#include <FrameRateStats.hpp>
#include <RoyaleProfiler.hpp>
#include <atomic>

namespace royale
{
    namespace bdd
    {

        /**
         * @brief The MockRawCaptureListener class is a simple implementation for registering to the CameraModule data delivery
         */
        class MockExtendedCaptureListener : public royale::IExtendedDataListener
        {
        public:
            MockExtendedCaptureListener();
            ~MockExtendedCaptureListener();

            void onNewData (const IExtendedData *data);

            /** Number of captured use cases (equal to number of calls to captureCallback) */
            int getCounterCallbacks();

            /** Number of individual RawFrames in the captured UCDs */
            int getCounterFrames();

            /* Get timemark of first received frame */
            royale::common::RoyaleProfiler getTimeofFirstCallback();

            /** Gets the last timestamp */
            std::chrono::microseconds getLastTimeStamp();

            /** Get frame rate statistics */
            FrameRateStats &getFrameRateStats();

            /** Resets the counter */
            void resetCounters();

        private:
            std::atomic<int> m_counterUCDs;
            std::atomic<int> m_counterFrames;
            std::atomic<std::chrono::microseconds> m_lastTimeStamp;
            FrameRateStats m_frameRateStats;

            royale::common::RoyaleProfiler m_timeMark;
        };


        /**
         * @brief The MockRawCaptureListener class is a simple implementation for registering to the CameraModule data delivery
         */
        class MockDepthCaptureListener : public royale::IDepthDataListener
        {
        public:
            MockDepthCaptureListener();
            ~MockDepthCaptureListener();

            void onNewData (const DepthData *data);

            int getCallBackCounter() const;

            /** Gets the last timestamp */
            std::chrono::microseconds getLastTimeStamp();

            /** Get frame rate statistics */
            FrameRateStats &getFrameRateStats();

            /* Get timemark of first received frame */
            royale::common::RoyaleProfiler getTimeofFirstCallback();

            /** Resets the counter */
            void resetCounters();

            int getWidth() const;
            int getHeight() const;
            size_t getPointCount() const;

        private:
            std::atomic<int> m_callbackCounter;
            std::atomic<int> m_Width;
            std::atomic<int> m_Height;
            std::atomic<size_t> m_Points;
            std::atomic<std::chrono::microseconds> m_lastTimeStamp;
            FrameRateStats m_frameRateStats;

            royale::common::RoyaleProfiler m_timeMark;
        };

    }
}
