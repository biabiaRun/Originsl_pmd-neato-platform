/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerUseCaseDefinition.hpp>

#include <common/exceptions/LogicError.hpp>

#include <stdint.h>

using namespace royale::imager;

ImagerUseCaseDefinition::ImagerUseCaseDefinition() :
    m_targetRate (1),
    m_imageColumns (0),
    m_imageRows (0),
    m_roiCMin (0),
    m_roiRMin (0),
    m_rawFrameRate (0),
    m_enableSSC (false),
    m_enableMixedMode (false)
{
}

ImagerUseCaseDefinition::ImagerUseCaseDefinition (const ImagerUseCaseDefinition &iucd) = default;

bool ImagerUseCaseDefinition::operator== (const ImagerUseCaseDefinition &rhs) const
{
    if (m_targetRate == rhs.m_targetRate &&
            m_rawFrameRate == rhs.m_rawFrameRate &&
            m_enableSSC == rhs.m_enableSSC &&
            m_imageColumns == rhs.m_imageColumns &&
            m_rawFrames.size () == rhs.m_rawFrames.size ())
    {
        for (std::size_t i = 0; i < m_rawFrames.size(); ++i)
        {
            if (m_rawFrames.at (i) != rhs.m_rawFrames.at (i))
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool ImagerUseCaseDefinition::operator!= (const ImagerUseCaseDefinition &rhs) const
{
    return !operator== (rhs);
}

uint16_t ImagerUseCaseDefinition::getRawFrameRate() const
{
    return m_rawFrameRate;
}

bool ImagerUseCaseDefinition::getSSCEnabled() const
{
    return m_enableSSC;
}

bool ImagerUseCaseDefinition::getMixedModeEnabled() const
{
    return m_enableMixedMode;
}

const std::vector<ImagerRawFrame> &ImagerUseCaseDefinition::getRawFrames() const
{
    return m_rawFrames;
}

uint16_t ImagerUseCaseDefinition::getTargetRate() const
{
    return m_targetRate;
}

void ImagerUseCaseDefinition::getImage (uint16_t &columns, uint16_t &rows) const
{
    columns = m_imageColumns;
    rows = m_imageRows;
}

void ImagerUseCaseDefinition::getStartOfROI (uint16_t &roiCMin, uint16_t &roiRMin) const
{
    roiCMin = m_roiCMin;
    roiRMin = m_roiRMin;
}

std::chrono::microseconds ImagerUseCaseDefinition::getTailTime() const
{
    // \todo ROYAL-2414 For mixed mode, this currently returns the average tail time; this is a workaround!
    auto expoTimes = std::chrono::microseconds::zero();
    auto nClockAlignedFrames = size_t{ 0 };
    for (const auto &rawFrame : m_rawFrames)
    {
        expoTimes += std::chrono::microseconds (rawFrame.exposureTime);
        if (rawFrame.alignment == ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED)
        {
            ++nClockAlignedFrames;
        }
    }
    if (nClockAlignedFrames == 0)
    {
        ++nClockAlignedFrames;
    }

    auto timePerSequence = nClockAlignedFrames * std::chrono::microseconds (1000000 / m_targetRate);
    if (timePerSequence > expoTimes)
    {
        return (timePerSequence - expoTimes) / nClockAlignedFrames;
    }

    throw common::LogicError ("tailTime can't be <=0"); // it can't be 0 because read-out, setup etc. also take time
}
