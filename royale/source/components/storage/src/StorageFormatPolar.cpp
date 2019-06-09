/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageFormatPolar.hpp>

#include <common/EndianConversion.hpp>
#include <common/Crc32.hpp>

#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace royale::storage;
using namespace royale::pal;
using namespace royale::common;

namespace
{
    /**
     * Expected contents of EEPROMHeader::magic.
     * The NUL terminator is not part of the expected data.
     */
    const char *FLASH_HEADER_MAGIC = "PMDTEC";

    /**
     * Length of the FLASH_HEADER_MAGIC, excluding the NUL terminator.
     */
    const std::size_t SIZEOF_HEADER_MAGIC = 6;

    /**
     * sizeof(version 2 FlashHeader) with the compiler and settings that the flash images use
     *
     * This is the deprecated version 2 header, in the first run of C2 devices.
     */
    const std::size_t SIZE_OF_V2_FLASH_HEADER = 28;

    /**
     * sizeof(FlashHeader) with the compiler and settings that the flash images use
     */
    const std::size_t SIZE_OF_V3_FLASH_HEADER = 16;

    /**
     * Special version with 2 v3 headers directly after another
     */
    const std::size_t SIZE_OF_V101_FLASH_HEADER = 2 * SIZE_OF_V3_FLASH_HEADER;

    /**
     * If the calibration data is larger than this, then StorageFormatPolar assumes it has
     * read corrupt data from the flash's header.
     */
    const std::size_t MAXIMUM_CALIBRATION_SIZE = 20 * 1024 * 1024;

    /**
     * Please take a look at
     * https://git.ifm.com/projects/ROYALE/repos/royaledoc/browse/content/eeprom_format.md
     * for a description of the header format v6
     */
    enum DataBlockID
    {
        SPC_ASTON = 0,
        PRODUCT_ID_V1 = 1,
        LENSDATA_V1 = 2,
        EFFICIENCY_V1 = 3
    };

    /**
     * sizeof(FlashHeader) with the compiler and settings that the flash images use
     */
    const std::size_t SIZE_OF_V6_FLASH_HEADER = 16;

    /**
     * Size of the complete V7-format header.
     */
    const std::size_t SIZE_OF_V7_FLASH_HEADER = 67;

    /** The V7 header has a fixed-size identifier */
    const std::size_t SIZE_OF_V7_IDENTIFIER = 16;

    /** The V7 header has a variabled-size suffix */
    const std::size_t MAX_SIZE_OF_V7_SUFFIX = 16;

    /** The V7 header has a fixed-size serial number */
    const std::size_t SIZE_OF_V7_SERIAL_NUMBER = 19;

    /**
     * If the V7 identifier field is completely filled with zeros then it should be treated
     * as empty. This returns true if the Vector has any non-zero components.
     */
    bool isNonEmptyIdentifier (const royale::Vector<uint8_t> &identifier)
    {
        auto is_non_zero = [] (uint8_t i)
        {
            return i;
        };
        return std::any_of (identifier.cbegin(), identifier.cend(), is_non_zero);
    }
}

/**
 * A structure for storing the data read from the header.
 *
 * Historically, this matched the binary structure of the version 3 header, but it's now simply a
 * structure for returning data from readHeaderFromEEPROM().
 *
 * The header (any version) is at offset zero, starting with the magic and version.  The calibration
 * data itself is immediately after the header.
 */
struct StorageFormatPolar::FlashHeader
{
    std::array<char, 6> magic;
    uint16_t version;
    uint32_t checksum;
    std::size_t calibrationSize;
    /** If there was an identifier, the conversion of that to a Vector. If there was no identifier, an empty vector. */
    royale::Vector<uint8_t> identifier;
    /** If there was a suffix, the conversion of that to a String; if there wasn't then this is an empty String */
    royale::String suffixString;
    /** If there was a serial number, the conversion of that to a String; if there wasn't then this is an empty String */
    royale::String serialString;

    // The following members aren't part of the header, but are derived from it
    /**
     * The data may be correct but either these is no checksum, or the checksum may be wrong.
     * This is for supporting version 2 headers, which have no checksum.
     */
    bool ignoreChecksum;
    /** Address where the calibration data starts, equal to the header size */
    std::size_t calibrationOffset;
};

StorageFormatPolar::StorageFormatPolar (std::shared_ptr<royale::pal::IStorageReadRandom> bridge,
                                        bool identifierIsMandatory) :
    m_bridge {bridge},
    m_identifierIsMandatory {identifierIsMandatory}
{
    if (bridge == nullptr)
    {
        throw LogicError ("null ref exception");
    }
}

