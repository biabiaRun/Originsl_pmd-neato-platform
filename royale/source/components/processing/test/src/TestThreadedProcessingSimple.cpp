/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <processing/ProcessingSimple.hpp>

#include <FixtureTestProcessing.hpp>
#include <FrameGeneratorStub.hpp>
#include <MockProcessingListeners.hpp>

#include <gtest/gtest.h>

using namespace royale;
using namespace royale::collector;
using namespace royale::common;
using namespace royale::processing;
using namespace royale::stub::processing;
using namespace royaletest;

using TestThreadedProcessingSimple = FixtureTestProcessing<ProcessingSimple>;

TEST_F (TestThreadedProcessingSimple, ValidCallbacks)
{
    setupUseCaseDefault();

    // ProcessingSimple doesn't need calibration, it's already ready to process depth data
    ASSERT_TRUE (m_processing->isReadyToProcessDepthData());
    ASSERT_NO_FATAL_FAILURE (m_frameGenerator->setFrameCaptureListener (m_processing));
    ASSERT_NO_FATAL_FAILURE (m_frameGenerator->generateCallback (*m_useCase, m_useCase->getStreamIds().at (0)));
}

/**
 * For each stream, the DepthDataListener callbacks should happen in the same order as the
 * FrameCaptureListener callbacks.  If they're reordered then the object may seem to be moving
 * backwards.
 *
 * Between streams this limitation isn't needed, so for a capture of
 *     HT1, HT2, HT3, ES, HT4, HT5
 * it would be acceptable for the ES frame to take longer to process, and the HT4 callback
 * to happen before the ES callback.
 *
 * This test only uses a single stream.
 */
TEST_F (TestThreadedProcessingSimple, sequentialCallbacks)
{
    setupUseCaseDefault();

    ASSERT_TRUE (m_processing->isReadyToProcessDepthData());
    ASSERT_NO_FATAL_FAILURE (m_frameGenerator->setFrameCaptureListener (m_processing));

    auto checkingListener = std::make_shared<SequenceCheckingDepthDataListener>();
    replaceListener (checkingListener);

    const auto expectedCallbacks = 10;
    for (auto i = 0; i < expectedCallbacks; i++)
    {
        ASSERT_NO_FATAL_FAILURE (m_frameGenerator->generateCallback (*m_useCase, m_useCase->getStreamIds().at (0)));
    }

    ASSERT_NO_FATAL_FAILURE (checkingListener->checkForThreadedAssert());
}
