/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <collector/BufferActionCalcSuper.hpp>
#include <royale/IEventListener.hpp>
#include <common/events/EventRawFrameStats.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseMixedXHt.hpp>

#include <BufferGeneratorStub.hpp>
#include <FixtureTestFrameCollector.hpp>
#include <StubTemperatureSensor.hpp>
#include <MockFrameCaptureListener.hpp>

#include <gtest/gtest.h>

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

using TestFrameCollectorSuper = royaletest::FixtureTestFrameCollector<BufferActionCalcSuper>;
using TestFrameCollectorSuperMixedMode = royaletest::FixtureTestFrameCollectorMixedMode<BufferActionCalcSuper>;

/**
 * Basic test that captured frames result in calls to the listener.
 */
TEST_F (TestFrameCollectorSuper, ValidBufferCallbacks)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // This test uses a consistent frame counter through all of the tests,
    // but it won't need to wrap-round during the test.
    uint16_t frameCounter = 0;

    const uint16_t BUFFER_COUNT = 10;
    for (uint16_t i = 0 ; i < BUFFER_COUNT; i++)
    {
        m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
    }
    ASSERT_EQ (BUFFER_COUNT, m_listener->getCounterCallbacks (BUFFER_COUNT));
}

/**
 * Test using a mixed-mode use case with a 1:1 HT:ES ratio.
 */
TEST_F (TestFrameCollectorSuperMixedMode, MixedModeValidBufferCallbacks)
{
    const uint16_t ratio = 1;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    // This test uses a consistent frame counter through all of the tests,
    // but it won't need to wrap-round during the test.
    uint16_t frameCounter = 0;

    const std::size_t expectedCallbacks = 10;
    const uint16_t CYCLE_COUNT = static_cast<uint16_t> (expectedCallbacks);
    for (uint16_t i = 0 ; i < CYCLE_COUNT; i++)
    {
        m_bridge->generateSizesStart (m_measurementBlocks, frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
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
TEST_F (TestFrameCollectorSuperMixedMode, MixedMode5to1)
{
    const uint16_t ratio = 5;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    // This test uses a consistent frame counter through all of the tests,
    // but it won't need to wrap-round during the test.
    uint16_t frameCounter = 0;

    const std::size_t expectedCycles = 10;
    const uint16_t CYCLE_COUNT = static_cast<uint16_t> (expectedCycles);
    for (uint16_t i = 0 ; i < CYCLE_COUNT; i++)
    {
        m_bridge->generateSizesStart (m_measurementBlocks, frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
    }

    ASSERT_EQ (1 * expectedCycles, m_listener->getStreamCallbacks (m_esStreamId, 1 * expectedCycles));
    ASSERT_EQ (ratio * expectedCycles, m_listener->getStreamCallbacks (m_htStreamId, ratio * expectedCycles));
    ASSERT_EQ ( (ratio + 1) * expectedCycles, m_listener->getCounterCallbacks ());
}

/**
 * Test using a mixed-mode use case with very high HT:ES ratio, where frames are being dropped.
 *
 * The FCIndividual tests have both this and MixedModeHighRatioWithEveryOtherFrameDrops, but only
 * one makes sense for the FCSuper. The one that drops every other frame in individual mode would
 * drop every buffer in superframe mode.
 */
TEST_F (TestFrameCollectorSuperMixedMode, MixedModeHighRatioWithPatternedDrops)
{
    const uint16_t ratio = HIGH_HT_ES_RATIO;
    ASSERT_NO_FATAL_FAILURE (setupUseCaseMixedXHt (ratio));

    // Don't parse the UCD on every callback
    m_listener->setExpensiveTestsEnabled (false);

    // Drop one frame from every two HT sets, but none from the ES sets
    auto drop = std::vector<bool> (m_useCase->getRawFrameCount(), false);
    for (std::size_t i = 0; i < ratio; i += 2)
    {
        // With superframes it is unimportant which raw frame we add to the drop list, the simulated
        // bridge will drop the whole HT superframe. The last frame is chosen to match the
        // FCIndividual test.
        const auto rfsIdx = m_useCase->getRawFrameSetIndices (m_htStreamId, i).back();
        const auto frameIdx = m_useCase->getSequenceIndicesForRawFrameSet (rfsIdx).back();
        drop[frameIdx] = true;
    }
    m_bridge->simulateFrameDrops (std::move (drop));

    // Run one cycle of the use case, with drops, then a second one with no drops.  As there are
    // only two cycles of the use case, the frame counter won't wrap round during the test, and
    // simulateFrameDrops() has only set up drops that will affect the first cycle.
    ASSERT_NO_THROW (m_bridge->generateSizesStart (m_measurementBlocks, 0));
    ASSERT_NO_THROW (m_bridge->generateSizesStart (m_measurementBlocks, narrow_cast<uint16_t> (m_useCase->getRawFrameCount())));

    // Check if we dropped half the HT frame groups of the first cycle
    const std::size_t expectedCycles = 2;
    const std::size_t expectedEsCallbacks = expectedCycles;
    const std::size_t expectedHtCallbacks = ratio * (expectedCycles - 1) + (ratio / 2);
    ASSERT_EQ (expectedEsCallbacks, m_listener->getStreamCallbacks (m_esStreamId, expectedEsCallbacks));
    ASSERT_EQ (expectedHtCallbacks, m_listener->getStreamCallbacks (m_htStreamId, expectedHtCallbacks));
    ASSERT_EQ (expectedEsCallbacks + expectedHtCallbacks, m_listener->getCounterCallbacks ());
}

/**
 * Some devices will pad captured buffers to a set size, in this system an individual frame's buffer
 * would be padded with dummy data.  The FrameCollector should only look at the first frames in
 * the buffer and then ignore the padding afterwards.  The logic for this is the BufferActionMap
 * generated by BufferActionCalcSuper.
 */
TEST_F (TestFrameCollectorSuper, SuperPaddedToBiggerSuper)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // This test uses a consistent frame counter through all of the tests,
    // but it won't need to wrap-round during the test.
    uint16_t frameCounter = 0;

    const uint16_t BUFFER_COUNT = 10;
    for (uint16_t i = 0 ; i < BUFFER_COUNT; i++)
    {
        std::vector<uint16_t> frameNumbers;
        std::vector<uint16_t> sequenceIndices;
        for (uint16_t j = 0; j < m_useCase->getRawFrameCount() ; j++)
        {
            // add a real frame
            frameNumbers.push_back (frameCounter++);
            sequenceIndices.push_back (j);
        }
        for (auto j = m_useCase->getRawFrameCount() ; j < 9 ; j++)
        {
            // add a dummy frame
            frameNumbers.push_back (0);
            sequenceIndices.push_back (0);
        }
        m_bridge->generateBufferCallback (frameNumbers);
    }
    ASSERT_EQ (BUFFER_COUNT, m_listener->getCounterCallbacks (BUFFER_COUNT));
}

/**
 * Test that the temperature is reported.
 *
 * This doesn't test that it updates, only that it is read once.
 */
TEST_F (TestFrameCollectorSuper, Temperature)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // Generate exactly one callback, and wait for the threads to process it
    m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), 0);
    ASSERT_EQ (1u, m_listener->getCounterCallbacks (1u));

    ASSERT_FLOAT_EQ (m_temperatureSensor->getTemperature(), m_listener->getLastTemperature());
}

