/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
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
    namespace config
    {
        /**
        * Defines the type of a illumination temperature sensor that is read
        * out by the imager  and added to the pseudodata. A module using this
        * should also set TemperatureSensorConfig to
        * TemperatureSensorType::PSEUDODATA to read this data.
        */
        enum class ImConnectedTemperatureSensor
        {
            /**
            * No illumination temperature sensor connected to the imager
            */
            NONE,

            /**
            * NTC temperature sensor
            */
            NTC
        };

    }
}
