/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <imager/ImagerRawFrame.hpp>

#include <map>

using namespace royale::imager;

// define the const value
const uint32_t ImagerRawFrame::M_MODFREQ_AUTO = 0;

ImagerRawFrame::ImagerRawFrame() :
    modulationFrequency (0),
    ssc_freq (0.),
    ssc_kspread (0.),
    ssc_delta (0.),
    grayscale (false),
    dutyCycle (ImagerDutyCycle::DC_0),
    phaseAngle (0),
    exposureTime (0),
    alignment (ImagerAlignment::START_ALIGNED),
    isStartOfLinkedRawFrames (false),
    isEndOfLinkedRawFrames (false),
    isEndOfLinkedMeasurement (false),
    tEyeSafety (0.)
{
}

ImagerRawFrame::ImagerRawFrame (uint32_t freq, bool grayscale,
                                bool isStartOfLinkedRawFrames, bool isEndOfLinkedRawFrames,
                                bool isEndOfLinkedMeasurement, ImagerDutyCycle cycle,
                                uint32_t expoTime, uint16_t phaseAngle, ImagerAlignment align,
                                double tEye, double ssc_freq, double ssc_kspread, double ssc_delta) :
    modulationFrequency (freq),
    ssc_freq (ssc_freq),
    ssc_kspread (ssc_kspread),
    ssc_delta (ssc_delta),
    grayscale (grayscale),
    dutyCycle (cycle),
    phaseAngle (phaseAngle),
    exposureTime (expoTime),
    alignment (align),
    isStartOfLinkedRawFrames (isStartOfLinkedRawFrames),
    isEndOfLinkedRawFrames (isEndOfLinkedRawFrames),
    isEndOfLinkedMeasurement (isEndOfLinkedMeasurement),
    tEyeSafety (tEye)
{
}


uint32_t ImagerRawFrame::MODFREQ_AUTO()
{
    return ImagerRawFrame::M_MODFREQ_AUTO;
}

bool ImagerRawFrame::operator== (const ImagerRawFrame &rhs) const
{
    if (modulationFrequency == rhs.modulationFrequency &&
            ssc_freq == rhs.ssc_freq &&
            ssc_kspread == rhs.ssc_kspread &&
            ssc_delta == rhs.ssc_delta &&
            grayscale == rhs.grayscale &&
            isEndOfLinkedRawFrames == rhs.isEndOfLinkedRawFrames &&
            isEndOfLinkedMeasurement == rhs.isEndOfLinkedMeasurement &&
            dutyCycle == rhs.dutyCycle &&
            exposureTime == rhs.exposureTime &&
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

bool ImagerRawFrame::operator!= (const ImagerRawFrame &rhs) const
{
    return !operator== (rhs);
}
