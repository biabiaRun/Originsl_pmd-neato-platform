/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)

#include <usb/bridge/BridgeEnclustraCyApi.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/EndianConversion.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <limits>

using namespace royale::usb::bridge;
using namespace royale::usb::pal;
using namespace royale::common;
using namespace royale::buffer;
using namespace royale;
using std::size_t;

namespace
{
    /**
     * The Enclustra uses three communication streams.  This enum is the indices for the USBDevice::EndPoints array.
     *
     * IN and OUT are from the viewpoint of the application processor or PC.
     */
    enum CyEndpoint : UCHAR
    {
        CONTROL_OUT = 0x02,
        CONTROL_IN = 0x03,
        /** Unidirectional communication stream containing the image data */
        DATA_IN = 0x01,
    };
}

BridgeEnclustraCyApi::BridgeEnclustraCyApi (std::unique_ptr<CCyUSBDevice> device) :
    m_deviceHandle (std::move (device)),
    m_commandLock(),
    m_transferBuffer()
{
}


bool BridgeEnclustraCyApi::isConnected() const
{
    return m_isConnected && (m_deviceHandle != nullptr);
}

BridgeEnclustraCyApi::~BridgeEnclustraCyApi()
{
    if (isConnected())
    {
        try
        {
            closeConnection();
        }
        catch (...)
        {
        }
    }
}

void BridgeEnclustraCyApi::openConnection()
{
    // CyApi doesn't allow us to find a device without opening it,
    // but also closing it once will mean it needs to be probed to reopen it
    if (!m_deviceHandle)
    {
        throw CouldNotOpen ("EnclustraCyApi handles can not be opened once closed");
    }

    flushInputBuffer();

    m_deviceHandle->EndPoints[CONTROL_OUT]->TimeOut = ENCLUSTRA_TIMEOUT;
    m_deviceHandle->EndPoints[CONTROL_IN]->TimeOut = ENCLUSTRA_TIMEOUT;

    m_deviceHandle->EndPoints[DATA_IN]->TimeOut = ENCLUSTRA_DATA_TIMEOUT;
    m_isConnected = true;
}

void BridgeEnclustraCyApi::closeConnection()
{
    stopCapture();

    if (m_deviceHandle)
    {
        m_deviceHandle->Close();
        m_deviceHandle.reset();
    }

    m_isConnected = false;
}

float BridgeEnclustraCyApi::getPeakTransferSpeed()
{
    if (!m_deviceHandle)
    {
        m_isConnected = false;
        throw Disconnected();
    }

    UsbSpeed usbSpeed;
    if (m_deviceHandle->bSuperSpeed)
    {
        // this should probably be a limit from the FX3 datasheet, but is the USB value
        usbSpeed = UsbSpeed::SUPER;
    }
    else
    {
        // assume USB 2.0, and return the high-speed value
        usbSpeed = UsbSpeed::HIGH;
    }
    return UsbSpeedUtils::calculatePeakTransferSpeed (usbSpeed, ENCLUSTRA_TRANSFER_FORMAT);
}

namespace
{
    /**
     * Common error handling - map the CyAPI return codes to Royale exceptions.
     *
     * This will return normally if the IO completed successfully.
     */
    void checkUsbError (const bool usbRet, const size_t expectedTransfer, const LONG transferred, CCyUSBEndPoint *endPoint)
    {
        ULONG usbdStatus = endPoint->UsbdStatus;
        ULONG ntStatus = endPoint->NtStatus;
        if (usbdStatus == 0xC0000011 && (ntStatus == 0xC0000001 || ntStatus == 0xC000009D))
        {
            // generic UNSUCCESSFUL (0xC0000001) or DEVICE_NOT_CONNECTED (0xC000009D)
            throw Disconnected ();
        }
        else if (usbdStatus == 0xC0010000 && ntStatus == 0xc0000120)
        {
            throw Timeout ();
        }
        else if (transferred != expectedTransfer)
        {
            LOG (ERROR) << "BridgeEnclustra USB IO short read/write: usbRet==" << usbRet << ", transferred==" << transferred;
            throw RuntimeError ("USB transfer was not the expected size");
        }
        else if (!usbRet)
        {
            throw RuntimeError ("Unknown USB IO error");
        }
    };
}

