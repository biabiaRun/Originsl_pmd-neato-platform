/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
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
#include <storage/StorageFile.hpp>
#include <pal/II2cBusAccess.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <common/exceptions/DeviceDetectedError.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <algorithm>
#include <chrono>
#include <thread>

using namespace royale::common;
using namespace royale::storage;
using namespace royale::config;
using namespace royale::pal;

StorageFile::StorageFile (const FlashMemoryConfig &config,
                          const royale::String &filename) :
    m_filename (filename)
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

    // Should this support FlashMemoryConfig::image_size as an expected file size (i.e. minimum
    // size) or expected storage limit in the final device (i.e. maximum size)?
    if (config.imageSize != 0)
    {
        throw NotImplemented ("The imageSize is not supported");
    }

    if (!royale::common::fileexists (m_filename))
    {
        throw CouldNotOpen ("Source file for StorageFile not found");
    }
}

void StorageFile::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    royale::Vector<uint8_t> buffer;
    auto bytesRead = royale::common::readFileToVector (m_filename, buffer);

    if (bytesRead < startAddr + recvBuffer.size())
    {
        throw OutOfBounds ("Read was larger than then file size");
    }

    std::copy_n (&buffer.at (startAddr), recvBuffer.size(), recvBuffer.begin());
}
