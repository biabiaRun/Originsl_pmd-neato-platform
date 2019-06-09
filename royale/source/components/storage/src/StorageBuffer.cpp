/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/EndianConversion.hpp>
#include <common/FileSystem.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <storage/StorageBuffer.hpp>
#include <pal/II2cBusAccess.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <algorithm>
#include <chrono>
#include <thread>

using namespace royale::common;
using namespace royale::storage;
using namespace royale::config;
using namespace royale::pal;

StorageBuffer::StorageBuffer (const FlashMemoryConfig &config,
                              const royale::Vector<uint8_t> &buffer) :
    m_buffer (buffer)
{
    if (config.accessOffset != 0u)
    {
        throw NotImplemented ("The accessOffset is not supported");
    }

    if (config.pageSize != 0)
    {
        throw NotImplemented ("The pageSize is not supported");
    }

    if (config.writeTime != std::chrono::microseconds::zero())
    {
        throw NotImplemented ("The writeTime is not supported");
    }

    if (config.imageSize != 0)
    {
        throw NotImplemented ("The imageSize is not supported");
    }

    if (m_buffer.empty())
    {
        throw RuntimeError ("Buffer is empty");
    }
}

void StorageBuffer::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    if (m_buffer.size() < startAddr + recvBuffer.size())
    {
        throw OutOfBounds ("Read was beyond buffer limit");
    }

    std::copy_n (&m_buffer.at (startAddr), recvBuffer.size(), recvBuffer.begin());
}
