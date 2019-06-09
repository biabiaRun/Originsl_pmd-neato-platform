/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/


#include <device/PsdTemperatureSensorCheckAdapter.hpp>
#include <common/events/EventOverTemperature.hpp>


using namespace royale::device;
using namespace royale::common;


PsdTemperatureSensorCheckAdapter::PsdTemperatureSensorCheckAdapter (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor,
        std::shared_ptr<ITemperatureAcceptor> checker)
    : m_baseSensor (sensor),
      m_checker (checker)
{
}

float PsdTemperatureSensorCheckAdapter::calcTemperature (const std::vector<ICapturedRawFrame *> &frames)
{
    float temp = m_baseSensor->calcTemperature (frames);
    m_checker->acceptTemperature (temp);
    return temp;
}

