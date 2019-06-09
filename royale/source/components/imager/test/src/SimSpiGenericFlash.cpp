/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <SimSpiGenericFlash.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>

using namespace royale::stub;
using namespace royale::common;

namespace
{
    enum SpiCommands : uint8_t
    {
        WRITE_DATA = 0x02,
        READ_DATA = 0x03,
        STATUS = 0x05,
        ENABLE_WRITE = 0x06,
        ERASE_SECTOR = 0x20
    };

    const auto SECTOR_SIZE = std::size_t
                             {
                                 4096
                             };
}

SimSpiGenericFlash::SimSpiGenericFlash () :
    m_flashMemorySpace (std::make_shared<std::map<uint32_t, uint8_t>> ())
{
}

std::vector<uint8_t> SimSpiGenericFlash::transfer (const std::vector<uint8_t> &mosi)
{
    // SPI is a full-duplex protocol - for each byte sent by the master, a byte is sent by the
    // slave. To read data from the slave, the master must pad the mosi (Master Out Slave In) vector
    // so that the miso (Master In Slave Out) is large enough.
    //
    // Correspondingly, the slave must add paddng at the start of the miso corresponding to the
    // bytes that it sends while receiving the master's command.

    std::vector<uint8_t> miso;

    const auto spiCommand = mosi.at (0);
    miso.push_back (0);
    switch (spiCommand)
    {
        case SpiCommands::ENABLE_WRITE:
            {
                if (mosi.size() != 1u)
                {
                    throw LogicError ("An enable_write is expected to be exactly 1 byte");
                }
                // The setting of m_writeEnabled happens after the switch block, so that it's
                // automatically reset by any command other than ENABLE_WRITE.
                break;
            }
        case SpiCommands::READ_DATA:
            {
                if (mosi.size() < 5u)
                {
                    throw LogicError ("A read would need at least 5 bytes");
                }
                const auto startAddr = bufferToHostBe24 (mosi.data() + 1);
                miso.push_back (0);
                miso.push_back (0);
                miso.push_back (0);
                for (auto i = std::size_t {0}; miso.size() < mosi.size(); ++i)
                {
                    const auto addr = narrow_cast<uint32_t> (startAddr + i);
                    if (0 == m_flashMemorySpace->count (addr))
                    {
                        throw OutOfBounds ("SPI query accesses uninitialized part of simulated flash memory space");
                    }
                    miso.push_back (m_flashMemorySpace->at (addr));
                }
                break;
            }
        case SpiCommands::WRITE_DATA:
            {
                if (mosi.size() < 5u)
                {
                    throw LogicError ("A write would need at least 5 bytes");
                }
                if (!m_writeEnabled)
                {
                    throw LogicError ("Write without setting write enable");
                }
                const auto startAddr = bufferToHostBe24 (mosi.data() + 1);
                miso.push_back (0);
                miso.push_back (0);
                miso.push_back (0);
                for (auto i = std::size_t {0}; miso.size() < mosi.size(); ++i)
                {
                    const auto addr = narrow_cast<uint32_t> (startAddr + i);
                    (*m_flashMemorySpace) [addr] = mosi.at (miso.size());
                    miso.push_back (0);
                }
                break;
            }
        case SpiCommands::ERASE_SECTOR:
            {
                if (mosi.size() != 4u)
                {
                    throw LogicError ("An erase is expected to be exactly 4 bytes");
                }
                if (!m_writeEnabled)
                {
                    throw LogicError ("Erase without setting write enable");
                }
                const auto startAddr = bufferToHostBe24 (mosi.data() + 1);
                miso.push_back (0);
                miso.push_back (0);
                miso.push_back (0);
                for (auto i = std::size_t {0}; i < SECTOR_SIZE; ++i)
                {
                    const auto addr = narrow_cast<uint32_t> (startAddr + i);
                    (*m_flashMemorySpace) [addr] = 0xff;
                }
                break;
            }
        case SpiCommands::STATUS:
            {
                if (mosi.size() != 2u)
                {
                    throw LogicError ("A status query is expected to be exactly 2 bytes");
                }
                // This zero is data, not a padding byte. The only bit that we're interested in is
                // bit zero, which is the write-in-progress or erase-in-progress bit. At the moment
                // the time spent writing isn't simulated, we just return that we're always ready.
                miso.push_back (0);
                break;
            }
        default:
            throw NotImplemented ("Received an SPI flash command that isn't simulated");
    }

    // set or reset the write-enabled flag
    m_writeEnabled = (spiCommand == SpiCommands::ENABLE_WRITE);

    if (miso.size() != mosi.size())
    {
        throw LogicError ("Didn't generate the expected size while handling SPI");
    }
    return miso;
}
