/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <storage/ImagerSpiFirmwareM2452.hpp>
#include <storage/StorageSpiImagerM2452.hpp>

#include <royale/Vector.hpp>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

#include <common/EndianConversion.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/IntegerMath.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;
using namespace royale::storage::imagerspi_m2452;

namespace
{
    /** Hardware limit for payload, in 8-bit bytes */
    const std::size_t numBytesPerTransfer = 256;

    /** Hardware limit for payload, in 16-bit registers */
    const std::size_t maxRegistersPerTransfer = 128;
    const std::size_t regSize = 2;
    static_assert (numBytesPerTransfer == maxRegistersPerTransfer *regSize,
                   "payload size in bytes doesn't match payload size in registers");

    // The memory is assumed to have 4096 bytes per sector
    const int numBytesPerSector = 4096;

    /**
     * When transferring data, it is transferred in a block of registers, starting at register zero.
     *
     * There are maxRegistersPerTransfer consecutive registers available, starting at this address.
     */
    const uint16_t storeReadDataArea = 0u;

    const auto DEFAULT_TIME_ERASE = std::chrono::seconds (2);
    const auto DEFAULT_TIME_READ = std::chrono::microseconds (1500);
    const auto DEFAULT_TIME_WRITE = std::chrono::microseconds (1000);
    const auto POLLING_INTERVAL = std::chrono::milliseconds (10);

    /**
     * The number of times that pollUntilZero sleeps for pollingInterval, before it throws a
     * Timeout.
     */
    const auto MAX_POLL_ZERO_RETRIES = 10;

    /**
     * The SPI Master firmware (and the SPI chips that we're using) put the data addresses in to 24
     * bits (see packCommandAndAddress). Therefore this is the limit of the supportable image size.
     */
    const auto MAX_IMAGE_SIZE = 1u << 24;

    /**
     * All of the commands are started by packing the command and 24-bit address in to a
     * pair of 16-bit registers, in the format that the storage chip will expect to receive
     * from the SPI master.
     */
    static std::vector<uint16_t> packCommandAndAddress (uint8_t command, std::size_t address)
    {
        // The caller should have already checked the address, and it's protected by the MAX_IMAGE_SIZE
        if (address > 0xffffff)
        {
            throw OutOfBounds ("");
        }

        std::vector<uint16_t> registers
        {
            static_cast<uint16_t> ( (command << 8) | ( (address >> 16) & 0xff)),
            static_cast<uint16_t> (address & 0xffff)
        };
        return registers;
    }
}

StorageSpiImagerM2452::StorageSpiImagerM2452 (const royale::config::FlashMemoryConfig &config,
        std::shared_ptr<royale::hal::IBridgeImager> access,
        royale::config::ImagerAsBridgeType firmwareType) :
    m_access {access},
    m_firmwareType {firmwareType},
    m_eraseTime {config.eraseTime},
    m_readTime {config.readTime},
    m_writeTime {config.writeTime},
    m_imageSize {config.imageSize}
{
    if (m_firmwareType != royale::config::ImagerAsBridgeType::M2452_SPI)
    {
        throw NotImplemented ("SpiMaster firmware is not supported for this imager");
    }

    if (config.accessOffset != 0u)
    {
        throw NotImplemented ("The accessOffset is not supported");
    }

    if (config.pageSize != 0u && config.pageSize != numBytesPerTransfer)
    {
        throw NotImplemented ("Only the hardcoded page size is supported");
    }

    if (config.sectorSize != 0u && config.sectorSize != numBytesPerSector)
    {
        throw NotImplemented ("Only the hardcoded sector size is supported");
    }

    if (m_eraseTime == std::chrono::microseconds::zero())
    {
        m_eraseTime = DEFAULT_TIME_ERASE;
    }

    if (m_readTime == std::chrono::microseconds::zero())
    {
        m_readTime = DEFAULT_TIME_READ;
    }

    if (m_writeTime == std::chrono::microseconds::zero())
    {
        m_writeTime = DEFAULT_TIME_WRITE;
    }

    if (m_imageSize > MAX_IMAGE_SIZE)
    {
        throw LogicError ("SpiMaster firmware only supports a 24-bit address space");
    }
    else if (m_imageSize == 0)
    {
        m_imageSize = MAX_IMAGE_SIZE;
    }
}

StorageSpiImagerM2452::~StorageSpiImagerM2452()
{
    try
    {
        // Reset the imager to be sure it's left in a known state
        m_access->setImagerReset (true);
        m_access->sleepFor (std::chrono::microseconds (1));
        m_access->setImagerReset (false);
    }
    catch (...)
    {
        // ignore, if this can't reset the imager then it's unlikely that an IImager implementation
        // will be able to access it either
    }
}

void StorageSpiImagerM2452::loadFirmwareOnce()
{
    if (!m_firmwareLoaded)
    {
        resetAndLoadFirmware();
        m_firmwareLoaded = true;
    }
}

