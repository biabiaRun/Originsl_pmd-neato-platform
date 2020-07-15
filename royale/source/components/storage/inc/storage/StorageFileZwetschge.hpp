/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <storage/StorageFile.hpp>

namespace royale
{
    namespace storage
    {
        /**
         * Wrapper for reading a Zwetschge image from disk. The Zwetschge file can be with or without reserved area.
         */
        class StorageFileZwetschge :
            public royale::storage::StorageFile
        {
        public:
            ROYALE_API StorageFileZwetschge (const royale::config::FlashMemoryConfig &config,
                                             const royale::String &filename);
            ~StorageFileZwetschge() = default;

            // IStorageReadRandom
            void readStorage (std::size_t startAddr, std::vector<uint8_t> &buf) override;

        protected:
            royale::Vector<uint8_t> m_internalBuffer;
        };
    }
}
