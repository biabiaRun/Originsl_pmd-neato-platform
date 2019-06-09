/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <collector/CapturedUseCase.hpp>

using namespace royale;
using namespace royale::collector;
using namespace royale::common;

CapturedUseCase::CapturedUseCase (const IPseudoDataInterpreter *pdi, float temperature, std::chrono::microseconds timestamp, const Vector<uint32_t> &exposures) :
    m_pseudoDataInterpreter{ pdi },
    m_timestamp{ timestamp },
    m_illuminationTemperature{ temperature },
    m_exposureTimes (exposures)
{
}

CapturedUseCase::~CapturedUseCase()
{
}

const IPseudoDataInterpreter &CapturedUseCase::getInterpreter() const
{
    return *m_pseudoDataInterpreter;
}

const std::chrono::microseconds CapturedUseCase::getTimestamp() const
{
    return m_timestamp;
}

const float CapturedUseCase::getIlluminationTemperature() const
{
    return m_illuminationTemperature;
}

const Vector<uint32_t> &CapturedUseCase::getExposureTimes() const
{
    return m_exposureTimes;
}
