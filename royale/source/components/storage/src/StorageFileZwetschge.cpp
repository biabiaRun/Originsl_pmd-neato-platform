/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/FileSystem.hpp>
#include <storage/StorageFileZwetschge.hpp>
#include <pal/II2cBusAccess.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <algorithm>
#include <array>

using namespace royale::common;
using namespace royale::storage;
using namespace royale::config;
using namespace royale::pal;

namespace
{
    const auto ZWETSCHGE_TOC_MAGIC = std::array<uint8_t, 9>
                                     {
                                         {
                                             'Z', 'W', 'E', 'T', 'S', 'C', 'H', 'G', 'E'
                                         }
                                     };
}

StorageFileZwetschge::StorageFileZwetschge (const FlashMemoryConfig &config,
        const royale::String &filename) :
    StorageFile (config, filename)
{
    royale::Vector<uint8_t> buffer;
    royale::common::readFileToVector (m_filename, buffer);
    if (std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), buffer.begin()))
    {
        m_internalBuffer.resize (buffer.size() + 0x2000u, 0u);
        memcpy (&m_internalBuffer[0x2000], &buffer[0], buffer.size());
    }
    else
    {
        m_internalBuffer.resize (buffer.size(), 0u);
        memcpy (&m_internalBuffer[0], &buffer[0], buffer.size());
    }
}

void StorageFileZwetschge::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    if (startAddr + recvBuffer.size() > m_internalBuffer.size())
    {
        throw OutOfBounds ("Read was larger than then file size");
    }

    std::copy_n (&m_internalBuffer.at (startAddr), recvBuffer.size(), recvBuffer.begin());
}
