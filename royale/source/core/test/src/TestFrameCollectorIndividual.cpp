/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <collector/BufferActionCalcIndividual.hpp>
#include <royale/IEventListener.hpp>
#include <common/events/EventRawFrameStats.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseMixedXHt.hpp>

#include <BufferGeneratorPregen.hpp>
#include <BufferGeneratorStub.hpp>
#include <FixtureTestFrameCollector.hpp>
#include <StubTemperatureSensor.hpp>
#include <MockFrameCaptureListener.hpp>

#include <gtest/gtest.h>

#include <random>
#include <vector>
#include <royale/Vector.hpp>

using namespace royale::common;
using namespace royale::usecase;
using namespace royale::collector;
using namespace royale::hal;
using namespace royale::stub::hal;
using StreamId = royale::StreamId;

using std::string;
using std::vector;

using TestFrameCollectorIndividual = royaletest::FixtureTestFrameCollector<BufferActionCalcIndividual>;
using TestFrameCollectorIndividualMixedMode = royaletest::FixtureTestFrameCollectorMixedMode<BufferActionCalcIndividual>;

/**
 * Basic test that captured frames result in calls to the listener.
 */
TEST_F (TestFrameCollectorIndividual, ValidBufferCallbacks)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    const uint16_t FRAME_COUNT = 100;
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }
    auto expectedCallbacks = FRAME_COUNT / m_useCase->getRawFrameCount();
    ASSERT_EQ (expectedCallbacks, m_listener->getCounterCallbacks (expectedCallbacks));
}

/**
 * Test using a mixed-mode use case with a 1:1 HT:ES ratio.
 */
TEST_F (TestFrameCollectorIndividualMixedMode, MixedModeValidBufferCallbacks)
{
    const uint16_t ratio = 1;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    const std::size_t expectedCallbacks = 10;
    const uint16_t FRAME_COUNT = static_cast<uint16_t> (expectedCallbacks * m_useCase->getRawFrameCount());
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    for (const auto &id : m_useCase->getStreamIds())
    {
        ASSERT_EQ (expectedCallbacks, m_listener->getStreamCallbacks (id, expectedCallbacks));
    }
    ASSERT_EQ (2 * expectedCallbacks, m_listener->getCounterCallbacks ());
}

/**
 * Test using a mixed-mode use case with a 5:1 HT:ES ratio.
 */
TEST_F (TestFrameCollectorIndividualMixedMode, MixedMode5to1)
{
    const uint16_t ratio = 5;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    const std::size_t expectedCycles = 10;
    const uint16_t FRAME_COUNT = static_cast<uint16_t> (expectedCycles * m_useCase->getRawFrameCount());
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    ASSERT_EQ (1 * expectedCycles, m_listener->getStreamCallbacks (m_esStreamId, 1 * expectedCycles));
    ASSERT_EQ (ratio * expectedCycles, m_listener->getStreamCallbacks (m_htStreamId, ratio * expectedCycles));
    ASSERT_EQ ( (ratio + 1) * expectedCycles, m_listener->getCounterCallbacks ());
}

/**
 * Test where one in every two frames is dropped, while using a mixed-mode use case with very high
 * HT:ES ratio.
 */
TEST_F (TestFrameCollectorIndividualMixedMode, MixedModeHighRatioWithEveryOtherFrameDrops)
{
    const uint16_t ratio = HIGH_HT_ES_RATIO;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    // Don't parse the UCD on every callback
    m_listener->setExpensiveTestsEnabled (false);

    // Drop every other frame
    auto drop = std::vector<bool> (m_useCase->getRawFrameCount(), false);
    for (std::size_t i = 0; i < drop.size(); i += 2)
    {
        drop[i] = true;
    }
    m_bridge->simulateFrameDrops (std::move (drop));

    // Run one cycle of the use case, with drops, then a second one with no drops.  As there are
    // only two cycles of the use case, the frame counter won't wrap round during the test, and
    // simulateFrameDrops() has only set up drops that will affect the first cycle.
    const uint16_t FRAME_COUNT = static_cast<uint16_t> (m_useCase->getRawFrameCount());
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        ASSERT_NO_THROW (m_bridge->generateBufferCallback (i));
    }
    m_bridge->simulateFrameDrops ({});
    for (uint16_t i = FRAME_COUNT ; i < static_cast<uint16_t> (2 * FRAME_COUNT); i++)
    {
        ASSERT_NO_THROW (m_bridge->generateBufferCallback (i));
    }

    // Checks for dropping every other frame of the first cycle
    // Here we only expect to receive frames for the second cycle
    const std::size_t expectedCycles = 1;
    const std::size_t expectedEsCallbacks = expectedCycles;
    const std::size_t expectedHtCallbacks = ratio * expectedCycles;
    ASSERT_EQ (expectedEsCallbacks, m_listener->getStreamCallbacks (m_esStreamId, expectedEsCallbacks));
    ASSERT_EQ (expectedHtCallbacks, m_listener->getStreamCallbacks (m_htStreamId, expectedHtCallbacks));
    ASSERT_EQ (expectedEsCallbacks + expectedHtCallbacks, m_listener->getCounterCallbacks ());
}

