/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/bridge/BridgeEnclustra.hpp>
#include <buffer/BufferUtils.hpp>

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>

#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Disconnected.hpp>

#include <common/events/EventCaptureStream.hpp>
#include <common/events/EventDeviceDisconnected.hpp>

#include <common/RoyaleLogger.hpp>
#include <common/RoyaleProfiler.hpp>
#include <iostream>

#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

using namespace royale::usb::bridge;
using namespace royale::common;
using namespace royale::buffer;
using namespace royale::pal;
using std::size_t;

BridgeEnclustra::BridgeEnclustra () :
    m_i2cAddressImager (0x3D), // All imagers on Enclustra based boards use this.
    m_runAcquisition (false)
{
}

void BridgeEnclustra::doCommand (EnclustraCommand command)
{
    std::vector<uint8_t> writeBuffer;
    pushBack32 (writeBuffer, command);
    doCommand (command, writeBuffer);
}

void BridgeEnclustra::setImagerReset (bool state)
{
    if (state)
    {
        doCommand (EnclustraCommand::RESET_IMAGER);
    }
}

namespace
{
    /**
     * Whether the I2C address register is 8-bit or 16-bit. For no address, a different function is
     * used instead of pushBackCyI2cPreamble.
     */
    enum class I2cAddrSize
    {
        EIGHT_BIT,
        SIXTEEN_BIT
    };

    /**
     * Append to the buffer the commands for a CypressFX3 write or write/read pair.
     */
    void pushBackCyI2cPreamble (std::vector<uint8_t> &buffer, bool read, uint8_t devAddr, I2cAddrSize regAddrSize, unsigned int regAddr)
    {
        // 4 octet command code (see EnclustraCommand enum)
        // 4 octet payload length
        // 4 octet flags, including a "retry" flag, always zero in our implementation
        // the above should be pushed by the caller

        // 8 octet (fixed size) I2C preamble, containing one or two I2C preambles:
        // * Each preamble is:
        // * 7 bit I2C device address, followed by 1 bit read/write flag (set for read)
        // * 0-2 octets I2C register address, big endian
        //
        // 4 octets of flags and masks for the I2C preamble
        //
        // This works with the I2C method of reading from a register:
        // * Write the register address that will be read from
        // * Then do a read
        // For a read the control mask (which comes later) has a bitmask of which octets are part of
        // the write, and which are part of the read.
        //
        // For a read without an address, no write is required.

        const std::size_t startPreamble = buffer.size();

        // Writing the address register
        buffer.push_back (static_cast<uint8_t> (devAddr << 1));
        switch (regAddrSize)
        {
            case I2cAddrSize::EIGHT_BIT:
                buffer.push_back (static_cast<uint8_t> (regAddr));
                break;
            case I2cAddrSize::SIXTEEN_BIT:
                pushBackBe16 (buffer, static_cast<uint16_t> (regAddr));
                break;
        };

        // Triggering a read
        if (read)
        {
            buffer.push_back (static_cast<uint8_t> ( (devAddr << 1) | 1));
        }

        // Fill the rest of the 8-byte preamble
        const std::size_t preambleUsedBytes = buffer.size() - startPreamble;
        for (std::size_t i = preambleUsedBytes; i < 8 ; i++)
        {
            buffer.push_back (0);
        }

        // 1 octet indicating how many octets of the preamble are used
        buffer.push_back (static_cast<uint8_t> (preambleUsedBytes));
        // 1 octet flags
        buffer.push_back (0);
        // 2 octet control mask
        if (read)
        {
            // This is two I2C messages, writing the register address and then reading
            switch (regAddrSize)
            {
                case I2cAddrSize::EIGHT_BIT:
                    buffer.push_back (2);
                    break;
                case I2cAddrSize::SIXTEEN_BIT:
                    buffer.push_back (4);
                    break;
            };
            buffer.push_back (0);
        }
        else
        {
            // Just one I2C message
            buffer.push_back (0);
            buffer.push_back (0);
        }
    }

