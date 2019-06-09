/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/bridge/BridgeAmundsenCommon.hpp>
#include <usb/bridge/BridgeAmundsenLibUsb.hpp>
#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <common/events/EventCaptureStream.hpp>
#include <common/events/EventDeviceDisconnected.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>
#include <common/exceptions/Timeout.hpp>

#include <common/EndianConversion.hpp>
#include <common/IntegerMath.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <limits>

using namespace royale::buffer;
using namespace royale::common;
using namespace royale::usb::descriptor;
using namespace royale::usb::bridge;
using namespace royale::usb::pal;
using std::size_t;

namespace
{
    /**
     * The UVC protocol (which is what Amundsen is) uses a control stream and
     * and a data stream. The controls are over the USB control interface, and
     * the data is received via a bulk endpoint.
     *
     * IN and OUT are from the viewpoint of the application processor or PC.
     */
    enum UsbEndpoint : uint8_t
    {
        /** Unidirectional communication stream containing the image data */
        DATA_IN = 0x83,
    };

    enum UsbInterface : int
    {
        /**
         * For UVC devices Video Control interface; for Amundsen devices the only interface.
         */
        CONTROL = 0,
        /**
         * For Amundsen devices, the equivalent of the UVC Video Streaming interface.
         *
         * For Amundsen devices, both control and data stream are on the same interface (interface
         * zero), this is because Windows will represent multi-interface devices as two separate
         * devices.
         */
        AMUNDSEN_STREAMING = 0,
        /**
         * For non-Amundsen devices, the UVC Streaming interface (it's called that for both bulk and
         * iso data).
         */
        UVC_STREAMING = 1
    };

    UsbInterface streamInterface (BridgeAmundsenLibUsbDeviceType type)
    {
        switch (type)
        {
            case BridgeAmundsenLibUsbDeviceType::UVC:
                return UVC_STREAMING;
            case BridgeAmundsenLibUsbDeviceType::AMUNDSEN:
                return AMUNDSEN_STREAMING;
            default:
                throw LogicError ("");
        }
    }

    auto AMUNDSEN_REQ_TYPE_VIDEO_CONTROL_INTERFACE = uint8_t (0x21);

    /**
     * Amundsen normally uses SET_INTERFACE to start the streaming, and this also works with
     * the CX3 UVC firmware.  The corresponding part of the FX3 UVC firmware is a different
     * codebase, and it does not start streaming when it receives SET_INTERFACE, it requires a
     * UVC VS_COMMIT_CONTROL to be sent.
     *
     * BridgeAmundsenCyApi also sends VS_COMMIT_CONTROL, but for a different reason and with
     * different logic to trigger it.  With some USB chipsets the SET_INTERFACE call fails, when
     * that happens VS_COMMIT_CONTROL is used even with Amundsen firmware. That could be a driver
     * issue, so might not affect the other platforms.
     *
     * In normal UVC this is sent to the video streaming interface, from arctic_v0.16.1 onwards it
     * is also recognised when sent to the control interface (the only Amundsen interface).
     */
    const auto AMUNDSEN_WVALUE_COMMIT_CONTROL = uint16_t (2 << 8);

    /**
     * In normal UVC, the COMMIT_CONTROL command is sent at the end of a negotiation between the
     * host and the device.  As we already know that this is an Amundsen device, we omit the
     * negotiation and just send this configuration.
     *
     * 26 bytes is the length of the UVC 1.0 COMMIT_CONTROL's data. The CX3 firmware will accept any
     * length in the range 1-64, the FX3 firmware requires a length in the range 1-32.
     */
    const auto AMUNDSEN_DATA_COMMIT_CONTROL = std::vector<uint8_t> (26, 0);

    /**
     * The same values for bRequest as UVC's GET_CUR and SET_CUR.
     */
    enum UvcRequest : uint8_t
    {
        AMUNDSEN_REQ_SET_CUR = 0x01,
        AMUNDSEN_REQ_GET_CUR = 0x81
    };