/**
 * Test that hardware-provided timestamps are used.
 */
TEST_F (TestFrameCollectorSuper, HardwareTimestamps)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // Generate exactly one callback, and wait for the threads to process it
    const auto FRAME_COUNT = m_useCase->getRawFrameCount();
    const auto BASE_TIME = std::chrono::seconds (0x123456L);
    // frame numbers and sequence numbers can be the same for this test
    std::vector<uint16_t> numbers;
    std::vector<uint16_t> reconfigs;
    for (uint16_t i = 0 ; i < FRAME_COUNT; i++)
    {
        numbers.push_back (i);
        reconfigs.push_back (0);
    }
    m_bridge->generateBufferCallback (numbers, numbers, reconfigs,
                                      std::chrono::duration_cast<std::chrono::microseconds> (BASE_TIME + std::chrono::seconds (FRAME_COUNT / 2)).count());
    ASSERT_EQ (1u, m_listener->getCounterCallbacks (1u));

    // The second test is strictly less-than, FRAME_COUNT is more than the loop variable was
    auto duration = m_listener->getLastTimestamp();
    ASSERT_GE (duration, BASE_TIME);
    ASSERT_LT (duration, BASE_TIME + std::chrono::seconds (FRAME_COUNT));
}

/**
 * Test that local timestamps are used, if hardware-provided timestamps aren't present.
 */
TEST_F (TestFrameCollectorSuper, SoftwareTimestamps)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    auto startTime = std::chrono::duration_cast<std::chrono::microseconds> (CapturedUseCase::CLOCK_TYPE::now().time_since_epoch());
    // Generate exactly one callback, and wait for the threads to process it
    m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), 0);
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
TEST_F (TestFrameCollectorSuper, SeparateThread)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    auto listener = std::make_shared<SlowFrameCaptureListener> (std::chrono::milliseconds {100});
    replaceTestListener (listener);

    uint16_t frameCounter = 0;
    const uint16_t BUFFER_COUNT = 6;
    m_bridge->setMaxBuffersInFlight (BUFFER_COUNT);
    for (uint16_t i = 0; i < BUFFER_COUNT; i++)
    {
        m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
    }

    auto expectedCallbacks = static_cast<unsigned int> (BUFFER_COUNT);

    // Check that we didn't get blocked
    ASSERT_GE (expectedCallbacks / 2, listener->getCounterCallbacks());
    listener->runFast();
    ASSERT_EQ (expectedCallbacks, listener->getCounterCallbacks (expectedCallbacks));
}

/**
 * Test the shutdown code - check that the FrameCollector can return all the buffers to the Bridge
 * before the Bridge exits.
 */