void StorageSpiImagerM2452::resetAndLoadFirmware()
{
    // Reset the imager to be sure it's in a known state
    m_access->setImagerReset (true);
    m_access->sleepFor (std::chrono::microseconds (1));
    m_access->setImagerReset (false);

    // Check that the imager is the expected type and design step
    if (m_firmwareType == royale::config::ImagerAsBridgeType::M2452_SPI)
    {
        uint16_t step;
        m_access->readImagerRegister (SpiReg::ANAIP_DESIGNSTEP, step);
        const auto &STEPS = imagerspi_m2452::SUPPORTED_DESIGN_STEPS;
        if (std::find (STEPS.begin(), STEPS.end(), step) == STEPS.end())
        {
            throw NotImplemented ("SpiMaster firmware is not supported for this imager design step");
        }
    }
    else
    {
        throw NotImplemented ("SpiMaster firmware is not supported for this imager");
    }

    // Tell the imager that we will write a RAM fw
    m_access->writeImagerRegister (SpiReg::ISM_MEMPAGE, static_cast<uint16_t> (4 << 5));

    // Write the firmware
    m_access->writeImagerBurst (SpiReg::ISM_FWSTARTADDRESS, ImagerSpiFirmwareM2452);

    // Tell the imager that we've finished writing the firmware
    m_access->writeImagerRegister (SpiReg::ISM_MEMPAGE, 0x0000);
}

void StorageSpiImagerM2452::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    LOG (DEBUG) << "StorageSpiImager::readStorage, addr=0x" << std::hex << startAddr
                << ", size=" << std::dec << recvBuffer.size();

    if (startAddr + recvBuffer.size() > m_imageSize)
    {
        throw OutOfBounds ("Trying to read from an address that's larger than the storage");
    }

    // The data will be available in these registers, each of which is 16-bits wide,
    auto registers = std::vector<uint16_t> (maxRegistersPerTransfer);

    // Number of pages we have to read; these pages are a feature of the transfer through the
    // imager, and do not have any required alignment in the SPI device itself.
    const auto numPages = divideRoundUp (recvBuffer.size(), numBytesPerTransfer);

    loadFirmwareOnce();

    // If these throw an exception, then the destructor should reset the imager
    m_access->writeImagerRegister (SpiReg::ISM_CTRL, 0x0003); // ISM_CTRL -> RAM FW, Fastclk
    m_access->writeImagerRegister (SpiReg::MODE, 0x0002); // Continuous mode
    static_assert (SpiReg::ADR1 == SpiReg::CMD_ADR2 + 1, "Next line writes to the wrong registers");
    m_access->writeImagerBurst (SpiReg::CMD_ADR2, packCommandAndAddress (0x03 /* read */, startAddr));
    static_assert (0x100 == numBytesPerTransfer, "Byte count in the next line is wrong");
    m_access->writeImagerRegister (SpiReg::BYTE_COUNT, 0x0100); // Byte count (256 Bytes)
    m_access->writeImagerRegister (SpiReg::CONTINUE, 0x0001); // Continue = true
    m_access->writeImagerRegister (SpiReg::READY, 0x0001); // Ready = true

    m_access->writeImagerRegister (SpiReg::ISM_ENABLE, 0x0001); // ISM enable

    std::vector<uint8_t> flashContent;
    for (auto i = 0u; i < numPages; ++i)
    {
        // m_progressListener->reportProgress ("Reading page", i + 1, numPages);
        LOG (DEBUG) << "Reading page " << (i + 1) << " of " << numPages;

        pollUntilZero (SpiReg::READY, m_readTime, POLLING_INTERVAL);

        uint16_t errRegister;
        m_access->readImagerRegister (SpiReg::STATUS, errRegister);
        if (errRegister != 0)
        {
            throw RuntimeError ("Status error during read");
        }

        m_access->readImagerBurst (storeReadDataArea, registers);

        // Trigger the read out of the next 256 bytes
        if (i + 1 < numPages)
        {
            m_access->writeImagerRegister (SpiReg::READY, 0x0001); // Ready = true
        }

        for (auto curregister : registers)
        {
            uint16_t tmpval = (uint16_t) curregister;
            pushBackBe16 (flashContent, tmpval);
        }
    }
    if (flashContent.size() != roundUpToUnit (recvBuffer.size(), numBytesPerTransfer))
    {
        throw RuntimeError ("Mismatch between expected size and size read");
    }
    flashContent.resize (recvBuffer.size());
    recvBuffer.swap (flashContent);

    // Disable the firmware
    m_access->writeImagerRegister (SpiReg::CONTINUE, 0x0000); // Continue = false
    m_access->writeImagerRegister (SpiReg::READY, 0x0001); // Ready = true

    m_access->writeImagerRegister (SpiReg::ISM_ENABLE, 0x0000); // ISM disable
}

