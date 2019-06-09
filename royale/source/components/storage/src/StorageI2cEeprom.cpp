/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <storage/StorageI2cEeprom.hpp>
#include <pal/II2cBusAccess.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <common/exceptions/DeviceDetectedError.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <algorithm>
#include <thread>
#include <vector>

using namespace royale::common;
using namespace royale::storage;
using namespace royale::config;
using namespace royale::pal;

StorageI2cEeprom::StorageI2cEeprom (const FlashMemoryConfig &config,
                                    std::shared_ptr<II2cBusAccess> access,
                                    uint8_t devAddr) :
    m_access {access},
    m_devAddr {devAddr}
{
    if (config.accessOffset != 0u)
    {
        throw NotImplemented ("The accessOffset is not supported");
    }

    auto pageSize = config.pageSize;
    if (pageSize == 0)
    {
        m_eepromWriteSize = 1;
    }
    else
    {
        m_eepromWriteSize = pageSize;
    }

    m_writeTime = config.writeTime;
    m_imageSize = config.imageSize;
}

void StorageI2cEeprom::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    // Check if this read is inside the image size
    if (m_imageSize > 0 &&
            startAddr + recvBuffer.size() > m_imageSize)
    {
        throw OutOfBounds ("read beyond imageSize");
    }

    // Split bigger buffers in to smaller reads, to match the buffer size in the CX3
    const std::size_t BLOCK_SIZE = std::min(m_access->maximumDataSize(), recvBuffer.size());
    std::vector<uint8_t> block (BLOCK_SIZE);

    for (std::size_t i = 0; i + BLOCK_SIZE <= recvBuffer.size(); i += BLOCK_SIZE)
    {
        readStorageBlock (narrow_cast<uint32_t> (startAddr + i), block);
        std::memcpy (&recvBuffer[i], &block[0], BLOCK_SIZE);
    }
    // is there a final block?
    std::size_t finalSize = recvBuffer.size() % BLOCK_SIZE;
    if (finalSize)
    {
        auto finalOffset = recvBuffer.size() - finalSize;
        block.resize (finalSize);
        readStorageBlock (narrow_cast<uint32_t> (startAddr + finalOffset), block);
        std::memcpy (&recvBuffer[finalOffset], &block[0], block.size());
    }
}

void StorageI2cEeprom::readStorageBlock (uint32_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    // this is part of the main readStorage() function, both the lock and error-checking are
    // handled by the caller

    uint8_t addressTop = static_cast<uint8_t> (startAddr >> 16);
    auto addressLow = static_cast<uint16_t> (startAddr);

    m_access->readI2c (m_devAddr | addressTop, I2cAddressMode::I2C_16BIT, addressLow, recvBuffer);
}

void StorageI2cEeprom::writeStorage (const std::vector<uint8_t> &buffer)
{
    // Check if this write is inside the image size
    if (m_imageSize > 0 &&
            buffer.size() > m_imageSize)
    {
        throw OutOfBounds ("write beyond imageSize");
    }


    /**
     * Sometimes writing to the EEPROM fails, but retrying succeeds.  The write will abort (which
     * will leave the storage in a undefined state) if there are more errors than
     * (bytes written / RETRY_FAIL_RATIO).
     *
     * The value was arbitrarily chosen, and is a size_t so that int promotion doesn't cause a
     * signed/unsigned comparison.
     */
    const std::size_t RETRY_FAIL_RATIO = 100;

    // How many bytes can be written at once (a property of the EEPROM hardware).  The limitation is
    // not the write size itself, but that the complete write must fit within a single page of the
    // EEPROM.  This is satisfied by making all writes at most this size, and starting each one on a
    // page boundary (a multiple of this size).
    std::size_t writeSize = std::min (m_eepromWriteSize, m_access->maximumDataSize());
    std::vector<uint8_t> data (writeSize, 0);

    std::size_t errorCount = 0;

    for (std::size_t i = 0; i < buffer.size() ; i += writeSize)
    {
        // delay for the previous write to finish
        if (i > 0u && m_writeTime != std::chrono::microseconds::zero())
        {
            std::this_thread::sleep_for (m_writeTime);
        }

        // for the final write
        if (i + writeSize > buffer.size())
        {
            writeSize = buffer.size() - i;
            data.resize (writeSize);
        }

        uint8_t addressTop = static_cast<uint8_t> (i >> 16);
        auto addressLow = static_cast<uint16_t> (i);

        std::memcpy (&data[0], &buffer[i], writeSize);

        try
        {
            m_access->writeI2c (m_devAddr | addressTop, I2cAddressMode::I2C_16BIT, addressLow, data);
        }
        catch (RuntimeError &e)
        {
            errorCount++;

            if (i == 0)
            {
                // Maybe this is trying to write to the wrong I2C address.  If it's writing to the
                // correct device but getting an error, quitting now means the old data is likely
                // to still be useable.
                LOG (ERROR) << "Write failed, on the first page";
                throw RuntimeError ("Write failed, on the first write");
            }
            else if (errorCount > i / RETRY_FAIL_RATIO)
            {
                LOG (ERROR) << "Write failed, " << errorCount << " errors during the first " << i << " bytes";
                throw RuntimeError ("Write failed, some bytes written");
            }

            // log and retry
            LOG (ERROR) << "Error " << e.what() << " while writing page " << i << ", retrying";
            i -= writeSize;
            continue;
        }
    }
}