/**
 * Test using a mixed-mode use case with very high HT:ES ratio, where frames are being dropped but
 * some complete sets should still be received.
 *
 * This test drops one raw frame from every two HT sets, but none from the ES sets.
 *
 * One of the edge cases that this tests is that the FrameCollector releases buffers when the sets
 * that they're in can't be completed, even if the trigger buffer in the BufferActionMap is the one
 * that's dropped.  This test should fail if that isn't handled, because with the high HT:ES ratio
 * the simulated bridge will run out of buffers.
 */
TEST_F (TestFrameCollectorIndividualMixedMode, MixedModeHighRatioWithPatternedDrops)
{
    const uint16_t ratio = HIGH_HT_ES_RATIO;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    // Don't parse the UCD on every callback
    m_listener->setExpensiveTestsEnabled (false);

    // Drop one frame from every two HT sets, but none from the ES sets
    auto drop = std::vector<bool> (m_useCase->getRawFrameCount(), false);
    for (std::size_t i = 0; i < ratio; i += 2)
    {
        const auto rfsIdx = m_useCase->getRawFrameSetIndices (m_htStreamId, i).back();
        const auto frameIdx = m_useCase->getSequenceIndicesForRawFrameSet (rfsIdx).back();
        drop[frameIdx] = true;
    }
    m_bridge->simulateFrameDrops (std::move (drop));

    // Run one cycle of the use case, with drops, then a second one with no drops.  As there are
    // only two cycles of the use case, the frame counter won't wrap round during the test, and
    // simulateFrameDrops() has only set up drops that will affect the first cycle.
    const uint16_t FRAME_COUNT = static_cast<uint16_t> (m_useCase->getRawFrameCount());
    for (uint16_t i = 0 ; i < static_cast<uint16_t> (2 * FRAME_COUNT); i++)
    {
        ASSERT_NO_THROW (m_bridge->generateBufferCallback (i));
    }

    // Check if we dropped half the HT frame groups of the first cycle
    const std::size_t expectedCycles = 2;
    const std::size_t expectedEsCallbacks = expectedCycles;
    const std::size_t expectedHtCallbacks = ratio * (expectedCycles - 1) + (ratio / 2);
    ASSERT_EQ (expectedEsCallbacks, m_listener->getStreamCallbacks (m_esStreamId, expectedEsCallbacks));
    ASSERT_EQ (expectedHtCallbacks, m_listener->getStreamCallbacks (m_htStreamId, expectedHtCallbacks));
    ASSERT_EQ (expectedEsCallbacks + expectedHtCallbacks, m_listener->getCounterCallbacks ());
}

/**
 * Speed test of the frame collector.
 *
 * This test can take several minutes to run, which is why it's disabled by default. The time taken
 * depends mainly on the C++ runtime, for example debug builds with checked iterators, as well as
 * the CPU speed.
 */
