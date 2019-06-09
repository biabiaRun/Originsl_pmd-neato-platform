/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cstdint>

namespace royale
{
    namespace imager
    {
        /* This namespace only contains register addresses common to all M2453 design steppings,
        * and also the M2455 A11. */
        namespace same_in_M2453_and_M2455
        {
            const uint16_t PIXMEM =    0x0000; //!< The start address of the pixel memory
            const uint16_t CFGCNT =    0x9000; //!< The start address of the configuration container section
            const uint16_t SPICFG =    0xA087; //!< SPI Enable
            const uint16_t SPIWRADDR = 0xA088; //!< Identifies the address from where commands and data shall be written to SPI
            const uint16_t SPIRADDR =  0xA089; //!< Identifies the address where data read from SPI shall be written

            /**
            * SPI Command Length
            * Bit 15:14 RD: Define when/if to start sampling data from SDI
            * Bit 13:13 ER: Enable Read - enables sampling from SDI for read commands
            * Bit 8:0   SPI_LEN: Total number of bytes to be transferred - consists of Header + Payload
            */
            const uint16_t SPILEN =    0xA08A;

            const uint16_t SPITRIG =   0xA08B; //!< SPI Trigger Register
            const uint16_t SPISTATUS = 0xA08C; //!< SPI Status

            const uint16_t CFGCNT_S00_EXPOTIME  = 0x9000;  //!< First sequence entry exposure time (prescaler + counter)
            const uint16_t CFGCNT_S00_FRAMETIME = 0x9001;  //!< First sequence entry frame time
            const uint16_t CFGCNT_S01_EXPOTIME  = 0x9002;  //!< Second sequence entry exposure time (prescaler + counter)
            // ...the list goes on
            const size_t nSequenceEntries = 64;

            /**
            * User interaction flags
            * Bit 0:0 config_changed
            * Bit 1:1 use_case_changed
            */
            const uint16_t CFGCNT_FLAGS = 0x9402;

            const uint16_t ANAIP_DESIGNSTEP = 0xA0A5; //!< Design step register
        }

        namespace M2453
        {
            using namespace royale::imager::same_in_M2453_and_M2455;
        }

        /* This namespace only contains register addresses common to all M2453 design steppings, but
        * not the M2455. This is separated so that it doesn't get used accidentally in the abstract
        * class ImagerM2453, which is also used for M2455. */
        namespace M2453_ONLY
        {
            const uint16_t DMEM_CONFIG_CHANGED_COUNTER = 0x8330; //!< 12 bit reconfig counter. Not safe to read while capturing!
            const uint16_t MEMPAGE                     = 0x8429; //!< Select IRAM page
        }

        namespace M2453_A11
        {
            const uint16_t ANAIP_EFUSEVAL1 = 0xa096; //EFUSE Value 1
            const uint16_t ANAIP_EFUSEVAL2 = 0xa097; //EFUSE Value 2
            const uint16_t ANAIP_EFUSEVAL3 = 0xa098; //EFUSE Value 3
            const uint16_t ANAIP_EFUSEVAL4 = 0xa099; //EFUSE Value 4
        }

        namespace M2453_B11
        {
            const uint16_t ANAIP_EFUSEVAL1 = 0xa097; //EFUSE Value 1
            const uint16_t ANAIP_EFUSEVAL2 = 0xa098; //EFUSE Value 2
            const uint16_t ANAIP_EFUSEVAL3 = 0xa099; //EFUSE Value 3
            const uint16_t ANAIP_EFUSEVAL4 = 0xa09A; //EFUSE Value 4

            const uint16_t SFR_CFGCNT_TESTRES               = 0x93E2; // Result register for the SPI READ function
            const uint16_t SFR_CFGCNT_GENERIC_FW_PARAM_0    = 0x9380; // iSM function parameter 0
            const uint16_t SFR_CFGCNT_GENERIC_FW_PARAM_1    = 0x9381; // iSM function parameter 1
            const uint16_t FUNCNUM                          = 0x93E1; // iSM function number
            const uint16_t RESTARTV_EN                      = 0x8452; // iSM restart vector for enable & restart
            const uint16_t ISM_CTRL                         = 0x8401; // iSM control register

            const uint16_t MB0_FRAMETIME = 0x91D2; //!< MB0 framerate also master framerate for interleaved mode
        }
    }
}