    /**
     * There's no interpretation of the UVC USB descriptor, instead of values of the Arctic
     * firmware are hardcoded.
     */
    const auto AMUNDSEN_VENDOR_EXTENSION_TYPE = royale::usb::config::UvcExtensionType::Arctic;
    const auto AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER = uint16_t (3 << 8 | 0);

    /**
     * libusb's interface requires the data size to be cast to int.
     */
    const auto CAST_AMUNDSEN_STRIDE_SIZE = static_cast<int> (BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE);
    static_assert (BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE == CAST_AMUNDSEN_STRIDE_SIZE,
                   "AMUNDSEN_STRIDE_SIZE is too large for libusb's interface");

    /**
     * Maximum number of async I/O operations to have pending simultaneously.
     *
     * This constant is arbitrary.
     */
    const std::size_t NUMBER_OF_ASYNC_OPS = 30;
}

BridgeAmundsenLibUsb::BridgeAmundsenLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> desc) :
    m_deviceHandle (nullptr),
    m_controlLock(),
    m_deviceType (BridgeAmundsenLibUsbDeviceType::UNKNOWN)
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

bool BridgeAmundsenLibUsb::isConnected() const
{
    return m_isConnected && (m_deviceHandle != nullptr);
}

BridgeAmundsenLibUsbDeviceType BridgeAmundsenLibUsb::getDeviceType() const
{
    return m_deviceType;
}

BridgeAmundsenLibUsb::~BridgeAmundsenLibUsb()
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

