/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <collector/FrameCollectorBase.hpp>
#include <royale/IEventListener.hpp>
#include <common/events/EventRawFrameStats.hpp>
#include <common/MakeUnique.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseMixedXHt.hpp>

#include <BufferGeneratorStub.hpp>
#include <StubTemperatureSensor.hpp>
#include <MockFrameCaptureListener.hpp>

#include <gtest/gtest.h>

#include <royale/Vector.hpp>

namespace royaletest
{
    /**
     * Fixture for running most of the FrameCollectorIndividual and FrameCollectorSuperframe tests.
     *
     * The template subclass FixtureTestFrameCollector<FC> should be used, as it sets
     * m_frameCollector to the appropriate IFrameCollector subclass.
     *
     * Each test must set m_useCase, either setting it itself or by calling setupUseCase.
     */
    class ImplFixtureTestFrameCollector : public ::testing::Test, private royale::IEventListener
    {
    protected:
        explicit ImplFixtureTestFrameCollector () :
            m_listener {std::make_shared<royale::stub::hal::MockFrameCaptureListener> () },
            m_bridge {std::make_shared<royale::stub::hal::BufferGeneratorStub> () },
            m_frameCollector {nullptr}, // must be overridden by the subclass
            m_framesTotal (0),
            m_framesDroppedBridge (0),
            m_framesDroppedCollector (0)
        {
        }

        ~ImplFixtureTestFrameCollector() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * Set m_useCase, and configure m_bridge and m_frameCollector to use it.
         */
        void setupUseCase (std::unique_ptr<royale::usecase::UseCaseDefinition> useCase, const royale::Vector<std::size_t> &measurementBlocks);

        /**
         * Set m_useCase to be a 4+1 UseCaseFourPhase with grayscale, and call setupUseCase.
         */
        void setupUseCaseDefault();

        /**
         * Sets a different listener as the m_listener, for testing with the Slow or Async mock
         * listeners.
         *
         * This is intended to be called at the start of a test, before any frames have been created.
         * The FrameCollector should support this at any time, even in the middle of a test that creates
         * lots of frames, and even if there are some frames currently in the old listener.
         */
        void replaceTestListener (std::shared_ptr<royale::stub::hal::MockFrameCaptureListener> listener);

        /**
         * Updates the statistics in m_framesTotal, m_framesDroppedBridge and
         * m_framesDroppedCollector.
         */
        void onEvent (std::unique_ptr<royale::IEvent> &&event) override;

        /**
         * UseCaseDefinition with a lifetime longer than the FrameCollector, as required by the
         * definition of IFrameCollector::executeUseCase.
         */
        std::unique_ptr<royale::usecase::UseCaseDefinition> m_useCase;

        /**
         * If m_useCase has been set via setupUseCase (including the subclass'
         * setupUseCaseMixedXHt), then this is the corresponding argument for configureFromUseCase
         * and executeUseCase's list of buffer sizes.
         */
        royale::Vector<std::size_t> m_measurementBlocks;

        /** IFrameCaptureListener with a lifetime longer than the FrameCollector */
        std::shared_ptr<royale::stub::hal::MockFrameCaptureListener> m_listener;

        std::shared_ptr<royale::stub::hal::BufferGeneratorStub> m_bridge;
        std::shared_ptr<royale::stub::hal::StubTemperatureSensor> m_temperatureSensor;

        /**
         * The instance under test.  This uses the FrameCollectorBase class instead of the
         * IFrameCollector interface so that the tests can access flushRawFrameStatistics().
         */
        std::shared_ptr<royale::collector::FrameCollectorBase> m_frameCollector;

        size_t m_framesTotal;
        size_t m_framesDroppedBridge;
        size_t m_framesDroppedCollector;
    };

    /**
     * The template FixtureTestFrameCollectorMixedMode<FC> is for running MixedUseCaseXHt based tests.
     *
     * The test should start by calling setupUseCaseMixedXHt, which will populate all of the member
     * variables.
     */
    class ImplFixtureTestFrameCollectorMixedMode : public ImplFixtureTestFrameCollector
    {
    protected:
        /**
         * Some of the subclasses want to test a very high HT:ES ratio, to ensure that the frame
         * collector doesn't request an excessive number of buffers, and that it releases frames if
         * other frames in the same set are dropped.
         *
         * A 50:1 use case has 259 frames, and is a real-world use case.
         *
         * A 400:1 use case has 2009 frames, and is approximately the largest use case that can be
         * supported.  This is limited by the 12-bit frame counter and 12-bit sequence index which mean
         * that "greater than" starts to wrap round inside sequences with around 2048 frames.  Running
         * at 400:1 is a test for the logic of the FrameCollector, and which parts need to be optimized.
         */
        static const uint16_t HIGH_HT_ES_RATIO = 50;

        /**
         * Set m_useCase to be a UseCaseMixedXHt with the given ratio, and call executeUseCase with this
         * use case.
         *
         * This will set m_htStreamId and m_esStreamId for the caller's use.
         */
        void setupUseCaseMixedXHt (uint16_t ratio);

        /** The stream that's captured ratio times per sequence */
        royale::StreamId m_htStreamId;
        /** The stream that's captured once per sequence */
        royale::StreamId m_esStreamId;
    };

    template <typename FC>
    class FixtureTestFrameCollector : public ImplFixtureTestFrameCollector
    {
    protected:
        FixtureTestFrameCollector()
        {
            m_frameCollector = std::make_shared<royale::collector::FrameCollectorBase> (m_bridge->createInterpreter(), m_bridge, royale::common::makeUnique<FC>());
        }
    };

    template <typename FC>
    class FixtureTestFrameCollectorMixedMode : public ImplFixtureTestFrameCollectorMixedMode
    {
    protected:
        FixtureTestFrameCollectorMixedMode()
        {
            m_frameCollector = std::make_shared<royale::collector::FrameCollectorBase> (m_bridge->createInterpreter(), m_bridge, royale::common::makeUnique<FC>());
        }
    };

}
