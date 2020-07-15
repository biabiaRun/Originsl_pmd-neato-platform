/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
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
#include <usecase/UseCaseMixedXHt.hpp>
#include <usecase/UseCaseDefFactoryProcessingOnly.hpp>

using namespace royale::usecase;

/**
 * This tests the constructor that's intended for parsing Zwetschge files, where only the parts used
 * by the Processing and FrameCollector are included.
 *
 * This will be a 14-phase 1 + 4 + 4 + 4 + 1, similar to the one constructed in
 * TestUseCaseArbitraryPhases.create14Phase.
 */
TEST (TestUseCaseProcessingOnly, create14Phase)
{
    royale::Vector<ExposureGroup> expoGroups;
    royale::Vector<RawFrameSet> rfsUcd;

    ExposureGroup expo1;
    expo1.m_exposureLimits = royale::Pair<uint32_t, uint32_t> { 1u, 2000u };
    expo1.m_exposureTime = 100u;

    ExposureGroup expo2;
    expo2.m_exposureLimits = royale::Pair<uint32_t, uint32_t> { 1u, 2000u };
    expo2.m_exposureTime = 500u;

    expoGroups.push_back (expo1);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo1);

    rfsUcd.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::GRAYSCALE, RawFrameSet::DutyCycle::DC_AUTO, 0));
    rfsUcd.push_back (RawFrameSet (80320000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 1));
    rfsUcd.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 2));
    rfsUcd.push_back (RawFrameSet (80320000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 3));
    rfsUcd.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::GRAYSCALE, RawFrameSet::DutyCycle::DC_AUTO, 4));

    auto simple = UseCaseDefFactoryProcessingOnly::createUcd (
                      UseCaseIdentifier ("test"),
    {176u, 120u},
    {1u, 50u},
    10u,
    expoGroups,
    rfsUcd);

    ASSERT_EQ (UseCaseIdentifier ("test"), simple->getIdentifier());

    uint16_t width, height;
    simple->getImage (width, height);
    ASSERT_EQ (176u, width);
    ASSERT_EQ (120u, height);

    ASSERT_EQ (10u, simple->getTargetRate());
    ASSERT_EQ (5u, simple->getExposureGroups().size());
    ASSERT_EQ (5u, simple->getRawFrameSets().size());
    ASSERT_EQ (14u, simple->getRawFrameCount());
    ASSERT_EQ (1u, simple->getStreamIds().size());

    auto rfs = simple->getRawFrameSets();

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

/**
 * Test that the constructor with takes a UseCaseDefinition produces the same result as the
 * constructor for Zwetschge.
 */
TEST (TestUseCaseProcessingOnly, reduce14Phase)
{
    // Copied from TestUseCaseArbitraryPhases.create14Phase
    UseCaseArbitraryPhases ucap (10u,
                                 UseCaseIdentifier ("test"),
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

    uint16_t width, height;
    ucap.getImage (width, height);

    royale::Vector<ExposureGroup> expoGroups;
    royale::Vector<RawFrameSet> rfs;

    ExposureGroup expo1;
    expo1.m_exposureLimits = royale::Pair<uint32_t, uint32_t> { 1u, 2000u };
    expo1.m_exposureTime = 100u;

    ExposureGroup expo2;
    expo2.m_exposureLimits = royale::Pair<uint32_t, uint32_t> { 1u, 2000u };
    expo2.m_exposureTime = 500u;

    expoGroups.push_back (expo1);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo2);
    expoGroups.push_back (expo1);

    rfs.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::GRAYSCALE, RawFrameSet::DutyCycle::DC_AUTO, 0));
    rfs.push_back (RawFrameSet (80320000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 1));
    rfs.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 2));
    rfs.push_back (RawFrameSet (80320000u, RawFrameSet::PhaseDefinition::MODULATED_4PH_CW, RawFrameSet::DutyCycle::DC_AUTO, 3));
    rfs.push_back (RawFrameSet (60240000u, RawFrameSet::PhaseDefinition::GRAYSCALE, RawFrameSet::DutyCycle::DC_AUTO, 4));

    auto ucpo = UseCaseDefFactoryProcessingOnly::createUcd (
                    UseCaseIdentifier ("test"),
    {width, height},
    {1u, 10u},
    10u,
    expoGroups,
    rfs);

    ASSERT_NE (ucap, *ucpo);
    auto fromAP = UseCaseDefFactoryProcessingOnly::createUcd (ucap);
    ASSERT_EQ (*fromAP, *ucpo);
}

/**
 * Test that the constructor with takes a UseCaseDefinition doesn't crash when handling a mixed
 * mode. What it should do with a mixed mode is currently implementation defined, as Zwetschge does
 * not yet have a spec for handling mixed modes.
 */
TEST (TestUseCaseProcessingOnly, mixedMode)
{
    auto ucMixed = UseCaseMixedXHt (5u, 1u, 30000000, 20200000, 20600000, { 200u, 1000u }, { 200u, 1000u },
                                    1000u, 1000u, 1000u, 1000u, 1000u, ExposureGray::Off,
                                    ExposureGray::Off, IntensityPhaseOrder::IntensityFirstPhase);
    ASSERT_NO_THROW (UseCaseDefFactoryProcessingOnly::createUcd (ucMixed));
}
