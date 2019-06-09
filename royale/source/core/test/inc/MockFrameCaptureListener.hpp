/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <collector/IFrameCollector.hpp>
#include <hal/ICapturedBuffer.hpp>
#include <hal/ITemperatureSensor.hpp>

#include <gtest/gtest.h>
#include <ThreadedAssertSupport.hpp>

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <royale/Vector.hpp>

namespace royale
{
    namespace stub
    {
        namespace hal
        {
            class MockFrameCaptureListener : public royale::collector::IFrameCaptureListener, public royaletest::ThreadedAssertSupport
            {
            public:
                /**
                 * Duration (milliseconds) to wait for callbacks.
                 *
                 * The FrameCollector is multithreaded, and should make callbacks to the
                 * IFrameCaptureListener in a separate thread to the one that the
                 * BufferGeneratorStubM2450 uses to deliver frames to it.
                 */
                static const int THREAD_WAIT_TIME = 50;

                explicit MockFrameCaptureListener ();
                ~MockFrameCaptureListener () override;

                void setCaptureReleaser (std::shared_ptr<royale::collector::IFrameCaptureReleaser> releaser);

                void captureCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                      const royale::usecase::UseCaseDefinition &definition,
                                      royale::StreamId streamId,
                                      std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) override;

                void releaseAllFrames () override;

                /**
                 * If set true (which it is by default), the captureCallback() function will do
                 * in-depth comparisons of the received frames to the UseCaseDefinition.  These may
                 * be slow, particularly as it makes few assumptions about the UCD and therefore has
                 * to parse it every time.
                 */
                void setExpensiveTestsEnabled (bool enabled);

                /**
                 * Return the number of captures (the number of calls to captureCallback).
                 *
                 * As the capture may be happening in another thread, if less than the expected
                 * number of callbacks has been made this will wait for up to THREAD_WAIT_TIME for
                 * the FrameCollector to process frames and make callbacks for them.  If more than
                 * the expected number has been made (or if no expectation is given) this will
                 * return immedately.
                 *
                 * \param expectation if this number of callbacks has been made, return immediately.
                 */
                size_t getCounterCallbacks (size_t expectation = 0) const;

                /**
                 * As getCounterCallbacks, counting only a single stream in mixed mode.
                 */
                size_t getStreamCallbacks (royale::StreamId id, size_t expectation = 0);

                /**
                 * Returns values from the most recently seen CapturedUseCase.
                 */
                const royale::Vector<uint32_t> getLastExposureTimes () const;

                /**
                 * Returns a value from the most recently seen CapturedUseCase.
                 */
                const float getLastTemperature () const;

                /**
                 * Returns a value from the most recently seen CapturedUseCase.
                 */
                const std::chrono::microseconds getLastTimestamp () const;

            private:
                std::shared_ptr<royale::collector::IFrameCaptureReleaser> m_captureReleaser;
                /** During captureCallback(), enable logic that will take a comparitively long time. */
                bool m_expensiveTestsEnabled;
                mutable std::mutex m_mutex;
                /** Notified in captureCallback(), to unblock getCounterCallbacks(expectation) */
                mutable std::condition_variable m_cv;
                std::size_t m_counterUCDs;
                std::map<royale::StreamId, std::size_t> m_streamCounters;
                royale::Vector<uint32_t> m_lastExposureTimes;
                std::atomic<float> m_lastTemperature;
                std::atomic<std::chrono::microseconds> m_lastTimestamp;
            };

            /**
             * Simulates full processing, to check that it doesn't block the capture thread.
             *
             * Each captureCallback will sleep for the delay given in the constructor.
             */
            class SlowFrameCaptureListener : public MockFrameCaptureListener
            {
            public:
                explicit SlowFrameCaptureListener (std::chrono::milliseconds delay);

                void captureCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                      const royale::usecase::UseCaseDefinition &definition,
                                      royale::StreamId streamId,
                                      std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) override;

                /**
                 * After this is called, all captureCallbacks run without delay.  Also wakes any that are sleeping when runFast() is called.
                 */
                void runFast();

            private:
                std::mutex m_mutex;
                std::condition_variable m_cv;
                std::atomic<bool> m_runFast;
                const std::chrono::milliseconds m_delay;
            };

            /**
             * Simulates multithreaded processing.
             *
             * Each captureCallback will return immediately, but the frames will only be released
             * after the delay given in the constructor.
             *
             * The superclass' captureCallback will be called after the delay, not during the call
             * to AsyncFrameCaptureListener::captureCallback.  Therefore the getCounterCallbacks()
             * functions will wait for the delay, and will report only the frames that have already
             * been released.
             */
            class AsyncFrameCaptureListener : public MockFrameCaptureListener
            {
            public:
                explicit AsyncFrameCaptureListener (std::chrono::milliseconds delay);
                ~AsyncFrameCaptureListener() override;

                void captureCallback (std::vector<royale::common::ICapturedRawFrame *> &frames,
                                      const royale::usecase::UseCaseDefinition &definition,
                                      royale::StreamId streamId,
                                      std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) override;

                /**
                 * Returns the number of items queued and not yet counted by getCounterCallbacks.
                 */
                size_t getQueueSize (size_t expectation) const;

                /**
                 * When this is called, all currently-held frames are returned immediately.  Any
                 * subsequent calls to captureCallback return the frames during the callback.
                 */
                void runFast();

                void releaseAllFrames () override;

            private:
                using CLOCK_TYPE = std::chrono::steady_clock;
                /**
                 * The type that CLOCK_TYPE::now() returns.  This can be (and with MSVC++ is)
                 * time_point<superclass_of_CLOCK_TYPE> instead of time_point<CLOCK_TYPE>.
                 */
                using TIMEPOINT_TYPE = decltype (CLOCK_TYPE::now());

                void threadFunction();

                mutable std::mutex m_mutex;
                mutable std::condition_variable m_cv;
                /**
                 * This is also the control for ending the thread - when it's set, no more frames
                 * will be added to m_queue, and the thread should exit when the queue has been
                 * processed.
                 */
                bool m_runFast;
                const std::chrono::milliseconds m_delay;

                class WaitingData
                {
                public:

                    WaitingData (
                        TIMEPOINT_TYPE time,
                        std::vector<royale::common::ICapturedRawFrame *> &&frames,
                        const royale::usecase::UseCaseDefinition &definition,
                        royale::StreamId streamId,
                        std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase) :
                        time (std::move (time)),
                        frames (std::move (frames)),
                        definition (definition),
                        streamId (streamId),
                        capturedCase (std::move (capturedCase))
                    {
                    }

                    /**
                     * When this data should be passed to MockFrameCaptureListener::captureCallback.
                     */
                    TIMEPOINT_TYPE time;
                    std::vector<royale::common::ICapturedRawFrame *> frames;
                    royale::usecase::UseCaseDefinition definition;
                    royale::StreamId streamId;
                    std::unique_ptr<const royale::collector::CapturedUseCase> capturedCase;
                };
                std::queue<std::unique_ptr<WaitingData> > m_queue;
                std::thread m_thread;
            };
        }
    }
}
