/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <FixtureTestFrameCollector.hpp>

using namespace royaletest;

void ImplFixtureTestFrameCollector::SetUp()
{
    ASSERT_TRUE (m_frameCollector);
    m_listener->setCaptureReleaser (m_frameCollector);
    m_frameCollector->setCaptureListener (m_listener.get());
    m_bridge->setBufferCaptureListener (m_frameCollector.get());
    m_temperatureSensor = std::make_shared<royale::stub::hal::StubTemperatureSensor>();
    m_frameCollector->setTemperatureSensor (m_temperatureSensor);
    m_frameCollector->setEventListener (this);
}

void ImplFixtureTestFrameCollector::setupUseCase (std::unique_ptr<royale::usecase::UseCaseDefinition> useCase, const royale::Vector<std::size_t> &measurementBlocks)
{
    m_bridge->reduceToMinimumImage (useCase.get());
    m_useCase = std::move (useCase);
    m_measurementBlocks = std::move (measurementBlocks);
    ASSERT_NO_THROW (m_bridge->configureFromUseCase (m_useCase.get(), m_measurementBlocks));
    ASSERT_NO_THROW (m_frameCollector->executeUseCase (*m_bridge, m_useCase.get(), m_measurementBlocks));
    ASSERT_NO_THROW (m_bridge->checkExecuteFromUseCaseCalled());
}

void ImplFixtureTestFrameCollector::setupUseCaseDefault ()
{
    auto useCase = std::unique_ptr<royale::usecase::UseCaseDefinition> (new royale::usecase::UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    auto measurementBlocks = royale::Vector<std::size_t> {useCase->getRawFrameCount() };
    setupUseCase (std::move (useCase), std::move (measurementBlocks));
}

void ImplFixtureTestFrameCollector::replaceTestListener (std::shared_ptr<royale::stub::hal::MockFrameCaptureListener> listener)
{
    // Hold a refcount until this function exits
    auto oldListener = m_listener;
    // Set the new listener, even if the following calls throw
    m_listener = listener;
    // It's valid for both listeners to have the same releaser
    listener->setCaptureReleaser (m_frameCollector);
    m_frameCollector->setCaptureListener (listener.get());
    // The destructor of oldListener should release any frames in that listener,
    // which will happen now unless the test is deliberately holding a refcount to
    // extend the life of the oldListener.
}

void ImplFixtureTestFrameCollector::TearDown()
{
    m_frameCollector->releaseAllBuffers();
    m_listener->checkForThreadedAssert();
    ASSERT_TRUE (m_bridge->checkCounterBuffersBalance());
}

void ImplFixtureTestFrameCollector::onEvent (std::unique_ptr<royale::IEvent> &&event)
{
    auto stats = dynamic_cast<royale::event::EventRawFrameStats *> (event.get());
    if (stats)
    {
        m_framesTotal += stats->m_totalFrames;
        m_framesDroppedBridge += stats->m_frameDropsBridge;
        m_framesDroppedCollector += stats->m_frameDropsCollector;
    }
}

void ImplFixtureTestFrameCollectorMixedMode::setupUseCaseMixedXHt (uint16_t ratio)
{
    // This scope ends when useCase and mutableBufferSizes move the m_ variables
    {
        auto useCase = std::unique_ptr<royale::usecase::UseCaseDefinition> (new royale::usecase::UseCaseMixedXHt (5u, ratio, 30000000, 30000000, 20200000, {50, 1000}, {50, 1000}, 50, 51, 52, 53, 54));
        auto mutableBufferSizes = royale::Vector<std::size_t> (ratio, 5); // the HT frames
        mutableBufferSizes.emplace_back (4); // the frames of ES1
        mutableBufferSizes.emplace_back (5); // the frames of ES2
        ASSERT_NO_FATAL_FAILURE (setupUseCase (std::move (useCase), std::move (mutableBufferSizes)));
    }

    const auto streamIds = m_useCase->getStreamIds();
    ASSERT_EQ (2u, streamIds.size()) << "Test expectation failed, expected two streams in this usecase";

    // Recognise which stream is which. The HT frames are clock-aligned, so the stream with
    // frame 0 must be the HT stream.
    if (m_useCase->getRawFrameSetIndices (streamIds.at (0), 0).front() == 0u)
    {
        m_htStreamId = streamIds.at (0);
        m_esStreamId = streamIds.at (1);
    }
    else
    {
        m_htStreamId = streamIds.at (1);
        m_esStreamId = streamIds.at (0);
    }
    ASSERT_EQ (0u, m_useCase->getRawFrameSetIndices (m_htStreamId, 0).front());
    ASSERT_EQ (ratio, m_useCase->getFrameGroupCount (m_htStreamId));
    ASSERT_EQ (1u, m_useCase->getFrameGroupCount (m_esStreamId));
}
