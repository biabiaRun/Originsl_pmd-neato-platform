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

#include <imager/M2450_A12/ImagerRegisters.hpp>

#include <map>

namespace royale
{
    namespace imager
    {
        namespace M2450_A12
        {
            const std::map < uint16_t, uint16_t > BaseConfig
            {
                //LUT1 = 30MHz

                { CFGCNT_PLLCFG1_LUT1, 0x2241 },   // CFGCNT_PLLCFG1_LUT1
                { CFGCNT_PLLCFG2_LUT1, 0x13B2 },   // CFGCNT_PLLCFG2_LUT1
                { CFGCNT_PLLCFG3_LUT1, 0x03BB },   // CFGCNT_PLLCFG3_LUT1

                //LUT2 = 60MHz

                { CFGCNT_PLLCFG1_LUT2, 0x2041 },
                { CFGCNT_PLLCFG2_LUT2, 0x13B1 },
                { CFGCNT_PLLCFG3_LUT2, 0x03BB },

                //LUT3 = 20MHz (warning: suboptimal frequency used)

                { CFGCNT_PLLCFG1_LUT3, 0x4459 },
                { CFGCNT_PLLCFG2_LUT3, 0xC4EC },
                { CFGCNT_PLLCFG3_LUT3, 0x0BCE },

                //LUT4 = 99.5MHz
                // already defined by reset config
            };
        }
    }
}
