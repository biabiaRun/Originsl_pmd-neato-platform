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
#include <memory>
#include <hal/IPsdTemperatureSensor.hpp>

namespace royale
{
    namespace config
    {
        /**
        * Configuration struct for temperature sensors that are based on a Negative Temperature Coefficient Thermistor (NTC)
        * and are read out via pseudo data from the imager.
        */
        struct NTCTemperatureSensorConfig
        {
            explicit NTCTemperatureSensorConfig (float resistanceR1, float resistanceRntc0, float refTp, float thermistorBeta) :
                resistanceR1 (resistanceR1),
                resistanceRntc0 (resistanceRntc0),
                refTp (refTp),
                thermistorBeta (thermistorBeta)
            {
            }
            float resistanceR1;                                         ///< resistance of resistor R1 in Ohm
            float resistanceRntc0;                                      ///< resistance of the thermistor at reference temperature in Ohm
            float refTp;                                                ///< reference temperature in degree Celsius
            float thermistorBeta;                                       ///< temperature coefficient
        };
        /**
         * Parameters of the illumination unit temperature sensor in the module.
         */
        struct TemperatureSensorConfig
        {
            /**
            * Defines the different types of temperature sensors which are available for creating a module
            */
            enum class TemperatureSensorType
            {
                /**
                * Microchip MCP9843 or MCP98243 I2C temperature sensor.
                */
                MCP98x43,
                /**
                * Texas Instruments TMP102 I2C temperature sensor.
                */
                TMP102,
                /**
                * The temperature will be read out by the imager/firmware
                * and written to the pseudo data. A module using this
                * should also set ImagerConfig::tempSensor to
                * ImConnectedTemperatureSensor::NTC to enable
                * temperature read out by the imager.
                */
                PSEUDODATA,
                /**
                * ADS1013 ADC (http://www.ti.com/product/ADS1013) connected
                * to a NTC.
                */
                ADS1013_NTC
            };

            TemperatureSensorType type;
            std::shared_ptr<NTCTemperatureSensorConfig> ntcConfig; ///< this will only be valid if the temperature sensor uses a NTC. Currently these are PSEUDODATA and ADS1013_NTC
            hal::IPsdTemperatureSensor::PseudoDataPhaseSync phaseSyncConfig;     ///< which phase should be used for the calculation. This is only valid for pseudodata sensors
        };
    }
}
