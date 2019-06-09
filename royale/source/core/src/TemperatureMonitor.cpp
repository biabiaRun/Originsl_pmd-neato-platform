/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/


#include <device/TemperatureMonitor.hpp>
#include <common/events/EventOverTemperature.hpp>


using namespace royale::device;


TemperatureMonitor::TemperatureMonitor (float softLimit, float hardLimit, float hysteresis)
    : m_eventForwarder(),
      m_softLimit (softLimit),
      m_hardLimit (hardLimit),
      m_hysteresis (hysteresis),
      m_softLimitTriggered (false),
      m_hardLimitTriggered (false)
{
}


void TemperatureMonitor::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

bool TemperatureMonitor::softAlarm() const
{
    return m_softLimitTriggered;
}

bool TemperatureMonitor::hardAlarm() const
{
    return m_hardLimitTriggered;
}

void TemperatureMonitor::retrigger()
{
    m_softLimitTriggered = false;
    m_hardLimitTriggered = false;
}

void TemperatureMonitor::acceptTemperature (float temp) const
{
    if (!m_hardLimitTriggered && (temp >= m_hardLimit))
    {
        m_hardLimitTriggered = true;
        m_eventForwarder.event<event::EventOverTemperature> (royale::EventSeverity::ROYALE_ERROR, temp, m_hardLimit);
    }
    else if (m_hardLimitTriggered && (temp < m_hardLimit - m_hysteresis))
    {
        m_hardLimitTriggered = false;
    }

    if (!m_softLimitTriggered && (temp >= m_softLimit))
    {
        m_softLimitTriggered = true;
        m_eventForwarder.event<event::EventOverTemperature> (royale::EventSeverity::ROYALE_WARNING, temp, m_softLimit);
    }
    else if (m_softLimitTriggered && (temp < m_softLimit - m_hysteresis))
    {
        m_softLimitTriggered = false;
    }
}
