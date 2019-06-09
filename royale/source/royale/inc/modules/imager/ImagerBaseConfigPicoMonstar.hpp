/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <utility>

namespace royale
{
    namespace access
    {
        namespace imager
        {
            namespace M2450_A12
            {
                using namespace royale::imager::M2450_A12;

                const std::map < uint16_t, uint16_t > BaseConfigPicoMonstar
                {
                    // Chip settings for IRS11x5C - A12
                    { ANAIP_GPIO_CLK_CFG1, 0x0647 },   // disable internal alive clock divider
                    { ANAIP_PADGPIOCFG9, 0x0413 },   // GPIO19 tristate
                    { MTCU_POWERCTRL, 0x13FF },   //
                    { CFGCNT_EXPCFG2, 0x031E },   // global-select pattern to avoid memory effect
                    { MTCU_SPARE, 0x7608 },   // enable global select during pre-illumination (default 10 cycles) and warmup (default 30 cycles) phase

                    //Illumination interface
                    { ANAIP_GPIOMUX5, 0x0000 },   // GPIO16 = EN_ILLU
                    { ANAIP_PADGPIOCFG8, 0x1315 },   // GPIO16 push/pull
                };
            }
        }
    }
}