TEST (TestFrameCollectorIndividualNoFixture, DISABLED_SpeedTest)
{
    std::unique_ptr<UseCaseFourPhase> useCase;
    useCase.reset (new UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    auto listener = std::make_shared<MockFrameCaptureListener> ();
    auto bridge = std::make_shared<BufferGeneratorStub> ();
    auto temperatureSensor = std::make_shared<StubTemperatureSensor>();

    // To test the speed, this needs to measure time without the overhead of the buffer allocation,
    // so a BufferGeneratorPregen is used for this test.
    auto pregenBuffers = std::make_shared<BufferGeneratorPregen> (bridge);
    bridge->setBufferCaptureListener (pregenBuffers.get());
    auto frameCollector = std::make_shared<FrameCollectorBase> (bridge->createInterpreter(), pregenBuffers, makeUnique<BufferActionCalcIndividual>());
    pregenBuffers->setBufferCaptureListener (frameCollector.get());
    listener->setCaptureReleaser (frameCollector);
    frameCollector->setCaptureListener (listener.get());
    frameCollector->setTemperatureSensor (temperatureSensor);

    // Don't parse the UCD on every callback, and use small buffers
    listener->setExpensiveTestsEnabled (false);
    bridge->reduceToMinimumImage (useCase.get());

    // The executeUseCase call ignores the BufferGeneratorPregen that's in the pipeline.  This is
    // not allowed by the documented assumptions of the IFrameCollector interface, instead
    // BufferGeneratorPregen should implement the full IBridgeDataReceiver interface and pass
    // through the executeUseCase() call.  However for a limited speed test this should be
    // acceptable.
    const auto bufferSizes = royale::Vector<std::size_t> {useCase->getRawFrameCount() };
    ASSERT_NO_THROW (bridge->configureFromUseCase (useCase.get(), bufferSizes));
    ASSERT_NO_THROW (frameCollector->executeUseCase (*bridge, useCase.get(), bufferSizes));

    // Set up the buffers in the BufferGeneratorPregen
    const uint16_t FRAME_COUNT = 100;
    bridge->setMaxBuffersInFlight (FRAME_COUNT);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        bridge->generateBufferCallback (i);
    }

    // now time how long it takes to collect those buffers
    const auto REPEAT_COUNT = 100000;
    for (auto i = 0 ; i < REPEAT_COUNT ; i++)
    {
        pregenBuffers->sendAllBuffers();
        // Wait until the buffers from the previous loop have been processed, so the number of
        // buffers waiting to be pushed to the listener is never more than 2 * FRAME_COUNT.
        auto previousLoopFinished = i * FRAME_COUNT / useCase->getRawFrameCount();
        listener->getCounterCallbacks (previousLoopFinished);
    }

    auto expectedCallbacks = REPEAT_COUNT * FRAME_COUNT / useCase->getRawFrameCount();
    ASSERT_EQ (expectedCallbacks, listener->getCounterCallbacks (expectedCallbacks));

    pregenBuffers->releaseAllBuffers();
}

/**
 * Some devices will pad captured buffers to a set size, in this system an individual frame's buffer
 * would be padded with dummy data.  The FrameCollector should only look at the frame identified in
 * the BufferActionMap, and completely ignore the padding.
 */
TEST_F (TestFrameCollectorIndividual, IndividualPaddedToSuper)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // We have to use the (frames, sequences, timestamp) version of generateBufferCallback,
    // the other versions will give the real frame of each "superframe" sequence index zero.
    //
    // The zeroth entry in these is used for the real frames, the rest are padding.
    std::vector<uint16_t> frames;
    std::vector<uint16_t> sequence;
    std::vector<uint16_t> reconfigs;
    for (uint16_t i = 0; i < 9; i++)
    {
        frames.push_back (i);
        sequence.push_back (i);
        reconfigs.push_back (0);
    }

    const uint16_t FRAME_COUNT = 100;
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        frames[0] = i;
        sequence[0] = narrow_cast<uint16_t> (i % m_useCase->getRawFrameCount());
        m_bridge->generateBufferCallback (frames, sequence, reconfigs, 0);
    }
    auto expectedCallbacks = FRAME_COUNT / m_useCase->getRawFrameCount();
    ASSERT_EQ (expectedCallbacks, m_listener->getCounterCallbacks (expectedCallbacks));
}

/**
 * Test that the temperature is reported.
 *
 * This doesn't test that it updates, only that it is read once.
 */
