/****************************************************************************\
 * Copyright (C) 2020 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/TemperatureSensorTMP103.hpp>
#include <common/EndianConversion.hpp>

#include <vector>

using namespace royale::sensors;
using namespace royale::common;
using namespace royale::pal;

TemperatureSensorTMP103::TemperatureSensorTMP103 (std::shared_ptr<II2cDeviceAccess> device) :
    m_device {device}
{
}

TemperatureSensorTMP103::~TemperatureSensorTMP103()
{
}

float TemperatureSensorTMP103::getTemperature()
{
    std::vector<uint8_t> buf;
    buf.resize (1);
    m_device->readI2cNoAddress (buf);

    if (buf[0] < 0x80)
    {
        // Positive value
        return static_cast<float> (buf[0]);
    }
    else
    {
        // Negative value
        uint8_t tmp = static_cast<uint8_t> (~buf[0] + 1);
        return -1.0f * static_cast<float> (tmp);
    }
}
