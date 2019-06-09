/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <SimImagerM2455.hpp>
#include <imager/M2455/ImagerRegisters.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>

using namespace royale::stub;
using namespace royale::common;
using namespace royale::imager::M2455;

SimImagerM2455::SimImagerM2455 ()
{
    m_simulatorRegisters = std::map<uint16_t, uint16_t>
    {
        { CFGCNT_FLAGS, 0x0000 }
    };
}

SimImagerM2455::~SimImagerM2455()
{
}

void SimImagerM2455::writeRegister (uint16_t regAddr, uint16_t value)
{
    m_simulatorRegisters[regAddr] = value;
}

void SimImagerM2455::runSimulation (std::chrono::microseconds sleepDuration)
{
    // simulate the imager reacting to the register value change
    if (m_simulatorRegisters.count (SPITRIG))
    {
        auto value = m_simulatorRegisters[SPITRIG];
        if ( (value & 0x0002) != 0) //the trigger bit is set
        {
            try
            {
                m_simulatorRegisters[SPISTATUS] = 0u; // SPI is busy
                initiateSpiTransfer();
                m_simulatorRegisters[SPISTATUS] = 1u; // SPI is ready
            }
            catch (const LogicError &)
            {
                m_simulatorRegisters[SPISTATUS] = 3u; // ready + error flag
            }

        }
        m_simulatorRegisters[SPITRIG] = 0u; //SPITRIG contains write-clear bits only
    }

    (void) sleepDuration;
}

void SimImagerM2455::initiateSpiTransfer()
{
    if (!m_simulatorRegisters.count (SPIWRADDR) ||
            !m_simulatorRegisters.count (SPIRADDR) ||
            !m_simulatorRegisters.count (SPILEN))
    {
        throw LogicError ("Incomplete configuration of registers");
    }

    //the imager address the incoming data should be written to
    const auto rdDataAndStoreToAdr = m_simulatorRegisters.at (SPIRADDR);

    //the imager address where the SPI query data is located
    const auto queryDataAdr = m_simulatorRegisters.at (SPIWRADDR);

    // Interpret the SPILEN register.
    // Here (length == 5 means 5 bytes), it doesn't have the offset-by-one
    const std::size_t length = 1 + (m_simulatorRegisters.at (SPILEN) & 0x1FF);
    const bool readEnabled = (m_simulatorRegisters.at (SPILEN) & (1 << 13)) != 0;
    if (!readEnabled)
    {
        // A bug in M2455_A11 that isn't in M2453
        throw LogicError ("M2455_A11 will not perform SPI if the read-enable bit is unset");
    }
    const std::size_t readDelay = 1 + (m_simulatorRegisters.at (SPILEN) >> 14);
    if ( (length <= readDelay) && ! ( (length == 1) && (readDelay == 1)))
    {
        // Because a single-byte write (for example setting the flash memory's write-enable flag)
        // needs to have readEnabled set, this accepts length == 1 with readDelay == 1.
        throw LogicError ("Probably incorrect SPILEN configuration, starts reading after the length is already finished");
    }

    // the data that will be sent from the imager to the flash (Master Out, Slave In)
    std::vector<uint8_t> mosi;

    // Copy data from registers to mosi. There's a sanity check that the registers containing command
    // and address are set, but for the other registers just substitute zero (it's probably padding
    // to get enough bytes in the miso).
    if (!m_simulatorRegisters.count (queryDataAdr))
    {
        throw LogicError ("The register for the command (and maybe first byte of the address) to send hasn't been set");
    }
    if (length > 2 && !m_simulatorRegisters.count (narrow_cast<uint16_t> (queryDataAdr + 1)))
    {
        throw LogicError ("The registers for the third and fourth bytes to send haven't been set");
    }
    for (auto i = queryDataAdr; mosi.size() < length; i++)
    {
        if (m_simulatorRegisters.count (i))
        {
            pushBackBe16 (mosi, m_simulatorRegisters.at (i));
        }
        else
        {
            pushBackBe16 (mosi, 0);
        }
    }

    // If length is odd, drop the final byte
    mosi.resize (length);

    auto miso = m_flashMemory.transfer (mosi);
    if (miso.size() != length)
    {
        throw LogicError ("Simulation didn't do full-duplex SPI (buffer sizes don't match)");
    }

    if ( (miso.size() - readDelay) % 2)
    {
        // pad miso so that the next loop doesn't need a special case to work with 16-bit registers
        miso.push_back (0);
    }
    for (size_t i = 0; i + readDelay < miso.size(); i += 2u)
    {
        m_simulatorRegisters[narrow_cast<uint16_t> (rdDataAndStoreToAdr + i / 2)] = bufferToHostBe16 (miso.data() + i + readDelay);
    }
}

void SimImagerM2455::startCapturing()
{

}

void SimImagerM2455::stopCapturing()
{

}

uint16_t SimImagerM2455::readCurrentRegisterValue (uint16_t regAddr)
{
    if (m_simulatorRegisters.find (regAddr) == m_simulatorRegisters.end())
    {
        throw DataNotFound ("This register's standard value is not defined");
    }
    else
    {
        return m_simulatorRegisters.at (regAddr);
    }
}
