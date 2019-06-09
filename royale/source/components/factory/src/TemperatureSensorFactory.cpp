/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/TemperatureSensorFactory.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <common/MakeUnique.hpp>

#include <pal/II2cBusAccess.hpp>
#include <pal/Access2I2cDeviceAdapter.hpp>

#include <temperature/TemperatureSensorTMP102.hpp>
#include <temperature/TemperatureSensorMCP98x43.hpp>
#include <temperature/TemperatureSensorADS1013_NTC.hpp>

#include <memory>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

std::shared_ptr<royale::hal::ITemperatureSensor> TemperatureSensorFactory::createTemperatureSensor (
    royale::factory::IBridgeFactory &bridgeFactory,
    const royale::config::TemperatureSensorConfig &sensorConfig,
    const royale::common::ISensorRoutingConfig *sensorRouting)
{
    std::shared_ptr<royale::hal::ITemperatureSensor> tempSensor;

    if (sensorConfig.type == TemperatureSensorConfig::TemperatureSensorType::PSEUDODATA)
    {
        return nullptr;
    }

    try
    {
        // All sensors supported by the following need an I2C routing and I2C access
        auto i2cRoute = dynamic_cast<const SensorRoutingConfigI2c *> (sensorRouting); // avoid std::bad_cast
        if (i2cRoute == nullptr)
        {
            throw InvalidValue ("Can't create temperature sensor, no I2C routing found");
        }

        auto i2cAccess = bridgeFactory.create<royale::pal::II2cBusAccess>();
        if (i2cAccess == nullptr)
        {
            throw LogicError ("Can't create the temperature sensor, missing I2C access");
        }

        auto device = std::make_shared<royale::pal::Access2I2cDeviceAdapter> (i2cAccess, *i2cRoute);

        switch (sensorConfig.type)
        {
            case TemperatureSensorConfig::TemperatureSensorType::MCP98x43:
                tempSensor.reset (new royale::sensors::TemperatureSensorMCP98x43 (device));
                break;

            case TemperatureSensorConfig::TemperatureSensorType::TMP102:
                tempSensor.reset (new royale::sensors::TemperatureSensorTMP102 (device));
                break;
            case TemperatureSensorConfig::TemperatureSensorType::ADS1013_NTC:
                tempSensor = createADC1013TemperatureSensor (device, sensorConfig);
                break;

            default:
                throw LogicError ("Temperature sensor type is not supported");
        }
    }
    catch (...)
    {
        // log and rethrow error
        LOG (ERROR) << "Failed to initialize temperature sensor.";
        throw;
    }

    return tempSensor;
}

std::shared_ptr<royale::hal::ITemperatureSensor> TemperatureSensorFactory::createADC1013TemperatureSensor (
    std::shared_ptr<royale::pal::II2cDeviceAccess> device,
    const TemperatureSensorConfig &sensorConfig)
{
    auto ntc = makeUnique<royale::common::NtcTempAlgo> (
                   sensorConfig.ntcConfig->resistanceR1,
                   sensorConfig.ntcConfig->resistanceRntc0,
                   sensorConfig.ntcConfig->refTp,
                   sensorConfig.ntcConfig->thermistorBeta);
    return std::make_shared<royale::sensors::TemperatureSensorADS1013_NTC> (device, std::move (ntc));
}
