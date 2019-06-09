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

#include <common/exceptions/OutOfBounds.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>

#include <gtest/gtest.h>

#include <memory>

namespace royale
{
    namespace stub
    {
        namespace storage
        {
            /**
             * For testing the data-parsing classes that read from an IStorageReadRandom, this mock
             * wraps a vector<uint8_t> and provides IStorageReadRandom and IStorageWriteFullOverwrite
             * methods to access it.
             */
            class MockRandomAccessStorage :
                public royale::pal::IStorageReadRandom,
                public royale::pal::IStorageWriteFullOverwrite
            {
                /**
                 * This is a special-case to allow the size of StorageFormatPolar's V7 header to be
                 * read, even if the sample data is the smaller v2, v3 or v101 header (with no
                 * calibration data).  On a real device, it will always be possible to read this
                 * amount of data (it just overlaps with the start of the actual calibration data).
                 */
                static const std::size_t MINIMUM_READABLE_SIZE = 67;

            public:
                explicit MockRandomAccessStorage (const std::vector<uint8_t> &rawData) :
                    m_rawData (rawData)
                {
                }

                ~MockRandomAccessStorage() override = default;

                void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuf) override
                {
                    if (startAddr + recvBuf.size() <= m_rawData.size())
                    {
                        std::memcpy (recvBuf.data(), &m_rawData[startAddr], recvBuf.size());
                    }
                    else if (startAddr + recvBuf.size() <= MINIMUM_READABLE_SIZE)
                    {
                        std::memset (recvBuf.data(), 0, recvBuf.size());
                        if (startAddr < m_rawData.size())
                        {
                            const std::size_t readable = m_rawData.size() - startAddr;
                            std::memcpy (recvBuf.data(), &m_rawData[startAddr], readable);
                        }
                    }
                    else
                    {
                        throw royale::common::OutOfBounds();
                    }
                }

                void writeStorage (const std::vector<uint8_t> &buffer) override
                {
                    m_rawData = buffer;
                }

            private:
                std::vector<uint8_t> m_rawData;
            };
        }
    }
}
