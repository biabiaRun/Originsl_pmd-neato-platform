/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/


#include <device/TemperatureSensorCheckAdapter.hpp>
#include <common/events/EventOverTemperature.hpp>


using namespace royale::device;


TemperatureSensorCheckAdapter::TemperatureSensorCheckAdapter (std::shared_ptr<royale::hal::ITemperatureSensor> sensor,
        std::unique_ptr<ITemperatureAcceptor> checker)
    : m_baseSensor (sensor),
      m_checker (std::move (checker))
{
}

float TemperatureSensorCheckAdapter::getTemperature()
{
    float temp { m_baseSensor->getTemperature() };
    m_checker->acceptTemperature (temp);
    return temp;
}

