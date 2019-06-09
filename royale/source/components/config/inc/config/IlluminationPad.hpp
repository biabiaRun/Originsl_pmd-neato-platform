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
        * Illumination pad configuration used by the imager to enable the desired pad and adust the duty cycle configuration to it.
        */
        enum class IlluminationPad
        {
            /**
            * The illumination circuit is connected to the imager pad labeled as 'MODSEP'.
            */
            SE_P,

            /**
            * The illumination circuit is connected to the imager pad labeled as 'MODSEN'.
            */
            SE_N,

            /**
            * The illumination circuit is connected to the imager pads labeled as 'MODLVDSN' and 'MODLVDSP'.
            */
            LVDS
        };

    }
}

