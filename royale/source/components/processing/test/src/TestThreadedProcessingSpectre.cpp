/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <processing/ProcessingSpectre.hpp>

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

using TestThreadedProcessingSpectre = FixtureTestProcessing<ProcessingSpectre>;

TEST_F (TestThreadedProcessingSpectre, notReadyForProcessing)
{
    // ProcessingSpectre needs calibration, so this just asserts that the ProcessingSpectre class
    // can be created and destroyed.
    ASSERT_FALSE (m_processing->isReadyToProcessDepthData());
}
