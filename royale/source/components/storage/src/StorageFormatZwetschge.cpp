/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageFormatZwetschge.hpp>

#include <common/EndianConversion.hpp>
#include <common/Crc32.hpp>

#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongDataFormatFound.hpp>

#include <common/RoyaleLogger.hpp>

#include <processing/ProcessingParameterId.hpp>
#include <usecase/UseCaseDefFactoryProcessingOnly.hpp>
#include <usecase/UseCaseIdentifier.hpp>
#include <imager/ImagerUseCaseIdentifier.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

using namespace royale::common;
using namespace royale::config;
using namespace royale::pal;
using namespace royale::storage;
using namespace royale::usecase;
using IImagerExternalConfig = royale::imager::IImagerExternalConfig;
using TimedRegisterList = royale::imager::TimedRegisterList;
using WrapperImagerExternalConfig = royale::imager::WrapperImagerExternalConfig;
using ImagerUseCaseData = royale::imager::IImagerExternalConfig::UseCaseData;
using ProcessingParameterId = royale::processing::ProcessingParameterId;

struct StorageFormatZwetschge::AddrAndSize
{
    uint32_t address;
    uint32_t size;
};

struct StorageFormatZwetschge::TableOfContents
{
    uint32_t version;
    uint32_t zwetschgeCrc;
    AddrAndSize embeddedSpecificData;
    std::array<uint8_t, 4> productIssuer;
    std::array<uint8_t, 16> productCode;
    uint32_t systemFrequency;
    AddrAndSize registerMaps;
    AddrAndSize calibration;
    uint32_t caliCrc;
    std::size_t numberOfUseCases;
    AddrAndSize useCases;
    std::array<char, 19> moduleSerial;
};

namespace
{
    /**
     * If data size or address is greater than this, assume we have read corrupt data.
     */
    const auto SANITY_CHECK_SIZE = uint32_t (20 * 1024 * 1024);

    uint32_t narrow_cast_24 (uint32_t value)
    {
        if (value & 0xff000000)
        {
            throw OutOfBounds ("narrow_cast to 24-bit failed");
        }
        return value;
    }

    /**
     * The StorageFormatZwetschge specification defines that the Table of Contents starts on page 32.
     */
    const auto ZWETSCHGE_TOC_ADDRESS = uint32_t
                                       {
                                           0x2000u
                                       };
    const auto ZWETSCHGE_TOC_MAGIC = std::array<uint8_t, 9>
                                     {
                                         {
                                             'Z', 'W', 'E', 'T', 'S', 'C', 'H', 'G', 'E'
                                         }
                                     };

    const auto ELENA_TOUC_MAGIC = std::array<uint8_t, 5>
                                  {
                                      {
                                          'E', 'l', 'e', 'n', 'a'
                                      }
                                  };

    const auto ELENA_TORM_MAGIC = std::array<uint8_t, 5>
                                  {
                                      {
                                          'e', 'L', 'E', 'N', 'A'
                                      }
                                  };

    /**
     * The identifier for Polar (or other PMDTEC formats) isn't expected to be found, this is just
     * used for giving a more useful error message.
     */
    const auto POLAR_MAGIC = std::array<uint8_t, 6>
                             {
                                 {
                                     'P', 'M', 'D', 'T', 'E', 'C'
                                 }
                             };
    /**
     * The identifier for Lena is also used for giving a more useful error message.
     */
    const auto LENA_MAGIC = std::array<uint8_t, 23>
                            {
                                {
                                    'R', 'O', 'Y', 'A', 'L', 'E', '-', 'I', 'M', 'A', 'G', 'E', 'R',
                                    '-', 'L', 'E', 'N', 'A', '-', 'F', 'I', 'L', 'E'
                                }
                            };

    /**
     * UnaryPredictate to match the magic number in flash memory that's been erased but not written.
     */
    struct UninitializedFlashMagicMatcher
    {
        bool operator() (uint8_t x) const
        {
            return x == 0xff;
        }
    };

    /**
     * Locations of data in the table of contents.
     *
     * Here the vXXX refers to Zwetschge specification version.
     */
    enum : std::size_t
    {
        TOC_OFFSET_MAGIC = 0,
        TOC_OFFSET_CRC = 9,
        TOC_OFFSET_VERSION = 13,
        TOC_V146_OFFSET_EMBEDDED_SPECIFIC = 16,
        TOC_V146_OFFSET_PRODUCT_ISSUER = 22,
        TOC_V146_OFFSET_PRODUCT_CODE = 26,
        TOC_V146_OFFSET_SYSTEM_FREQUENCY = 42,
        TOC_V146_OFFSET_REGISTER_MAPS = 46,
        TOC_V146_OFFSET_CALIBRATION = 52,
        TOC_V146_OFFSET_CALI_CRC = 58,
        TOC_V146_OFFSET_NUMBER_OF_USE_CASES = 62,
        TOC_V146_OFFSET_USE_CASES = 63,
        TOC_V147_OFFSET_MODULE_SERIAL = 69
    };

    const auto ZWETSCHGE_TOC_V146_SIZE = 69;
    const auto ZWETSCHGE_TOC_V147_SIZE = 88;

    /**
     * For all versions of this format, the minimum read size which will include full ToC data.
     */
    const auto ZWETSCHGE_TOC_MAX_SIZE = ZWETSCHGE_TOC_V147_SIZE;

    /**
     * The minimum read size for magic number recognition, including for Polar and Lena.
     */
    const auto MAGIC_MAX_SIZE = static_cast<uint32_t> (LENA_MAGIC.size());

