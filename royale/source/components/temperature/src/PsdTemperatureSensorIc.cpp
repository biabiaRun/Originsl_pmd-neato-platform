/****************************************************************************\
 * Copyright (C) 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/PsdTemperatureSensorIc.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>

using namespace royale::sensors;
using namespace royale::common;

PsdTemperatureSensorIc::PsdTemperatureSensorIc (std::unique_ptr<const IPseudoDataInterpreter> psdInterpreter) :
    m_psdInterpreter (std::move (psdInterpreter))
{
}

PsdTemperatureSensorIc::~PsdTemperatureSensorIc()
{
}

float PsdTemperatureSensorIc::calcTemperature (const std::vector<ICapturedRawFrame *> &frames)
{
    const ICapturedRawFrame *frameToUse = frames.front();
    auto values = m_psdInterpreter->getTemperatureRawValues (*frameToUse);
    auto digTemp = values[0];
    float temp = 25.0f + (float(digTemp) - 296.0f) * 0.185f;
    return temp;
}