TEST_F (TestFrameCollectorSuper, ReleasingAllBuffers)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    // This SlowFrameCaptureListener's delay is much less than MockFrameCaptureListener::THREAD_WAIT_TIME.  We can queue
    // up lots of buffers, tell the FrameCollector to release all the buffers (which will block
    // waiting for SlowFrameCaptureListener::captureCallback to return) and still expect it to complete before
    // MockFrameCaptureListener::getCounterCallbacks(expectedCallbacks) times out.
    auto listener = std::make_shared<SlowFrameCaptureListener> (std::chrono::milliseconds {MockFrameCaptureListener::THREAD_WAIT_TIME / 3});
    replaceTestListener (listener);

    uint16_t frameCounter = 0;
    const uint16_t BUFFER_COUNT = 6;
    m_bridge->setMaxBuffersInFlight (BUFFER_COUNT);
    for (uint16_t i = 0; i < BUFFER_COUNT; i++)
    {
        m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
    }

    m_frameCollector->releaseAllBuffers();
    // this one shouldn't need the wait that's in checkCounterBuffersBalance()
    ASSERT_EQ (m_bridge->getCounterBuffersDequeued(), m_bridge->getCounterBuffersQueued());
}

/**
 * Tests that correct exposure times are reported.  This pushes simulated captured frames through the Bridge,
 * sets the exposure time that will be reported in future, and checks what the results are.
 */
TEST_F (TestFrameCollectorSuper, DelayedExposureChanges)
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

        std::vector<uint16_t> frameNumbers;
        std::vector<uint16_t> sequenceIndices;
        std::vector<uint16_t> reconfigs;
        for (auto i = 0u; i < m_useCase->getRawFrameCount(); i++)
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

            frameNumbers.push_back (frameCounter);
            sequenceIndices.push_back (static_cast<uint16_t> (i));
            reconfigs.push_back (reconfigIndex);
            frameCounter++;
        }
        m_bridge->generateBufferCallback (frameNumbers, sequenceIndices, reconfigs, 0u);

        // the next line will wait for the IFrameCollector's thread
        ASSERT_EQ (expectedCallbacks, m_listener->getCounterCallbacks (expectedCallbacks));
        for (size_t i = 0; i < m_useCase->getRawFrameSets().size(); i++)
        {
            //LOG (DEBUG) << "Comparing expected exposures: " << (*expectation)[i] << ", " << m_listener.getLastExposureTimes ()[i];
            ASSERT_EQ ( (*expectation) [i], m_listener->getLastExposureTimes() [i]);
        }
    }
}

/**
* Test the raw frame statistics.
*/
TEST_F (TestFrameCollectorSuper, RawFrameStats)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());

    ASSERT_EQ (0u, m_framesTotal);
    ASSERT_EQ (0u, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    auto ucRawFrames = m_useCase->getRawFrameCount();
    uint16_t frameCounter = 0;

    // Generate one frame set (good case)
    m_bridge->generateSizeStart (ucRawFrames, frameCounter);
    frameCounter = narrow_cast<uint16_t> (frameCounter + ucRawFrames);
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    ASSERT_EQ (ucRawFrames, m_framesTotal);
    ASSERT_EQ (0u, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    // Skip a complete frame set.
    frameCounter = narrow_cast<uint16_t> (frameCounter + ucRawFrames);

    // Generate another frame set (bridge frame drop case)
    m_bridge->generateSizeStart (ucRawFrames, frameCounter);
    m_frameCollector->flushRawFrameStatistics (true); // force the FC to generate events.

    // If you extend this test to generate another set of frames, remember to add
    // frameCounter = narrow_cast<uint16_t> (frameCounter + ucRawFrames);
    // to the previous block.

    ASSERT_EQ (ucRawFrames * 3, m_framesTotal);
    ASSERT_EQ (ucRawFrames, m_framesDroppedBridge);
    ASSERT_EQ (0u, m_framesDroppedCollector);

    // Not tested: single raw frame drop. Doesn't happen with super frames.
}

/**
 * Test that an exception from queueBuffer is handle.  This test was added because queueBuffer was
 * called from BufferHolder's destructor, so could result in an exception from a destructor.
 */
TEST_F (TestFrameCollectorSuper, ExceptionWhileRequeueing)
{
    ASSERT_NO_FATAL_FAILURE (setupUseCaseDefault());
    m_bridge->simulateExceptionInQueueBuffer (true);

    uint16_t frameCounter = 0;

    const uint16_t BUFFER_COUNT = 10;
    for (uint16_t i = 0 ; i < BUFFER_COUNT; i++)
    {
        m_bridge->generateSizeStart (m_useCase->getRawFrameCount(), frameCounter);
        frameCounter = narrow_cast<uint16_t> (frameCounter + m_useCase->getRawFrameCount());
    }

    // Wait for two frame groups to be processed, which means the first group should already have
    // been requeued.
    ASSERT_NO_THROW (m_listener->getCounterCallbacks (2));
}
