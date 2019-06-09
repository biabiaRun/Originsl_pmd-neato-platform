/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <storage/SpiBusMasterM2453.hpp>

#include <pal/ISpiBusAccess.hpp>

#include <royale/Vector.hpp>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>
#include <iomanip>

#include <common/EndianConversion.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/IntegerMath.hpp>
#include <common/NarrowCast.hpp>

#include <common/RoyaleLogger.hpp>

// \todo: ROYAL-2908 dependencies between the components
#include <imager/M2453/ImagerRegisters.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::storage;

namespace
{
    /**
     * Hardware limit for payload, in 8-bit bytes.  Technically this isn't the maximum, the actual
     * limit is that preamble + data must fit in 260 bytes.
     */
    const std::size_t MAX_DATA_BYTES_PER_TRANSFER = 256;

    /**
     * Max data transmitted in the transmit part of a transmit+receive transaction.
     */
    const std::size_t MAX_PREAMBLE_BYTES = 4;

    /** The data will be transferred in 16-bit registers */
    const std::size_t REGISTER_SIZE = 2;

    /**
     * A large data buffer in the M2453's memory, which we can use (provided we have exclusive
     * access to the imager).  It's the image capture buffer, so will be overwritten if the normal
     * software imager is controlling the imager.
     *
     * This is the address of a register (so arithmetic should treat this as a *uint16_t).
     */
    const uint32_t PIXMEM = 0u;

    /**
     * The M2453 does not interpret SPI commands, instead Royale must form the command bytes that
     * will be sent to the device, and store them somewhere in the M2453's memory. For write
     * requests, the transmitted request must include all of the data (up to 260 bytes).
     */
    const uint16_t WRITE_BUFFER = PIXMEM;

    /**
     * When reading data, it is transferred in a block of registers, starting at this address.
     *
     * There are at least MAX_DATA_BYTES_PER_TRANSFER consecutive registers available, starting at
     * this address.
     */
    const uint16_t READ_BUFFER = PIXMEM + 256u;

    // The longest supported command would be a write of 256 bytes, which would need a 1 + 3 + 256 byte buffer
    // (1 for command, 3 for address).  Sanity check that the reserved areas are big enough
    //
    // The hardware does support the buffers overlapping (tested with the M2455), at least for
    // WRITE_BUFFER < READ_BUFFER.
    static_assert (260u <= REGISTER_SIZE * (WRITE_BUFFER < READ_BUFFER ? READ_BUFFER - WRITE_BUFFER : WRITE_BUFFER - READ_BUFFER),
                   "Overlap between WRITE_BUFFER and READ_BUFFER");

    /**
     * The number of times that waitForTransfer sleeps for pollingInterval, before it throws a
     * Timeout.
     */
    const auto MAX_POLL_RETRIES = 10;

    /**
     * Local definitions for the M2453's register addresses. Currently defined by using constants in
     * M2453/ImagerRegisters.hpp, which could be removed if the imager component becomes a separate
     * project.
     */
    enum ImagerSpiReg : uint16_t
    {
        SPI_WR_ADDR = royale::imager::M2453::SPIWRADDR,
        SPI_RD_ADDR = royale::imager::M2453::SPIRADDR,
        SPI_LEN = royale::imager::M2453::SPILEN,
        SPI_TRIG = royale::imager::M2453::SPITRIG,
        SPI_STATUS = royale::imager::M2453::SPISTATUS,
        SPI_CFG = royale::imager::M2453::SPICFG,
    };

    /**
     * An upper limit for the time to for the SPI bus to transfer 260 bytes (in each direction).
     */
    const auto TIME_FOR_TRANSFER = std::chrono::microseconds (500);
    const auto POLLING_INTERVAL = std::chrono::milliseconds (10);
}

SpiBusMasterM2453::SpiBusMasterM2453 (std::shared_ptr<royale::hal::IBridgeImager> access,
                                      royale::config::ImagerAsBridgeType firmwareType) :
    m_imager (access)
{
    switch (firmwareType)
    {
        case royale::config::ImagerAsBridgeType::M2453_SPI:
            m_transmitRequiresReadEnabled = false;
            break;
        case royale::config::ImagerAsBridgeType::M2455_SPI:
            m_transmitRequiresReadEnabled = true;
            break;
        default:
            throw NotImplemented ("This storage access class only supports M2453 and M2455");
    }
}

void SpiBusMasterM2453::initializeOnce()
{
    // Reset the imager to be sure it's in a known state
    m_imager->setImagerReset (true);
    m_imager->sleepFor (std::chrono::microseconds (1));
    m_imager->setImagerReset (false);

    // No additional firmware is required, the existing ROM firmware is sufficient.  The SPI_CFG write
    // enables it and sets the clock divisor as expected.
    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    m_imager->writeImagerRegister (SPI_CFG, static_cast<uint16_t> (spiEnable | spiClockDiv8));
}

SpiBusMasterM2453::~SpiBusMasterM2453()
{
    try
    {
        // Reset the imager to be sure it's left in a known state
        m_imager->setImagerReset (true);
        m_imager->sleepFor (std::chrono::microseconds (1));
        m_imager->setImagerReset (false);
    }
    catch (...)
    {
        // ignore, if this can't reset the imager then it's unlikely that an IImager implementation
        // will be able to access it either
    }
}

