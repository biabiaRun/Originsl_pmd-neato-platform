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

#include <processing/IProcessing.hpp>

#include <FrameGeneratorStub.hpp>
#include <MockProcessingListeners.hpp>

#include <gtest/gtest.h>

namespace royaletest
{
    /**
     * Fixture for running most of the ProcessingSimple and ProcessingSpectre tests.
     *
     * The template subclass FixtureTestProcessing<P> should be used, as it sets
     * m_frameCollector to the appropriate IProcessing subclass.
     *
     * Each test must set m_useCase, either setting it itself or by calling setupUseCase.
     */
    class ImplFixtureTestProcessing : public ::testing::Test
    {
    protected:
        ImplFixtureTestProcessing() :
            m_useCase {nullptr},
            m_frameGenerator {std::make_shared<royale::stub::processing::FrameGeneratorStub> () },
            m_depthDataListener {std::make_shared<royale::stub::processing::MockDepthDataListener> () },
            m_processing {nullptr} // must be overridden by the subclass
        {
        }

        ~ImplFixtureTestProcessing() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * Sets a listener for the processed data, and puts it in a shared_ptr which will
         * be destroyed after the m_processing shared_ptr.
         */
        void replaceListener (std::shared_ptr<royale::IDepthDataListener> depthDataListener);

        /**
         * This (or setupUseCaseDefault) should be called at the start of each test.
         */
        void setupUseCase (std::unique_ptr<royale::usecase::UseCaseDefinition> useCase);

        /**
         * Call setupUseCaseDefault with a standard UseCaseFourPhase.
         */
        void setupUseCaseDefault();

        /**
         * UseCaseDefinition with a lifetime longer than the test.
         */
        std::unique_ptr<royale::usecase::UseCaseDefinition> m_useCase;

        std::shared_ptr<royale::stub::processing::FrameGeneratorStub> m_frameGenerator;
        std::shared_ptr<royale::IDepthDataListener> m_depthDataListener;

        /**
         * The instance under test.
         */
        std::shared_ptr<royale::processing::IProcessing> m_processing;
    };

    template <typename P>
    class FixtureTestProcessing : public ImplFixtureTestProcessing
    {
    protected:
        FixtureTestProcessing()
        {
            m_processing.reset (new P {m_frameGenerator.get() });
        }
    };
}
