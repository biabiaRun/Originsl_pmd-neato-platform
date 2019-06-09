/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <storage/SpiBusMasterM2453.hpp>
#include <storage/SpiGenericFlash.hpp>

#include <royale/Vector.hpp>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>
#include <iomanip>

#include <common/EndianConversion.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/IntegerMath.hpp>
#include <common/NarrowCast.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;

namespace
{
    /**
     * These SPI chips put the data addresses in to 24 bits. Therefore this is the limit of the
     * supportable image size.
     */
    const auto MAX_IMAGE_SIZE = 1u << 24;

    /**
     * Command set for the expected storage device
     */
    enum WS25Q10EW_COMMAND_SET : uint8_t
    {
        ERASE = 0x20,
        READ = 0x03,
        STATUS = 0x05,
        WRITE_ENABLE = 0x06,
        WRITE = 0x02,
    };

    /**
     * We can write at most one page at a time
     */
    const std::size_t numBytesPerWrite = 256u;

    /**
     * The memory is assumed to have 4096 bytes per sector
     */
    const std::size_t numBytesPerSector = 4096;

    /**
     * The time (in milliseconds) that waitForWriteComplete sleeps between polls, and how many polls to make.
     *
     * Some reference times from datasheets:
     * M25P40's page-program takes <1ms typical, 5ms max, its sector erase is 600ms typical, 3000ms max.
     * WS25Q10EW's page-program is <1ms max, its sector erase is 45ms typical, 400ms max.
     */
    const auto POLL_PATTERN = std::vector<int>
                              {
                                  {
                                      0, 1, 1, 3, 5, 40, 100, 150, 150, 150, 150, 150, 1000, 1000, 1000, 1000
                                  }
                              };
}

SpiGenericFlash::SpiGenericFlash (const royale::config::FlashMemoryConfig &config,
                                  std::shared_ptr<royale::pal::ISpiBusAccess> access,
                                  const royale::common::SensorRoutingConfigSpi &busAddress) :
    m_access (access),
    m_busAddress {busAddress},
    m_accessOffset {narrow_cast<device_spi_size_t> (config.accessOffset) },
    m_imageSize {narrow_cast<device_spi_size_t> (config.imageSize) }
{
    if (m_imageSize > MAX_IMAGE_SIZE)
    {
        throw LogicError ("SpiGenericFlash only supports a 24-bit address space");
    }
    else if (m_imageSize == 0)
    {
        m_imageSize = MAX_IMAGE_SIZE;
    }

    if (config.pageSize && config.pageSize != m_access->maximumReadSize().receive)
    {
        throw NotImplemented ("Only the hardcoded page size is supported");
    }
}

SpiGenericFlash::~SpiGenericFlash() = default;

void SpiGenericFlash::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    auto lock = m_access->selectDevice (m_busAddress.getAddress());

    startAddr += m_accessOffset;

    if (m_imageSize && (startAddr + recvBuffer.size() - 1 > m_imageSize))
    {
        throw OutOfBounds ("Read exceeds storage size");
    }

    // Split bigger buffers in to smaller reads, to match the hardware limit
    const std::size_t BLOCK_SIZE = m_access->maximumReadSize().receive;
    std::vector<uint8_t> block (BLOCK_SIZE);

    for (std::size_t i = 0; i + BLOCK_SIZE <= recvBuffer.size(); i += BLOCK_SIZE)
    {
        readStorageBlock (narrow_cast<device_spi_size_t> (startAddr + i), block);
        std::memcpy (&recvBuffer[i], &block[0], BLOCK_SIZE);
    }
    // is there a final block?
    const std::size_t finalSize = recvBuffer.size() % BLOCK_SIZE;
    if (finalSize)
    {
        auto finalOffset = recvBuffer.size() - finalSize;
        block.resize (finalSize);
        readStorageBlock (narrow_cast<device_spi_size_t> (startAddr + finalOffset), block);
        std::memcpy (&recvBuffer[finalOffset], &block[0], finalSize);
    }
}