    /**
     * Offsets for a TimedRegisterListEntry.
     */
    enum : std::size_t
    {
        TIMED_REG_LIST_ENTRY_OFFSET_ADDRESS = 0,
        TIMED_REG_LIST_ENTRY_OFFSET_VALUE = 2,
        TIMED_REG_LIST_ENTRY_OFFSET_SLEEP_TIME = 4,
        TIMED_REG_LIST_ENTRY_SIZE = 6
    };

    /**
     * Conversion factor from the data in TIMED_REG_LIST_ENTRY_OFFSET_SLEEP_TIME to microseconds.
     */
    const auto TIMED_REG_LIST_SLEEP_UNIT = uint32_t
                                           {
                                               32
                                           };

    /**
     * The access level enum is a different enum to Royale's internal ones.
     */
    enum class AccessLevel : uint8_t
    {
        NORMAL = 0,
        L3_AND_RAW = 1
    };

    royale::CallbackData levelToCallbackData (AccessLevel level)
    {
        switch (level)
        {
            case AccessLevel::L3_AND_RAW:
                return royale::CallbackData::Raw;
            default:
                return royale::CallbackData::Depth;
        }
    }

    royale::CameraAccessLevel levelToCameraAccessLevel (AccessLevel level)
    {
        switch (level)
        {
            case AccessLevel::L3_AND_RAW:
                return royale::CameraAccessLevel::L3;
            default:
                return royale::CameraAccessLevel::L1;
        }
    }

    /**
     * Read a TimedRegisterList from a data pointer (in to memory that was previously read
     * from the storage) and count of registers.
     *
     * TIMED_REG_LIST_ENTRY_SIZE * count bytes will be read.
     */
    royale::imager::TimedRegisterList bufferToTimedRegisterList (const uint8_t *src, std::size_t count)
    {
        royale::imager::TimedRegisterList timedRegisterList;
        {
            for (auto i = std::size_t {0u}; i < count; i++)
            {
                const auto currentItemStart = src + i * TIMED_REG_LIST_ENTRY_SIZE;
                auto addr = bufferToHost16 (currentItemStart + TIMED_REG_LIST_ENTRY_OFFSET_ADDRESS);
                auto value = bufferToHost16 (currentItemStart + TIMED_REG_LIST_ENTRY_OFFSET_VALUE);
                auto sleepTime = TIMED_REG_LIST_SLEEP_UNIT * bufferToHost16 (currentItemStart + TIMED_REG_LIST_ENTRY_OFFSET_SLEEP_TIME);
                timedRegisterList.push_back ({addr, value, sleepTime});
            }
        }
        return timedRegisterList;
    }

    /**
     * Read a sequential register map from a data pointer and convert it to a timed register list with zero
     * delays. This should be used when no information is stored on the flash. In this case using the usual
     * sequential register maps will fail.
     */
    royale::imager::TimedRegisterList bufferFromSeqToTimedRegisterList (const uint8_t *src, const royale::imager::SequentialRegisterHeader &seqHeader)
    {
        royale::imager::TimedRegisterList timedRegisterList;

        auto numRegisters = seqHeader.flashConfigSize / 2;
        if (numRegisters > 0)
        {
            timedRegisterList.resize (numRegisters);
        }
        else
        {
            throw DataNotFound ("No sequential registers found");
        }

        uint16_t imagerStartAddress = seqHeader.imagerAddress;
        for (auto i = std::size_t{ 0u }; i < numRegisters; i++)
        {
            const auto currentItemStart = src + i * 2;
            // For sequential register maps the values are stored consecutively
            auto addr = static_cast<uint16_t> (imagerStartAddress + i);
            auto value = bufferToHostBe16 (currentItemStart);
            timedRegisterList[i] = { addr, value, 0u };
        }

        return timedRegisterList;
    }

    /**
     * Similar to bufferToHost32 etc, converts data to the host-endian AddrAndSize structure.
     */
    StorageFormatZwetschge::AddrAndSize bufferToAddrAndSize (const uint8_t *src)
    {
        return StorageFormatZwetschge::AddrAndSize {bufferToHost24 (src), bufferToHost24 (src + 3) };
    }

    StorageFormatZwetschge::AddrAndSize bufferToAddrAndSize (const royale::Vector<uint8_t> &recvBuffer, const std::size_t offset)
    {
        return bufferToAddrAndSize (recvBuffer.data() + offset);
    }

    royale::imager::SequentialRegisterHeader bufferToSeqRegHeader (const uint8_t *src)
    {
        return {bufferToHost24 (src), bufferToHost24 (src + 3), bufferToHost16 (src + 6) };
    }

    /**
     * Sanity check for the data in a SequentialRegisterHeader.
     *
     * Returns true if the check fails. It fails if any member is zero and any other is non-zero.
     */
    bool hasInconsistentZero (const royale::imager::SequentialRegisterHeader &s)
    {
        if (s.flashConfigAddress || s.flashConfigSize || s.imagerAddress)
        {
            return ! (s.flashConfigAddress && s.flashConfigSize && s.imagerAddress);
        }
        return false; // all three are zero
    }


