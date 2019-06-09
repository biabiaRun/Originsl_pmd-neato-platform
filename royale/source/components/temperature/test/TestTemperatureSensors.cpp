/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <pal/II2cDeviceAccess.hpp>

#include <temperature/TemperatureSensorTMP102.hpp>
#include <temperature/TemperatureSensorMCP98x43.hpp>
#include <temperature/TemperatureSensorADS1013_NTC.hpp>

#include <gtest/gtest.h>

#include <common/MakeUnique.hpp>

#include <vector>

using namespace royale::common;

using std::string;
using std::vector;

namespace
{
    /**
     * The data returned from a TMP102 temperature sensor, at twenty degrees celsius.  12-bit
     * 2's-complement, in the 12 most significant bits.
     */
    const vector<uint8_t> TMP102_20C =
    {
        0x14, 0x00
    };

    /**
     * The data returned from an MCP98x43 temperature sensor, at twenty degrees celsius.  13-bit
     * 2's-complement, in the 13 least significant bits.
     */
    const vector<uint8_t> MCP98x43_20C =
    {
        0x01, 0x40
    };

    /**
     * The data returned from an ADS1013_NTC temperature sensor, at twenty degrees celsius.
     */
    const vector<uint8_t> ADS1013_NTC_20C =
    {
        0x73, 0xA0
    };

    /**
     * A simulated device which will respond to all read requests by returning a copy of the data
     * passed to the constructor. This assumes (and this is true for the currently-supported
     * devices), that the only read requests will be for the register with the temperature it in.
     */
    class MockTemperatureSensor : public royale::pal::II2cDeviceAccess
    {
    public:
        explicit MockTemperatureSensor (vector<uint8_t> data) :
            m_data (std::move (data))
        {
        }

        /** The temperature that the other methods will return, as a plain number */
        float getReferenceTemperature()
        {
            return 20.0f;
        }

        /** The temperature precision that the other methods will return, as a plain number */
        float getReferenceTemperaturePrecision()
        {
            return 0.05f;
        }

        void readI2cNoAddress (std::vector<uint8_t> &buffer) override
        {
            ASSERT_EQ (buffer.size(), m_data.size());
            buffer = m_data;
        }

        void readI2cAddress8 (uint8_t regAddr, std::vector<uint8_t> &buffer) override
        {
            ASSERT_EQ (buffer.size(), m_data.size());
            buffer = m_data;
        }

        void readI2cAddress16 (uint16_t regAddr, std::vector<uint8_t> &buffer) override
        {
            ASSERT_EQ (buffer.size(), m_data.size());
            buffer = m_data;
        }

        void writeI2cNoAddress (const std::vector<uint8_t> &buffer) override { }
        void writeI2cAddress8 (uint8_t regAddr, const std::vector<uint8_t> &buffer) override { }
        void writeI2cAddress16 (uint16_t regAddr, const std::vector<uint8_t> &buffer) override { }

    private:
        const vector<uint8_t> m_data;
    };
}

TEST (TestTemperatureSensors, MCP98x43)
{
    auto device = std::make_shared<MockTemperatureSensor> (MCP98x43_20C);
    std::unique_ptr<royale::hal::ITemperatureSensor> sensor =
        royale::common::makeUnique<royale::sensors::TemperatureSensorMCP98x43> (device);
    ASSERT_FLOAT_EQ (sensor->getTemperature(), device->getReferenceTemperature());
}

TEST (TestTemperatureSensors, TMP102)
{
    auto device = std::make_shared<MockTemperatureSensor> (TMP102_20C);
    std::unique_ptr<royale::hal::ITemperatureSensor> sensor =
        royale::common::makeUnique<royale::sensors::TemperatureSensorTMP102> (device);
    ASSERT_FLOAT_EQ (sensor->getTemperature(), device->getReferenceTemperature());
}
#define DIVIDER_R1 (100000.0f) // resistance of resistive divider resistor in ntc circuit
#define THERMISTOR_BETA (4250.0f) // in KELVIN

TEST (TestTemperatureSensors, ADS1013_NTC)
{
    auto device = std::make_shared<MockTemperatureSensor> (ADS1013_NTC_20C);
    auto ntcAlgo = makeUnique<royale::common::NtcTempAlgo> (DIVIDER_R1, 100000.0f, 25.0f, THERMISTOR_BETA);
    std::unique_ptr<royale::hal::ITemperatureSensor> sensor =
        royale::common::makeUnique<royale::sensors::TemperatureSensorADS1013_NTC> (device, std::move (ntcAlgo));
    ASSERT_NEAR (sensor->getTemperature(), device->getReferenceTemperature(), device->getReferenceTemperaturePrecision());
}

