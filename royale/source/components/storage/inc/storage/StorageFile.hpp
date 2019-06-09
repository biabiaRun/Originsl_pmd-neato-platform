/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <royale/String.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * Wrapper for reading a StorageFormatZwetschge (or other storage format) image from disk.
         */
        class StorageFile :
            public royale::pal::IStorageReadRandom
        {
        public:
            ROYALE_API StorageFile (const royale::config::FlashMemoryConfig &config,
                                    const royale::String &filename);
            ~StorageFile () = default;

            // IStorageReadRandom
            void readStorage (std::size_t startAddr, std::vector<uint8_t> &buf) override;

        private:
            /**
             * What this means is implementation-defined, please read the comments in
             * StorageRoutingFilename.
             */
            const royale::String m_filename;
        };
    }
}