TEST_F (TestFrameCollectorIndividual, Temperature)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // Generate exactly one callback, and wait for the threads to process it
    const auto FRAME_COUNT = m_useCase->getRawFrameCount();
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }
    ASSERT_EQ (1u, m_listener->getCounterCallbacks (1u));

    ASSERT_FLOAT_EQ (m_listener->getLastTemperature(), m_temperatureSensor->getTemperature());
}

/**
 * Test that hardware-provided timestamps are used.
 */
TEST_F (TestFrameCollectorIndividual, HardwareTimestamps)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // Generate exactly one callback, and wait for the threads to process it
    const auto FRAME_COUNT = m_useCase->getRawFrameCount();
    const auto BASE_TIME = std::chrono::seconds (0x123456L);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        // frame numbers and sequence numbers can be the same for this test
        std::vector<uint16_t> numbers;
        std::vector<uint16_t> reconfigs;
        numbers.push_back (i);
        reconfigs.push_back (0);
        m_bridge->generateBufferCallback (numbers, numbers, reconfigs,
                                          std::chrono::duration_cast<std::chrono::microseconds> (BASE_TIME + std::chrono::seconds (i)).count());
    }
    ASSERT_EQ (1u, m_listener->getCounterCallbacks (1u));

    // The second test is strictly less-than, FRAME_COUNT is more than the loop variable was
    auto duration = m_listener->getLastTimestamp();
    ASSERT_GE (duration, BASE_TIME);
    ASSERT_LT (duration, BASE_TIME + std::chrono::seconds (FRAME_COUNT));
}

/**
 * Test that local timestamps are used, if hardware-provided timestamps aren't present.
 */
TEST_F (TestFrameCollectorIndividual, SoftwareTimestamps)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    auto startTime = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
    // Generate exactly one callback, and wait for the threads to process it
    const auto FRAME_COUNT = m_useCase->getRawFrameCount();
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        // frame numbers and sequence numbers can be the same for this test
        std::vector<uint16_t> numbers;
        std::vector<uint16_t> reconfigs;
        numbers.push_back (i);
        reconfigs.push_back (0);
        m_bridge->generateBufferCallback (numbers, numbers, reconfigs, 0u);
    }
    ASSERT_EQ (1u, m_listener->getCounterCallbacks (1u));
    auto stopTime = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());

    ASSERT_GE (m_listener->getLastTimestamp(), startTime);
    ASSERT_LE (m_listener->getLastTimestamp(), stopTime);
}

/**
 * Test that the IFrameCollector provides a thread, and doesn't block the Bridge's thread.
 *
 * This test takes 600ms to fail, and when it passes it will be faster.
 */
TEST_F (TestFrameCollectorIndividual, SeparateThread)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    auto listener = std::make_shared<SlowFrameCaptureListener> (std::chrono::milliseconds {100});
    replaceTestListener (listener);

    const auto FRAME_COUNT = narrow_cast<uint16_t> (6 * m_useCase->getRawFrameCount());
    m_bridge->setMaxBuffersInFlight (FRAME_COUNT);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    auto expectedCallbacks = FRAME_COUNT / m_useCase->getRawFrameCount();

    // Check that we didn't get blocked
    ASSERT_GE (expectedCallbacks / 2, listener->getCounterCallbacks());
    listener->runFast();
    ASSERT_EQ (expectedCallbacks, listener->getCounterCallbacks (expectedCallbacks));
}

/**
 * Test the shutdown code - check that the FrameCollector can return all the buffers to the Bridge
 * before the Bridge exits.
 */
TEST_F (TestFrameCollectorIndividual, ReleasingAllBuffers)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // This SlowFrameCaptureListener's delay is much less than MockFrameCaptureListener::THREAD_WAIT_TIME.  We can queue
    // up lots of buffers, tell the FrameCollector to release all the buffers (which will block
    // waiting for SlowFrameCaptureListener::captureCallback to return) and still expect it to complete before
    // MockFrameCaptureListener::getCounterCallbacks(expectedCallbacks) times out.
    auto listener = std::make_shared<SlowFrameCaptureListener> (std::chrono::milliseconds {MockFrameCaptureListener::THREAD_WAIT_TIME / 3});
    replaceTestListener (listener);

    // 5 complete and 1 incomplete sequence, releaseAllBuffers should release them all
    const auto FRAME_COUNT = narrow_cast<uint16_t> (6 * m_useCase->getRawFrameCount() - 1);
    m_bridge->setMaxBuffersInFlight (FRAME_COUNT);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    m_frameCollector->releaseAllBuffers();
    // this one shouldn't need the wait that's in checkCounterBuffersBalance()
    ASSERT_EQ (m_bridge->getCounterBuffersDequeued(), m_bridge->getCounterBuffersQueued());
}

