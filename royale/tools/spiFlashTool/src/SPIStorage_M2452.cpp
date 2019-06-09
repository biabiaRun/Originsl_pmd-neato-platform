/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <SPIStorage_M2452.hpp>
#include <ImagerSPIFirmware_M2452_B1x.hpp>

#include <stdint.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <common/EndianConversion.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/IntegerMath.hpp>
#include <imager/ImagerM2452.hpp>

#include <common/RoyaleLogger.hpp>

using namespace royale;
using namespace royale::common;
using namespace spiFlashTool::storage;

namespace
{
    // we can read or write 256 Bytes per transfer
    const std::size_t numBytesPerTransfer = 256;

    // The memory is assumed to have 4096 bytes per sector
    const int numBytesPerSector = 4096;

    /**
     * Returns the set of registers to transfer the data in, corresponding to number of registers to
     * transfer (numBytesPerTransfer).  Each register is 16 bits wide.
     */
    Vector<Pair<String, uint64_t>> generateTransferRegisters()
    {
        decltype (generateTransferRegisters()) registers;
        for (auto i = 0u; i < numBytesPerTransfer / sizeof (uint16_t); ++i)
        {
            std::stringstream sstr;
            sstr << std::hex << "0x" << std::setfill ('0') << std::setw (4) << i;
            registers.push_back (Pair<String, uint64_t> (sstr.str(), 0));
        }

        return registers;
    }
}

SPIStorage_M2452::SPIStorage_M2452 (std::shared_ptr<royale::ICameraDevice> cameraDevice,
                                    IProgressReportListener *progressListener) :
    SPIStorageBase (progressListener),
    m_cameraDevice {cameraDevice}
{
}

CameraStatus SPIStorage_M2452::writeCommandAndAddress (uint8_t command, std::size_t address)
{
    // The caller should have already checked the address
    if (address > 0xffffff)
    {
        throw OutOfBounds ("");
    }

    Vector<Pair<String, uint64_t>> registers
    {
        {"0xB041", (command << 8) | ( (address >> 16) & 0xff) },
        {"0xB042", address & 0xffff}
    };
    return m_cameraDevice->writeRegisters (std::move (registers));
}

