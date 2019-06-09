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
         * This provides read-only access to a single area of non-volatile storage (eeprom, flash, a
         * file on disk, etc), which is accessed using reads starting at arbitrary locations.
         *
         * Only a single such storage area is supported by this interface; if a device has multiple
         * areas of non-volatile storage then it may provide multiple instances of this interface.
         *
         * The implementation is assumed to know how to map addresses to the underlying storage, even where
         * this is complex (paged memory, 17-bit addresses where the top bit must be sent
         * separately, etc).
         *
         * The client is assumed to know the limit of the address size (but the Bridge will throw an
         * exception if an access is detectably out-of-bounds).
         *
         * A client of this class is likely to also be a subclass of INonVolatileStorage, this is
         * the layer below that.
         *
         * If the memory is also writable, then the class implementing this may also implement a
         * write interface (which might not allow full random-access).
         */
        class IStorageReadRandom
        {
        public:
            virtual ~IStorageReadRandom() = default;

            /**
             * Read the contents of the memory (if present) in the module.  There is no alignment
             * restriction on the start address, and the buffer size determines how many bytes are
             * read.
             *
             * This class will split the read in to smaller reads, if necessary.  If any paging
             * needs to happen, it will be done transparently by the implementation.
             *
             * \param startAddr the offset where the data should be read from
             *
             * \param recvBuffer how much data to read, and where to store the data
             */
            virtual void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) = 0;
        };
    }
}
