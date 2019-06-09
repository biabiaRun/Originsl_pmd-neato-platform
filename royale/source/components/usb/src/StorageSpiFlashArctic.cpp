/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <usb/bridge/StorageSpiFlashArctic.hpp>

#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <algorithm>

using namespace royale::common;
using namespace royale::config;
using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;
using namespace royale::pal;

StorageSpiFlashArctic::StorageSpiFlashArctic (const FlashMemoryConfig &config,
        std::shared_ptr<royale::usb::pal::arctic::SpiBusAccessArctic> access,
        uint8_t deviceId) :
    m_access {access},
    m_devAddr {deviceId}
{
    m_imageSize = config.imageSize;

    auto pageSize = config.pageSize;
    if (pageSize == 0)
    {
        m_eepromWriteSize = 1;
    }
    else
    {
        m_eepromWriteSize = pageSize;
    }

    // If the sectorSize is zero, this class will throw an exception if there's a call to
    // writeStorage.  But it will still function as a read-only memory.
    m_sectorSize = config.sectorSize;

    m_accessOffset = config.accessOffset;

    if (m_sectorSize && (m_accessOffset % m_sectorSize))
    {
        throw NotImplemented ("Offset memory access is only supported when aligned to sector boundaries");
    }
}

void StorageSpiFlashArctic::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    // \todo ROYAL-1322 refactor these multi-block reads in to a common algorithm.  For now, this
    // code is as close as possible to the StorageI2cEeprom's implementation.
    auto lock = selectDevice();

    startAddr += m_accessOffset;

    if (m_imageSize && (startAddr + recvBuffer.size() - 1 > m_imageSize))
    {
        throw OutOfBounds ("Read exceeds storage size");
    }

    // Split bigger buffers in to smaller reads, to match the buffer size in the CX3
    const std::size_t BLOCK_SIZE = m_access->maximumReadSize().receive;
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

void StorageSpiFlashArctic::readStorageBlock (device_spi_size_t startAddr, std::vector<uint8_t> &buffer)
{
    // caller handles auto lock = selectDevice(); and the m_accessOffset
    uint8_t addressHigh = static_cast<uint8_t> (startAddr >> 16);
    uint16_t addressLow = static_cast<uint16_t> (startAddr);

    m_access->getExtension().checkedGet (VendorRequest::SPI_PAGES, addressHigh, addressLow, buffer);
}

void StorageSpiFlashArctic::writeStorage (const std::vector<uint8_t> &buffer)
{
    if (m_imageSize && (m_accessOffset + buffer.size() - 1 > m_imageSize))
    {
        throw OutOfBounds ("Write exceeds storage size");
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

    // The configured m_eepromWriteSize is the number of bytes that can written at once (a property
    // of the EEPROM hardware).  The limitation is not the write size itself, but that the complete
    // write must fit within a single page of the EEPROM.  This is satisfied by making all writes
    // except the last one exactly this size, so that the following write starts on a multiple of
    // this size.
    //
    // Sanity check the size, compared to the Arctic protocol.  As this class uses the Arctic
    // firmware's VendorRequest::SPI_PAGES directly, the maximumWriteSize() function isn't directly
    // applicable, but if the length doesn't fit then something unexpected has happened, and this
    // function bails out before erasing the existing data.
    {
        const auto lengthCommandAndData = std::size_t (4) + m_eepromWriteSize;
        if (lengthCommandAndData > m_access->maximumWriteSize()
            || lengthCommandAndData > arctic::MAXIMUM_DATA_SIZE)
        {
            throw LogicError ("configured page size is larger than the spi bus master can support");
        }
    }
    std::size_t writeSize = m_eepromWriteSize;
    std::vector<uint8_t> data (writeSize, 0);

    std::size_t errorCount = 0;

    auto lock = selectDevice();

    // erase
    if (m_sectorSize == 0)
    {
        throw InvalidValue ("The memory has only been configured for reading, not writing");
    }
    for (std::size_t i = 0; i < buffer.size() ; i += m_sectorSize)
    {
        bool noSuccessfulIO = true;
        try
        {
            eraseSpiSector (i + m_accessOffset);
            noSuccessfulIO = false;
        }
        catch (const RuntimeError &)
        {
            if (noSuccessfulIO)
            {
                // Maybe this is trying to write to the wrong device.  If it's writing to the
                // correct device but getting an error, quitting now means the old data may
                // still be useable.
                LOG (ERROR) << "Erase failed, on the first sector";
                throw RuntimeError ("Erase failed, on the first write");
            }
            else
            {
                LOG (ERROR) << "Erase failed, after the first sector";
                throw RuntimeError ("Erase failed, some bytes erased");
            }
        }
    }

    for (std::size_t i = 0; i < buffer.size() ; i += writeSize)
    {
        // for the final write
        if (i + writeSize > buffer.size())
        {
            writeSize = buffer.size() - i;
            data.resize (writeSize);
        }

        std::memcpy (&data[0], &buffer[i], writeSize);

        try
        {
            writeSpi (i + m_accessOffset, data);
        }
        catch (RuntimeError &e)
        {
            errorCount++;

            if (errorCount > i / RETRY_FAIL_RATIO)
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

std::unique_lock<std::recursive_mutex> StorageSpiFlashArctic::selectDevice()
{
    // This locks more than just the SPI
    return m_access->selectDevice (m_devAddr);
}

void StorageSpiFlashArctic::writeSpi (std::size_t startAddr, const std::vector<uint8_t> &buffer)
{
    uint8_t addressHigh = static_cast<uint8_t> (startAddr >> 16);
    uint16_t addressLow = static_cast<uint16_t> (startAddr);

    m_access->getExtension().checkedSet (VendorRequest::SPI_PAGES, addressHigh, addressLow, buffer);
}

void StorageSpiFlashArctic::eraseSpiSector (std::size_t startAddr)
{
    auto lock = selectDevice();

    uint8_t addressHigh = static_cast<uint8_t> (startAddr >> 16);
    uint16_t addressLow = static_cast<uint16_t> (startAddr);
    m_access->getExtension().checkedSet (VendorRequest::SPI_ERASE, addressHigh, addressLow);

    while (isWriteInProgress())
    {
        //spin
    }
}

bool StorageSpiFlashArctic::isWriteInProgress ()
{
    std::vector<uint8_t> buf (1);
    m_access->getExtension().checkedGet (VendorRequest::SPI_ERASE, 0, 0, buf);
    return buf[0] != 0u;
}
