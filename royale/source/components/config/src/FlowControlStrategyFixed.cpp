/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <config/FlowControlStrategyFixed.hpp>

using namespace royale::common;

FlowControlStrategyFixed::FlowControlStrategyFixed (uint16_t rawFrameRate)
    : m_rawFrameRate (rawFrameRate)
{
}

uint16_t FlowControlStrategyFixed::getRawFrameRate (const royale::usecase::UseCaseDefinition &useCase)
{
    return m_rawFrameRate;
}
