/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

namespace royale
{
    namespace storage
    {
        namespace imagerspi_m2452
        {
            /**
             * This firmware makes the M2452 act as an SPI bus master for peripheral access,
             * and not as an imager.
             */
            extern const std::vector<uint16_t> ImagerSpiFirmwareM2452;

            /**
             * Registers, some of which are likely to be common to all implementations of the SPI
             * firmware, and some are likely to be specific to the M2452.  For now, all of these are
             * in an M2452-only namespace.
             */
            enum SpiReg : uint16_t
            {
                READY = 0x9800,
                CONTINUE = 0x9801,
                BYTE_COUNT = 0xb040,
                CMD_ADR2 = 0xb041,
                ADR1 = 0xb042,
                MODE = 0xb043,
                STATUS = 0xb41f,

                ANAIP_DESIGNSTEP = 0xa0a5,
                /** Where to load the firmware (start of the firmware program address space) */
                ISM_FWSTARTADDRESS = 0xa800,
                ISM_ENABLE = 0xb400,
                ISM_CTRL = 0xb401,
                ISM_MEMPAGE = 0xb402
            };

            /** Values for SpiReg::MODE */
            enum Mode : uint16_t
            {
                STANDALONE = 0x0001,
                CONTINUOUS = 0x0002,
            };

            /**
             * Expected values to read from ANAIP_DESIGNSTEP, which will be supported by the
             * ImagerSpiFirmwareM2452.
             */
            extern const std::vector<uint16_t> SUPPORTED_DESIGN_STEPS;
        }
    }
}
