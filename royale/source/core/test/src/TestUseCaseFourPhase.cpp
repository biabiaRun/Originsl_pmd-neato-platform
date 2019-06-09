/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <royale/Status.hpp>
#include <usecase/UseCaseFourPhase.hpp>

using namespace royale::usecase;

TEST (TestUseCaseFourPhase, create)
{
    UseCaseFourPhase simple (45u, 30000000, {50u, 1000u}, 1000u, 0u);
    ASSERT_EQ (1u, simple.getRawFrameSets().size());
    ASSERT_EQ (4u, simple.getRawFrameSets() [0].countRawFrames());
}

TEST (TestUseCaseFourPhase, createWithExpoGrayOn)
{
    const uint32_t modFreq = 30000000;
    const uint32_t expo = 1000u;
    UseCaseFourPhase simple (45u, modFreq, { 50u, expo }, expo, expo, ExposureGray::On, royale::usecase::IntensityPhaseOrder::IntensityLastPhase);
    ASSERT_EQ (2u, simple.getRawFrameSets().size());
    ASSERT_EQ (4u, simple.getRawFrameSets() [0].countRawFrames());
    ASSERT_EQ (1u, simple.getRawFrameSets() [1].countRawFrames());
    ASSERT_EQ (true, simple.getRawFrameSets().at (1).isGrayscale());
    ASSERT_EQ (modFreq, simple.getRawFrameSets().at (1).modulationFrequency);
    ASSERT_EQ (2u, simple.getExposureGroups().size());
    ASSERT_EQ (expo, simple.getExposureGroups().at (1).m_exposureTime);
}
