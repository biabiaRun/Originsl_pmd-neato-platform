/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <royale/Status.hpp>
#include <usecase/UseCaseArbitraryPhases.hpp>

using namespace royale::usecase;

/**
 * Create a simple 1 gray frame use case without SSC
 */
TEST (TestUseCaseArbitraryPhases, createGray)
{
    UseCaseArbitraryPhases simple (5u,
                                   royale::Vector < royale::usecase::UseCaseArbitraryPhaseSetting >
    {
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000u}, 100u
        }
    },
    false);

    ASSERT_EQ (5u, simple.getTargetRate());
    ASSERT_EQ (1u, simple.getExposureGroups().size());
    ASSERT_EQ (1u, simple.getRawFrameSets().size());
}

/**
 * Create a 9 phase sequence (1 gray @ 60MHz + 4 phases @ 80MHz + 4 phases @60MHz)
 * running at 10 fps without SSC.
 */
TEST (TestUseCaseArbitraryPhases, create9Phase)
{
    UseCaseArbitraryPhases simple (10u,
                                   royale::Vector < royale::usecase::UseCaseArbitraryPhaseSetting >
    {
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000u}, 100u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            80320000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u
        }
    },
    false);

    ASSERT_EQ (10u, simple.getTargetRate());
    ASSERT_EQ (3u, simple.getExposureGroups().size());
    ASSERT_EQ (3u, simple.getRawFrameSets().size());
    ASSERT_EQ (9u, simple.getRawFrameCount());
    ASSERT_EQ (1u, simple.getStreamIds().size());

    auto rfs = simple.getRawFrameSets();

    ASSERT_FALSE (rfs[0].isModulated());
    ASSERT_EQ (1u, rfs[0].countRawFrames());
    ASSERT_EQ (60240000u, rfs[0].modulationFrequency);

    ASSERT_TRUE (rfs[1].isModulated());
    ASSERT_EQ (4u, rfs[1].countRawFrames());
    ASSERT_EQ (80320000u, rfs[1].modulationFrequency);

    ASSERT_TRUE (rfs[2].isModulated());
    ASSERT_EQ (4u, rfs[2].countRawFrames());
    ASSERT_EQ (60240000u, rfs[2].modulationFrequency);

    auto expoGroups = simple.getExposureGroups();
    EXPECT_STRCASEEQ ("gray1", expoGroups[0].m_name.c_str ());
    EXPECT_STRCASEEQ ("mod1", expoGroups[1].m_name.c_str());
    EXPECT_STRCASEEQ ("mod2", expoGroups[2].m_name.c_str());
}

/**
 * Create a 14 phase sequence (1 gray @ 60MHz + 4 phases @ 80MHz + 4 phases @60MHz + 4 phases
 * @60MHz + 1 gray @60MHz) running at 10 fps without SSC.
 *
 * This is just testing that unusual use cases are supported, it's not specified by a customer.
 */
TEST (TestUseCaseArbitraryPhases, create14Phase)
{
    UseCaseArbitraryPhases simple (10u,
                                   royale::Vector < royale::usecase::UseCaseArbitraryPhaseSetting >
    {
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000u}, 100u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            80320000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            80320000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000u}, 100u
        }
    },
    false);

    ASSERT_EQ (10u, simple.getTargetRate());
    ASSERT_EQ (5u, simple.getExposureGroups().size());
    ASSERT_EQ (5u, simple.getRawFrameSets().size());
    ASSERT_EQ (14u, simple.getRawFrameCount());
    ASSERT_EQ (1u, simple.getStreamIds().size());

    auto rfs = simple.getRawFrameSets();

    ASSERT_FALSE (rfs[0].isModulated());
    ASSERT_EQ (1u, rfs[0].countRawFrames());
    ASSERT_EQ (60240000u, rfs[0].modulationFrequency);

    ASSERT_TRUE (rfs[1].isModulated());
    ASSERT_EQ (4u, rfs[1].countRawFrames());
    ASSERT_EQ (80320000u, rfs[1].modulationFrequency);

    ASSERT_TRUE (rfs[2].isModulated());
    ASSERT_EQ (4u, rfs[2].countRawFrames());
    ASSERT_EQ (60240000u, rfs[2].modulationFrequency);

    ASSERT_TRUE (rfs[3].isModulated());
    ASSERT_EQ (4u, rfs[3].countRawFrames());
    ASSERT_EQ (80320000u, rfs[3].modulationFrequency);

    ASSERT_FALSE (rfs[4].isModulated());
    ASSERT_EQ (1u, rfs[4].countRawFrames());
    ASSERT_EQ (60240000u, rfs[4].modulationFrequency);
}
