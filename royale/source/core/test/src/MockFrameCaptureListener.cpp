/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <MockFrameCaptureListener.hpp>

#include <collector/IFrameCollector.hpp>
#include <hal/ICapturedBuffer.hpp>
#include <hal/ITemperatureSensor.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <royale/Vector.hpp>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;
using namespace royale::hal;
using namespace royale::stub::hal;

MockFrameCaptureListener::MockFrameCaptureListener () :
    m_captureReleaser {nullptr},
    m_expensiveTestsEnabled {true},
    m_counterUCDs {0},
    m_streamCounters {},
    m_lastExposureTimes {},
    m_lastTemperature {0.0f},
    m_lastTimestamp {}
{
}

MockFrameCaptureListener::~MockFrameCaptureListener()
{
    // If this class is the last owner of the shared_ptr, ensure that the FrameCollector's
    // destructor runs while this class can still handle a call to releaseAllFrames().
    m_captureReleaser.reset();
}

void MockFrameCaptureListener::setCaptureReleaser (std::shared_ptr<IFrameCaptureReleaser> releaser)
{
    m_captureReleaser = releaser;
}

void MockFrameCaptureListener::setExpensiveTestsEnabled (bool enabled)
{
    m_expensiveTestsEnabled = enabled;
}

void MockFrameCaptureListener::captureCallback (std::vector<ICapturedRawFrame *> &frames,
        const UseCaseDefinition &definition,
        royale::StreamId streamId,
        std::unique_ptr<const CapturedUseCase> capturedCase)
{
    // scope for the lock, the lock is needed for the exposure times array
    {
        std::lock_guard<std::mutex> lock {m_mutex};
        m_streamCounters[streamId]++;
        m_counterUCDs++;
        m_lastExposureTimes = capturedCase->getExposureTimes();
        m_lastTemperature = capturedCase->getIlluminationTemperature();
        m_lastTimestamp = capturedCase->getTimestamp();
        m_cv.notify_all();
    }

    assertEq (definition.getExposureTimes().size(), m_lastExposureTimes.size(), "Exposure size does not match use case");

    bool streamMatch = false;
    for (const auto id : definition.getStreamIds())
    {
        if (streamId == id)
        {
            streamMatch = true;
            break;
        }
    }
    assertTrue (streamMatch, "Received a callback with unrecognised streamId");

    auto &interpreter = capturedCase->getInterpreter();
    const auto baseFrame = interpreter.getFrameNumber (*frames[0]);
    const auto baseIndex = interpreter.getSequenceIndex (*frames[0]);
    // For non-mixed mode use cases, we expect the first sequence number to always be zero
    if (definition.getStreamIds().size() == 1u)
    {
        assertEq (baseIndex, uint16_t {0}, "First frame corrupt or has non-zero sequence index");
    }
    // With mixed mode the frames may be non-sequential, but baseFrame + sequence number == baseIndex + frame number
    for (const auto &frame : frames)
    {
        assertEq (
            interpreter.getFollowingFrameNumber (interpreter.getFrameNumber (*frame), baseIndex),
            interpreter.getFollowingFrameNumber (interpreter.getSequenceIndex (*frame), baseFrame),
            "Frame number doesn't match sequence number");
    }

    if (m_expensiveTestsEnabled)
    {
        // The next block interates over all the frame groups in this stream, and checks that the
        // received frames match one of the groups.
        bool matchedAGroup = false;
        const auto groupCount = definition.getFrameGroupCount (streamId);
        for (std::size_t possibleGroup = 0 ; possibleGroup < groupCount ; possibleGroup++)
        {
            std::vector<uint16_t> possibleSequence;
            for (auto frameSetId : definition.getRawFrameSetIndices (streamId, possibleGroup))
            {
                for (auto frameId : definition.getSequenceIndicesForRawFrameSet (frameSetId))
                {
                    possibleSequence.push_back (frameId);
                }
            }
            assertEq (possibleSequence.size(), frames.size(), "Frame count does not match use case");

            bool groupMayMatch = true;
            for (std::size_t i = 0; i < frames.size(); i++)
            {
                if (possibleSequence[i] != interpreter.getSequenceIndex (*frames[i]))
                {
                    groupMayMatch = false;
                }
            }
            if (groupMayMatch)
            {
                matchedAGroup = true;
            }
        }
        assertTrue (matchedAGroup, "Received frames do not match use case");
    }

    m_captureReleaser->releaseCapturedFrames (frames);
}

void MockFrameCaptureListener::releaseAllFrames()
{
    // all the releaseCapturedFrame() calls are already done in the captureCallback
}

size_t MockFrameCaptureListener::getCounterCallbacks (size_t expectation) const
{
    std::unique_lock<std::mutex> lock {m_mutex};
    std::chrono::milliseconds delay (THREAD_WAIT_TIME);
    m_cv.wait_for (lock, delay, [this, expectation] { return m_counterUCDs >= expectation; });
    return m_counterUCDs;
}