void BridgeAmundsenLibUsb::openConnection()
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

    // Claim ownership of the control interface
    if (usberr == 0)
    {
        usberr = libusb_claim_interface (m_deviceHandle, UsbInterface::CONTROL);
        if (usberr == LIBUSB_ERROR_BUSY)
        {
            usberr = libusb_set_auto_detach_kernel_driver (m_deviceHandle, 1);
            if (usberr == 0)
            {
                usberr = libusb_claim_interface (m_deviceHandle, UsbInterface::CONTROL);
            }
            else
            {
                LOG (ERROR) << "Found a matching USB device, but couldn't detach the another driver";
            }
        }
        if (usberr != 0)
        {
            LOG (ERROR) << "Found a matching USB device, but couldn't claim the control interface, error " << std::dec << usberr;
        }

        // Work out whether this is a UVC or an Amundsen device, therefore which interface is the
        // data interface, and claim ownership if it's not the same as the control interface.
        if (usberr == 0)
        {
            usberr = libusb_claim_interface (m_deviceHandle, UsbInterface::UVC_STREAMING);
            if (usberr == LIBUSB_SUCCESS)
            {
                LOG (DEBUG) << "BridgeAmundsen is being used for a UVC device";
                m_deviceType = BridgeAmundsenLibUsbDeviceType::UVC;
            }
            else if (usberr == LIBUSB_ERROR_NOT_FOUND)
            {
                LOG (DEBUG) << "BridgeAmundsen is being used for an Amundsen device";
                m_deviceType = BridgeAmundsenLibUsbDeviceType::AMUNDSEN;
                usberr = LIBUSB_SUCCESS;
            }
            else // (usberr != 0)
            {
                LOG (ERROR) << "Found a matching USB device, but couldn't claim the streaming interface, error " << std::dec << usberr;
            }
        }

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

    // Allocate the data which will be used for asynchronous read requests
    const int NO_ISO_PACKETS = 0;
    m_transferQueue.resize (NUMBER_OF_ASYNC_OPS);
    for (auto &transfer : m_transferQueue)
    {
        // The allocation isn't expected to fail, but if it does then this loop has to finish
        // to ensure that all pointers are either valid or nullptr.
        transfer.libusbTransfer = libusb_alloc_transfer (NO_ISO_PACKETS);
        transfer.first = nullptr;
    }
    for (auto &transfer : m_transferQueue)
    {
        if (!transfer.libusbTransfer)
        {
            throw RuntimeError ("libusb alloc transfer failed");
        }
    }

    // Send a CLEAR_FEATURE (endpoint halt) to put the firmware in a known state
    {
        auto ret = libusb_control_transfer (m_deviceHandle,
                                            LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_ENDPOINT, // bRequestType
                                            LIBUSB_REQUEST_CLEAR_FEATURE, // bRequest
                                            0, // wValue, endpoint halt
                                            0x83, // wIndex, CX3_EP_BULK_VIDEO
                                            nullptr,
                                            0,
                                            AMUNDSEN_TIMEOUT);
        if (ret < 0)
        {
            LOG (WARN) << "CLEAR_FEATURE failed (status " << ret << ")";
            throw RuntimeError ("CLEAR_FEATURE failed (status)");
        }
    }

    // Prepare to send a SET_INTERFACE to start the streaming
    if (m_deviceType == BridgeAmundsenLibUsbDeviceType::AMUNDSEN)
    {
        // First set the interface to alternate setting 1, to keep the same behavior as the Windows
        // version. On Linux the system will happily send a SET_INTERFACE for the
        // currently-selected alternate setting.  On Windows/CyAPI the request is only sent
        // to the device if it would change to a different alternate setting.
        //
        // On both Windows and Linux, the call will fail if the firmware does not have the requested
        // alternate setting for that interface.  For UVC devices, alternate setting 1 is the
        // isochoronous data mode, on Amundsen it's just a dummy mode to ensure that the USB stack
        // sends the next request to set it back to alternate setting zero.
        //
        // But the Arctic UVC firmware didn't support isosynchronous mode, and doesn't have an
        // alternative setting for this interface.  So this call will fail on the UVC firmware.
        auto ret = libusb_set_interface_alt_setting (m_deviceHandle,
                   streamInterface (m_deviceType),
                   1); // wValue, alternate setting 1 (isochronous data)

        if (ret < 0)
        {
            LOG (WARN) << "SET_INTERFACE for iso setting failed (status " << ret << ")";
            throw RuntimeError ("SET_INTERFACE for iso setting failed (status)");
        }
    }

    // Now send the SET_INTERFACE that actually starts the streaming
    {
        auto ret = libusb_set_interface_alt_setting (m_deviceHandle,
                   streamInterface (m_deviceType),
                   0); // wValue, alternate setting 0 (bulk data)

        if (ret < 0)
        {
            LOG (WARN) << "SET_INTERFACE failed (status " << ret << ")";
            throw RuntimeError ("SET_INTERFACE failed (status)");
        }
    }

    // The FX3 UVC firmware images don't start when they receive SET_INTERFACE, they're waiting for
    // COMMIT_CONTROL. This is also sent to CX3 UVC devices, although they do not require it.
    if (m_deviceType == BridgeAmundsenLibUsbDeviceType::UVC)
    {
        auto &data = AMUNDSEN_DATA_COMMIT_CONTROL;
        auto ret = libusb_control_transfer (m_deviceHandle,
                                            LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, // bRequestType
                                            AMUNDSEN_REQ_SET_CUR, // bRequest
                                            AMUNDSEN_WVALUE_COMMIT_CONTROL, // wValue
                                            UVC_STREAMING, // wIndex
                                            const_cast<uint8_t *> (data.data()),
                                            narrow_cast<uint16_t> (data.size()),
                                            AMUNDSEN_TIMEOUT);
        if (ret < 0)
        {
            LOG (WARN) << "Data transfer for COMMIT_CONTROL failed (status " << ret << ")";
            throw RuntimeError ("Data transfer for COMMIT_CONTROL failed (status)");
        }
        else if (static_cast<size_t> (ret) != data.size ())
        {
            throw RuntimeError ("Data transfer for COMMIT_CONTROL was not the expected size");
        }
    }

    m_isConnected = true;
}

void BridgeAmundsenLibUsb::closeConnection()
{
    stopCapture();

    for (auto &transfer : m_transferQueue)
    {
        libusb_free_transfer (transfer.libusbTransfer);
    }
    m_transferQueue.clear();

    if (m_deviceHandle != nullptr)
    {
        // For each call to libusb_release_interface, the libusb library will check that the
        // interface was previously claimed, and return LIBUSB_ERROR_NOT_FOUND if it wasn't.
        // As the library already handles that, we're not concerned about edge cases that are
        // reached by returning early after errors in openConnection.
        if (m_deviceType == BridgeAmundsenLibUsbDeviceType::UVC)
        {
            libusb_release_interface (m_deviceHandle, UsbInterface::UVC_STREAMING);
        }
        libusb_release_interface (m_deviceHandle, UsbInterface::CONTROL);
        libusb_close (m_deviceHandle);
        m_deviceHandle = nullptr;
    }
    m_isConnected = false;
    // The libusb_exit is triggered by the destruction of m_usbContext
}