void SPIStorage_M2452::readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer)
{
    LOG (DEBUG) << "SPIStorage::readStorage, addr=0x" << std::hex << startAddr
                << ", size=" << std::dec << recvBuffer.size();

    if (startAddr + recvBuffer.size() > 0x1000000)
    {
        throw OutOfBounds ("Trying to read from an address that's higher than 24-bit");
    }

    // The data will be available in these registers, each of which is 16-bits wide,
    auto registers = generateTransferRegisters();

    // Number of pages we have to read; these pages are a feature of the transfer through the
    // imager, and do not have any required alignment in the SPI device itself.
    const auto numPages = divideRoundUp (recvBuffer.size(), numBytesPerTransfer);

    loadFirmwareOnce();

    // This is missing the status checking for all the calls, but this should be fine,
    // as this tool is only available for internal usage
    m_cameraDevice->writeRegisters ({ { "0xB401", 0x0003 } }); // ISM_CTRL -> RAM FW, Fastclk
    m_cameraDevice->writeRegisters ({ { "0xB043", 0x0002 } }); // Continuous mode
    writeCommandAndAddress (0x03 /* read */, startAddr);
    static_assert (0x100 == numBytesPerTransfer, "Byte count in the next line is wrong");
    m_cameraDevice->writeRegisters ({ { "0xB040", 0x0100 } }); // Byte count (256 Bytes)
    m_cameraDevice->writeRegisters ({ { "0x9801", 0x0001 } }); // Continue = true
    m_cameraDevice->writeRegisters ({ { "0x9800", 0x0001 } }); // Ready = true

    m_cameraDevice->writeRegisters ({ { "0xB400", 0x0001 } }); // ISM enable

    std::vector<uint8_t> flashContent;
    for (auto i = 0u; i < numPages; ++i)
    {
        m_progressListener->reportProgress ("Reading page", i + 1, numPages);

        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        bool done = false;
        while (!done)
        {
            Vector<Pair<String, uint64_t>> rdyRegister = { { "0x9800", 0x0 } };
            m_cameraDevice->readRegisters (rdyRegister);
            if (rdyRegister.at (0).second == 0)
            {
                done = true;
            }
            else
            {
                std::this_thread::sleep_for (std::chrono::microseconds (10));
            }
        }

        Vector<Pair<String, uint64_t>> errRegister = { { "0xB41F", 0x0 } };
        m_cameraDevice->readRegisters (errRegister);

        if (errRegister[0].second != 0)
        {
            throw RuntimeError ( (QString ("Read error : ") + QString::number (errRegister[0].second)).toStdString());
        }

        m_cameraDevice->readRegisters (registers);

        // Trigger the read out of the next 256 bytes
        if (i + 1 < numPages)
        {
            m_cameraDevice->writeRegisters ({ { "0x9800", 0x0001 } }); // Ready = true
        }

        for (auto curregister : registers)
        {
            uint16_t tmpval = (uint16_t) curregister.second;
            pushBackBe16 (flashContent, tmpval);
        }
    }
    if (flashContent.size() != roundUpToUnit (recvBuffer.size(), numBytesPerTransfer))
    {
        auto error = QString ("Mismatch between expected size and size read (%1, %2)").arg (flashContent.size()).arg (recvBuffer.size());
        throw RuntimeError (error.toStdString());
    }
    flashContent.resize (recvBuffer.size());
    recvBuffer.swap (flashContent);

    // Disable the firmware
    m_cameraDevice->writeRegisters ({ { "0x9801", 0x0000 } }); // Continue = false
    m_cameraDevice->writeRegisters ({ { "0x9800", 0x0001 } }); // Ready = true

    m_cameraDevice->writeRegisters ({ { "0xB400", 0x0000 } }); // ISM disable
}

