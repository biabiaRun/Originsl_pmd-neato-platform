/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageFormatPicoLegacy.hpp>

#include <common/EndianConversion.hpp>

#include <common/exceptions/Exception.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace royale::common;
using namespace royale::storage;
using namespace royale::config;

namespace
{
    /**
     * Expected contents of FlashHeader::magic.
     * The NUL terminator is not part of the expected data.
     */
    const char *FLASH_HEADER_MAGIC = "PMDTEC";

    /** sizeof(FlashHeader) with the compiler and settings that the flash images use */
    const std::size_t SIZE_OF_FLASH_HEADER = 28;

    /** Version that is used for all pico flexx cameras */
    const std::size_t STANDARD_PICO_FLEXX_HEADER_VERSION = 100;

    /**
     * If the calibration data is larger than this, StorageFormatPicoLegacy will assume it has
     * read corrupt data from the flash's header.
     */
    const std::size_t MAXIMUM_CALIBRATION_SIZE = 2 * 1024 * 1024;
}

/**
 * The data in the flash is this structure, packed to 32-bit alignment, little endian, and using
 * uint32_t instead of std::size_t.
 */
struct StorageFormatPicoLegacy::FlashHeader
{
    char magic[6];
    // 2 bytes of padding
    uint32_t headerVersion;
    uint32_t serialNumber;
    uint32_t hardwareRevision;
    uint32_t calibrationAddress;
    uint32_t calibrationSize;
};

StorageFormatPicoLegacy::StorageFormatPicoLegacy (std::shared_ptr<IBridgeWithPagedFlash> bridge, const FlashMemoryConfig &memoryConfig)
{
    if (bridge == nullptr)
    {
        throw LogicError ("nullref exception");
    }

    if (memoryConfig.type != FlashMemoryConfig::FlashMemoryType::PICO_PAGED)
    {
        throw LogicError ("Unsupported flash memory type");
    }

    if (memoryConfig.imageSize == 0u)
    {
        throw LogicError ("Pico requires the imageSize to be configured");
    }

    if (memoryConfig.accessOffset != 0u)
    {
        throw LogicError ("The accessOffset is not supported");
    }

    m_bridge = bridge;
    m_imageSize = memoryConfig.imageSize;
    m_pageSize = memoryConfig.pageSize;
    m_sectorSize = memoryConfig.sectorSize;
}

bool StorageFormatPicoLegacy::readDataFromHardware (struct FlashHeader &header)
{
    std::vector<uint8_t> recvBuffer;
    recvBuffer.resize (m_pageSize);

    // The header is stored at the end of the flash image.  Work out which page number that is,
    // and at what byte offset within the page the final byte of the image is.
    uint32_t lastFlashPage = static_cast<uint32_t> (m_imageSize / m_pageSize);
    std::size_t flashEndInPage = m_imageSize % m_pageSize;
    if (flashEndInPage < SIZE_OF_FLASH_HEADER)
    {
        // The header is 28 bytes.  Assuming that the page size is 256 bytes, the header will fit if
        // the imageSize is either a decimal multiple of 500000 or any hexadecimal number that has
        // two zeros on the end.
        throw NotImplemented ("reading a flash header across a page boundary");
    }

    m_bridge->readFlashPage (lastFlashPage, recvBuffer);

    const uint8_t *headerStart = &recvBuffer[flashEndInPage - SIZE_OF_FLASH_HEADER];
    memcpy (header.magic, headerStart, sizeof (header.magic));
    header.headerVersion = bufferToHost32 (headerStart + 8);
    header.serialNumber = bufferToHost32 (headerStart + 12);
    header.hardwareRevision = bufferToHost32 (headerStart + 16);
    header.calibrationAddress = bufferToHost32 (headerStart + 20);
    header.calibrationSize = bufferToHost32 (headerStart + 24);

    if (0 != std::memcmp (FLASH_HEADER_MAGIC, header.magic, sizeof (header.magic)))
    {
        return false;
    }

    return true;
}

royale::Vector<uint8_t> StorageFormatPicoLegacy::getModuleIdentifier()
{
    // The StorageFormatPicoLegacy format doesn't have a identifier, but it has an unused hardware revision
    // field, which is normally zero if flashed by Royale.  Return that as the identifier.
    struct FlashHeader header;
    if (!readDataFromHardware (header))
    {
        return{};
    }
    // The hardwareRevision has already been converted to host endian, so use the push-back function
    // to return it to a fixed endianness.
    std::vector<uint8_t> identifier;
    pushBack32 (identifier, header.hardwareRevision);
    return identifier;
}

royale::String StorageFormatPicoLegacy::getModuleSuffix ()
{
    //PicoFlexx devices have no module suffix
    return "";
}

royale::String StorageFormatPicoLegacy::getModuleSerialNumber ()
{
    // The module serial number is documented as a opaque byte array, maximum length 128 bytes.
    // On PicoS, it's stored in the flash as a 32-bit number.
    // If no valid blob was found, an empty string is returned
    struct FlashHeader header;
    if (!readDataFromHardware (header))
    {
        return "";
    }
    std::ostringstream serialNr;
    serialNr << header.serialNumber;
    return serialNr.str();
}

