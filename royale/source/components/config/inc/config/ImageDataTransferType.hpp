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

namespace royale
{
    namespace config
    {
        /**
        * Defines the type of interface that is used to connect the imager to a receiver module.
        */
        enum class ImageDataTransferType
        {
            /**
            * Parallel Interface
            */
            PIF,

            /**
            * CSI2 MIPI 1 Lane
            */
            MIPI_1LANE,

            /**
            * CSI2 MIPI 2 Lanes
            */
            MIPI_2LANE
        };

    }
}