    /**
     * Append to the buffer the commands for a CypressFX3 write or write/read pair, for a device
     * read/write that doesn't include a register number.
     */
    void pushBackCyI2cPreambleNoAddress (std::vector<uint8_t> &buffer, bool read, uint8_t devAddr)
    {
        // Compared to the version with a register address, there is no write required
        // to do a read.

        const unsigned int readBit = read ? 1 : 0;
        buffer.push_back (static_cast<uint8_t> ( (devAddr << 1) | readBit));

        // Fill the rest of the 8-byte preamble
        const std::size_t preambleUsedBytes = 1;
        for (std::size_t i = preambleUsedBytes; i < 8 ; i++)
        {
            buffer.push_back (0);
        }

        // 1 octet indicating how many octets of the preamble are used
        buffer.push_back (static_cast<uint8_t> (preambleUsedBytes));
        // 1 octet flags
        buffer.push_back (0);
        // 2 octet control mask.  It's just one I2C message.
        buffer.push_back (0);
        buffer.push_back (0);
    }

    /** For an I2C command, the amount of data that isn't register values */
    const std::size_t CY_I2C_COMMAND_AND_PREAMBLE_SIZE = 24;
}

void BridgeEnclustra::readImagerRegister (uint16_t regAddr, uint16_t &value)
{
    std::vector<uint8_t> recvBuffer;
    recvBuffer.resize (2); // reading a single 16-bit value
    readI2c (m_i2cAddressImager, I2cAddressMode::I2C_16BIT, regAddr, recvBuffer);
    value = bufferToHostBe16 (&recvBuffer[0]);
}

void BridgeEnclustra::writeImagerRegister (uint16_t regAddr, uint16_t value)
{
    std::vector<uint8_t> sendBuffer;
    pushBackBe16 (sendBuffer, value);
    writeI2c (m_i2cAddressImager, I2cAddressMode::I2C_16BIT, regAddr, sendBuffer);
}

void BridgeEnclustra::readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values)
{
    const size_t maxRegistersPerWrite = maximumDataSize() / sizeof (uint16_t);
    const reg16VectorSize_t lastBlock = values.size() - (values.size() % maxRegistersPerWrite);

    for (reg16VectorSize_t i = 0; i < lastBlock ; i += maxRegistersPerWrite)
    {
        readImagerBurstAtomic (static_cast<uint16_t> (firstRegAddr + i), values, i, i + maxRegistersPerWrite);
    }
    if (lastBlock != values.size())
    {
        readImagerBurstAtomic (static_cast<uint16_t> (firstRegAddr + lastBlock), values, lastBlock, values.size());
    }
}

void BridgeEnclustra::readImagerBurstAtomic (uint16_t regAddr, std::vector<uint16_t> &values, reg16VectorSize_t start, reg16VectorSize_t end)
{
    std::vector<uint8_t> recvBuffer;
    recvBuffer.resize ( (end - start) * sizeof (uint16_t));
    readI2c (m_i2cAddressImager, I2cAddressMode::I2C_16BIT, regAddr, recvBuffer);
    for (reg16VectorSize_t i = 0 ; i + start < end ; i++)
    {
        values[i + start] = bufferToHostBe16 (&recvBuffer[i * sizeof (uint16_t)]);
    }
}

void BridgeEnclustra::writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values)
{
    const std::size_t maxRegistersPerWrite = (getCommandMessageSize() - CY_I2C_COMMAND_AND_PREAMBLE_SIZE) / sizeof (uint16_t);
    const reg16VectorSize_t lastBlock = values.size() - (values.size() % maxRegistersPerWrite);

    for (reg16VectorSize_t i = 0; i < lastBlock ; i += maxRegistersPerWrite)
    {
        writeImagerBurstAtomic (static_cast<uint16_t> (firstRegAddr + i), values, i, i + maxRegistersPerWrite);
    }
    if (lastBlock != values.size())
    {
        writeImagerBurstAtomic (static_cast<uint16_t> (firstRegAddr + lastBlock), values, lastBlock, values.size());
    }
}

