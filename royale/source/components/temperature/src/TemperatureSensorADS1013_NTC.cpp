/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <temperature/TemperatureSensorADS1013_NTC.hpp>
#include <common/EndianConversion.hpp>
#include <common/MakeUnique.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/LogicError.hpp>

#include <vector>
#include <cmath>

namespace
{
    const uint16_t i2cAdcConversionRegister = 0x00;
    const uint16_t i2cAdcConfigRegister = 0x01;

    const float supplyVoltage = 3.3f; // supply voltage of NTC circuit

    // conversion factor: convert register value to voltage
    // register value FSR of ADS1013 = +/- 2.048 V (default configuration)
    // => voltage = register_value * 2.048 V / (2^11) = register_value * (1 / 1000) V = 0.001 V
    const float conversionFactor = 0.001f;
}

using namespace royale::sensors;
using namespace royale::common;
using namespace royale::pal;

TemperatureSensorADS1013_NTC::TemperatureSensorADS1013_NTC (std::shared_ptr<II2cDeviceAccess> device, std::unique_ptr<NtcTempAlgo> ntcAlgorithm) :
    m_initialized {false},
    m_device {device},
    m_ntcAlgorithm (std::move (ntcAlgorithm))
{
    // In the first run set the ADC into
    // continuous-conversion mode
    std::vector<uint8_t> buf{ 0x84, 0x83 };

    try
    {
        m_device->writeI2cAddress8 (i2cAdcConfigRegister, buf);
        m_initialized = true;
    }
    catch (const RuntimeError &)
    {
        // log error only, but do not abort (for testing purposes only!)
        LOG (WARN)
                << "Failed to initialize temperature sensor - disabled!";
    }

}

TemperatureSensorADS1013_NTC::~TemperatureSensorADS1013_NTC()
{
}

float TemperatureSensorADS1013_NTC::getTemperature()
{
    if (!m_initialized)
    {
        throw RuntimeError ("Temperature sensor not initialized");
    }
    std::vector<uint8_t> buf;
    buf.resize (2);

    // Read conversion register
    m_device->readI2cAddress8 (i2cAdcConversionRegister, buf);

    // Calculate register value
    // "12 bits of data in binary two's complement format that is left justified within 16-bit data word"
    int32_t register_value = bufferToHostBe16 (&buf[0]) >> 4;
    // Now check if it was negative
    if (register_value >= 0x800)
    {
        register_value -= 0x1000;
    }

    // Convert register value to voltage
    const float voltage = static_cast <float> (register_value) * conversionFactor;

    return m_ntcAlgorithm->calcTemperature (supplyVoltage, voltage);
}