void SPIStorage_M2452::writeStorage (const std::vector<uint8_t> &buffer)
{
    LOG (DEBUG) << "SPIStorage::writeStorage, size=" << std::dec << buffer.size();

    if (buffer.size() > 0x1000000)
    {
        throw OutOfBounds ("Would write to an address that's higher than 24-bit");
    }

    // The data will be written via these registers, each of which is 16-bits wide,
    auto registers = generateTransferRegisters();

    // number of pages we have to write
    const auto numPages = divideRoundUp (buffer.size(), numBytesPerTransfer);
    const auto numSectors = divideRoundUp (buffer.size(), numBytesPerSector);

    loadFirmwareOnce();

    // Erase the sectors
    // -------------------

    m_cameraDevice->writeRegisters ({ { "0xB401", 0x0003 } }); // ISM_CTRL -> RAM FW, Fastclk
    m_cameraDevice->writeRegisters ({ { "0xB043", 0x0001 } }); // Standalone mode
    m_cameraDevice->writeRegisters ({ { "0xB040", 0x0000 } }); // Byte count (256 Bytes)

    for (auto i = 0u; i < numSectors; ++i)
    {
        m_progressListener->reportProgress ("Erasing sector", i + 1, numSectors);

        writeCommandAndAddress (0x20 /* erase */, i * numBytesPerSector);
        m_cameraDevice->writeRegisters ({ { "0xB400", 0x0001 } }); // ISM enable
        // The erasing uses standalone mode, the iSM will disable itself when done

        std::this_thread::sleep_for (std::chrono::milliseconds (2000));
        bool done = false;
        while (!done)
        {
            Vector<Pair<String, uint64_t>> ismRegister = { { "0xB400", 0x0 } };
            m_cameraDevice->readRegisters (ismRegister);
            if (ismRegister.at (0).second == 0)
            {
                done = true;
            }
            else
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (10));
            }
        }
    }


    // Program the data
    // -------------------

    m_cameraDevice->writeRegisters ({ { "0xB043", 0x0002 } }); // Continuous mode
    writeCommandAndAddress (0x02, 0); /* write starting at address zero */
    static_assert (0x100 == numBytesPerTransfer, "Byte count in the next line is wrong");
    m_cameraDevice->writeRegisters ({ { "0xB040", 0x0100 } }); // Byte count (256 Bytes)
    m_cameraDevice->writeRegisters ({ { "0x9801", 0x0001 } }); // Continue = true
    m_cameraDevice->writeRegisters ({ { "0x9800", 0x0000 } }); // Ready = false

    m_cameraDevice->writeRegisters ({ { "0xB400", 0x0001 } }); // ISM enable

    auto flashData = buffer;
    // We can only write words (2-byte units), but we'd also like to pad the final transfer to the
    // full transfer size, with zeros written in the padding.
    flashData.resize (roundUpToUnit (flashData.size(), numBytesPerTransfer), 0);

    size_t curIdx = 0u;
    for (auto i = 0u; i < numPages; ++i)
    {
        m_progressListener->reportProgress ("Writing page", i + 1, numPages);

        for (auto &curregister : registers)
        {
            if (curIdx < flashData.size())
            {
                // The buffer is a byte stream, but the conversion to registers is to treat each
                // pair of bytes as a 16-bit word. The register values themselves are host-endian at
                // this stage, and will be converted to the imager's endianness in the software
                // imager.
                curregister.second = bufferToHostBe16 (&flashData.at (curIdx));
            }
            else
            {
                curregister.second = 0;
            }
            curIdx += 2u;
        }

        m_cameraDevice->writeRegisters (registers);

        // Trigger the writing of the next 256 bytes
        m_cameraDevice->writeRegisters ({ { "0x9800", 0x0001 } }); // Ready = true

        std::this_thread::sleep_for (std::chrono::microseconds (1000));
        bool done = false;
        while (!done)
        {
            Vector<Pair<String, uint64_t>> rdyRegister = { { "0x9800", 0x0 } };
            m_cameraDevice->readRegisters (rdyRegister);
            if (rdyRegister.at (0).second == 0)
            {
                done = true;
            }
            else
            {
                std::this_thread::sleep_for (std::chrono::microseconds (10));
            }
        }

        Vector<Pair<String, uint64_t>> errRegister = { { "0xB41F", 0x0 } };
        m_cameraDevice->readRegisters (errRegister);

        if (errRegister[0].second != 0)
        {
            throw RuntimeError ( (QString ("Write error : ") + QString::number (errRegister[0].second)).toStdString());
        }
    }

    // Disable the firmware
    m_cameraDevice->writeRegisters ({ { "0x9801", 0x0000 } }); // Continue = false
    m_cameraDevice->writeRegisters ({ { "0x9800", 0x0001 } }); // Ready = true

    m_cameraDevice->writeRegisters ({ { "0xB400", 0x0000 } }); // ISM disable
}

void SPIStorage_M2452::loadFirmware()
{
    // The ISM should not be running, but disable it anyway
    m_cameraDevice->writeRegisters ({ { "0xB400", 0x0000 } });

    // Tell the imager that we will write a RAM fw
    m_cameraDevice->writeRegisters ({ { "0xB402", 4 << 5 } });

    m_cameraDevice->writeRegisters (SPIFirmware_Page1);
    m_cameraDevice->writeRegisters ({ { "0xB402", 0x0000 } });

    m_firmwareLoaded = true;
}

void SPIStorage_M2452::loadFirmwareOnce()
{
    if (!m_firmwareLoaded)
    {
        loadFirmware();
    }
}

royale::Vector<royale::Pair<royale::String, uint64_t>> SPIStorage_M2452::getEFuseRegisters()
{
    return
    {
        { "0xA0A1", 0u },
        { "0xA0A2", 0u },
        { "0xA0A3", 0u },
        { "0xA0A4", 0u }
    };
}