void BridgeEnclustra::writeImagerBurstAtomic (uint16_t regAddr, const std::vector<uint16_t> &values, reg16VectorSize_t start, reg16VectorSize_t end)
{
    std::vector<uint8_t> sendBuffer;
    for (reg16VectorSize_t i = start ; i < end; i++)
    {
        pushBackBe16 (sendBuffer, values[i]);
    }
    writeI2c (m_i2cAddressImager, I2cAddressMode::I2C_16BIT, regAddr, sendBuffer);
}

std::size_t BridgeEnclustra::executeUseCase (int imageWidth, int imageHeight, std::size_t bufferCount)
{
    // There's a reliance on stopCapture() first, so that the acquisition thread isn't holding a
    // buffer.  Royale always does stop the capture, so this can be relied on.
    if (m_runAcquisition)
    {
        throw NotImplemented ("BridgeEnclustra only supports changing use case when stopped");
    }

    // This is called from the FrameCollector, the FrameCollector frees its own
    // buffers, we just need to wait for it.
    waitCaptureBufferDealloc();

    // The Enclustra protocol puts its overhead after the image, so the pixels start
    // at byte zero.  Included in the calculations for clarity.
    const size_t pixelOffset = 0;

    const size_t pixelCount = imageWidth * imageHeight;

    size_t bufferSize = pixelCount * sizeof (uint16_t);
    // If the size is an exact multiple of ENCLUSTRA_FRAME_PADDING_SIZE, add a full packet of
    // overhead, otherwise just round up to the next multiple of ENCLUSTRA_FRAME_PADDING_SIZE.
    bufferSize += ENCLUSTRA_FRAME_PADDING_SIZE;
    bufferSize -= bufferSize % ENCLUSTRA_FRAME_PADDING_SIZE;

    createAndQueueInternalBuffers (bufferCount, bufferSize, pixelOffset, pixelCount);

    // as createAndQueueInternalBuffers didn't throw, we have all the requested buffers
    return bufferCount;
}

void BridgeEnclustra::startCapture()
{
    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    if (m_runAcquisition)
    {
        throw LogicError ("Acquisition thread is already running");
    }

    m_runAcquisition = true;
    m_acquisitionThread = std::thread (&BridgeEnclustra::acquisitionFunction, this);
    // The START_IMAGER is done with the mutex still held, so that a simultaneous call to
    // stopCapture can't send the STOP_IMAGER and then have it overwritten by the next line.

    // Send a STOP first to ensure that the firmware is in a known state before restarting the
    // capture.  If the firmware's capture cycle is already started (by a previous run of the
    // application that crashed), then sending another START without a STOP can make it start
    // sending junk data.  RESET_IMAGER is already being sent, but doesn't reset the error
    // condition.
    doCommand (EnclustraCommand::STOP_IMAGER);
    doCommand (EnclustraCommand::START_IMAGER);
}

void BridgeEnclustra::stopCapture()
{
    std::lock_guard<std::mutex> lock (m_acquisitionStartStopLock);
    if (m_runAcquisition)
    {
        m_runAcquisition = false;
        unblockDequeueThread();
        m_acquisitionThread.join();

        // The STOP_IMAGER is sent after the acquisition thread has finished, otherwise the
        // acquisition thread might block until the USB read times out.  And the conveyance
        // thread needs to be stopped even if doCommand throws an exception.
        doCommand (EnclustraCommand::STOP_IMAGER);
    }
}

