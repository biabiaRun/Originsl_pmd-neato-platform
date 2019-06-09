/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseArbitraryPhases.hpp>
#include <common/StringFunctions.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;

//! generate definition for arbitrary sequences
UseCaseArbitraryPhases::UseCaseArbitraryPhases (uint16_t targetFrameRate,
        const royale::Vector<UseCaseArbitraryPhaseSetting> &Settings,
        bool enableSSC)
    : UseCaseArbitraryPhases (targetFrameRate, {}, Settings, enableSSC)
{
}

UseCaseArbitraryPhases::UseCaseArbitraryPhases (uint16_t targetFrameRate,
        const UseCaseIdentifier &identifier,
        const royale::Vector<UseCaseArbitraryPhaseSetting> &Settings,
        bool enableSSC)
    : UseCaseDefinition (targetFrameRate)
{
    m_identifier = identifier;
    m_typeName = "ArbPhase";
    m_targetRate = targetFrameRate;
    m_enableSSC = enableSSC;
    m_imageColumns = 176;
    m_imageRows = 120;

    royale::Vector<RawFrameSet> rawFrameSets;

    int modCount = 0;
    int grayCount = 0;
    std::string groupName;

    for (auto &s : Settings)
    {
        switch (s.phaseSettingType)
        {
            case UseCaseArbitraryPhaseSettingType::FourPhase:
                groupName = "mod" + toStdString (++modCount);
                rawFrameSets.push_back (createPhaseRFS (s.modulationFrequency, createExposureGroup (groupName, s.exposureLimits, s.exposureTime),
                                                        s.ssc_freq, s.ssc_kspread, s.ssc_delta));
                break;
            case UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff:
                groupName = "gray" + toStdString (++grayCount);
                rawFrameSets.push_back (createGrayRFS (createExposureGroup (groupName, s.exposureLimits, s.exposureTime), ExposureGray::Off, s.modulationFrequency));
                break;
            case UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOn:
                groupName = "gray" + toStdString (++grayCount);
                rawFrameSets.push_back (createGrayRFS (createExposureGroup (groupName, s.exposureLimits, s.exposureTime), ExposureGray::On, s.modulationFrequency));
                break;
            default:
                throw royale::common::InvalidValue ("Unexpected FrameSetType.");
        }
    }

    constructNonMixedUseCase (std::move (rawFrameSets));
    verifyClassInvariants();
}

RawFrameSet UseCaseArbitraryPhases::createPhaseRFS (
    uint32_t modulationFrequency,
    ExposureGroupIdx exposureGroupIdx,
    double ssc_freq,
    double ssc_kspread,
    double ssc_delta)
{
    RawFrameSet rfs
    {
        modulationFrequency,
        RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
        RawFrameSet::DutyCycle::DC_AUTO,
        exposureGroupIdx,
        RawFrameSet::Alignment::START_ALIGNED,
        0.,
        ssc_freq,
        ssc_kspread,
        ssc_delta
    };
    return rfs;
}