    /**
     * INonVolatileStorage where the metadata is provided to the constructor (from the Zwetschge
     * ToC), but the calibration data is read only on demand.
     */
    class CalibrationProxy : public royale::hal::INonVolatileStorage
    {
    public:
        CalibrationProxy (const royale::Vector<uint8_t> &identifier,
                          uint32_t calibrationDataChecksum,
                          const StorageFormatZwetschge::AddrAndSize &calibrationAddrAndSize,
                          std::shared_ptr<royale::pal::IStorageReadRandom> bridge,
                          const StorageFormatZwetschge::AddrAndSize &suffix,
                          const royale::String &serialNumber = "") :
            m_identifier (identifier),
            m_calibrationDataChecksum (calibrationDataChecksum),
            m_calibrationAddrAndSize (calibrationAddrAndSize),
            m_bridge (bridge),
            m_suffixAddrAndSize (suffix),
            m_serialNumber (serialNumber)
        {
            if (SANITY_CHECK_SIZE < narrow_cast_24 (calibrationAddrAndSize.address) + narrow_cast_24 (calibrationAddrAndSize.size))
            {
                throw OutOfBounds ("Data points to an unlikely size of data");
            }
        }

        royale::Vector<uint8_t> getModuleIdentifier() override
        {
            return m_identifier;
        }

        royale::String getModuleSuffix() override
        {
            if (m_suffixAddrAndSize.size == 0)
            {
                LOG (WARN) << "Zwetschge image with no module suffix";
                return "";
            }

            auto recvBuffer = std::vector<uint8_t> (m_suffixAddrAndSize.size);
            if (m_suffixAddrAndSize.size > 0)
            {
                m_bridge->readStorage (m_suffixAddrAndSize.address, recvBuffer);
            }

            auto suffix = std::string (reinterpret_cast<const char *> (&recvBuffer[0]), recvBuffer.size());

            return royale::String::fromStdString (suffix);
        }

        royale::String getModuleSerialNumber() override
        {
            return m_serialNumber;
        }

        royale::Vector<uint8_t> getCalibrationData() override
        {
            if (m_calibrationAddrAndSize.size == 0)
            {
                LOG (WARN) << "Zwetschge image with no calibration data";
                throw DataNotFound ("Zwetschge image with no calibration data");
            }

            auto recvBuffer = std::vector<uint8_t> (m_calibrationAddrAndSize.size);
            if (m_calibrationAddrAndSize.size > 0)
            {
                m_bridge->readStorage (m_calibrationAddrAndSize.address, recvBuffer);
                if (m_calibrationDataChecksum != calculateCRC32 (recvBuffer.data(), recvBuffer.size()))
                {
                    LOG (ERROR) << "Corrupt calibration, does not match CRC in header";
                    // \todo ROYAL-3413 add a more specific exception for checksum failures
                    throw RuntimeError ("calibration checksum failed");
                }
            }
            return recvBuffer;
        }

        uint32_t getCalibrationDataChecksum() override
        {
            return m_calibrationDataChecksum;
        }

        void writeCalibrationData (const royale::Vector<uint8_t> &data) override
        {
            throw NotImplemented ("Writing calibration data is not supported");
        }

        void writeCalibrationData (const royale::Vector<uint8_t> &data,
                                   const royale::Vector<uint8_t> &identifier,
                                   const royale::String &suffix,
                                   const royale::String &serialNumber) override
        {
            throw NotImplemented ("Writing calibration data is not supported");
        }

    private:
        const royale::Vector<uint8_t> m_identifier;
        const uint32_t m_calibrationDataChecksum;
        const StorageFormatZwetschge::AddrAndSize m_calibrationAddrAndSize;
        std::shared_ptr<royale::pal::IStorageReadRandom> m_bridge;
        const StorageFormatZwetschge::AddrAndSize m_suffixAddrAndSize;
        const royale::String m_serialNumber;
    };

}

StorageFormatZwetschge::StorageFormatZwetschge (std::shared_ptr<royale::pal::IStorageReadRandom> bridge) :
    m_bridge {bridge}
{
    if (bridge == nullptr)
    {
        throw LogicError ("null ref exception");
    }
}

std::vector<uint8_t> StorageFormatZwetschge::readBlock (const AddrAndSize &target)
{
    if (SANITY_CHECK_SIZE < narrow_cast_24 (target.address) + narrow_cast_24 (target.size))
    {
        throw OutOfBounds ("Data points to an unlikely size of data");
    }

    auto recvBuffer = std::vector<uint8_t> (target.size);
    m_bridge->readStorage (target.address, recvBuffer);
    return recvBuffer;
}

struct StorageFormatZwetschge::TableOfContents StorageFormatZwetschge::getToC()
{
    auto recvBuffer = readBlock ({ZWETSCHGE_TOC_ADDRESS, ZWETSCHGE_TOC_MAX_SIZE});

