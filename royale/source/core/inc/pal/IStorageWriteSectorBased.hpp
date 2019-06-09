/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <config/FlashMemoryConfig.hpp>

#include <cstdint>

namespace royale
{
    namespace pal
    {
        /**
         * This provides access to a single area of non-volatile storage (eeprom, flash or a file
         * on disk), which can be written to. The behavior is that of flash memory: to write to a
         * location, an area must be erased, and the area erased is probably larger than the data to
         * be written.
         *
         * Although this interface does not specify any read method, implementations are likely to
         * also provide IStorageReadRandom.
         */
        class IStorageWriteSectorBased
        {
        public:
            virtual ~IStorageWriteSectorBased() = default;

            /**
             * Write data to the storage starting at address startAddr, taking account of any access
             * offset.
             *
             * This will result in the relevant sector(s) being erased, with any data that was in
             * them overwritten. The startAddr must be aligned to the start of a sector because the
             * start of the sector would be overwritten anyway, the end does not need to be aligned.
             *
             * \throws RuntimeError if startAddr is not aligned to the start of a sector
             * \throws RuntimeError if an error occurs
             */
            virtual void writeSectorBased (std::size_t startAddr, const std::vector<uint8_t> &buffer) = 0;
        };
    }
}