namespace
{
    /**
     * Called from libusb to set the UsbTransfer.completed flag. Thread-safety is provided by libusb
     * and the completed flag itself, and this function could be called from any thread that's using
     * libusb.
     */
    void LIBUSB_CALL callbackFromLibUsb (struct libusb_transfer *libusbTransfer)
    {
        if (libusbTransfer->status != LIBUSB_TRANSFER_COMPLETED && libusbTransfer->status != LIBUSB_TRANSFER_CANCELLED)
        {
            LOG (DEBUG) << "Callback from libUsb, status = " << libusbTransfer->status;
        }
        auto asyncTransfer = reinterpret_cast<BridgeAmundsenLibUsb::UsbTransfer *> (libusbTransfer->user_data);
        asyncTransfer->completed = 1;
    }
}

BridgeAmundsenLibUsb::PrepareStatus BridgeAmundsenLibUsb::prepareToReceivePayload (uint8_t *first)
{
    if (m_countQueued == m_transferQueue.size())
    {
        return PrepareStatus::ALL_BUFFERS_ALREADY_PREPARED;
    }

    const auto index = circularAdd (m_firstQueued, m_countQueued, m_transferQueue.size());
    auto &asyncTransfer = m_transferQueue.at (index);
    asyncTransfer.completed = 0;
    asyncTransfer.first = first;
    libusb_fill_bulk_transfer (asyncTransfer.libusbTransfer, m_deviceHandle, UsbEndpoint::DATA_IN, first, CAST_AMUNDSEN_STRIDE_SIZE, callbackFromLibUsb, &asyncTransfer, AMUNDSEN_DATA_TIMEOUT);
    auto status = libusb_submit_transfer (asyncTransfer.libusbTransfer);
    if (status == LIBUSB_ERROR_NO_DEVICE)
    {
        throw Disconnected();
    }
    else if (status != LIBUSB_SUCCESS)
    {
        throw RuntimeError ("libusb transfer failed");
    }
    m_countQueued++;
    return PrepareStatus::SUCCESS;
}

void BridgeAmundsenLibUsb::cancelPendingPayloads()
{
#ifndef ROYALE_TARGET_PLATFORM_APPLE
    // The libusb_cancel_transfer call was breaking the use case switch
    // under OSX that's why we removed it for this platform.
    // In this case cancelPendingPayloads will just wait for the pending
    // transfers to complete.
    for (std::size_t i = 0u; i < m_countQueued; i++)
    {
        auto index = circularAdd (m_firstQueued, i, m_transferQueue.size());
        auto &asyncTransfer = m_transferQueue.at (index);
        libusb_cancel_transfer (asyncTransfer.libusbTransfer);
    }
#endif
    for (std::size_t i = 0u; i < m_countQueued; i++)
    {
        auto index = circularAdd (m_firstQueued, i, m_transferQueue.size());
        auto &asyncTransfer = m_transferQueue.at (index);
        while (!asyncTransfer.completed)
        {
            libusb_handle_events_completed (*m_usbContext, &asyncTransfer.completed);
        }
    }
    circularIncrement (m_firstQueued, m_transferQueue.size(), m_countQueued);
    m_countQueued = 0;
}