void StorageSpiImagerM2452::writeStorage (const std::vector<uint8_t> &buffer)
{
    LOG (DEBUG) << "StorageSpiImager::writeStorage, size=" << std::dec << buffer.size();

    if (buffer.size() > m_imageSize)
    {
        throw OutOfBounds ("Writing more data than can fit in the storage");
    }

    // The data will be written via these registers, each of which is 16-bits wide,
    auto registers = std::vector<uint16_t> (maxRegistersPerTransfer);

    // number of pages we have to write
    const auto numPages = divideRoundUp (buffer.size(), numBytesPerTransfer);
    const auto numSectors = divideRoundUp (buffer.size(), numBytesPerSector);

    loadFirmwareOnce();

    // Erase the sectors
    // -------------------

    m_access->writeImagerRegister (SpiReg::ISM_CTRL, 0x0003); // ISM_CTRL -> RAM FW, Fastclk
    m_access->writeImagerRegister (SpiReg::MODE, 0x0001); // Standalone mode
    m_access->writeImagerRegister (SpiReg::BYTE_COUNT, 0x0000); // Byte count (256 Bytes)

    for (auto i = 0u; i < numSectors; ++i)
    {
        LOG (DEBUG) << "Erasing sector " << (i + 1) << " of " << numSectors;

        static_assert (SpiReg::ADR1 == SpiReg::CMD_ADR2 + 1, "Next line writes to the wrong registers");
        m_access->writeImagerBurst (SpiReg::CMD_ADR2, packCommandAndAddress (0x20 /* erase */, i * numBytesPerSector));
        m_access->writeImagerRegister (SpiReg::ISM_ENABLE, 0x0001); // ISM enable
        // The erasing uses standalone mode, the iSM will disable itself when done

        pollUntilZero (SpiReg::ISM_ENABLE, m_eraseTime, POLLING_INTERVAL);
    }


    // Program the data
    // -------------------

    m_access->writeImagerRegister (SpiReg::MODE, 0x0002); // Continuous mode
    static_assert (SpiReg::ADR1 == SpiReg::CMD_ADR2 + 1, "Next line writes to the wrong registers");
    m_access->writeImagerBurst (SpiReg::CMD_ADR2, packCommandAndAddress (0x02, 0)); /* write starting at address zero */
    static_assert (0x100 == numBytesPerTransfer, "Byte count in the next line is wrong");
    m_access->writeImagerRegister (SpiReg::BYTE_COUNT, 0x0100); // Byte count (256 Bytes)
    m_access->writeImagerRegister (SpiReg::CONTINUE, 0x0001); // Continue = true
    m_access->writeImagerRegister (SpiReg::READY, 0x0000); // Ready = false

    m_access->writeImagerRegister (SpiReg::ISM_ENABLE, 0x0001); // ISM enable

    auto flashData = buffer;
    // We can only write words (2-byte units), but we'd also like to pad the final transfer to the
    // full transfer size, with zeros written in the padding.
    flashData.resize (roundUpToUnit (flashData.size(), numBytesPerTransfer), 0);

    size_t curIdx = 0u;
    for (auto i = 0u; i < numPages; ++i)
    {
        LOG (DEBUG) << "Writing page " << (i + 1) << " of " << numPages;

        for (auto &curregister : registers)
        {
            if (curIdx < flashData.size())
            {
                // The buffer is a byte stream, but the conversion to registers is to treat each
                // pair of bytes as a 16-bit word. The register values themselves are host-endian at
                // this stage, and will be converted to the imager's endianness in the software
                // imager.
                curregister = bufferToHostBe16 (&flashData.at (curIdx));
            }
            else
            {
                curregister = 0;
            }
            curIdx += 2u;
        }

        m_access->writeImagerBurst (storeReadDataArea, registers);

        // Trigger the writing of the next 256 bytes
        m_access->writeImagerRegister (SpiReg::READY, 0x0001); // Ready = true

        pollUntilZero (SpiReg::READY, m_writeTime, POLLING_INTERVAL);

        uint16_t errRegister;
        m_access->readImagerRegister (SpiReg::STATUS, errRegister);
        if (errRegister != 0)
        {
            throw RuntimeError ("Error during write");
        }
    }

    // Disable the firmware
    m_access->writeImagerRegister (SpiReg::CONTINUE, 0x0000); // Continue = false
    m_access->writeImagerRegister (SpiReg::READY, 0x0001); // Ready = true

    m_access->writeImagerRegister (SpiReg::ISM_ENABLE, 0x0000); // ISM disable
}

void StorageSpiImagerM2452::pollUntilZero (uint16_t reg, const std::chrono::microseconds firstSleep, const std::chrono::microseconds pollSleep)
{
    m_access->sleepFor (firstSleep);
    bool done = false;

    uint16_t val;
    m_access->readImagerRegister (reg, val);
    if (val == 0u)
    {
        done = true;
    }

    int retries = 0;
    while (!done && retries < MAX_POLL_ZERO_RETRIES)
    {
        LOG (DEBUG) << "Additional sleep in pollUntilZero";
        retries++;
        m_access->sleepFor (pollSleep);
        m_access->readImagerRegister (reg, val);
        if (val == 0u)
        {
            done = true;
        }
    }

    if (!done)
    {
        throw Timeout ("");
    }
}