void SpiGenericFlash::readStorageBlock (device_spi_size_t startAddr, std::vector<uint8_t> &buffer)
{
    // caller handles auto lock = selectDevice(); and the m_accessOffset
    std::vector<uint8_t> commandBuffer;
    commandBuffer.push_back (WS25Q10EW_COMMAND_SET::READ);
    pushBackBe24 (commandBuffer, startAddr);
    m_access->readSpi (commandBuffer, buffer);
}

void SpiGenericFlash::writeSectorBased (std::size_t startAddr, const std::vector<uint8_t> &buffer)
{
    auto lock = m_access->selectDevice (m_busAddress.getAddress());

    startAddr += m_accessOffset;
    if (startAddr % numBytesPerSector)
    {
        throw LogicError ("Write does not start on a sector boundary");
    }

    if (m_imageSize && (startAddr + buffer.size() - 1 > m_imageSize))
    {
        throw OutOfBounds ("Write exceeds storage size");
    }

    // erase sectors
    for (std::size_t sectorAddress = startAddr; sectorAddress < startAddr + buffer.size(); sectorAddress += numBytesPerSector)
    {
        std::vector<uint8_t> commandBuffer;
        commandBuffer.push_back (WS25Q10EW_COMMAND_SET::WRITE_ENABLE);
        m_access->writeSpi (commandBuffer);

        commandBuffer.clear();
        commandBuffer.push_back (WS25Q10EW_COMMAND_SET::ERASE);
        pushBackBe24 (commandBuffer, narrow_cast<uint32_t> (sectorAddress));
        m_access->writeSpi (commandBuffer);

        waitForWriteComplete();
    }

    // Split bigger buffers in to smaller writes, to match the hardware limit
    const std::size_t BLOCK_SIZE = numBytesPerWrite;

    for (std::size_t i = 0; i + BLOCK_SIZE <= buffer.size(); i += BLOCK_SIZE)
    {
        const auto block = std::vector<uint8_t> (buffer.cbegin() + i, buffer.cbegin() + i + BLOCK_SIZE);
        writeStorageBlock (narrow_cast<device_spi_size_t> (startAddr + i), block);
    }
    // is there a final block that's smaller than BLOCK_SIZE?
    const std::size_t finalSize = buffer.size() % BLOCK_SIZE;
    if (finalSize)
    {
        auto finalOffset = buffer.size() - finalSize;
        auto block = std::vector<uint8_t> (buffer.cbegin() + finalOffset, buffer.cend());
        writeStorageBlock (narrow_cast<device_spi_size_t> (startAddr + finalOffset), block);
    }
}

void SpiGenericFlash::writeStorageBlock (device_spi_size_t startAddr, const std::vector<uint8_t> &buffer)
{
    // caller handles auto lock = selectDevice(); and the m_accessOffset
    std::vector<uint8_t> commandBuffer;
    commandBuffer.push_back (WS25Q10EW_COMMAND_SET::WRITE_ENABLE);
    m_access->writeSpi (commandBuffer);

    commandBuffer.clear();
    commandBuffer.push_back (WS25Q10EW_COMMAND_SET::WRITE);
    pushBackBe24 (commandBuffer, startAddr);
    pushBackIterable (commandBuffer, buffer);
    m_access->writeSpi (commandBuffer);

    waitForWriteComplete();
}

void SpiGenericFlash::waitForWriteComplete()
{
    const uint16_t BUSY_FLAG = 1;
    const auto queryCommand = std::vector<uint8_t> {WS25Q10EW_COMMAND_SET::STATUS};

    // on success, this loop exits via the return statement
    for (const auto &delay : POLL_PATTERN)
    {
        if (delay)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds {delay});
        }

        auto status = std::vector<uint8_t> (1);
        m_access->readSpi (queryCommand, status);
        if (! (status[0] & BUSY_FLAG))
        {
            return;
        }
    }

    throw Timeout ("Spi flash device still busy");
}
