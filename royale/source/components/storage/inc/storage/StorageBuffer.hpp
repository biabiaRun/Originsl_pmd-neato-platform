/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
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
#include <royale/Vector.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * Wrapper for reading a StorageFormatZwetschge (or other storage format) image from a buffer.
         */
        class StorageBuffer :
            public royale::pal::IStorageReadRandom
        {
        public:
            ROYALE_API StorageBuffer (const royale::config::FlashMemoryConfig &config,
                                      const royale::Vector<uint8_t> &buffer);
            ~StorageBuffer() = default;

            // IStorageReadRandom
            void readStorage (std::size_t startAddr, std::vector<uint8_t> &buf) override;

        private:
            const royale::Vector<uint8_t> m_buffer;
        };
    }
}
