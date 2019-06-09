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

#include <cstddef>
#include <cstdint>
#include <vector>

namespace royale
{
    namespace pal
    {
        /**
         * This provides access to a single area of non-volatile storage (eeprom, flash or a file
         * on disk), which can be written to.  Any write replaces the previous data, starting at the
         * first byte.
         *
         * If the new data is smaller than the old data, then the result of trying to read the
         * "non-overwritten" bytes causes undefined behaviour.  This allows flash memory to erase
         * blocks, or file-based storage to be truncated.
         *
         * The implementation is assumed to know how to map addresses to the underlying storage, even where
         * this is complex (paged memory, 17-bit addresses where the top bit must be sent
         * separately, etc).
         *
         * The client is assumed to know the limit of the address size.
         */
        class IStorageWriteFullOverwrite
        {
        public:
            virtual ~IStorageWriteFullOverwrite() = default;

            /**
             * Write to the memory, the buffer size determines how many bytes are written.
             *
             * This class will split the read in to smaller writes, and erase blocks, if necessary.
             * If any paging needs to happen, it will be done transparently by the implementation.
             */
            virtual void writeStorage (const std::vector<uint8_t> &buffer) = 0;
        };
    }
}
