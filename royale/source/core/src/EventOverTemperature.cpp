/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventOverTemperature.hpp>
#include <sstream>

using namespace royale::event;


EventOverTemperature::EventOverTemperature (royale::EventSeverity severity, float value, float limit)
    : m_severity (severity),
      m_value (value),
      m_limit (limit)
{
}

royale::EventSeverity EventOverTemperature::severity() const
{
    return m_severity;
}

const royale::String EventOverTemperature::describe() const
{
    std::ostringstream os;
    os << "Illumination Unit too hot! T = " << m_value << " deg C, limit is " << m_limit << " deg C!";

    return os.str();
}

royale::EventType EventOverTemperature::type() const
{
    return royale::EventType::ROYALE_OVER_TEMPERATURE;
}