    if (! std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), recvBuffer.begin()))
    {
        bool isMisalignedZwetschge = false;
        bool isPolar = false;
        bool isLena = false;
        // See if there's the magic for Polar at the usual offset for Zwetschge's magic (this is the
        // expected location, as it would be after the read-only area, and the existing devices that
        // are using Polar as a substitute for Zwetschge are putting it there).
        if (std::equal (POLAR_MAGIC.begin(), POLAR_MAGIC.end(), recvBuffer.begin()))
        {
            isPolar = true;
        }
        else if (std::equal (LENA_MAGIC.begin(), LENA_MAGIC.end(), recvBuffer.begin()))
        {
            isLena = true;
        }
        else
        {
            if (std::all_of (recvBuffer.begin(), recvBuffer.end(), UninitializedFlashMagicMatcher()))
            {
                LOG (DEBUG) << "When trying to read Zwetschge, found all-0xff data at ZWETSCHGE_TOC_ADDRESS";
            }

            // Do extra I/O to give a more useful error if the device really has a known data, but
            // at offset zero (which is Polar's normal location). This only happens on an error path
            // where the device is going to fail to open, and the time taken for the extra I/O
            // should be worth the developer and user time that it could save.
            recvBuffer = readBlock ({0u, MAGIC_MAX_SIZE });
            if (std::equal (ZWETSCHGE_TOC_MAGIC.begin(), ZWETSCHGE_TOC_MAGIC.end(), recvBuffer.begin()))
            {
                isMisalignedZwetschge = true;
            }
            else if (std::equal (POLAR_MAGIC.begin(), POLAR_MAGIC.end(), recvBuffer.begin()))
            {
                isPolar = true;
            }
            else if (std::equal (LENA_MAGIC.begin(), LENA_MAGIC.end(), recvBuffer.begin()))
            {
                isLena = true;
            }
            else if (std::all_of (recvBuffer.begin(), recvBuffer.end(), UninitializedFlashMagicMatcher()))
            {
                LOG (DEBUG) << "When trying to read Zwetschge, found all-0xff data at start";
                throw DataNotFound ("Expected Zwetschge, found all-0xff data");
            }
        }

        if (isMisalignedZwetschge)
        {
            throw WrongDataFormatFound ("Expected Zwetschge with imager pages, found Zwetschge without preface");
        }
        if (isPolar)
        {
            throw WrongDataFormatFound ("Expected Zwetschge, found Polar / PMDTEC");
        }
        if (isLena)
        {
            throw WrongDataFormatFound ("Expected Zwetschge, found Lena");
        }
    }

    const auto crcExpected = bufferToHost32 (recvBuffer.data() + TOC_OFFSET_CRC);
    const auto version = bufferToHost24 (recvBuffer.data() + TOC_OFFSET_VERSION);

    switch (version)
    {
        case 0x145:
            {
                // there's no need for backward compatibility for 0x145
                throw WrongDataFormatFound ("Expected Zwetschge 0x146, found Zwetschge 0x145");
            }
        case 0x146:
            {
                const auto crcCalculated = calculateCRC32 (recvBuffer.data() + TOC_OFFSET_VERSION, ZWETSCHGE_TOC_V146_SIZE - TOC_OFFSET_VERSION);
                if (crcExpected != crcCalculated)
                {
                    LOG (ERROR) << std::hex << "crcExpected == " << crcExpected << ", crcCalculated == " << crcCalculated;
                    throw RuntimeError ("Checksum failed on ToC data");
                }

                StorageFormatZwetschge::TableOfContents toc;
                toc.version = version;
                toc.zwetschgeCrc = crcExpected;
                toc.embeddedSpecificData = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_EMBEDDED_SPECIFIC);
                std::copy_n (recvBuffer.data() + TOC_V146_OFFSET_PRODUCT_ISSUER, toc.productIssuer.size(), toc.productIssuer.begin());
                std::copy_n (recvBuffer.data() + TOC_V146_OFFSET_PRODUCT_CODE, toc.productCode.size(), toc.productCode.begin());
                toc.systemFrequency = bufferToHost32 (recvBuffer.data() + TOC_V146_OFFSET_SYSTEM_FREQUENCY);
                toc.registerMaps = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_REGISTER_MAPS);
                toc.calibration = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_CALIBRATION);
                toc.caliCrc = bufferToHost32 (recvBuffer.data() + TOC_V146_OFFSET_CALI_CRC);
                toc.numberOfUseCases = recvBuffer[TOC_V146_OFFSET_NUMBER_OF_USE_CASES];
                toc.useCases = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_USE_CASES);
                memset (&toc.moduleSerial[0], 0, toc.moduleSerial.size());
                return toc;
            }
        case 0x147:
            {
                const auto crcCalculated = calculateCRC32 (recvBuffer.data() + TOC_OFFSET_VERSION, ZWETSCHGE_TOC_V147_SIZE - TOC_OFFSET_VERSION);
                if (crcExpected != crcCalculated)
                {
                    LOG (ERROR) << std::hex << "crcExpected == " << crcExpected << ", crcCalculated == " << crcCalculated;
                    throw RuntimeError ("Checksum failed on ToC data");
                }

                StorageFormatZwetschge::TableOfContents toc;
                toc.version = version;
                toc.zwetschgeCrc = crcExpected;
                toc.embeddedSpecificData = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_EMBEDDED_SPECIFIC);
                std::copy_n (recvBuffer.data() + TOC_V146_OFFSET_PRODUCT_ISSUER, toc.productIssuer.size(), toc.productIssuer.begin());
                std::copy_n (recvBuffer.data() + TOC_V146_OFFSET_PRODUCT_CODE, toc.productCode.size(), toc.productCode.begin());
                toc.systemFrequency = bufferToHost32 (recvBuffer.data() + TOC_V146_OFFSET_SYSTEM_FREQUENCY);
                toc.registerMaps = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_REGISTER_MAPS);
                toc.calibration = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_CALIBRATION);
                toc.caliCrc = bufferToHost32 (recvBuffer.data() + TOC_V146_OFFSET_CALI_CRC);
                toc.numberOfUseCases = recvBuffer[TOC_V146_OFFSET_NUMBER_OF_USE_CASES];
                toc.useCases = bufferToAddrAndSize (recvBuffer, TOC_V146_OFFSET_USE_CASES);
                std::copy_n (recvBuffer.data() + TOC_V147_OFFSET_MODULE_SERIAL, toc.moduleSerial.size(), toc.moduleSerial.begin());
                return toc;

            }
        default:
            {
                LOG (ERROR) << "Unsupported Zwetschge header version";
            }
    }

    throw DataNotFound(); // unsupported header version, or no data
}