royale::Vector<uint8_t> StorageFormatPicoLegacy::getCalibrationData()
{
    struct FlashHeader header;
    if (!readDataFromHardware (header))
    {
        throw RuntimeError ("No valid PMD data blob was found");
    }

    if ( (header.calibrationSize > MAXIMUM_CALIBRATION_SIZE) ||
            (header.calibrationAddress > m_imageSize) ||
            (header.calibrationSize + header.calibrationAddress > m_imageSize) ||
            !header.calibrationSize)
    {
        // the double-check against the m_imageSize is to check for arithmetic overflow
        throw RuntimeError ("Read flash header, found unexpected data");
    }

    // As an optimisation - which was already done:
    // The Enclustra firmware supports reading a large number of pages in a single USB read.
    // It may support reading the entire calibration data in a single read.
    //
    // Single USB flash reads (one read command per execution; executed in a loop) have
    // taken 1200ms all together.
    //
    // By reading the whole calibration data at once the time consumption of the whole
    // process could be optimized to ~660ms

    const std::size_t firstFlashPage    = header.calibrationAddress / m_pageSize;
    const std::size_t lastFlashPage     = (header.calibrationAddress + header.calibrationSize - 1) / m_pageSize;
    const std::size_t pageCount         = 1 + lastFlashPage - firstFlashPage;
    const std::size_t offset            = header.calibrationAddress % m_pageSize;

    std::vector<uint8_t> data;
    data.resize (header.calibrationSize);

    std::vector<uint8_t> recvBuffer;
    recvBuffer.resize (m_pageSize * pageCount);

    m_bridge->readFlashPage (firstFlashPage, recvBuffer, pageCount);    // read all pages at once here
    std::memcpy (&data[0], &recvBuffer[offset], header.calibrationSize);

    return std::move (data);
}

void StorageFormatPicoLegacy::writeCalibrationData (const royale::Vector<uint8_t> &data)
{
    // If the device has a product identifier (hardwareRevision in this format's terminology), the
    // L2 Royale API methods must write the same identifier to the device.  Only the L3 methods can
    // change or remove the identifier.
    //
    // The implementation of that is likely to add a "preserveAdditionalData" argument to this
    // method.  It's currently not in the API, instead it's defined on the next line.
    const bool preserveAdditionalData = true;

    if (!m_pageSize || !m_sectorSize)
    {
        throw RuntimeError ("No page size or sector size given");
    }

    struct FlashHeader header;

    memcpy (&header.magic, FLASH_HEADER_MAGIC, sizeof (header.magic));
    header.headerVersion = STANDARD_PICO_FLEXX_HEADER_VERSION;
    header.serialNumber = 0;
    header.hardwareRevision = 0;
    header.calibrationSize = static_cast<uint32_t> (data.size());
    size_t calibAddress = m_imageSize - sizeof (struct FlashHeader) - data.size();
    header.calibrationAddress = static_cast<uint32_t> (calibAddress);

    if (preserveAdditionalData)
    {
        struct FlashHeader existingHeader;
        if (readDataFromHardware (existingHeader))
        {
            header.serialNumber = existingHeader.serialNumber;
            header.hardwareRevision = existingHeader.hardwareRevision;
        }
    }

    const std::size_t pagesPerSector = m_sectorSize / m_pageSize;
    const std::size_t firstSector = calibAddress / m_sectorSize;
    const std::size_t lastSector = (m_imageSize + m_sectorSize - 1) / m_sectorSize;
    const std::size_t sectorsToErase = lastSector - firstSector + 1;

    // The header is stored at the end of the flash image.  Work out which page number that is,
    // and at what byte offset within the page the final byte of the image is.
    const std::size_t firstFlashPage = firstSector * pagesPerSector;
    const std::size_t lastFlashPage = (lastSector + 1) * pagesPerSector;

    const std::size_t pageCount = lastFlashPage - firstFlashPage + 1;
    const std::size_t headerOffset = m_imageSize - (firstFlashPage * m_pageSize) - SIZE_OF_FLASH_HEADER;
    const std::size_t offset = calibAddress % m_sectorSize;

    std::vector<uint8_t> sendBuffer;
    sendBuffer.resize (m_pageSize * pageCount);

    // First read the pages, then replace the data
    m_bridge->readFlashPage (firstFlashPage, sendBuffer, pageCount); // read all pages at once here

    // The sectors have to be erased before they are written!
    m_bridge->eraseSectors (firstSector, sectorsToErase);

    memcpy (&sendBuffer[headerOffset], &header, sizeof (struct FlashHeader));
    memcpy (&sendBuffer[offset], &data[0], data.size());

    m_bridge->writeFlashPage (firstFlashPage, sendBuffer, pageCount);
}

void StorageFormatPicoLegacy::writeCalibrationData (const royale::Vector<uint8_t> &data,
        const royale::Vector<uint8_t> &identifier,
        const royale::String &suffix,
        const royale::String &serialNumber)
{
    // The format does include serial numbers, but as the functionality for writing them hasn't been
    // missed yet, it seems unlikely to be needed.
    throw NotImplemented ("The serial number and suffix are not supported for StorageFormatPicoLegacy");
}

uint32_t StorageFormatPicoLegacy::getCalibrationDataChecksum()
{
    // This doesn't support the checksum
    return 0;
}