bool StorageFormatPolar::readHeaderFromEEPROM (struct FlashHeader &header)
{
    // Worst case is currently the v7 header
    std::vector<uint8_t> recvBuffer (SIZE_OF_V7_FLASH_HEADER);

    // The header is stored at the start of the flash image.
    m_bridge->readStorage (0, recvBuffer);

    const uint8_t *headerStart = &recvBuffer[0];
    memcpy (header.magic.data(), headerStart, header.magic.size());
    header.version = bufferToHost16 (headerStart + 6);

    if (header.version == 2 &&
            bufferToHost32 (headerStart + 12) != 0)
    {
        // This is a workaround for bug ROYAL-1076
        // There were some devices (flashed with the FPGA with the wrong
        // version number)
        header.version = 3;
    }

    switch (header.version)
    {
        case 7:
            {
                header.checksum = bufferToHost32 (headerStart + 8);
                header.calibrationSize = static_cast<std::size_t> (bufferToHost32 (headerStart + 12));
                header.ignoreChecksum = false;
                header.calibrationOffset = SIZE_OF_V7_FLASH_HEADER;

                const auto identifierBegin = reinterpret_cast<const uint8_t *> (headerStart + 16);
                const auto identifierEnd = identifierBegin + SIZE_OF_V7_IDENTIFIER;
                auto identifier = royale::Vector<uint8_t> (identifierBegin, identifierEnd);
                if (isNonEmptyIdentifier (identifier))
                {
                    header.identifier = std::move (identifier);
                }

                const auto suffixBegin = reinterpret_cast<const char *> (headerStart + 32);
                const auto suffixLength = strnlen (suffixBegin, MAX_SIZE_OF_V7_SUFFIX);
                header.suffixString = royale::String (suffixBegin, suffixLength);

                const auto serialBegin = reinterpret_cast<const char *> (headerStart + 48);
                const auto serialLength = strnlen (serialBegin, SIZE_OF_V7_SERIAL_NUMBER);
                header.serialString = royale::String (serialBegin, serialLength);
                break;
            }
        case 0x101:
            {
                auto serial = bufferToHost32 (headerStart + 8);
                std::ostringstream serialNr;
                serialNr << std::setfill ('0') << std::setw (8) << std::hex << serial;
                header.serialString = royale::String (serialNr.str());

                header.calibrationSize = static_cast<std::size_t> (bufferToHost32 (headerStart +
                                         SIZE_OF_V3_FLASH_HEADER + 12)); // use actual data size in the duplicate header
                header.calibrationOffset = SIZE_OF_V101_FLASH_HEADER;
                header.ignoreChecksum = true;
                break;
            }
        case 6:
            {
                header.checksum = bufferToHost32 (headerStart + 8);
                uint16_t dataBlockID = bufferToHost16 (headerStart + SIZE_OF_V6_FLASH_HEADER);
                header.calibrationSize = 0;
                if (dataBlockID == static_cast<uint16_t> (DataBlockID::SPC_ASTON))
                {
                    header.calibrationSize = static_cast<std::size_t> (bufferToHost32 (headerStart + SIZE_OF_V6_FLASH_HEADER + 2));
                }
                else
                {
                    // We currently only support version 6 headers where the first data
                    // block is the calibration data
                    return false;
                }
                header.ignoreChecksum = true; // We have to ignore the checksum as it also includes the other data
                header.calibrationOffset = 22;
                break;
            }
        case 3:
            {
                header.checksum = bufferToHost32 (headerStart + 8);
                header.calibrationSize = static_cast<std::size_t> (bufferToHost32 (headerStart + 12));
                header.ignoreChecksum = false;
                header.calibrationOffset = SIZE_OF_V3_FLASH_HEADER;
                break;
            }
        case 2:
            {
                header.checksum = 0;
                header.calibrationSize = static_cast<std::size_t> (bufferToHost32 (headerStart + 24));
                header.ignoreChecksum = true;
                header.calibrationOffset = SIZE_OF_V2_FLASH_HEADER;
                break;
            }
        default:
            {
                LOG (ERROR) << "Unsupported calibration header version";
                return false;
            }
    }

    if (0 != std::memcmp (FLASH_HEADER_MAGIC, header.magic.data(), header.magic.size()))
    {
        return false;
    }

    return true;
}

royale::Vector<uint8_t> StorageFormatPolar::getModuleIdentifier()
{
    struct FlashHeader header;
    if (!readHeaderFromEEPROM (header))
    {
        return {};
    }
    return header.identifier;
}

royale::String StorageFormatPolar::getModuleSuffix()
{
    struct FlashHeader header;
    if (!readHeaderFromEEPROM (header))
    {
        return "";
    }
    return header.suffixString;
}

royale::String StorageFormatPolar::getModuleSerialNumber()
{
    struct FlashHeader header;
    if (!readHeaderFromEEPROM (header))
    {
        return "";
    }
    return header.serialString;
}

uint32_t StorageFormatPolar::getCalibrationDataChecksum()
{
    struct FlashHeader header;
    if (!readHeaderFromEEPROM (header))
    {
        return 0;
    }
    return header.checksum;
}