std::shared_ptr<royale::hal::INonVolatileStorage> StorageFormatZwetschge::getCalibrationProxy (const TableOfContents &toc)
{
    royale::String moduleSerial;
    bool moduleSerialEmpty = true;
    for (auto curChar : toc.moduleSerial)
    {
        if (curChar != 0)
        {
            moduleSerialEmpty = false;
        }
    }

    if (!moduleSerialEmpty)
    {
        moduleSerial = String::fromStdString (std::string (toc.moduleSerial.begin(), toc.moduleSerial.end()));
    }

    return std::make_shared<CalibrationProxy> (royale::Vector<uint8_t> {toc.productCode.begin(), toc.productCode.end() },
            toc.caliCrc,
            toc.calibration,
            m_bridge,
            toc.embeddedSpecificData,
            moduleSerial);
}

StorageFormatZwetschge::UseCasePartOfConfig StorageFormatZwetschge::getUseCaseList (const TableOfContents &toc, bool convertSeqRegMaps,
        bool keepSeqAddressInfo)
{
    enum : std::size_t
    {
        TOUC_OFFSET_MAGIC = 0,
        TOUC_OFFSET_CRC = 5,
        TOUC_OFFSET_CHECKSUMMED_DATA = 9,
        TOUC_OFFSET_DYNAMIC_DATA = TOUC_OFFSET_CHECKSUMMED_DATA
    };

    // If the ToC says that there are no use cases, and the address and size of the use case table
    // agree, then return an empty vector. If they don't agree then throw.
    if (toc.numberOfUseCases == 0)
    {
        LOG (WARN) << "Device has a StorageFormatZwetschge calibration header, but no use cases";
        if (toc.useCases.address == 0 && toc.useCases.size == 0)
        {
            LOG (DEBUG) << "... and no use case table";
        }
        else if (toc.useCases.size != TOUC_OFFSET_DYNAMIC_DATA)
        {
            LOG (ERROR) << "... but has a use case table, address " << toc.useCases.address
                        << ", size " << toc.useCases.size;
            throw RuntimeError ("Corrupt calibration header (inconsistency with use case table)");
        }
        return {};
    }

    if (toc.useCases.address == 0 || toc.useCases.size < TOUC_OFFSET_CHECKSUMMED_DATA || toc.useCases.size > SANITY_CHECK_SIZE)
    {
        LOG (ERROR) << "Corrupt calibration header (or unexpected large calibration data)";
        throw RuntimeError ("Corrupt calibration header (or unexpectedly large calibration data)");
    }

    auto recvBuffer = readBlock (toc.useCases);
    if (! std::equal (ELENA_TOUC_MAGIC.begin(), ELENA_TOUC_MAGIC.end(), recvBuffer.begin()))
    {
        throw DataNotFound ("No valid StorageFormatZwetschge use-case blob was found");
    }

    const auto crcExpected = bufferToHost32 (recvBuffer.data() + TOUC_OFFSET_CRC);
    const auto crcCalculated = calculateCRC32 (recvBuffer.data() + TOUC_OFFSET_CHECKSUMMED_DATA, toc.useCases.size - TOUC_OFFSET_CHECKSUMMED_DATA);
    if (crcExpected != crcCalculated)
    {
        LOG (ERROR) << std::hex << "crcExpected == " << crcExpected << ", crcCalculated == " << crcCalculated;
        throw RuntimeError ("Checksum failed on TableOfUseCases data");
    }

    enum : std::size_t
    {
        UC_OFFSET_METADATA_SIZE = 0,
        UC_OFFSET_SEQ_REG_HEADER = 2,
        UC_OFFSET_IMAGE_WIDTH = 10,
        UC_OFFSET_IMAGE_HEIGHT = 12,
        UC_OFFSET_UUID = 14,
        UC_OFFSET_FPS_START = 30,
        UC_OFFSET_FPS_MIN = 31,
        UC_OFFSET_FPS_MAX = 32,
        UC_OFFSET_PROC_PARAMS_UUID = 33,
        UC_OFFSET_WAIT_TIME = 49,
        UC_OFFSET_ACCESS_LEVEL = 52,

        UC_OFFSET_NAME_LENGTH = 53,
        UC_OFFSET_MEASURE_COUNT = 54,
        UC_OFFSET_FREQUENCY_COUNT = 56,
        UC_OFFSET_TIMED_REG_MAP_COUNT = 58,
        UC_OFFSET_STREAM_COUNT = 60,
        UC_OFFSET_EXPO_GROUP_COUNT = 61,
        UC_OFFSET_RFS_COUNT = 62,
        UC_OFFSET_RESERVED_SIZE = 64,
        UC_OFFSET_DYNAMIC_DATA = 65,
        ELENA_UC_MIN_SIZE = UC_OFFSET_DYNAMIC_DATA
    };

    std::vector<ImagerUseCaseData> imagerUseCaseList;
    std::vector<UseCase> royaleUseCaseList;

    std::size_t currentUcStart = TOUC_OFFSET_CHECKSUMMED_DATA;
    for (std::size_t i = 0 ; i < toc.numberOfUseCases ; i++)
    {
        if (currentUcStart + ELENA_UC_MIN_SIZE > toc.useCases.size)
        {
            LOG (ERROR) << "Expected to find " << toc.numberOfUseCases << " use cases, but ran out of data first";
            throw RuntimeError ("Number of use cases didn't match data size");
        }
        const auto dataBlock = recvBuffer.data() + currentUcStart;

        const auto blockSize = bufferToHost16 (dataBlock + UC_OFFSET_METADATA_SIZE);
        const auto nameLength = dataBlock[UC_OFFSET_NAME_LENGTH];
        const auto measureCount = bufferToHost16 (dataBlock + UC_OFFSET_MEASURE_COUNT);
        const auto frequencyCount = bufferToHost16 (dataBlock + UC_OFFSET_FREQUENCY_COUNT);
        const auto regMapEntryCount = bufferToHost16 (dataBlock + UC_OFFSET_TIMED_REG_MAP_COUNT);
        const auto streamCount = dataBlock[UC_OFFSET_STREAM_COUNT];
        const auto expoGroupCount = dataBlock[UC_OFFSET_EXPO_GROUP_COUNT];
        const auto rfsCount = bufferToHost16 (dataBlock + UC_OFFSET_RFS_COUNT);
        const auto reservedSize = dataBlock[UC_OFFSET_RESERVED_SIZE];

        if (currentUcStart + blockSize > toc.useCases.size)
        {
            LOG (ERROR) << "Use case data for block " << i << " exceeds data size";
            throw RuntimeError ("Use case data for block exceeds data size");
        }
        if (blockSize != ELENA_UC_MIN_SIZE
                + nameLength
                + 2 * measureCount
                + 4 * frequencyCount
                + TIMED_REG_LIST_ENTRY_SIZE * regMapEntryCount
                + 2 * streamCount
                + 6 * expoGroupCount
                + 6 * rfsCount
                + reservedSize)
        {
            LOG (ERROR) << "Use case block " << i << ": size doesn't match variable data";
            throw RuntimeError ("Use case block size doesn't match variable data");
        }

        const auto seqRegHeader = bufferToSeqRegHeader (dataBlock + UC_OFFSET_SEQ_REG_HEADER);
        if (hasInconsistentZero (seqRegHeader))
        {
            LOG (ERROR) << "Use case's seqRegHeader data block mixes zero and non-zero: " << seqRegHeader.flashConfigAddress << ", " << seqRegHeader.flashConfigSize << ", " << seqRegHeader.imagerAddress;
            throw RuntimeError ("Use case's seqRegHeader data block mixes zero and non-zero");
        }

        const auto imageWidth = bufferToHost16 (dataBlock + UC_OFFSET_IMAGE_WIDTH);
        const auto imageHeight = bufferToHost16 (dataBlock + UC_OFFSET_IMAGE_HEIGHT);

        // UUID (128-bit, little-endian)
        static_assert (UC_OFFSET_FPS_START - UC_OFFSET_UUID == std::tuple_size<UseCaseIdentifier::datatype>::value,
                       "size of use case identifier guid doesn't match the format");
        auto ucGuidData = UseCaseIdentifier::datatype {};
        std::copy_n (dataBlock + UC_OFFSET_UUID, ucGuidData.size(), ucGuidData.begin());
        const auto ucGuid = UseCaseIdentifier (std::move (ucGuidData));

        const auto fpsStart = dataBlock[UC_OFFSET_FPS_START];
        const auto fpsMin = dataBlock[UC_OFFSET_FPS_MIN];
        const auto fpsMax = dataBlock[UC_OFFSET_FPS_MAX];

        static_assert (UC_OFFSET_WAIT_TIME - UC_OFFSET_PROC_PARAMS_UUID == std::tuple_size<ProcessingParameterId::datatype>::value,
                       "size of processing parameter identifier guid doesn't match the format");
        auto ppGuidData = ProcessingParameterId::datatype {};
        std::copy_n (dataBlock + UC_OFFSET_PROC_PARAMS_UUID, ppGuidData.size(), ppGuidData.begin());
        const auto ppGuid = ProcessingParameterId (std::move (ppGuidData));

        const auto waitTime = std::chrono::microseconds (bufferToHost24 (dataBlock + UC_OFFSET_WAIT_TIME));
        const auto accessLevel = static_cast<AccessLevel> (dataBlock[UC_OFFSET_ACCESS_LEVEL]);

        auto dataStart = dataBlock + UC_OFFSET_DYNAMIC_DATA;
        // Use case name (ASCII string, without terminator)
        auto name = std::string (reinterpret_cast<const char *> (dataStart), nameLength);
        dataStart += nameLength;

        // Sizes of measurement blocks, corresponding to IImager::getMeasurementBlockSizes() (16-bit)
        // In ImagerUseCaseData they're std::size_t, but for Zwetschge we only need 16 bits
        auto measures = bufferToHostVector16<std::vector<std::size_t>> (dataStart, measureCount);
        dataStart += 2 * measureCount;

        auto frequencies = bufferToHostVector32<std::vector<uint32_t>> (dataStart, frequencyCount);
        dataStart += 4 * frequencyCount;

        const auto registerMap = bufferToTimedRegisterList (dataStart, regMapEntryCount);
        dataStart += TIMED_REG_LIST_ENTRY_SIZE * regMapEntryCount;

        auto streamIds = bufferToHostVector16<std::vector<StreamId>> (dataStart, streamCount);
        dataStart += 2 * streamCount;

        royale::Vector<UseCaseDefFactoryProcessingOnly::ProcOnlyExpo> expoGroups;
        for (auto i = std::size_t {0u}; i < expoGroupCount; i++)
        {
            auto expoStart = bufferToHost16 (dataStart);
            auto expoMin = uint32_t {bufferToHost16 (dataStart + 2) };
            auto expoMax = uint32_t {bufferToHost16 (dataStart + 4) };
            expoGroups.push_back (UseCaseDefFactoryProcessingOnly::ProcOnlyExpo {{expoMin, expoMax}, expoStart});
            dataStart += 6;
        }

        royale::Vector<UseCaseDefFactoryProcessingOnly::ProcOnlyRFS> rfses;
        for (auto i = std::size_t {0u}; i < rfsCount; i++)
        {
            auto frameCount = dataStart[0];
            auto frequency = bufferToHost32 (dataStart + 1);
            auto expoGroupIdx = dataStart[5];
            rfses.push_back (UseCaseDefFactoryProcessingOnly::ProcOnlyRFS {frameCount, frequency, expoGroupIdx});
            dataStart += 6;
        }

        if (! seqRegHeader.empty())
        {
            if (!registerMap.empty())
            {
                throw RuntimeError ("Use case provides both a SequentialRegisterHeader and a TimedRegisterList");
            }

            IImagerExternalConfig::UseCaseData iucd;
            if (convertSeqRegMaps)
            {
                auto ucBuffer = readBlock (StorageFormatZwetschge::AddrAndSize{ seqRegHeader.flashConfigAddress, seqRegHeader.flashConfigSize });

                const auto seqRegisterMap = bufferFromSeqToTimedRegisterList (ucBuffer.data(), seqRegHeader);
                if (keepSeqAddressInfo)
                {
                    iucd = ImagerUseCaseData{ royale::imager::toImagerUseCaseIdentifier (ucGuid), name, std::move (measures), std::move (frequencies),
                                              std::move (seqRegHeader), std::move (seqRegisterMap), waitTime };
                }
                else
                {
                    iucd = ImagerUseCaseData{ royale::imager::toImagerUseCaseIdentifier (ucGuid), name, std::move (measures), std::move (frequencies),
                                              std::move (seqRegisterMap), waitTime };
                }
            }
            else
            {
                iucd = ImagerUseCaseData{ royale::imager::toImagerUseCaseIdentifier (ucGuid), name, std::move (measures), std::move (frequencies),
                                          std::move (seqRegHeader), waitTime };
            }
            imagerUseCaseList.push_back (std::move (iucd));
        }
        else
        {
            // This can be used for both registerMap.empty() and !registerMap.empty()
            auto iucd = ImagerUseCaseData {royale::imager::toImagerUseCaseIdentifier (ucGuid), name, std::move (measures), std::move (frequencies),
                                           std::move (registerMap), waitTime };
            imagerUseCaseList.push_back (std::move (iucd));
        }

        {

            auto ucd = UseCaseDefFactoryProcessingOnly::createUcd (ucGuid, Pair<uint16_t, uint16_t> {imageWidth, imageHeight}, Pair<uint16_t, uint16_t> {fpsMin, fpsMax}, fpsStart, expoGroups, rfses);
            auto uc = UseCase {name, std::move (ucd), ppGuid, levelToCallbackData (accessLevel), levelToCameraAccessLevel (accessLevel) };
            royaleUseCaseList.push_back (std::move (uc));
        }

        currentUcStart += blockSize;
    }

    return {imagerUseCaseList, royaleUseCaseList};
}

