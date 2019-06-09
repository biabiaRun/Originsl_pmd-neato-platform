/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/ISensorRoutingConfig.hpp>
#include <common/SensorRole.hpp>
#include <map>
#include <memory>


namespace royale
{
    namespace common
    {
        /**
        * Map sensor role to sensor routing config.
        * Mainly used for the I2C addresses of flash, temperature sensor and imager control channel.
        */
        typedef std::map<common::SensorRole, std::shared_ptr<ISensorRoutingConfig>> SensorMap;

        /**
        * Helper function for querying SensorMap.
        * Searches the map for a given SensorRole and returns the config stored for it.
        * Throws InvalidValue if no matching entry is found.
        *
        * \param sensorMap the map to query
        * \param role the SensorRole to search for
        * \return a matching std::shared_ptr<ISensorRoutingConfig>> from the map
        *
        */
        const SensorMap::mapped_type findSensorRoute (const SensorMap &sensorMap, SensorRole role);
    }
}