namespace
{
    /**
     * When an unaligned frame is received, the acquisitionFunction will try to recover the current
     * frame.  If more frames require realignment then the acquisitionFunction will drop data for
     * speed.
     *
     * These control how long the bridge stays in the aggressive mode dropping frames for speed.
     * After the first unaligned frame, it needs SINGLE_REALIGN_COUNT successful captures, once it
     * switches to the aggressive mode then it needs DOUBLE_REALIGN_COUNT.
     */
    const unsigned int SINGLE_REALIGN_COUNT = 2;
    const unsigned int DOUBLE_REALIGN_COUNT = 100;
}

bool BridgeEnclustra::shouldBlockForDequeue()
{
    return m_runAcquisition;
}

void BridgeEnclustra::setEventListener (royale::IEventListener *listener)
{
    m_eventForwarder.setEventListener (listener);
}

void BridgeEnclustra::acquisitionFunction()
{
    // The next data capture goes in to this buffer
    OffsetBasedCapturedBuffer *frame = nullptr;

    // If an unaligned frame is received while this is non-zero, the Bridge will drop the unaligned
    // data.  The Bridge stays in the aggressive mode dropping unaligned frames for speed until the
    // fastRealignCounter counter decrements to zero.
    unsigned int fastRealignCounter = 0;

    // If the acquisition thread catches an unexpected exception (possibly a Disconnect) then the
    // thread should stop.  But leave m_runAcquisition as true so that the thread cleanup and join()
    // is still done.
    bool keepRunning = true;

    while (m_runAcquisition && keepRunning)
    {
        try
        {
            if (!frame)
            {
                frame = dequeueInternalBuffer();
                if (!frame)
                {
                    // We're probably stopping after stopCapture has been called.  If it hasn't,
                    // have a short delay so that this doesn't become a busy loop.
                    if (m_runAcquisition)
                    {
                        LOG (ERROR) << "No free buffers from BridgeInternalBufferAlloc";
                        m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_WARNING, "No free buffers from BridgeInternalBufferAlloc");
                        std::chrono::microseconds usec (100000);
                        std::this_thread::sleep_for (usec);
                    }
                    continue;
                    // In the Enclustra, data starts being discarded if we're not reading it.  There
                    // will be a dropped frame when we start reading again, but that is handled by the
                    // existing error handling for dropped frames.
                }
            }

            if (! receiveRawFrame (*frame, 0))
            {
                // recoverable error, run the loop again (tests m_runAcquisition if the read has a timeout)
                LOG (ERROR) << "receiveRawFrame() returned false!";
                continue;
            }

            auto alignment = checkAlignment (*frame);
            // note: (alignment == 0) is the success case, no need for error handling
            if (alignment > 0)
            {
                // A frame has been dropped in the firmware (GPIF II state SCK_NOT_RDY), and this is
                // the first unaligned read.  This may be the first frame read by the acquisition
                // thread, or it may indicate that the acquisition thread is lagging.
                if (fastRealignCounter == 0)
                {
                    LOG (DEBUG) << "Frame not aligned, mismatched by " << alignment;
                    fastRealignCounter = SINGLE_REALIGN_COUNT;
                    auto buffer = frame->getUnderlyingBuffer ();
                    auto goodBytes = frame->getUnderlyingBufferSize () - alignment;
                    std::memmove (buffer, buffer + alignment, goodBytes);
                    if (receiveRawFrame (*frame, goodBytes))
                    {
                        alignment = checkAlignment (*frame);
                    }
                    else
                    {
                        LOG (ERROR) << "receiveRawFrame() returned false while trying to realign!";
                    }
                    if (alignment != 0)
                    {
                        LOG (DEBUG) << "Frame not aligned after realignment, dropping";
                        continue;
                    }
                }
                else
                {
                    // The acquisition thread has already tried to realign.  Spending too long trying
                    // to realign can cause problems with the next frame too, so just read the
                    // remaining data to clear the hardware's buffer.
                    auto goodBytes = frame->getUnderlyingBufferSize () - alignment;
                    if (!receiveRawFrame (*frame, goodBytes))
                    {
                        LOG (DEBUG) << "Failed to read data from USB";
                    }
                    fastRealignCounter = DOUBLE_REALIGN_COUNT;
                    // The buffer now contains junk, but the next frame should be readable
                    continue;
                }
            }
            else if (alignment < 0)
            {
                // Frame can not be aligned, just drop it
                continue;
            }

            BufferUtils::normalizeEnclustra (*frame);

            bufferCallback (frame); // may throw

            frame = nullptr;

            if (fastRealignCounter != 0)
            {
                fastRealignCounter--;
            }
        }
        catch (const Disconnected &)
        {
            m_isConnected = false;
            m_eventForwarder.event<royale::event::EventDeviceDisconnected>();

            LOG (ERROR) << "USB Device disconnected.";
            keepRunning = false;
        }
        catch (const Exception &e)
        {
            m_eventForwarder.event<royale::event::EventCaptureStream> (royale::EventSeverity::ROYALE_ERROR, e.getUserDescription());
            LOG (ERROR) << "Caught an exception in the acquisition thread, no more frames will be captured";
            keepRunning = false;
        }
    }

    if (frame)
    {
        queueBuffer (frame);
    }
}

