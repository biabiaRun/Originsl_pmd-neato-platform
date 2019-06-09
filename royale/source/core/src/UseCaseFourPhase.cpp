/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseFourPhase.hpp>
#include <common/exceptions/LogicError.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

//! generate a standard use case with four raw frames plus one grayscale
UseCaseFourPhase::UseCaseFourPhase (uint16_t targetFrameRate,
                                    uint32_t modulationFrequency,
                                    Pair<uint32_t, uint32_t> exposureLimits,
                                    uint32_t exposureModulation,
                                    uint32_t exposureGray,
                                    ExposureGray expoOnForGray,
                                    IntensityPhaseOrder intensityAsFirstPhase,
                                    bool enableSSC,
                                    double ssc_freq,
                                    double ssc_kspread,
                                    double ssc_delta)
    : UseCaseDefinition (targetFrameRate)
{
    m_typeName = "FourPhase";
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
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray, modulationFrequency);
            }
            else
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray);
            }
            rawFrameSets.push_back (rfs);
        }
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency, createExposureGroup ("mod", exposureLimits, exposureModulation),
                                                ssc_freq, ssc_kspread, ssc_delta));
    }
    else
    {
        rawFrameSets.push_back (createPhaseRFS (modulationFrequency, createExposureGroup ("mod", exposureLimits, exposureModulation),
                                                ssc_freq, ssc_kspread, ssc_delta));
        if (exposureGray > 0)
        {
            royale::usecase::RawFrameSet rfs;
            if (expoOnForGray == ExposureGray::On)
            {
                rfs = createGrayRFS (createExposureGroup ("gray", exposureLimits, exposureGray), expoOnForGray, modulationFrequency);
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

RawFrameSet UseCaseFourPhase::createPhaseRFS (
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
