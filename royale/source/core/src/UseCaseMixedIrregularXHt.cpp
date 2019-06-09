/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseMixedIrregularXHt.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/NarrowCast.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using Alignment = RawFrameSet::Alignment;

namespace
{
    uint16_t calculateTargetRate (uint16_t htRate, uint16_t ratio)
    {
        if (ratio == 0u)
        {
            throw LogicError ("Can't create a mixed use case with zero HT framegroups");
        }

        return narrow_cast<uint16_t> (htRate + (htRate / ratio) * 2);
    }
}

UseCaseMixedIrregularXHt::UseCaseMixedIrregularXHt (
    uint16_t htRate,
    uint16_t ratio,
    uint32_t modulationFrequencyHt,
    uint32_t modulationFrequencyEs1,
    uint32_t modulationFrequencyEs2,
    royale::Pair<uint32_t, uint32_t> exposureLimitsHt,
    royale::Pair<uint32_t, uint32_t> exposureLimitsEs,
    uint32_t exposureModulationHt,
    uint32_t exposureModulationEs1,
    uint32_t exposureModulationEs2,
    uint32_t exposureGrayHt,
    uint32_t exposureGrayEs,
    ExposureGray expoOnForGrayHt,
    ExposureGray expoOnForGrayEs,
    IntensityPhaseOrder intensityAsFirstPhase,
    bool enableSSC,
    double ssc_freq,
    double ssc_kspread,
    double ssc_delta_modHt,
    double ssc_delta_modEs1,
    double ssc_delta_modEs2)
    : UseCaseDefinition (calculateTargetRate (htRate, ratio))
{
    m_typeName = String ("MixedIrregularXHT_") + String::fromInt (htRate)
                 + String ("_") + String::fromInt (htRate / ratio);
    m_enableSSC = enableSSC;
    m_imageColumns = 176;
    m_imageRows = 120;

    auto htStream = createStream ();
    auto esStream = createStream ();

    auto htExposure = createExposureGroup ("ht", exposureLimitsHt, exposureModulationHt);
    auto es1Exposure = createExposureGroup ("es1", exposureLimitsEs, exposureModulationEs1);
    auto es2Exposure = createExposureGroup ("es2", exposureLimitsEs, exposureModulationEs2);

    royale::Vector<RawFrameSet> frameGroupHt;
    royale::Vector<RawFrameSet> frameGroupEs1;
    royale::Vector<RawFrameSet> frameGroupEs2;

    frameGroupHt.emplace_back (
        modulationFrequencyHt,
        RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
        RawFrameSet::DutyCycle::DC_AUTO,
        htExposure,
        RawFrameSet::Alignment::START_ALIGNED,
        0.,
        ssc_freq,
        ssc_kspread,
        ssc_delta_modHt
    );

    frameGroupEs1.emplace_back (
        modulationFrequencyEs1,
        RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
        RawFrameSet::DutyCycle::DC_AUTO,
        es1Exposure,
        RawFrameSet::Alignment::START_ALIGNED,
        0.,
        ssc_freq,
        ssc_kspread,
        ssc_delta_modEs1
    );

    frameGroupEs2.emplace_back (
        modulationFrequencyEs2,
        RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
        RawFrameSet::DutyCycle::DC_AUTO,
        es2Exposure,
        RawFrameSet::Alignment::START_ALIGNED,
        0.,
        ssc_freq,
        ssc_kspread,
        ssc_delta_modEs2
    );

    if (exposureGrayHt > 0)
    {
        auto grayExposureHt = createExposureGroup ("grayHt", exposureLimitsHt, exposureGrayHt);
        auto grayRfsHt = createGrayRFS (grayExposureHt, expoOnForGrayHt);

        if (intensityAsFirstPhase == IntensityPhaseOrder::IntensityFirstPhase)
        {
            frameGroupHt.insert (frameGroupHt.begin(), grayRfsHt);
        }
        else
        {
            frameGroupHt.push_back (grayRfsHt);
        }
    }

    if (exposureGrayEs > 0)
    {
        auto grayExposureEs = createExposureGroup ("grayEs", exposureLimitsEs, exposureGrayEs);
        auto grayRfsEs (createGrayRFS (grayExposureEs, expoOnForGrayEs));

        if (intensityAsFirstPhase == IntensityPhaseOrder::IntensityFirstPhase)
        {
            frameGroupEs1.insert (frameGroupEs1.begin(), grayRfsEs);
        }
        else
        {
            frameGroupEs2.push_back (grayRfsEs);
        }
    }

    for (int i = 0 ; i < ratio ; i++)
    {
        constructFrameGroup (htStream, frameGroupHt, Alignment::CLOCK_ALIGNED);
    }
    constructFrameGroup (esStream, frameGroupEs1, Alignment::NEXTSTOP_ALIGNED);
    constructFrameGroup (esStream, frameGroupEs2, Alignment::CLOCK_ALIGNED, true);

    verifyClassInvariants();
}
