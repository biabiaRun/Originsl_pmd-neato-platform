/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

namespace royale
{
    namespace common
    {
        /**
         * The same model of TemperatureSensor, CurrentSensor, etc may be used in different physical positions in the
         * module. This is used to tell the Bridge which sensor is being read.
         */
        enum class SensorRole
        {
            /** The ToF sensor */
            MAIN_IMAGER,

            /**
             * A TemperatureSensor near the infrared light source. This sensor is typically used to get the
             * overall module's temperature.
             */
            TEMP_ILLUMINATION,

            /** A TemperatureSensor near the ToF sensor */
            TEMP_IMAGER,

            /**
             * The flash memory, eeprom, etc, that contains the calibration data.
             * This is used by the UVC (Arctic) bridges, but not by the Enclustra or FPGA ones.
             */
            STORAGE_CALIBRATION
        };
    }
}
