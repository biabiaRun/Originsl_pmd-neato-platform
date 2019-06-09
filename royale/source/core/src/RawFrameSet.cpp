/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usecase/RawFrameSet.hpp>
#include <map>

using namespace royale::usecase;

// define the const value
//lowest frequency common for M245x derivates shall be used for grayscale
//exposures to enable longest possible exposure times
const uint32_t RawFrameSet::M_MODFREQ_AUTO = 3152500;

RawFrameSet::RawFrameSet() :
    modulationFrequency (0),
    ssc_freq (0.),
    ssc_kspread (0.),
    ssc_delta (0.),
    phaseDefinition (PhaseDefinition::MODULATED_4PH_CW),
    dutyCycle (DutyCycle::DC_0),
    exposureGroupIdx (0),
    alignment (Alignment::START_ALIGNED),
    tEyeSafety (0.)
{
}

RawFrameSet::RawFrameSet (uint32_t freq, PhaseDefinition phases, DutyCycle cycle, ExposureGroupIdx egroup, Alignment align, double tEye,
                          double ssc_freq, double ssc_kspread, double ssc_delta) :
    modulationFrequency (freq),
    ssc_freq (ssc_freq),
    ssc_kspread (ssc_kspread),
    ssc_delta (ssc_delta),
    phaseDefinition (phases),
    dutyCycle (cycle),
    exposureGroupIdx (egroup),
    alignment (align),
    tEyeSafety (tEye)
{
}


uint32_t RawFrameSet::MODFREQ_AUTO()
{
    return RawFrameSet::M_MODFREQ_AUTO;
}

bool RawFrameSet::operator== (const RawFrameSet &rhs) const
{
    if (modulationFrequency == rhs.modulationFrequency &&
            ssc_freq == rhs.ssc_freq &&
            ssc_kspread == rhs.ssc_kspread &&
            ssc_delta == rhs.ssc_delta &&
            phaseDefinition == rhs.phaseDefinition &&
            dutyCycle == rhs.dutyCycle &&
            exposureGroupIdx == rhs.exposureGroupIdx &&
            alignment == rhs.alignment &&
            tEyeSafety == rhs.tEyeSafety)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool RawFrameSet::operator!= (const RawFrameSet &rhs) const
{
    return !operator== (rhs);
}

bool RawFrameSet::isModulated() const
{
    switch (phaseDefinition)
    {
        case PhaseDefinition::MODULATED_4PH_CW:
            return true;
        default:
            return false;
    }
}

bool RawFrameSet::isGrayscale() const
{
    switch (phaseDefinition)
    {
        case PhaseDefinition::GRAYSCALE:
            return true;
        default:
            return false;
    }
}

std::size_t RawFrameSet::countRawFrames() const
{
    return getPhaseAngles().size();
}

const std::vector<uint16_t> &RawFrameSet::getPhaseAngles() const
{
    static const std::map < RawFrameSet::PhaseDefinition, std::vector< uint16_t > > phaseDef2phaseAngle =
    {
        { PhaseDefinition::GRAYSCALE, { 0 } },
        { PhaseDefinition::MODULATED_4PH_CW, { 0, 90, 180, 270 } }
    };

    static const std::vector<uint16_t> emptyPhaseAngles;

    const auto angles = phaseDef2phaseAngle.find (phaseDefinition);
    if (angles != phaseDef2phaseAngle.end())
    {
        return angles->second;
    }

    return emptyPhaseAngles;
}
