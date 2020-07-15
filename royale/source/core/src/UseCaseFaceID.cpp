/****************************************************************************\
* Copyright (C) 2020 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseFaceID.hpp>
#include <common/exceptions/LogicError.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using Alignment = RawFrameSet::Alignment;

UseCaseFaceID::UseCaseFaceID (
    royale::usecase::UseCaseIdentifier identifier,
    uint16_t targetRate,
    uint32_t modulationFrequencyIR,
    uint32_t modulationFrequencyDepth,
    uint32_t modulationFrequencyGray,
    royale::Pair<uint32_t, uint32_t> exposureLimitsIR,
    royale::Pair<uint32_t, uint32_t> exposureLimitsDepth,
    royale::Pair<uint32_t, uint32_t> exposureLimitsGray,
    uint32_t exposureIR,
    uint32_t exposureDepth,
    uint32_t exposureGray)
    : UseCaseDefinition (targetRate)
{
    m_identifier = identifier;
    m_typeName = String ("FaceID");
    m_targetRate = targetRate;
    m_enableSSC = false;
    m_imageColumns = 176;
    m_imageRows = 120;

    auto irStream = createStream ();
    auto depthStream = createStream ();

    auto irExposureGroup = createExposureGroup ("ir", exposureLimitsIR, exposureIR);
    auto grayExposureGroup = createExposureGroup ("gray", exposureLimitsGray, exposureGray);
    auto depthExposureGroup = createExposureGroup ("depth", exposureLimitsDepth, exposureDepth);

    royale::Vector<RawFrameSet> frameGroupIr;
    royale::Vector<RawFrameSet> frameGroupDepth;

    frameGroupIr.emplace_back (
        modulationFrequencyIR,
        RawFrameSet::PhaseDefinition::GRAYSCALE,
        RawFrameSet::DutyCycle::DC_AUTO,
        irExposureGroup,
        RawFrameSet::Alignment::START_ALIGNED
    );

    frameGroupIr.emplace_back (
        modulationFrequencyIR,
        RawFrameSet::PhaseDefinition::GRAYSCALE,
        RawFrameSet::DutyCycle::DC_AUTO,
        irExposureGroup,
        RawFrameSet::Alignment::START_ALIGNED
    );

    frameGroupDepth.emplace_back (
        modulationFrequencyGray,
        RawFrameSet::PhaseDefinition::GRAYSCALE,
        RawFrameSet::DutyCycle::DC_AUTO,
        grayExposureGroup,
        RawFrameSet::Alignment::START_ALIGNED
    );

    frameGroupDepth.emplace_back (
        modulationFrequencyDepth,
        RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
        RawFrameSet::DutyCycle::DC_AUTO,
        depthExposureGroup,
        RawFrameSet::Alignment::START_ALIGNED
    );


    constructFrameGroup (irStream, frameGroupIr, Alignment::CLOCK_ALIGNED);
    constructFrameGroup (depthStream, std::move (frameGroupDepth), Alignment::START_ALIGNED);

    verifyClassInvariants();
}
