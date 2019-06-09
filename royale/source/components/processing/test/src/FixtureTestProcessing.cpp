/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <processing/IProcessing.hpp>

#include <FixtureTestProcessing.hpp>
#include <FrameGeneratorStub.hpp>
#include <MockProcessingListeners.hpp>

#include <common/exceptions/NotImplemented.hpp>
#include <common/MakeUnique.hpp>

#include <usecase/UseCaseFourPhase.hpp>

#include <gtest/gtest.h>

using namespace royale;
using namespace royale::collector;
using namespace royale::common;
using namespace royale::processing;
using namespace royale::stub::processing;
using namespace royaletest;

void ImplFixtureTestProcessing::SetUp()
{
    // replaceListener can also be used for the initial setup
    replaceListener (m_depthDataListener);
    m_frameGenerator->setFrameCaptureListener (m_processing);
}

void ImplFixtureTestProcessing::TearDown()
{
    DataListeners listeners;
    // DataListeners contains only nullptrs
    m_processing->registerDataListeners (listeners);
}

void ImplFixtureTestProcessing::replaceListener (std::shared_ptr<royale::IDepthDataListener> depthDataListener)
{
    DataListeners listeners;
    auto oldListener = m_depthDataListener;
    m_depthDataListener = depthDataListener;
    listeners.depthDataListener = m_depthDataListener.get();
    m_processing->registerDataListeners (listeners);
}

/**
 * This (or setupUseCaseDefault) should be called at the start of each test.
 */
void ImplFixtureTestProcessing::setupUseCase (std::unique_ptr<royale::usecase::UseCaseDefinition> useCase)
{
    // Configure the use case to the minimum size needed, to speed up the test
    m_frameGenerator->reduceToMinimumImage (useCase.get());
    m_useCase = std::move (useCase);
    ASSERT_NO_THROW (m_processing->setUseCase (*m_useCase));
}

void ImplFixtureTestProcessing::setupUseCaseDefault()
{
    auto useCase = std::unique_ptr<royale::usecase::UseCaseDefinition> (new royale::usecase::UseCaseFourPhase (45u, 30000000, {50u, 1000u}, 1000u, 1000u));
    setupUseCase (std::move (useCase));
}
