/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/bridge/BridgeEnclustraLibUsb.hpp>
#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/EndianConversion.hpp>
#include <common/exceptions/Timeout.hpp>

#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <cstring>

using namespace royale::buffer;
using namespace royale::common;
using namespace royale::usb::descriptor;
using namespace royale::usb::bridge;
using namespace royale::usb::pal;
using std::size_t;

namespace
{
    /**
     * The Enclustra uses three communication streams, the numbers are always the same.
     *
     * IN and OUT are from the viewpoint of the application processor or PC.
     */
    enum UsbEndpoint
    {
        CONTROL_OUT = 0x02,
        CONTROL_IN = 0x82,
        /** Unidirectional communication stream containing the image data */
        DATA_IN = 0x81,
    };
}

BridgeEnclustraLibUsb::BridgeEnclustraLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> desc) :
    m_deviceHandle (nullptr),
    m_commandLock(),
    m_transferBuffer()
{
    if (desc == nullptr)
    {
        throw LogicError ("CameraDescriptor is null");
    }

    m_usbContext = desc->getContext();
    // m_usbContext is a std::shared_ptr, no need to call libusb_ref_device
#if defined(TARGET_PLATFORM_ANDROID)
    // m_androidUsbDeviceFD does not take ownership, it's owned by the system service
    m_androidUsbDeviceFD = desc->getAndroidUsbDeviceFD();
#else
    m_unopenedDevice = desc->getLibUsbDevice();
    libusb_ref_device (m_unopenedDevice);
    // cameraDescriptor is released when the method returns
#endif
}

bool BridgeEnclustraLibUsb::isConnected() const
{
    return m_isConnected && (m_deviceHandle != nullptr);
}

BridgeEnclustraLibUsb::~BridgeEnclustraLibUsb()
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
#ifndef TARGET_PLATFORM_ANDROID
    libusb_unref_device (m_unopenedDevice);
#endif
}
void BridgeEnclustraLibUsb::openConnection()
{
    if (isConnected())
    {
        LOG (ERROR) << "bridge already connected";
        throw LogicError ("bridge already connected");
    }

#if defined(TARGET_PLATFORM_ANDROID)
    int usberr = libusb_wrap_fd (*m_usbContext, m_androidUsbDeviceFD, &m_deviceHandle);
#else
    int usberr = libusb_open (m_unopenedDevice, &m_deviceHandle);
#endif
    if (usberr == 0)
    {
        usberr = libusb_claim_interface (m_deviceHandle, 0);
        if (usberr != 0)
        {
            libusb_close (m_deviceHandle);
            m_deviceHandle = nullptr;
        }
    }

    if (!m_deviceHandle)
    {
        LOG (ERROR) << "Found a matching USB device, but couldn't open it";
        throw CouldNotOpen ("Found a matching USB device, but couldn't open it");
    }

    flushInputBuffer();
    m_isConnected = true;
}

void BridgeEnclustraLibUsb::closeConnection()
{
    stopCapture();

    if (m_deviceHandle != nullptr)
    {
        libusb_close (m_deviceHandle);
        m_deviceHandle = nullptr;
    }
    m_isConnected = false;
    // The libusb_exit is triggered by the destruction of m_usbContext
}

float BridgeEnclustraLibUsb::getPeakTransferSpeed()
{
    auto device = libusb_get_device (m_deviceHandle);
    auto speed = libusb_get_device_speed (device);
    if (speed < 0)
    {
        m_isConnected = false;
        throw Disconnected();
    }

    UsbSpeed usbSpeed;
    switch (speed)
    {
        case LIBUSB_SPEED_LOW:
            usbSpeed = UsbSpeed::LOW;
            break;
        case LIBUSB_SPEED_FULL:
            usbSpeed = UsbSpeed::FULL;
            break;
        case LIBUSB_SPEED_HIGH:
            usbSpeed = UsbSpeed::HIGH;
            break;
        case LIBUSB_SPEED_SUPER:
            usbSpeed = UsbSpeed::SUPER;
            break;
        case LIBUSB_SPEED_UNKNOWN:
// This is a workaround for ROYAL-2173
#ifdef TARGET_PLATFORM_ANDROID
            usbSpeed = UsbSpeed::HIGH;
            break;
#endif
        default:
            usbSpeed = UsbSpeed::UNKNOWN;
            break;
    }

    return UsbSpeedUtils::calculatePeakTransferSpeed (usbSpeed, ENCLUSTRA_TRANSFER_FORMAT);
}

