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

#include <cstddef>
#include <cstdint>
#include <vector>

namespace royale
{
    namespace storage
    {
        /**
         * This Bridge provides access to an area of flash memory, which is accessed using reads of
         * complete pages.
         *
         * The Bridge is assumed to know which flash memory is being accessed, the client only has
         * to provide the page number.
         */
        class IBridgeWithPagedFlash
        {
        public:
            virtual ~IBridgeWithPagedFlash() = default;

            /**
             * Read the contents of the flash memory (if present) in the module.
             *
             * The caller must be configured to know where in the flash the data is, and also to
             * know how large the flash pages are.  The vector must have been resize()'d by the
             * caller to the correct page size.
             *
             * Conversion from addresses to page numbers should be handled by the caller.
             *
             * \param page to read, (measured in pages, not in bytes, from the start of the flash)
             *
             * \param recvBuffer how much data to read, and where to store the data
             *
             * \param noPages number of pages to read
             */
            virtual void readFlashPage (std::size_t page, std::vector<uint8_t> &recvBuffer, std::size_t noPages = 1) = 0;

            /**
            * Write into the flash memory (if present) of the module.
            *
            * The caller must be configured to know where in the flash the data is, and also to
            * know how large the flash pages are.  The vector must have been resize()'d by the
            * caller to the correct page size.
            *
            * Conversion from addresses to page numbers should be handled by the caller.
            *
            * \param page to write, (measured in pages, not in bytes, from the start of the flash)
            *
            * \param sndBuffer data that should be written
            *
            * \param noPages number of pages to write
            */
            virtual void writeFlashPage (std::size_t page, const std::vector<uint8_t> &sndBuffer, std::size_t noPages = 1) = 0;


            /**
            * Erases the given sectors of the flash memory of the module.
            *
            * \param startSector first sector which should be erased
            *
            * \param noSectors number of sectors to erase
            */
            virtual void eraseSectors (std::size_t startSector, std::size_t noSectors) = 0;
        };
    }
}
