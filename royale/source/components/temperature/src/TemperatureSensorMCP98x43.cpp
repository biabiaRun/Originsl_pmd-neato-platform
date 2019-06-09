/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/TemperatureSensorMCP98x43.hpp>
#include <common/EndianConversion.hpp>

#include <vector>

using namespace royale::sensors;
using namespace royale::common;
using namespace royale::pal;

namespace
{
    const uint8_t MCP98x43_REGISTER_TEMPERATURE = 0x05;
}

TemperatureSensorMCP98x43::TemperatureSensorMCP98x43 (std::shared_ptr<II2cDeviceAccess> device) :
    m_device {device}
{
}

TemperatureSensorMCP98x43::~TemperatureSensorMCP98x43()
{
}

float TemperatureSensorMCP98x43::getTemperature()
{
    std::vector<uint8_t> buf;
    buf.resize (2);
    m_device->readI2cAddress8 (MCP98x43_REGISTER_TEMPERATURE, buf);

    // The value is a 12-bit 2's-complement number, in the 12 least significant bits.
    // The 4 most significant bits are flags, with 0x1000 being the sign.
    //
    // \todo This needs testing with negative values.  The specification seems to be a
    // complex way of saying "13-bit 2's-complement", which is what I've implemented.
    signed int fixTemp = bufferToHostBe16 (&buf[0]) & 0x1fff;
    if (fixTemp >= 0x1000)
    {
        fixTemp -= 0x2000;
    }
    return 0.0625f * static_cast<float> (fixTemp);
}