namespace
{
    /**
     * Common error handling - map the libusb return codes to Royale exceptions.
     *
     * This will return normally if the IO completed successfully.
     */
    void checkUsbError (const int usbRet, const size_t expectedTransfer, const size_t transferred)
    {
        if ( (usbRet == LIBUSB_ERROR_NO_DEVICE) || (usbRet == LIBUSB_ERROR_IO && transferred == 0))
        {
            throw Disconnected();
        }
        else if (usbRet == LIBUSB_ERROR_TIMEOUT)
        {
            throw Timeout();
        }
        else if (transferred != expectedTransfer)
        {
            LOG (ERROR) << "BridgeEnclustra USB IO short read/write: usbRet==" << usbRet << ", transferred==" << transferred;
            throw RuntimeError ("USB transfer was not the expected size");
        }
        else if (usbRet != LIBUSB_SUCCESS)
        {
            throw RuntimeError ("Unknown USB IO error");
        }
    }
}

/**
* This runs a command, and waits for the Enclustra to send the ACK back after running the
* command.
*/
void BridgeEnclustraLibUsb::doCommand (const EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer, std::vector<uint8_t> *recvBuffer, const std::vector<uint8_t> *sendBuffer)
{
    std::lock_guard<std::mutex> lock (m_commandLock);

    if (command != bufferToHost32 (&cmdBuffer[0]))
    {
        LOG (ERROR) << "command (" << command << ") != cmdBuffer (" << bufferToHost32 (&cmdBuffer[0]) << ")";
        throw LogicError ("Bug in BridgeEnclustra: command does not match command in cmdBuffer");
    }

    // The protocol requires all packets to be exactly the maximum transfer size
    auto device = libusb_get_device (m_deviceHandle);
    const int blockSizeOut = libusb_get_max_packet_size (device, UsbEndpoint::CONTROL_OUT);
    const int blockSizeIn = libusb_get_max_packet_size (device, UsbEndpoint::CONTROL_IN);
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
    if (cmdBuffer.size () < m_transferBuffer.size ())
    {
        memset (&m_transferBuffer[cmdBuffer.size()], 0, m_transferBuffer.size() - cmdBuffer.size());
    }

    // Send the command here
    int transferred = 0;
    int usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_OUT, &m_transferBuffer[0], blockSizeOut, &transferred, ENCLUSTRA_TIMEOUT);
    checkUsbError (usbRet, blockSizeOut, transferred);

    uint32_t ackTimeout = ENCLUSTRA_TIMEOUT;
    switch (command)
    {
        case EnclustraCommand::READ_FLASH:
            {
                if (recvBuffer != nullptr)
                {
                    // This transfermode shall only apply if we are reading FLASH; otherwise the write's and read's
                    // have to be of the blockSizeOut and blockSizeIn respectively.
                    // With LibUSB we can read in theory read arbitrary sizes of data, but it appears there's a
                    // limit (in Enclustra?) of slightly under 64k, therefore we use the same chunk size as
                    // in the CyAPI bridge.
                    // Also, with Enclustra we have to pad the total to a multiple of blockSizeIn, because the
                    // firmware will pad the flash data to fill the final chunk.
                    if (recvBuffer->size() > m_transferBuffer.size())       // This enhanced logic is only supported for flash reads!
                    {
                        /* Split into chunks of the size of blockSizeOut here and transfer them in a loop */
                        uint32_t chunks = static_cast<uint32_t> (recvBuffer->size() / blockSizeIn);
                        uint32_t lastChunkSize = static_cast<uint32_t> (recvBuffer->size() % blockSizeIn);

                        // loop through the 1024 or 512 byte chunks and read them separately
                        for (size_t i = 0; i < chunks; ++i)
                        {
                            usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                            checkUsbError (usbRet, blockSizeIn, transferred);
                            std::memcpy (recvBuffer->data() + (i * blockSizeIn), &m_transferBuffer[0], transferred);
                        }

                        // Next we read the remaining chunkLength if the whole buffer size was not a product of the
                        // block size.
                        if (lastChunkSize > 0)
                        {
                            usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                            checkUsbError (usbRet, blockSizeIn, transferred);
                            std::memcpy (recvBuffer->data() + (chunks * blockSizeIn), &m_transferBuffer[0], lastChunkSize);
                        }
                    }
                    else
                    {
                        usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                        checkUsbError (usbRet, blockSizeIn, transferred);
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
                        uint32_t chunks = static_cast<uint32_t> (sendBuffer->size() / blockSizeIn);
                        uint32_t lastChunkSize = static_cast<uint32_t> (sendBuffer->size() % blockSizeIn);

                        // loop through the 1024 or 512 byte chunks and write them separately
                        for (size_t i = 0; i < chunks; ++i)
                        {
                            std::memcpy (&m_transferBuffer[0], sendBuffer->data() + (i * blockSizeIn), transferred);
                            usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_OUT, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                            checkUsbError (usbRet, blockSizeIn, transferred);
                        }

                        // Next we write the remaining chunkLength if the whole buffer size was not a product of the
                        // block size.
                        if (lastChunkSize > 0)
                        {
                            std::memcpy (&m_transferBuffer[0], sendBuffer->data() + (chunks * blockSizeIn), lastChunkSize);
                            if (m_transferBuffer.size() - lastChunkSize)
                            {
                                memset (&m_transferBuffer[lastChunkSize], 0xFF, m_transferBuffer.size() - lastChunkSize);
                            }
                            usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_OUT, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                            checkUsbError (usbRet, blockSizeIn, transferred);
                        }
                    }
                    else
                    {
                        memcpy (&m_transferBuffer[0], sendBuffer->data(), sendBuffer->size());
                        if (m_transferBuffer.size() - sendBuffer->size())
                        {
                            memset (&m_transferBuffer[sendBuffer->size()], 0xFF, m_transferBuffer.size() - sendBuffer->size());
                        }
                        usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_OUT, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                        checkUsbError (usbRet, blockSizeIn, transferred);
                    }
                }
                break;
            }
        case EnclustraCommand::ERASE_FLASH:
            {
                ackTimeout = 90000;
                break;
            }
        default:
            {
                if (recvBuffer != nullptr)
                {
                    // Straight forward part of the code; this part is used for sending normal commands in which
                    // send and receive framelength should be equal
                    usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_TIMEOUT);
                    checkUsbError (usbRet, blockSizeIn, transferred);
                    memcpy (recvBuffer->data(), &m_transferBuffer[0], recvBuffer->size());
                }
                break;
            }
    }

    // Read the ACK response back.  It's two 32-bit numbers, padded up to the blockSizeIn.
    usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ackTimeout);
    checkUsbError (usbRet, blockSizeIn, transferred);
    uint32_t commandBeingAcked = bufferToHost32 (&m_transferBuffer[0]);
    uint32_t commandStatus = bufferToHost32 (&m_transferBuffer[4]);
    if (commandBeingAcked != command || commandStatus != 0)
    {
        LOG (ERROR) << "BridgeEnclustra ack error: acked==" << commandBeingAcked << ", status==" << commandStatus;
        throw RuntimeError ("BridgeEnclustra: module reports that the command failed");
    }
}