std::unique_lock<std::recursive_mutex> SpiBusMasterM2453::selectDevice (uint8_t id)
{
    initializeOnce();

    if (id != ONLY_DEVICE_ON_IMAGERS_SPI)
    {
        throw LogicError ("Only one SPI slave is supported");
    }
    return std::unique_lock<std::recursive_mutex> (m_lock);
}

SpiBusMasterM2453::ReadSize SpiBusMasterM2453::maximumReadSize ()
{
    return {MAX_PREAMBLE_BYTES, MAX_DATA_BYTES_PER_TRANSFER};
}

std::size_t SpiBusMasterM2453::maximumWriteSize ()
{
    return MAX_PREAMBLE_BYTES + MAX_DATA_BYTES_PER_TRANSFER;
}

void SpiBusMasterM2453::doTransfer (const std::vector<uint8_t> &transmit, std::size_t receiveSize)
{
    static_assert (REGISTER_SIZE == 2, "Hardcoded conversions between uint8_t and registers are wrong");

    std::vector<uint16_t> transmitRegs;
    pushBackIterableAsHighFirst16 (transmitRegs, transmit);
    m_imager->writeImagerBurst (WRITE_BUFFER, transmitRegs);

    if (transmit.empty())
    {
        throw LogicError ("No command to send");
    }
    auto totalLength = transmit.size() + receiveSize;
    if (totalLength > 260u)
    {
        throw LogicError ("Combined command and receive buffers are too long");
    }
    uint16_t spiLen;
    if (receiveSize > 0)
    {
        // this is a transmit + receive, also known as a read. Set the read-delay to start sampling
        // MISO after the MOSI data has been transferred
        if (transmit.size() > MAX_PREAMBLE_BYTES)
        {
            throw LogicError ("Can't send a command longer than 4 bytes when triggering a read");
        }
        const auto readEnable = 1 << 13;
        spiLen = narrow_cast<uint16_t> ( (transmit.size() - 1) << 14 | readEnable | (totalLength - 1));
    }
    else if (m_transmitRequiresReadEnabled)
    {
        // This is a transmit without receiving data afterwards, also known as a write.
        // As a workaround for a bug in the M2455 A11 design step, readEnable needs to be set.
        const auto readEnable = 1 << 13;
        spiLen = narrow_cast<uint16_t> (readEnable | (totalLength - 1));
    }
    else
    {
        // this is a write, and no flags need to be set in the spiLen
        spiLen = narrow_cast<uint16_t> (totalLength - 1);
    }

    // Check that the registers are in the right order so that we can use a burst write
    static_assert (SPI_WR_ADDR + 1 == SPI_RD_ADDR, "These registers should be consecutive");
    static_assert (SPI_RD_ADDR + 1 == SPI_LEN, "These registers should be consecutive");
    static_assert (SPI_LEN + 1 == SPI_TRIG, "These registers should be consecutive");

    std::vector<uint16_t> data;
    data.push_back (WRITE_BUFFER); // SPI_WR_ADDR
    data.push_back (READ_BUFFER); // SPI_RD_ADDR
    data.push_back (spiLen); // SPI_LEN
    data.push_back (2u); // SPI_TRIG
    m_imager->writeImagerBurst (SPI_WR_ADDR, data);

    //wait for the transfer to finish
    waitForTransfer();
}

void SpiBusMasterM2453::readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive)
{
    if (receive.empty())
    {
        throw LogicError ("Trying to read zero bytes");
    }

    doTransfer (transmit, receive.size());

    auto receiveRegs = std::vector<uint16_t> (divideRoundUp (receive.size(), REGISTER_SIZE));
    m_imager->readImagerBurst (READ_BUFFER, receiveRegs);
    std::vector<uint8_t> temp;
    for (const auto reg : receiveRegs)
    {
        pushBackBe16 (temp, reg);
    }
    // if receive.size() is odd then temp will be one byte larger
    temp.resize (receive.size());
    std::swap (temp, receive);
}

void SpiBusMasterM2453::writeSpi (const std::vector<uint8_t> &transmit)
{
    doTransfer (transmit, 0);
}

void SpiBusMasterM2453::waitForTransfer()
{
    const uint16_t ERROR_FLAG = 2;
    const uint16_t DONE_VALUE = 1;

    // on success, this loop exits via the return statement
    for (int i = 0; i <= MAX_POLL_RETRIES; ++i)
    {
        if (i == 0)
        {
            m_imager->sleepFor (TIME_FOR_TRANSFER);
        }
        else
        {
            LOG (DEBUG) << "Additional sleep in pollUntil";
            m_imager->sleepFor (POLLING_INTERVAL);
        }

        uint16_t polledVal;
        m_imager->readImagerRegister (SPI_STATUS, polledVal);
        if (polledVal & ERROR_FLAG)
        {
            throw RuntimeError ("SPI hardware set the error flag");
        }
        if (polledVal == DONE_VALUE)
        {
            return;
        }
    }

    throw Timeout ("SPI not ready after data transfer");
}
