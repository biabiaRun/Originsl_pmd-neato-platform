/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/


#include <device/PsdTemperatureSensorFilter.hpp>


using namespace royale::device;
using namespace royale::common;


PsdTemperatureSensorFilter::PsdTemperatureSensorFilter (std::shared_ptr<royale::hal::IPsdTemperatureSensor> sensor, float weight)
    : m_baseSensor (sensor), m_average (weight)
{
}

float PsdTemperatureSensorFilter::calcTemperature (const std::vector<ICapturedRawFrame *> &frames)
{
    float temp = m_baseSensor->calcTemperature (frames);
    return m_average.calc (temp);
}