std::size_t BridgeAmundsenLibUsb::receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout)
{
    if (!m_countQueued)
    {
        throw LogicError ("no prepared buffers");
    }
    auto &asyncTransfer = m_transferQueue.at (m_firstQueued);
    if (first != asyncTransfer.first)
    {
        throw LogicError ("circular buffer pointers mixed up");
    }

    while (!asyncTransfer.completed)
    {
        // This function returns an error code, but expected errors include that the thread was
        // interrupted or timed out.  The correct handling is simply to retry, as the while loop
        // that we're in already does.
        libusb_handle_events_completed (*m_usbContext, &asyncTransfer.completed);
    }
    asyncTransfer.first = nullptr;
    circularIncrement (m_firstQueued, m_transferQueue.size());
    m_countQueued--;

    auto status = asyncTransfer.libusbTransfer->status;
    auto transferred = asyncTransfer.libusbTransfer->actual_length;

    if (status == LIBUSB_TRANSFER_NO_DEVICE || status == LIBUSB_TRANSFER_ERROR)
    {
        LOG (ERROR) << "RawFrame disconnected??? status=" << status << " transferred=" << transferred;
        return 0; //throw Disconnected();
    }
    else if (status == LIBUSB_TRANSFER_TIMED_OUT)
    {
        LOG (DEBUG) << "RawFrame read timeout";
        if (transferred != 0)
        {
            // libusb explicitly says this is intended to happen, the data will be handled as if
            // there wasn't a timeout
            LOG (DEBUG) << "... but transferred " << transferred << " bytes first";
        }
    }
    else if (status == LIBUSB_TRANSFER_OVERFLOW)
    {
        LOG (ERROR) << "RawFrame read overflow";
        // All the data that's been received may be valid, and may start with the last payload of
        // the current frame. However, it may also be corrupt, this will be detected by the caller.
    }
    else if (status != LIBUSB_TRANSFER_COMPLETED)
    {
        LOG (DEBUG) << "Unknown USB IO error, status=" << status;
        throw RuntimeError ("Unknown USB IO error");
    }
    // The amount transferred may be less than the size of the buffer, that is OK.
    return transferred;
}

royale::usb::config::UvcExtensionType BridgeAmundsenLibUsb::getVendorExtensionType() const
{
    return AMUNDSEN_VENDOR_EXTENSION_TYPE;
}

std::unique_lock<std::recursive_mutex> BridgeAmundsenLibUsb::lockVendorExtension()
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    return lock;
}

bool BridgeAmundsenLibUsb::onlySupportsFixedSize() const
{
    return false;
}

void BridgeAmundsenLibUsb::vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data)
{
    auto ret = libusb_control_transfer (m_deviceHandle,
                                        LIBUSB_ENDPOINT_IN | AMUNDSEN_REQ_TYPE_VIDEO_CONTROL_INTERFACE, // bRequestType
                                        AMUNDSEN_REQ_GET_CUR, // bRequest
                                        narrow_cast<uint16_t> (id << 8), // wValue
                                        AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER, // wIndex
                                        data.data(),
                                        narrow_cast<uint16_t> (data.size()),
                                        AMUNDSEN_TIMEOUT);
    if (ret == LIBUSB_ERROR_PIPE)
    {
        // This is a stall
        throw PossiblyUsbStallError();
    }
    else if (ret < 0)
    {
        throw RuntimeError ("Data transfer failed (status)");
    }
    else if (static_cast<size_t> (ret) != data.size ())
    {
        throw RuntimeError ("Data transfer was not the expected size");
    }
}

void BridgeAmundsenLibUsb::vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data)
{
    auto ret = libusb_control_transfer (m_deviceHandle,
                                        LIBUSB_ENDPOINT_OUT | AMUNDSEN_REQ_TYPE_VIDEO_CONTROL_INTERFACE, // bRequestType
                                        AMUNDSEN_REQ_SET_CUR, // bRequest
                                        narrow_cast<uint16_t> (id << 8), // wValue
                                        AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER, // wIndex
                                        const_cast<uint8_t *> (data.data()),
                                        narrow_cast<uint16_t> (data.size()),
                                        AMUNDSEN_TIMEOUT);
    if (ret < 0)
    {
        LOG (WARN) << "Data transfer failed (status " << ret << ")";
        throw RuntimeError ("Data transfer failed (status)");
    }
    else if (static_cast<size_t> (ret) != data.size ())
    {
        throw RuntimeError ("Data transfer was not the expected size");
    }
}