/**
 * Test the multithreading / async listener support
 */
TEST_F (TestFrameCollectorIndividual, AsyncProcessing)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // A very long delay, but releaseAllFrames() will skip it
    auto listener = std::make_shared<AsyncFrameCaptureListener> (std::chrono::milliseconds {10000});
    replaceTestListener (listener);

    // 5 complete and 1 incomplete sequence, releaseAllBuffers should release them all
    const auto FRAME_COUNT = narrow_cast<uint16_t> (6 * m_useCase->getRawFrameCount() - 1);
    m_bridge->setMaxBuffersInFlight (FRAME_COUNT);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    auto expectedCallbacks = FRAME_COUNT / m_useCase->getRawFrameCount();

    // Wait for all the buffers to be in the callback
    ASSERT_EQ (expectedCallbacks, listener->getQueueSize (expectedCallbacks));
    ASSERT_EQ (0u, m_listener->getCounterCallbacks (0u));
    // releaseAllBuffers should filter through to releaseAllFrames
    m_frameCollector->releaseAllBuffers();
    ASSERT_EQ (0u, listener->getQueueSize (0u));
    ASSERT_EQ (expectedCallbacks, m_listener->getCounterCallbacks (expectedCallbacks));
}

/**
 * Test for race conditions while switching between listeners.
 *
 * In this test, the listener itself calls to change to a different listener.
 */
TEST_F (TestFrameCollectorIndividual, ChangingListenersFromCallbackThread)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    /**
     * When this class receives a callback, it calls the FrameCollector's setCaptureListener()
     * method with a semi-randomly selected listener.
     *
     * It takes a non-owning pointer to the list of possible listeners, which may include the
     * SwapperFrameCaptureListener itself.  For thread safety, it's assumed that this list stops
     * changing before the first captureCallback, and that members of the list aren't destroyed
     * until after the last captureCallback.
     */
    class SwapperFrameCaptureListener : public MockFrameCaptureListener
    {
    public:
        SwapperFrameCaptureListener (std::shared_ptr<IFrameCollector> frameCollector,
                                     const std::vector<std::shared_ptr<MockFrameCaptureListener>> *candidates) :
            m_frameCollector {frameCollector},
            m_candidates {candidates}
        {
        }

        void captureCallback (std::vector<ICapturedRawFrame *> &frames,
                              const UseCaseDefinition &definition,
                              royale::StreamId streamId,
                              std::unique_ptr<const CapturedUseCase> capturedCase)
        {
            MockFrameCaptureListener::captureCallback (frames, definition, streamId, std::move (capturedCase));
            auto &newListener = m_candidates->at (m_random() % m_candidates->size());
            m_frameCollector->setCaptureListener (newListener.get());
        }

    private:
        std::shared_ptr<IFrameCollector> m_frameCollector;
        const std::vector<std::shared_ptr<MockFrameCaptureListener>> *m_candidates;
        std::random_device m_random;
    };
    std::vector<std::shared_ptr<MockFrameCaptureListener> > listeners;
    listeners.push_back (std::make_shared<SwapperFrameCaptureListener> (m_frameCollector, &listeners));
    listeners.push_back (std::make_shared<SwapperFrameCaptureListener> (m_frameCollector, &listeners));
    listeners.push_back (std::make_shared<SwapperFrameCaptureListener> (m_frameCollector, &listeners));
    listeners.push_back (std::make_shared<SwapperFrameCaptureListener> (m_frameCollector, &listeners));
    listeners.push_back (std::make_shared<SwapperFrameCaptureListener> (m_frameCollector, &listeners));
    for (auto &listener : listeners)
    {
        listener->setCaptureReleaser (m_frameCollector);
        listener->setExpensiveTestsEnabled (false);
    }
    m_frameCollector->setCaptureListener (listeners.at (0).get());

    // 99 complete and 1 incomplete sequence
    const auto FRAME_COUNT = narrow_cast<uint16_t> (100 * m_useCase->getRawFrameCount() - 1);
    m_bridge->setMaxBuffersInFlight (FRAME_COUNT);
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    auto expectedCallbacks = FRAME_COUNT / m_useCase->getRawFrameCount();

    // Wait for all the buffers to trigger callbacks
    const auto timeoutTime = std::chrono::steady_clock::now() + std::chrono::seconds (5);
    std::size_t receivedCallbacks = 0u;
    while (std::chrono::steady_clock::now() < timeoutTime)
    {
        receivedCallbacks = 0u;
        for (auto &listener : listeners)
        {
            receivedCallbacks += listener->getCounterCallbacks (0u);
        }
        if (expectedCallbacks == receivedCallbacks)
        {
            break;
        }
        m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.
        if (expectedCallbacks == receivedCallbacks + m_framesDroppedCollector / m_useCase->getRawFrameCount())
        {
            // Record a failure, because frames dropped.  But maybe this should be counted as a
            // success.  Either way, don't block waiting for these frames).
            EXPECT_EQ (0u, m_framesDroppedCollector);
            break;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (1));
    }

    // The vector of listeners will go out of scope before the FrameCollector.  With the SwapperFCL,
    // there's a reference to the vector of listeners in the listeners themselves.  This means that
    // the bare pointer in the FrameCollector will become dangling unless its lifetime is extended
    // by being held in m_listener.  But if we put a SwapperFCL in to m_listener, that SwapperFCL
    // will itself have a dangling reference to the vector of SwapperFCLs, and there may still be
    // callbacks queued waiting for the FrameCollector's conveyance thread.
    replaceTestListener (std::make_shared<MockFrameCaptureListener> ());

    // Code that swaps listeners has responsibility for calling releaseAllFrames() on all listeners
    // except the one that is the FrameCollector's current listener.
    // Removing this code will give a unit test for ROYAL-2102, if that task is to be implemented.
    for (auto &listener : listeners)
    {
        listener->releaseAllFrames();
    }

    ASSERT_EQ (expectedCallbacks, receivedCallbacks);
}