size_t MockFrameCaptureListener::getStreamCallbacks (royale::StreamId id, size_t expectation)
{
    std::unique_lock<std::mutex> lock {m_mutex};
    std::chrono::milliseconds delay (THREAD_WAIT_TIME);
    // This will insert the counter if it isn't found, which a later captureCallback should
    // then increment.
    const auto &counter = m_streamCounters[id];
    m_cv.wait_for (lock, delay, [&counter, expectation] { return counter >= expectation; });
    return counter;
}

const royale::Vector<uint32_t> MockFrameCaptureListener::getLastExposureTimes () const
{
    royale::Vector<uint32_t> times;
    // scope for the lock
    {
        std::lock_guard<std::mutex> lock {m_mutex};
        times = m_lastExposureTimes;
    }
    return times;
}

const float MockFrameCaptureListener::getLastTemperature () const
{
    return m_lastTemperature;
}

const std::chrono::microseconds MockFrameCaptureListener::getLastTimestamp () const
{
    return m_lastTimestamp;
}

SlowFrameCaptureListener::SlowFrameCaptureListener (std::chrono::milliseconds delay) :
    m_runFast {false},
    m_delay {delay}
{
}

void SlowFrameCaptureListener::captureCallback (std::vector<ICapturedRawFrame *> &frames,
        const UseCaseDefinition &definition,
        royale::StreamId streamId,
        std::unique_ptr<const CapturedUseCase> capturedCase)
{
    // scope for the lock
    {
        std::unique_lock<std::mutex> lock {m_mutex};
        m_cv.wait_for (lock, m_delay, [this] { return m_runFast.load(); });
    }
    MockFrameCaptureListener::captureCallback (frames, definition, streamId, std::move (capturedCase));
}

void SlowFrameCaptureListener::runFast()
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_runFast = true;
    m_cv.notify_all();
}

AsyncFrameCaptureListener::AsyncFrameCaptureListener (std::chrono::milliseconds delay) :
    m_runFast {false},
    m_delay {delay},
    m_thread {&AsyncFrameCaptureListener::threadFunction, this}
{
}

AsyncFrameCaptureListener::~AsyncFrameCaptureListener()
{
    // Calling runFast empties the queue, and ensures nothing new joins the queue
    runFast();
    m_thread.join();
}

void AsyncFrameCaptureListener::captureCallback (std::vector<ICapturedRawFrame *> &frames,
        const UseCaseDefinition &definition,
        royale::StreamId streamId,
        std::unique_ptr<const CapturedUseCase> capturedCase)
{
    bool deliverImmediately = false;
    // scope for the lock
    {
        std::unique_lock<std::mutex> lock {m_mutex};
        deliverImmediately = m_runFast;
        if (!deliverImmediately)
        {
            auto deliveryTime = CLOCK_TYPE::now() + m_delay;
            auto item = common::makeUnique<WaitingData> (std::move (deliveryTime), std::move (frames), definition, streamId, std::move (capturedCase));
            m_queue.push (std::move (item));
            //LOG (DEBUG) << "Queued with difference: " << std::chrono::duration_cast<std::chrono::microseconds> (deliveryTime - CLOCK_TYPE::now()).count();
            m_cv.notify_all();
        }
    }
    if (deliverImmediately)
    {
        MockFrameCaptureListener::captureCallback (frames, definition, streamId, std::move (capturedCase));
    }
}

size_t AsyncFrameCaptureListener::getQueueSize (size_t expectation) const
{
    std::unique_lock<std::mutex> lock {m_mutex};
    std::chrono::milliseconds delay (THREAD_WAIT_TIME);
    m_cv.wait_for (lock, delay, [this, expectation] { return m_queue.size() >= expectation; });
    return m_queue.size();
}


void AsyncFrameCaptureListener::runFast()
{
    {
        std::lock_guard<std::mutex> lock {m_mutex};
        m_runFast = true;
    }
    releaseAllFrames();
}

void AsyncFrameCaptureListener::releaseAllFrames()
{
    std::lock_guard<std::mutex> lock {m_mutex};
    while (!m_queue.empty())
    {
        //LOG (DEBUG) << "Delivered during releaseAllFrames()";
        auto item = std::move (m_queue.front());
        m_queue.pop();
        MockFrameCaptureListener::captureCallback (item->frames, item->definition, item->streamId, std::move (item->capturedCase));
    }
    m_cv.notify_all();
}

void AsyncFrameCaptureListener::threadFunction()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock {m_mutex};
        if (m_queue.empty())
        {
            m_cv.wait (lock, [this] { return m_runFast || !m_queue.empty(); });
        }
        else
        {
            auto timepoint = m_queue.front()->time;
            m_cv.wait_until (lock, timepoint, [this] { return m_runFast; });
        }

        if ( (!m_queue.empty()) && m_queue.front()->time < CLOCK_TYPE::now())
        {
            //LOG (DEBUG) << "Time difference: " << std::chrono::duration_cast<std::chrono::nanoseconds> (CLOCK_TYPE::now() - m_queue.front().time).count();
            auto item = std::move (m_queue.front());
            m_queue.pop();
            MockFrameCaptureListener::captureCallback (item->frames, item->definition, item->streamId, std::move (item->capturedCase));
        }

        if (m_runFast)
        {
            break;
        }
    }
}
