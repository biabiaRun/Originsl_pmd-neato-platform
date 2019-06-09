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

#include <cstdint>
#include <map>
#include <imager/M2452/ImagerRegisters.hpp>

namespace royale
{
    namespace imager
    {
        namespace M2452_B1x
        {
            using namespace royale::imager::M2452;

            const std::map < uint16_t, uint16_t > BaseConfigForSkylla
            {
                // diasble GPIOs 0-3 and set them to HighZ to disable the imager
                // SPI interface. This allows the CX3 an undisturbed SPI bus
                // access
                { ANAIP_PADGPIOCFG0, 0x1313 },
                { ANAIP_PADGPIOCFG1, 0x1313 },
            };
        }
    }
}