/**
 * Tests that correct exposure times are reported.  This pushes simulated captured frames through the Bridge,
 * sets the exposure time that will be reported in future, and checks what the results are.
 */
TEST_F (TestFrameCollectorIndividual, DelayedExposureChanges)
{
    {
        // Some of the UseCase's arguments are given, the values are arbitrary but must
        // allow all the exposure times used in this test to be set, and to enable the
        // gray exposure.
        auto useCase = std::unique_ptr<UseCaseDefinition> (new UseCaseEightPhase {5u, 30000000, 30000000, {50, 1000}, 50, 51, 52});
        auto measurementBlocks = royale::Vector<std::size_t> {useCase->getRawFrameCount() };
        setupUseCase (std::move (useCase), std::move (measurementBlocks));
    }

    vector<uint32_t> initialExposureValues;
    vector<uint32_t> exposureValuesOfThirdCapture;
    vector<uint32_t> exposureValuesOfFourthCapture;
    for (const auto time : m_useCase->getExposureTimes())
    {
        initialExposureValues.push_back (time);
        exposureValuesOfThirdCapture.push_back (time + 300);
        exposureValuesOfFourthCapture.push_back (time + 400);
    }

    // Note: exposure times in m_useCase aren't updated (they are not used in the following)

    m_frameCollector->setReportedExposureTimes (4095, initialExposureValues);
    m_frameCollector->setReportedExposureTimes (0, exposureValuesOfThirdCapture);
    m_frameCollector->setReportedExposureTimes (1, exposureValuesOfFourthCapture);

    // This test needs a consistent frame counter through all of the tests,
    // but it won't need to wrap-round during the test.
    uint16_t frameCounter = 0;

    // Trigger several captures, and check the reported values for each one
    vector <vector<uint32_t> *> expectedExposures =
    {
        &initialExposureValues,
        &initialExposureValues, // second capture should be the same as the first
        &exposureValuesOfThirdCapture,
        &exposureValuesOfFourthCapture,
        &exposureValuesOfFourthCapture // and a fifth capture is the same as the fourth
    };

    // This uses the behavior of the AIO firmware in v3.3.0 and v2.3.1, the first frame with the new
    // exposures is the first frame with the new reconfIndex.
    // The first reconfigure happens at the end of the second sequence
    const uint16_t firstReconfig = (2 * 9);
    const uint16_t secondReconfig = (3 * 9);
    const uint16_t thirdReconfig = (4 * 9);
    uint16_t reconfigIndex = 0;

    for (const auto expectation : expectedExposures)
    {
        auto expectedCallbacks = m_listener->getCounterCallbacks() + 1;
        for (int i = 0; i < static_cast<int> (m_useCase->getRawFrameCount()); i++)
        {
            if (firstReconfig == frameCounter)
            {
                reconfigIndex++;
            }

            if (secondReconfig == frameCounter)
            {
                reconfigIndex++;
            }

            if (thirdReconfig == frameCounter)
            {
                reconfigIndex++;
            }

            m_bridge->generateBufferCallback (frameCounter, reconfigIndex);
            frameCounter++;
        }
        // the next line will wait for the IFrameCollector's thread
        ASSERT_EQ (expectedCallbacks, m_listener->getCounterCallbacks (expectedCallbacks));
        for (size_t i = 0; i < m_useCase->getRawFrameSets().size(); i++)
        {
            //LOG (DEBUG) << "Comparing expected exposures: " << (*expectation)[i] << ", " << m_listener->getLastExposureTimes ()[i];
            ASSERT_EQ ( (*expectation) [i], m_listener->getLastExposureTimes() [i]);
        }
    }
}

