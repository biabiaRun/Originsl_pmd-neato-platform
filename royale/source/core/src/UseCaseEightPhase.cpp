/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseEightPhase.hpp>
#include <common/exceptions/LogicError.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

//! generate definition for the two different sequence modes 4+4+1
UseCaseEightPhase::UseCaseEightPhase (uint16_t targetFrameRate,
                                      uint32_t modulationFrequency1,
                                      uint32_t modulationFrequency2,
                                      Pair<uint32_t, uint32_t> exposureLimits,
                                      uint32_t exposureModulation1,
                                      uint32_t exposureModulation2,
                                      uint32_t exposureGray,
                                      ExposureGray expoOnForGray,
                                      IntensityPhaseOrder intensityAsFirstPhase,
                                      bool enableSSC,
                                      double ssc_freq,
                                      double ssc_kspread,
                                      double ssc_delta_mod1,
                                      double ssc_delta_mod2)
    : UseCaseDefinition (targetFrameRate)
{
    m_typeName = "EightPhase";
    m_targetRate = targetFrameRate;
    m_enableSSC = enableSSC;
    m_imageColumns = 176;
    m_imageRows = 120;

    royale::Vector<RawFrameSet> rawFrameSets;

    if (intensityAsFirstPhase == IntensityPhaseOrder::IntensityFirstPhase)
    {
        if (exposureGray > 0)
        {
            royale::usecase::RawFrameSet rfs;
            if (expoOnForGray == ExposureGray::On)
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray, modulationFrequency2);
            }
            else
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray);
            }
            rawFrameSets.push_back (rfs);
        }
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency1, createExposureGroup ("mod1", exposureLimits, exposureModulation1),
                                                ssc_freq, ssc_kspread, ssc_delta_mod1));
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency2, createExposureGroup ("mod2", exposureLimits, exposureModulation2),
                                                ssc_freq, ssc_kspread, ssc_delta_mod2));
    }
    else
    {
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency1, createExposureGroup ("mod1", exposureLimits, exposureModulation1),
                                                ssc_freq, ssc_kspread, ssc_delta_mod1));
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency2, createExposureGroup ("mod2", exposureLimits, exposureModulation2),
                                                ssc_freq, ssc_kspread, ssc_delta_mod2));
        if (exposureGray > 0)
        {
            royale::usecase::RawFrameSet rfs;
            if (expoOnForGray == ExposureGray::On)
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray, modulationFrequency2);
            }
            else
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray);
            }
            rawFrameSets.push_back (rfs);
        }
    }

    constructNonMixedUseCase (std::move (rawFrameSets));
    verifyClassInvariants();
}

RawFrameSet UseCaseEightPhase::createPhaseRFS (
    uint32_t modulationFrequency,
    ExposureGroupIdx exposureGroupIdx,
    double ssc_freq,
    double ssc_kspread,
    double ssc_delta)
{
    RawFrameSet set
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
    return set;
}

