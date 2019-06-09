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

#include <imager/M2452/ImagerRegisters.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        namespace M2452_B1x
        {
            using namespace M2452;

            const std::map < uint16_t, uint16_t > BaseConfig
            {
                // ROYAL-1054 workaround for brownout issue, warm up the capacitors
                { ANAIP_PLLBGEN, 0x0001 },       // bandgap enable
                { ANAIP_VMODREG, 0x0007 },       // Vreg enable

                // ROYAL-1315 MIPI compliance
                { ANAIP_DPHYDLANE1CFG1, 0x01c5 },
                { ANAIP_DPHYDLANE2CFG1, 0x01c5 },
                { ANAIP_DPHYDLANE1CFG3, 0x0206 },
                { ANAIP_DPHYDLANE2CFG3, 0x0206 },
            };
        }
    }
}