/**
* Test the raw frame statistics.
*/
TEST_F (TestFrameCollectorIndividual, RawFrameStats)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    ASSERT_EQ (0u, m_framesTotal);
    ASSERT_EQ (0u, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    auto ucRawFrames = m_useCase->getRawFrameCount();
    uint16_t frameCounter = 0;

    // Generate one frame set (good case)
    for (size_t i = 0; i < ucRawFrames; ++i)
    {
        m_bridge->generateBufferCallback (frameCounter++);
    }
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    ASSERT_EQ (ucRawFrames, m_framesTotal);
    ASSERT_EQ (0u, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    // Skip a complete frame set.
    frameCounter = narrow_cast<uint16_t> (frameCounter + ucRawFrames);

    // Generate another frame set (bridge frame drop case)
    for (size_t i = 0; i < ucRawFrames; ++i)
    {
        m_bridge->generateBufferCallback (frameCounter++);
    }
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    ASSERT_EQ (ucRawFrames * 3, m_framesTotal);
    ASSERT_EQ (ucRawFrames, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    // Generate a frame set with the first frame missing.
    frameCounter++;
    for (size_t i = 1; i < ucRawFrames; ++i)
    {
        m_bridge->generateBufferCallback (frameCounter++);
    }
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    ASSERT_EQ (ucRawFrames * 4, m_framesTotal);
    ASSERT_EQ (ucRawFrames + 1, m_framesDroppedBridge);
    ASSERT_EQ (ucRawFrames - 1, m_framesDroppedCollector);

    // Generate another frame set, to see if the FC accepts it as it should.
    for (size_t i = 0; i < ucRawFrames; ++i)
    {
        m_bridge->generateBufferCallback (frameCounter++);
    }
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    ASSERT_EQ (ucRawFrames * 5, m_framesTotal);
    ASSERT_EQ (ucRawFrames + 1, m_framesDroppedBridge);
    ASSERT_EQ (ucRawFrames - 1, m_framesDroppedCollector);
}

/**
 * Test that an exception from queueBuffer is handled.  This test was added because queueBuffer was
 * called from BufferHolder's destructor, so could result in an exception from a destructor.
 */
TEST_F (TestFrameCollectorIndividual, ExceptionWhileRequeueing)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());
    m_bridge->simulateExceptionInQueueBuffer (true);

    const uint16_t FRAME_COUNT = 100;
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        m_bridge->generateBufferCallback (i);
    }

    // Wait for two frame groups to be processed, which means the first group should already have
    // been requeued.
    ASSERT_NO_THROW (m_listener->getCounterCallbacks (2));
}
