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

#include <config/CoreConfig.hpp>
#include <config/ImagerConfig.hpp>
#include <config/IlluminationConfig.hpp>
#include <config/TemperatureSensorConfig.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <common/SensorMap.hpp>

namespace royale
{
    namespace config
    {
        /**
         * Information needed for constructing the parts used by the CameraCore.
         *
         * This includes information on hardware like imager, sensors and flash
         * and the configuration of the CameraCore itself.
         *
         */
        struct ModuleConfig
        {
            /**
            * Configuration for the camera core.
            */
            royale::config::CoreConfig  coreConfigData;

            /*
            * Configuration for imager and illumination unit.
            */
            ImagerConfig                imagerConfig;         //!< Imager config (including type and base config)
            IlluminationConfig          illuminationConfig;   //!< maximal allowed modulation frequency for this module

            /**
            * Type and configuration of the illumination unit temperature sensor
            */
            TemperatureSensorConfig     temperatureSensorConfig;

            /**
            *  Type and configuration of the non-volatile storage.
            */
            FlashMemoryConfig           flashMemoryConfig;

            /**
            * Subcomponents of the hardware. These are necessary for correct operation, if the Bridge does
            * not support their ISensorRoutingConfig types, it must report an unrecoverable error.
            *
            * The map values are never nullptr.  Sensors that are not configured are not included in the map.
            */
            common::SensorMap           essentialSensors;
        };
    }
}
