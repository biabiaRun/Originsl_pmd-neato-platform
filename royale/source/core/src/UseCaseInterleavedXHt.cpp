/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseInterleavedXHt.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/MakeUnique.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using Alignment = RawFrameSet::Alignment;

UseCaseInterleavedXHt::UseCaseInterleavedXHt (
    uint16_t targetRate,
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
    : UseCaseDefinition (targetRate)
{
    m_typeName = String ("InterleavedXHT_") + String::fromInt (ratio);
    m_targetRate = targetRate;
    m_enableSSC = enableSSC;

    m_imageColumns = 176;
    m_imageRows = 120;

    if (ratio < 2u)
    {
        // Although ratio == 1 creating ES1 HT1 ES2 might be technically possible, it would need
        // logic to support having the first raw frame not being a CLOCK_ALIGNED frame.
        throw LogicError ("Can't create an interleaved use case with less than 2 HT framegroups");
    }
    const auto finalRepeat = ratio - 2;

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
        ssc_delta_modHt
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
        ssc_delta_modHt
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
        auto grayRfsEs = createGrayRFS (grayExposureEs, expoOnForGrayEs);

        if (intensityAsFirstPhase == IntensityPhaseOrder::IntensityFirstPhase)
        {
            frameGroupEs1.insert (frameGroupEs1.begin(), grayRfsEs);
        }
        else
        {
            frameGroupEs2.push_back (grayRfsEs);
        }
    }

    constructFrameGroup (htStream, frameGroupHt, Alignment::CLOCK_ALIGNED);
    constructFrameGroup (esStream, std::move (frameGroupEs1), Alignment::STOP_ALIGNED);
    constructFrameGroup (htStream, frameGroupHt, Alignment::CLOCK_ALIGNED);
    constructFrameGroup (esStream, std::move (frameGroupEs2), Alignment::START_ALIGNED, true);
    for (int i = 0 ; i < finalRepeat ; i++)
    {
        constructFrameGroup (htStream, frameGroupHt, Alignment::CLOCK_ALIGNED);
    }

    verifyClassInvariants();
}
