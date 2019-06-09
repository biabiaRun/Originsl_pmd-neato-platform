/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/TemperatureSensorTMP102.hpp>
#include <common/EndianConversion.hpp>

#include <vector>

using namespace royale::sensors;
using namespace royale::common;
using namespace royale::pal;

TemperatureSensorTMP102::TemperatureSensorTMP102 (std::shared_ptr<II2cDeviceAccess> device) :
    m_device {device}
{
}

TemperatureSensorTMP102::~TemperatureSensorTMP102()
{
}

float TemperatureSensorTMP102::getTemperature()
{
    std::vector<uint8_t> buf;
    buf.resize (2);
    m_device->readI2cNoAddress (buf);

    // The value is a 12-bit 2's-complement number, in the 12 most significant bits.
    signed int fixTemp = bufferToHostBe16 (&buf[0]) >> 4;
    // Now check if it was negative
    if (fixTemp >= 0x800)
    {
        fixTemp -= 0x1000;
    }
    return 0.0625f * static_cast<float> (fixTemp);
}