std::shared_ptr<WrapperImagerExternalConfig> StorageFormatZwetschge::getTableOfRegisterMaps (const AddrAndSize &target)
{
    // Byte offsets in to the table
    enum : std::size_t
    {
        TORM_OFFSET_MAGIC = 0,
        TORM_OFFSET_CRC = 5,
        TORM_OFFSET_VERSION = 9,
        TORM_OFFSET_FWPAGE1_SEQ_REG_HEADER = 12,
        TORM_OFFSET_FWPAGE2_SEQ_REG_HEADER = 20,
        TORM_OFFSET_INIT_COUNT = 28,
        TORM_OFFSET_FWPAGE1_TIMED_REG_COUNT = 30,
        TORM_OFFSET_FWPAGE2_TIMED_REG_COUNT = 32,
        TORM_OFFSET_FWSTART_COUNT = 34,
        TORM_OFFSET_START_COUNT = 36,
        TORM_OFFSET_STOP_COUNT = 38,
        TORM_OFFSET_DYNAMIC_DATA = 40,
        TORM_OFFSET_CHECKSUMMED_DATA = TORM_OFFSET_VERSION,
        TORM_OFFSET_MAP_SIZES = TORM_OFFSET_INIT_COUNT,
        ELENA_TORM_MIN_SIZE = TORM_OFFSET_DYNAMIC_DATA
    };

    // The data for {init, fwstart, start and stop} is a homogenous set of maps, so instead of reading each by name, the code below
    // reads NUMBER_OF_TIMED_LISTS.
    enum : std::size_t
    {
        MAP_ORDINAL_INIT = 0,
        MAP_ORDINAL_FW_PAGE_1,
        MAP_ORDINAL_FW_PAGE_2,
        MAP_ORDINAL_FWSTART,
        MAP_ORDINAL_START,
        MAP_ORDINAL_STOP,
        NUMBER_OF_TIMED_LISTS
    };

    if (target.address == 0 && target.size == 0)
    {
        LOG (ERROR) << "Zwetschge data has no table of register maps";
        throw DataNotFound ("Zwetschge without table of register maps");
    }
    else if (target.size < ELENA_TORM_MIN_SIZE)
    {
        LOG (ERROR) << "Wrong size for Zwetschge register map table";
        throw RuntimeError ("Wrong size for Zwetschge register map table");
    }
    if (target.address == 0)
    {
        // Page zero is reserved, we don't expect Zwetschge data to be there. Although the ToC
        // checksum must have passed, the ToC seems invalid.
        LOG (ERROR) << "Zwetschge register map table is at address zero, probably wrong";
        throw RuntimeError ("Zwetschge register map table has address zero");
    }

    auto recvBuffer = readBlock (target);
    if (! std::equal (ELENA_TORM_MAGIC.begin(), ELENA_TORM_MAGIC.end(), recvBuffer.begin()))
    {
        throw DataNotFound ("No valid Zwetschge register map table was found");
    }

    const auto crcExpected = bufferToHost32 (recvBuffer.data() + TORM_OFFSET_CRC);
    const auto crcCalculated = calculateCRC32 (recvBuffer.data() + TORM_OFFSET_CHECKSUMMED_DATA, target.size - TORM_OFFSET_CHECKSUMMED_DATA);
    if (crcExpected != crcCalculated)
    {
        LOG (ERROR) << std::hex << "crcExpected == " << crcExpected << ", crcCalculated == " << crcCalculated;
        throw RuntimeError ("Checksum failed on TableOfRegisterMaps data");
    }

    const auto fwHeader1 = bufferToSeqRegHeader (recvBuffer.data() + TORM_OFFSET_FWPAGE1_SEQ_REG_HEADER);
    const auto fwHeader2 = bufferToSeqRegHeader (recvBuffer.data() + TORM_OFFSET_FWPAGE2_SEQ_REG_HEADER);
    if (hasInconsistentZero (fwHeader1) || hasInconsistentZero (fwHeader2))
    {
        LOG (ERROR) << "Firmware pages in the table of register maps have an inconsistent zero";
        throw RuntimeError ("Firmware pages in the table of register maps have an inconsistent zero");
    }

    static_assert (NUMBER_OF_TIMED_LISTS * 2 == TORM_OFFSET_DYNAMIC_DATA - TORM_OFFSET_INIT_COUNT,
                   "This code expects to read NUMBER_OF_TIMED_LISTS, but that doesn't match the offsets");

    const auto mapSizes = bufferToHostVector16<std::vector<std::size_t>> (recvBuffer.data() + TORM_OFFSET_INIT_COUNT, NUMBER_OF_TIMED_LISTS);
    if (recvBuffer.size() != TORM_OFFSET_DYNAMIC_DATA + TIMED_REG_LIST_ENTRY_SIZE * std::accumulate (mapSizes.begin(), mapSizes.end(), std::size_t {0}))
    {
        throw RuntimeError ("Data size doesn't match map sizes in TableOfRegisterMaps");
    }

    std::vector<TimedRegisterList> maps;
    auto currentMapStart = recvBuffer.data() + TORM_OFFSET_DYNAMIC_DATA;
    for (const auto size : mapSizes)
    {
        maps.push_back (bufferToTimedRegisterList (currentMapStart, size));
        currentMapStart += TIMED_REG_LIST_ENTRY_SIZE * size;
    }

    static_assert (NUMBER_OF_TIMED_LISTS == 6, "The following static initialization list for WrapperImagerExternalConfig is wrong");

    const auto fwHeader1Bool = ! fwHeader1.empty();
    const auto fwHeader2Bool = ! fwHeader2.empty();
    const auto fwPage1Bool = ! maps.at (MAP_ORDINAL_FW_PAGE_1).empty();
    const auto fwPage2Bool = ! maps.at (MAP_ORDINAL_FW_PAGE_2).empty();
    if ( (fwHeader1Bool || fwHeader2Bool) && (fwPage1Bool && fwPage2Bool))
    {
        throw RuntimeError ("Imager firmware has both SequentialRegisterBlocks and TimedRegisterLists");
    }
    if ( (fwHeader2Bool && !fwHeader1Bool) || (fwPage2Bool && ! fwPage1Bool))
    {
        throw RuntimeError ("Imager has firmware page 2 without page 1");
    }
    if (fwHeader1Bool)
    {
        return std::make_shared<WrapperImagerExternalConfig> (
                   std::move (maps.at (MAP_ORDINAL_INIT)),
                   std::move (fwHeader1),
                   std::move (fwHeader2),
                   std::move (maps.at (MAP_ORDINAL_FWSTART)),
                   std::move (maps.at (MAP_ORDINAL_START)),
                   std::move (maps.at (MAP_ORDINAL_STOP)),
                   std::vector<ImagerUseCaseData> {});
    }
    else
    {
        return std::make_shared<WrapperImagerExternalConfig> (
                   std::move (maps.at (MAP_ORDINAL_INIT)),
                   std::move (maps.at (MAP_ORDINAL_FW_PAGE_1)),
                   std::move (maps.at (MAP_ORDINAL_FW_PAGE_2)),
                   std::move (maps.at (MAP_ORDINAL_FWSTART)),
                   std::move (maps.at (MAP_ORDINAL_START)),
                   std::move (maps.at (MAP_ORDINAL_STOP)),
                   std::vector<ImagerUseCaseData> {});
    }
}

ExternalConfig StorageFormatZwetschge::getExternalConfig (bool convertSeqRegMaps,
        bool keepSeqAddressInfo)
{
    StorageFormatZwetschge::TableOfContents toc = getToC();
    auto config = getTableOfRegisterMaps (toc.registerMaps);
    auto useCasePartOfConfig = getUseCaseList (toc, convertSeqRegMaps, keepSeqAddressInfo);
    config->m_useCases = std::move (useCasePartOfConfig.imagerUseCaseList);
    auto calibration = getCalibrationProxy (toc);
    ExternalConfig externalConfig {std::move (config), std::move (useCasePartOfConfig.royaleUseCaseList), std::move (calibration) };
    return externalConfig;
}

uint32_t StorageFormatZwetschge::getZwetschgeCrc()
{
    StorageFormatZwetschge::TableOfContents toc = getToC();
    return toc.zwetschgeCrc;
}