size_t BridgeEnclustraLibUsb::getCommandMessageSize ()
{
    auto device = libusb_get_device (m_deviceHandle);
    const int blockSizeOut = libusb_get_max_packet_size (device, UsbEndpoint::CONTROL_OUT);
    const int blockSizeIn  = libusb_get_max_packet_size (device, UsbEndpoint::CONTROL_IN);
    if (blockSizeOut < 0 || blockSizeIn < 0)
    {
        // While there are other things that could cause this, the likely reason is that
        // the device has been unplugged.
        m_isConnected = false;
        throw Disconnected();
    }

    // If the two numbers are different, return the min, not the max
    return std::min (blockSizeOut, blockSizeIn);
}

void BridgeEnclustraLibUsb::flushInputBuffer ()
{
    std::lock_guard<std::mutex> lock (m_commandLock);

    // The protocol requires all packets to be exactly the maximum transfer size
    auto device = libusb_get_device (m_deviceHandle);
    const int blockSizeIn  = libusb_get_max_packet_size (device, UsbEndpoint::CONTROL_IN);
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

    int usbRet = LIBUSB_SUCCESS;
    for (int i = 0; i < 1000 && usbRet == LIBUSB_SUCCESS; i++)
    {
        int transferred;
        usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::CONTROL_IN, &m_transferBuffer[0], blockSizeIn, &transferred, ENCLUSTRA_FLUSH_TIMEOUT);
    }
    // The successful exit from that loop is LIBUSB_ERROR_TIMEOUT.  Getting LIBUSB_SUCCESS 1000
    // times in a row suggests that what we've connected to a stream of data, not the control / ACK
    // channel of the Enclustra.
    if (usbRet != LIBUSB_ERROR_TIMEOUT)
    {
        LOG (ERROR) << "Device found, but not the expected control protocol";
        throw RuntimeError ("Device found, but not the expected control protocol");
    }
}

bool BridgeEnclustraLibUsb::receiveRawFrame (OffsetBasedCapturedBuffer &frame, size_t offset)
{
    int transferred;
    int expectedTransfer = static_cast<int> (frame.getUnderlyingBufferSize() - offset);
    int usbRet = libusb_bulk_transfer (m_deviceHandle, UsbEndpoint::DATA_IN, frame.getUnderlyingBuffer() + offset, expectedTransfer, &transferred, ENCLUSTRA_DATA_TIMEOUT);
    if (usbRet == LIBUSB_ERROR_TIMEOUT)
    {
        return false;
    }
    checkUsbError (usbRet, expectedTransfer, transferred);
    return true;
}