/**
 * This runs a command, and waits for the Enclustra to send the ACK back after running the
 * command.
 */
void BridgeEnclustraCyApi::doCommand (const EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer, std::vector<uint8_t> *recvBuffer, const std::vector<uint8_t> *sendBuffer)
{
    std::lock_guard<std::mutex> lock (m_commandLock);

    if (command != bufferToHost32 (&cmdBuffer[0]))
    {
        LOG (ERROR) << "command (" << command << ") != cmdBuffer (" << bufferToHost32 (&cmdBuffer[0]) << ")";
        throw LogicError ("Bug in BridgeEnclustra: command does not match command in cmdBuffer");
    }

    // The protocol requires all packets to be exactly the maximum transfer size
    const int blockSizeOut = m_deviceHandle->EndPoints[CONTROL_OUT]->MaxPktSize;
    const int blockSizeIn = m_deviceHandle->EndPoints[CONTROL_IN]->MaxPktSize;
    if (blockSizeOut < 0 || blockSizeIn < 0)
    {
        // While there are other things that could cause this, the likely reason is that
        // the device has been unplugged.
        m_isConnected = false;
        throw Disconnected();
    }

    m_transferBuffer.resize (std::max (blockSizeOut, blockSizeIn));

    if (cmdBuffer.size() > static_cast<unsigned> (blockSizeOut))
    {
        LOG (ERROR) << "BridgeEnclustra: command too long to send to Enclustra";
        throw InvalidValue ("Command too long to send to Enclustra");
    }

    if (command != EnclustraCommand::READ_FLASH &&
            command != EnclustraCommand::WRITE_FLASH)
    {
        if ( (recvBuffer != nullptr) && (recvBuffer->size() > static_cast<unsigned> (blockSizeIn)))
        {
            LOG (ERROR) << "BridgeEnclustra: expected response size is too big to read from Enclustra";
            throw InvalidValue ("Expected response size is too big to read from Enclustra");
        }
    }

    // copy command buffer to transfer buffer
    memcpy (&m_transferBuffer[0], &cmdBuffer[0], cmdBuffer.size());
    if (cmdBuffer.size() < m_transferBuffer.size())
    {
        memset (&m_transferBuffer[cmdBuffer.size()], 0, m_transferBuffer.size() - cmdBuffer.size());
    }

    // Send the command here
    LONG transferred = blockSizeOut;
    bool usbRet = m_deviceHandle->EndPoints[CONTROL_OUT]->XferData (&m_transferBuffer[0], transferred);
    checkUsbError (usbRet, blockSizeOut, transferred, m_deviceHandle->EndPoints[CONTROL_OUT]);

    switch (command)
    {
        case EnclustraCommand::READ_FLASH:
            {
                if (recvBuffer != nullptr)
                {
                    // This transfermode shall only apply if we are reading FLASH; otherwise the write's and read's
                    // have to be of the same size; the size that is read might differ from the size that was written
                    // in this mode - this is to read bigger amounts of data; with CyAPI we have to do that in a loop
                    // because it only supports reading 1024 or 512 chunks related to "m_deviceHandle->EndPoints[CONTROL_OUT]->MaxPktSize"
                    // which depends on high speed USB (USB2.0) and super speed USB (USB3.0)
                    // Also, with Enclustra we have to pad the total to a multiple of blockSizeIn, because the
                    // firmware will pad the flash data to fill the final chunk.
                    if (recvBuffer->size() > m_transferBuffer.size())       // This enhanced logic is only supported for flash reads!
                    {
                        /* Split into chunks of the size of blockSizeOut here and transfer them in a loop */
                        uint32_t chunks = static_cast<uint32_t> (recvBuffer->size() / m_transferBuffer.size());
                        uint32_t lastChunkSize = static_cast<uint32_t> (recvBuffer->size() % m_transferBuffer.size());

                        // loop through the 1024 or 512 byte chunks and read them separately (windows CyAPI driver dependent)
                        for (size_t i = 0; i < chunks; ++i)
                        {
                            transferred = static_cast<LONG> (m_transferBuffer.size());
                            usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
                            checkUsbError (usbRet, m_transferBuffer.size(), transferred, m_deviceHandle->EndPoints[CONTROL_IN]);
                            std::memcpy (recvBuffer->data() + (i * m_transferBuffer.size()), &m_transferBuffer[0], transferred);
                        }

                        // Next we read the remaining chunkLength if the whole buffer size was not a product of the
                        // block size.
                        if (lastChunkSize > 0)
                        {
                            usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
                            checkUsbError (usbRet, m_transferBuffer.size(), transferred, m_deviceHandle->EndPoints[CONTROL_IN]);
                            std::memcpy (recvBuffer->data() + (chunks * m_transferBuffer.size()), &m_transferBuffer[0], lastChunkSize);
                        }
                    }
                    else
                    {
                        usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
                        checkUsbError (usbRet, blockSizeIn, transferred, m_deviceHandle->EndPoints[CONTROL_IN]);
                        memcpy (recvBuffer->data(), &m_transferBuffer[0], recvBuffer->size());
                    }
                }
                break;
            }
        case EnclustraCommand::WRITE_FLASH:
            {
                if (sendBuffer != nullptr)
                {
                    // This transfermode shall only apply if we are writing FLASH
                    if (sendBuffer->size() > m_transferBuffer.size())
                    {
                        /* Split into chunks of the size of blockSizeOut here and transfer them in a loop */
                        uint32_t chunks = static_cast<uint32_t> (sendBuffer->size() / m_transferBuffer.size());
                        uint32_t lastChunkSize = static_cast<uint32_t> (sendBuffer->size() % m_transferBuffer.size());

                        // loop through the 1024 or 512 byte chunks and write them separately (windows CyAPI driver dependent)
                        for (size_t i = 0; i < chunks; ++i)
                        {
                            transferred = static_cast<LONG> (m_transferBuffer.size());
                            std::memcpy (&m_transferBuffer[0], sendBuffer->data() + (i * m_transferBuffer.size()), transferred);
                            usbRet = m_deviceHandle->EndPoints[CONTROL_OUT]->XferData (&m_transferBuffer[0], transferred);
                            checkUsbError (usbRet, m_transferBuffer.size(), transferred, m_deviceHandle->EndPoints[CONTROL_OUT]);
                        }

                        // Next we write the remaining chunkLength if the whole buffer size was not a product of the
                        // block size.
                        if (lastChunkSize > 0)
                        {
                            std::memcpy (&m_transferBuffer[0], sendBuffer->data() + (chunks * m_transferBuffer.size()), lastChunkSize);
                            if (m_transferBuffer.size() - lastChunkSize)
                            {
                                memset (&m_transferBuffer[lastChunkSize], 0xFF, m_transferBuffer.size() - lastChunkSize);
                            }
                            usbRet = m_deviceHandle->EndPoints[CONTROL_OUT]->XferData (&m_transferBuffer[0], transferred);
                            checkUsbError (usbRet, m_transferBuffer.size(), transferred, m_deviceHandle->EndPoints[CONTROL_OUT]);
                        }
                    }
                    else
                    {
                        memcpy (&m_transferBuffer[0], sendBuffer->data(), sendBuffer->size());
                        if (m_transferBuffer.size() - sendBuffer->size())
                        {
                            memset (&m_transferBuffer[sendBuffer->size()], 0xFF, m_transferBuffer.size() - sendBuffer->size());
                        }
                        usbRet = m_deviceHandle->EndPoints[CONTROL_OUT]->XferData (&m_transferBuffer[0], transferred);
                        checkUsbError (usbRet, blockSizeIn, transferred, m_deviceHandle->EndPoints[CONTROL_OUT]);
                    }
                }
                break;
            }
        case EnclustraCommand::ERASE_FLASH:
            {
                m_deviceHandle->EndPoints[CONTROL_IN]->TimeOut = 90000;
                break;
            }
        default :
            {
                if (recvBuffer != nullptr)
                {
                    // Straight forward part of the code; this part is used for sending normal commands in which
                    // send and receive framelength should be equal
                    usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
                    checkUsbError (usbRet, blockSizeIn, transferred, m_deviceHandle->EndPoints[CONTROL_IN]);
                    memcpy (recvBuffer->data(), &m_transferBuffer[0], recvBuffer->size());
                }
                break;
            }

    }

    // Read the ACK response back.  It's two 32-bit numbers, padded up to the blockSizeIn.
    transferred = blockSizeIn;
    usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
    checkUsbError (usbRet, blockSizeIn, transferred, m_deviceHandle->EndPoints[CONTROL_IN]);

    uint32_t commandBeingAcked = bufferToHost32 (&m_transferBuffer[0]);
    uint32_t commandStatus = bufferToHost32 (&m_transferBuffer[4]);

    m_deviceHandle->EndPoints[CONTROL_IN]->TimeOut = ENCLUSTRA_TIMEOUT;

    if (commandBeingAcked != command || commandStatus != 0)
    {
        LOG (ERROR) << "BridgeEnclustra ack error: acked==" << commandBeingAcked << ", status==" << commandStatus;
        throw RuntimeError ("BridgeEnclustra: module reports that the command failed");
    }
}

