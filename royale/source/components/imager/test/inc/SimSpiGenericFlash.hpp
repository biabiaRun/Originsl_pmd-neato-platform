/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <vector>

namespace royale
{
    namespace stub
    {
        /**
         * Simulation of the WS25Q10EW or WS25Q20EW storage devices, which are used in the Infineon
         * and PMD reference designs.
         */
        class SimSpiGenericFlash
        {
        public:
            explicit SimSpiGenericFlash();

            /**
             * The contents of the simulated storage may be read and modified by the caller, using
             * the returned pointer. However, the caller must ensure that the map is not accessed
             * at the same time as transfer() is accessing the map.
             *
             * Note that the SPI-via-imager code will try to access this memory using 16-bit aligned
             * reads and writes, to match the size of the imager's registers. Accesses don't need to
             * be aligned, so the caller should ensure that there is 1 byte of padding at the end
             * (regardless of the data alignment), so that a read that includes the final byte can
             * read a 16-bit area that includes the byte afterwards.
             */
            std::shared_ptr<std::map<uint32_t, uint8_t>> getFlashMemorySpace()
            {
                return m_flashMemorySpace;
            }

            /**
             * Respond to an SPI full-duplex transmission. The vector that is returned will be the
             * same size as the one passed in.
             *
             * @param mosi the data that is sent Master Out Slave In
             * @return the data that is sent Master In Slave Out
             */
            std::vector<uint8_t> transfer (const std::vector<uint8_t> &mosi);

        private:
            std::shared_ptr<std::map<uint32_t, uint8_t>> m_flashMemorySpace;
            /** Allow write or erase as the next operation */
            bool m_writeEnabled;
        };
    }
}