namespace
{
    /**
     * Given the data read from the Enclustra (still in little-endian format), and a byte offset
     * to the start byte of the 16-bit little endian pixel information, check the flags embedded
     * inside the pixel.
     *
     * The imager's pseudodata area is included in the pixel area.
     *
     * \return true if it's a pixel, false if it's between-frame padding.
     */
    // In Enclustra's 16-bit representation of 12-bit Imager data, bit 15 is set for data that is
    // in-frame, and not set for the end-of-frame lines.  We know that the complete data is 16-bit
    // aligned.
    inline bool isInPixelArea (const uint8_t rawData[], const size_t offset)
    {
        return (rawData[offset + 1] & 0x80) != 0;
    }
}

std::ptrdiff_t BridgeEnclustra::checkAlignment (OffsetBasedCapturedBuffer &frame) const
{
    // We know that the complete data read is 16-bit aligned, but not where the start point is
    const auto rawData = frame.getUnderlyingBuffer();
    const auto rawSize = frame.getUnderlyingBufferSize();

    // Assuming that the frame size is correct, test that the first pixel is part of the frame, and that the
    // last two pixels are part of the padding.
    if (isInPixelArea (rawData, 0) && (! isInPixelArea (rawData, rawSize - 2)) && (! isInPixelArea (rawData, rawSize - 4)))
    {
        // success, it is aligned
        return 0;
    }

    // It needs alignment.  If the alignment is not a multiple ENCLUSTRA_MIN_DATA_PACKET_SIZE, then
    // we can't realign by reading anyway. So only test conditions that we can deal with.
    for (size_t offset = ENCLUSTRA_MIN_DATA_PACKET_SIZE; offset < rawSize ; offset += ENCLUSTRA_MIN_DATA_PACKET_SIZE)
    {
        static_assert (ENCLUSTRA_MIN_DATA_PACKET_SIZE > 4, "Enclustra alignment code will test a negative array index");
        if (isInPixelArea (rawData, offset) && (! isInPixelArea (rawData, offset - 2)) && (! isInPixelArea (rawData, offset - 4)))
        {
            return offset;
        }
    }

    return -1;
}

void BridgeEnclustra::readFlashPage (std::size_t page, std::vector<uint8_t> &recvBuffer, std::size_t noPages)
{
    std::vector<uint8_t> sendBuffer;
    pushBack32 (sendBuffer, EnclustraCommand::READ_FLASH);
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (page));
    // This request will read up to "noPages" pages (default == 1)
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (noPages));
    doCommand (EnclustraCommand::READ_FLASH, sendBuffer, &recvBuffer);
}