size_t BridgeEnclustraCyApi::getCommandMessageSize ()
{
    const int blockSizeOut = m_deviceHandle->EndPoints[CONTROL_OUT]->MaxPktSize;
    const int blockSizeIn  = m_deviceHandle->EndPoints[CONTROL_IN ]->MaxPktSize;
    // If the two numbers are different, return the min, not the max
    return std::min (blockSizeOut, blockSizeIn);
}

inline void BridgeEnclustraCyApi::flushInputBuffer ()
{
    std::lock_guard<std::mutex> lock (m_commandLock);

    // The protocol requires all packets to be exactly the maximum transfer size
    const int blockSizeIn  = m_deviceHandle->EndPoints[CONTROL_IN ]->MaxPktSize;
    if (blockSizeIn < 0)
    {
        // While there are other things that could cause this, the likely reason is that
        // the device has been unplugged.
        m_isConnected = false;
        throw Disconnected();
    }

    // It would seem sensible to resize to the same size that doCommand uses.  But blockSizeIn and
    // blockSizeOut are AFAIK always the same size anyway.
    m_transferBuffer.resize (blockSizeIn);

    m_deviceHandle->EndPoints[CONTROL_IN]->TimeOut = ENCLUSTRA_FLUSH_TIMEOUT;

    bool usbRet = true;
    for (int i = 0; i < 1000 && usbRet; i++)
    {
        long transferred = blockSizeIn;
        usbRet = m_deviceHandle->EndPoints[CONTROL_IN]->XferData (&m_transferBuffer[0], transferred);
    }

    m_deviceHandle->EndPoints[CONTROL_IN]->TimeOut = ENCLUSTRA_TIMEOUT;

    // The successful exit from that loop is a TIMEOUT.  Getting a successful read 1000
    // times in a row suggests that what we've connected to a stream of data, not the control / ACK
    // channel of the Enclustra.
    if (usbRet)
    {
        LOG (ERROR) << "Device found, but not the expected control protocol";
        throw RuntimeError ("Device found, but not the expected control protocol");
    }
}

bool BridgeEnclustraCyApi::receiveRawFrame (OffsetBasedCapturedBuffer &frame, size_t offset)
{
    long expectedTransfer = static_cast<long> (frame.getUnderlyingBufferSize() - offset);

    long transferred = expectedTransfer;
    bool usbRet = m_deviceHandle->EndPoints[CyEndpoint::DATA_IN]->XferData (frame.getUnderlyingBuffer() + offset, transferred);
    try
    {
        checkUsbError (usbRet, expectedTransfer, transferred, m_deviceHandle->EndPoints[CyEndpoint::DATA_IN]);
    }
    catch (Timeout &)
    {
        return false;
    }

    return true;
}
