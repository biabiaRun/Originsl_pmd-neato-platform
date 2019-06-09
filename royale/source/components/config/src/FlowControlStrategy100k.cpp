/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <config/FlowControlStrategy100k.hpp>

using namespace royale::common;

FlowControlStrategy100k::FlowControlStrategy100k (uint16_t rawFrameRateSlow,
        uint16_t rawFrameRateFast)
    : m_rawFrameRateSlow (rawFrameRateSlow),
      m_rawFrameRateFast (rawFrameRateFast)
{
}

uint16_t FlowControlStrategy100k::getRawFrameRate (const royale::usecase::UseCaseDefinition &useCase)
{
    for (const auto &limit : useCase.getExposureLimits())
    {
        if (limit.second >= 1000u)
        {
            return m_rawFrameRateSlow;
        }
    }
    return m_rawFrameRateFast;
}