void BridgeEnclustra::writeFlashPage (std::size_t page, const std::vector<uint8_t> &sndBuffer, std::size_t noPages)
{
    std::vector<uint8_t> sendBuffer;
    pushBack32 (sendBuffer, EnclustraCommand::WRITE_FLASH);
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (page));
    // This request will send up to "noPages" pages (default == 1)
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (noPages));
    doCommand (EnclustraCommand::WRITE_FLASH, sendBuffer, nullptr, &sndBuffer);
}

void BridgeEnclustra::eraseSectors (std::size_t startSector, std::size_t noSectors)
{
    std::vector<uint8_t> sendBuffer;
    pushBack32 (sendBuffer, EnclustraCommand::ERASE_FLASH);
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (startSector));
    // This request will erase up to "noSectors" sectors
    pushBack32 (sendBuffer, narrow_cast<uint32_t> (noSectors));
    doCommand (EnclustraCommand::ERASE_FLASH, sendBuffer);
}

void BridgeEnclustra::readI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, std::vector<uint8_t> &recvBuffer)
{
    std::vector<uint8_t> sendBuffer;

    pushBack32 (sendBuffer, EnclustraCommand::READ_I2C);
    // how many bytes to read
    pushBack32 (sendBuffer, static_cast<uint32_t> (recvBuffer.size()));
    //retry flag
    pushBack32 (sendBuffer, 0);

    switch (addrMode)
    {
        case I2cAddressMode::I2C_NO_ADDRESS:
            pushBackCyI2cPreambleNoAddress (sendBuffer, true, devAddr);
            break;
        case I2cAddressMode::I2C_8BIT:
            pushBackCyI2cPreamble (sendBuffer, true, devAddr, I2cAddrSize::EIGHT_BIT, narrow_cast<uint8_t> (regAddr));
            break;
        case I2cAddressMode::I2C_16BIT:
            pushBackCyI2cPreamble (sendBuffer, true, devAddr, I2cAddrSize::SIXTEEN_BIT, regAddr);
            break;
        default:
            throw LogicError ();
    }
    doCommand (EnclustraCommand::READ_I2C, sendBuffer, &recvBuffer);
}

void BridgeEnclustra::writeI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, const std::vector<uint8_t> &buffer)
{
    std::vector<uint8_t> sendBuffer;

    pushBack32 (sendBuffer, EnclustraCommand::WRITE_I2C);
    // how many bytes to write
    pushBack32 (sendBuffer, static_cast<uint32_t> (buffer.size()));
    //retry flag
    pushBack32 (sendBuffer, 0);

    switch (addrMode)
    {
        case I2cAddressMode::I2C_NO_ADDRESS:
            pushBackCyI2cPreambleNoAddress (sendBuffer, false, devAddr);
            break;
        case I2cAddressMode::I2C_8BIT:
            pushBackCyI2cPreamble (sendBuffer, false, devAddr, I2cAddrSize::EIGHT_BIT, narrow_cast<uint8_t> (regAddr));
            break;
        case I2cAddressMode::I2C_16BIT:
            pushBackCyI2cPreamble (sendBuffer, false, devAddr, I2cAddrSize::SIXTEEN_BIT, regAddr);
            break;
        default:
            throw LogicError ();
    }
    for (auto b : buffer)
    {
        sendBuffer.push_back (b);
    }
    doCommand (EnclustraCommand::WRITE_I2C, sendBuffer);
}

void BridgeEnclustra::setBusSpeed (uint32_t bps)
{
    throw NotImplemented();
}

std::size_t BridgeEnclustra::maximumDataSize()
{
    // For non-I2C operations, this is slightly smaller than is actually possible
    return getCommandMessageSize() - CY_I2C_COMMAND_AND_PREAMBLE_SIZE;
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeEnclustra::getBridgeInfo()
{
    decltype (getBridgeInfo()) info;
    info.emplace_back ("BRIDGE_TYPE", "Enclustra");
    return info;
}

void BridgeEnclustra::sleepFor (std::chrono::microseconds sleepDuration)
{
    std::this_thread::sleep_for (sleepDuration);
}
