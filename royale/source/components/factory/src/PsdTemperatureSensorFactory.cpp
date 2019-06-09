/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <memory>

#include <common/NtcTempAlgo.hpp>
#include <common/MakeUnique.hpp>
#include <factory/PsdTemperatureSensorFactory.hpp>
#include <temperature/PsdTemperatureSensorNtc.hpp>
#include <device/PsdTemperatureSensorFilter.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

std::unique_ptr<royale::hal::IPsdTemperatureSensor> PsdTemperatureSensorFactory::createTemperatureSensor (
    royale::hal::IImager &imager,
    const royale::config::TemperatureSensorConfig &sensorConfig)
{
    if (sensorConfig.type == TemperatureSensorConfig::TemperatureSensorType::PSEUDODATA)
    {
        auto  pdi = imager.createPseudoDataInterpreter();
        auto ntc = makeUnique<royale::common::NtcTempAlgo> (
                       sensorConfig.ntcConfig->resistanceR1,
                       sensorConfig.ntcConfig->resistanceRntc0,
                       sensorConfig.ntcConfig->refTp,
                       sensorConfig.ntcConfig->thermistorBeta);
        auto sensor = makeUnique<royale::sensors::PsdTemperatureSensorNtc> (std::move (pdi), std::move (ntc), sensorConfig.phaseSyncConfig);
        std::unique_ptr<royale::hal::IPsdTemperatureSensor> filter = makeUnique<royale::device::PsdTemperatureSensorFilter> (std::move (sensor));
        return filter;
    }
    else
    {
        return nullptr;
    }
}