royale::Vector<uint8_t> StorageFormatPolar::getCalibrationData()
{
    struct FlashHeader header;
    if (!readHeaderFromEEPROM (header))
    {
        throw RuntimeError ("No valid PMD data blob was found");
    }
    if (header.calibrationSize == 0)
    {
        LOG (ERROR) << "Device has a PMD calibration header, but no calibration";
        throw DataNotFound ("Device has a PMD calibration header, but no calibration");
    }
    if (header.calibrationSize > MAXIMUM_CALIBRATION_SIZE)
    {
        LOG (ERROR) << "Corrupt calibration header (or unexpected large calibration data)";
        throw RuntimeError ("Corrupt calibration header (or unexpectedly large calibration data)");
    }

    std::vector<uint8_t> tempData (header.calibrationSize);
    m_bridge->readStorage (header.calibrationOffset, tempData);

    if (!header.ignoreChecksum)
    {
        uint32_t dataChecksum = calculateCRC32 (&tempData[0], header.calibrationSize);
        if (dataChecksum != header.checksum)
        {
            throw RuntimeError ("Checksum failed on calibration data");
        }
    }

    return tempData;
}

void StorageFormatPolar::writeV3orV7 (const royale::Vector<uint8_t> &data,
                                      const royale::Vector<uint8_t> *identifier,
                                      const royale::String *suffix,
                                      const royale::String *serialNumber)
{
    // Build a V3 or V7 header
    const bool writeV7 = (identifier != nullptr);
    std::vector<uint8_t> headerAndData (SIZEOF_HEADER_MAGIC, 0);
    memcpy (&headerAndData[0], FLASH_HEADER_MAGIC, SIZEOF_HEADER_MAGIC);
    if (writeV7)
    {
        pushBack16 (headerAndData, static_cast<uint16_t> (7));  // header version
    }
    else
    {
        pushBack16 (headerAndData, static_cast<uint16_t> (3));  // header version
    }
    pushBack32 (headerAndData, calculateCRC32 (&data[0], data.size()));
    pushBack32 (headerAndData, static_cast<uint32_t> (data.size()));
    if (writeV7)
    {
        const royale::String empty;
        pushBackPadded (headerAndData, *identifier, SIZE_OF_V7_IDENTIFIER);
        pushBackPadded (headerAndData, suffix ? *suffix : empty, MAX_SIZE_OF_V7_SUFFIX);
        pushBackPadded (headerAndData, serialNumber ? *serialNumber : empty, SIZE_OF_V7_SERIAL_NUMBER);
    }

    const auto headerSize = headerAndData.size();

    if (headerSize != SIZE_OF_V3_FLASH_HEADER && headerSize != SIZE_OF_V7_FLASH_HEADER)
    {
        throw LogicError ("The calibration-writing code doesn't match the calibration reading code");
    }

    headerAndData.resize (headerSize + data.size());
    memcpy (&headerAndData[headerSize], &data[0], data.size());

    auto writeAccess = std::dynamic_pointer_cast<IStorageWriteFullOverwrite> (m_bridge);
    if (writeAccess == nullptr)
    {
        throw LogicError ("Trying to write calibration through a read-only interface");
    }
    writeAccess->writeStorage (headerAndData);
}

void StorageFormatPolar::writeCalibrationData (const royale::Vector<uint8_t> &data,
        const royale::Vector<uint8_t> &identifier,
        const royale::String &suffix,
        const royale::String &serialNumber)
{
    if (identifier.empty() && m_identifierIsMandatory)
    {
        throw LogicError ("This type of module requires additional data");
    }

    if (!identifier.empty() && identifier.size() != SIZE_OF_V7_IDENTIFIER)
    {
        throw LogicError ("The module identifier would be padded to a different size");
    }

    writeV3orV7 (data, &identifier, &suffix, &serialNumber);
}

void StorageFormatPolar::writeCalibrationData (const royale::Vector<uint8_t> &data)
{
    // If the device has a product identifier, the L2 Royale API methods will write the same
    // identifier to the device.  Only the L3 methods can change or remove the identifier.
    //
    // The implementation of that is likely to add a "preserveAdditionalData" argument to this
    // method, which will tell us whether a V7 header is protected from being overwritten with a V3
    // header.  It's currently not in the API, instead it's defined on the next line.
    const bool preserveAdditionalData = true;
    if (preserveAdditionalData)
    {
        struct FlashHeader header;
        if (readHeaderFromEEPROM (header))
        {
            switch (header.version)
            {
                case 7:
                    {
                        // write a V7 with the additional data from the old V7
                        writeCalibrationData (data, header.identifier, header.suffixString, header.serialString);
                        return;
                    }
                case 3:
                case 2:
                    // write a V3
                    break;
                default:
                    throw NotImplemented ("Preserving data from the previous header is not supported");
            }
        }
    }

    if (m_identifierIsMandatory)
    {
        throw LogicError ("This type of module requires additional data");
    }

    writeV3orV7 (data, nullptr, nullptr, nullptr);
}

std::shared_ptr<IStorageWriteFullOverwrite> StorageFormatPolar::getUnderlyingWriteAccess()
{
    return std::dynamic_pointer_cast<IStorageWriteFullOverwrite> (m_bridge);
}
