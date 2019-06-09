/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)

#include <usb/bridge/BridgeAmundsenCommon.hpp>
#include <usb/bridge/BridgeAmundsenCyApi.hpp>
#include <usb/pal/UsbSpeed.hpp>

#include <buffer/BufferUtils.hpp>

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
using namespace royale::usb::bridge;
using namespace royale::usb::pal;
using std::size_t;

namespace
{
    /**
     * The UVC protocol (which is what Amundsen is) uses a control stream and
     * and a data stream.
     * This enum is the indices for the USBDevice::EndPoints array.
     *
     * IN and OUT are from the viewpoint of the application processor or PC.
     */
    enum CyEndpoint : UCHAR
    {
        /** Unidirectional communication stream containing the image data */
        DATA_IN = 0x02,
    };

    /**
     * The same values for bRequest as UVC's GET_CUR and SET_CUR.
     */
    enum UvcRequest : uint8_t
    {
        AMUNDSEN_REQ_SET_CUR = 0x01,
        AMUNDSEN_REQ_GET_CUR = 0x81
    };

    /**
     * Amundsen normally uses SET_INTERFACE to start the streaming. With some USB chipsets the
     * SET_INTERFACE call fails, when this happens an additional SET_CUR request corresponding to
     * UVC's VS_COMMIT_CONTROL is used.  The SET_INTERFACE failing could be a driver issue, so it
     * might not affect other platforms.
     *
     * BridgeAmundsenLibUsb also sends VS_COMMIT_CONTROL, but for a different reason and with
     * different logic to trigger it.  On non-Windows platforms, BridgeAmundsen can also be used
     * with UVC firmware, and some versions of firmware that it needs to support do not start
     * streaming when they receive SET_INTERFACE.
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
     * There's no interpretation of the UVC USB descriptor, instead of values of the Arctic
     * firmware are hardcoded.
     */
    const auto AMUNDSEN_VENDOR_EXTENSION_TYPE = royale::usb::config::UvcExtensionType::Arctic;
    const auto AMUNDSEN_STREAMING_INTERFACE_NUMBER = uint16_t (0);
    const auto AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER = uint16_t (3 << 8 | AMUNDSEN_STREAMING_INTERFACE_NUMBER);

    /**
     * CyAPI's interface requires the data size to be cast to long.
     */
    const auto CAST_AMUNDSEN_STRIDE_SIZE = static_cast<long> (BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE);
    static_assert (BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE == CAST_AMUNDSEN_STRIDE_SIZE,
                   "AMUNDSEN_STRIDE_SIZE is too large for CyAPI's interface");

    /**
     * Maximum number of async I/O operations to have pending simultaneously.
     *
     * This constant is arbitrary.
     */
    const std::size_t NUMBER_OF_ASYNC_OPS = 30;
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
            if (transferred == 0 && usbdStatus == 0 && ntStatus == 0)
            {
                // This can be a stall, and is the error handling path leading to a
                // DeviceDetectedError. The expectedTransfer must be non-zero, because stalls are
                // only detected during read requests (for write requests then happen after the
                // USB transaction has finished).
                throw PossiblyUsbStallError();
            }
            LOG (ERROR) << "BridgeAmundsen USB IO short read/write: usbRet==" << usbRet << ", transferred==" << transferred;
            throw RuntimeError ("USB transfer was not the expected size");
        }
        else if (!usbRet)
        {
            throw RuntimeError ("Unknown USB IO error");
        }
    };

    /**
     * The OVERLAPPED structure is expected to be reset before each use, except for the hEvent
     */
    void clearAllExceptHandle (OVERLAPPED &overlapped)
    {
        overlapped.Internal = 0;
        overlapped.InternalHigh = 0;
        overlapped.Offset = 0;
        overlapped.OffsetHigh = 0;
        overlapped.Pointer = nullptr;
    }
}

BridgeAmundsenCyApi::BridgeAmundsenCyApi (std::unique_ptr<CCyUSBDevice> device) :
    m_deviceHandle (std::move (device))
{
}

BridgeAmundsenCyApi::~BridgeAmundsenCyApi()
{
    if (m_deviceHandle)
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

void BridgeAmundsenCyApi::openConnection()
{
    // CyApi doesn't allow us to find a device without opening it,
    // but also closing it once will mean it needs to be probed to reopen it
    if (!m_deviceHandle)
    {
        throw CouldNotOpen ("CyApi handles can not be opened once closed");
    }

    std::unique_lock<std::recursive_mutex> lock (m_controlLock);

    // Allocate the data which will be used for asynchronous read requests
    m_transferQueue.resize (NUMBER_OF_ASYNC_OPS);
    for (auto &transfer : m_transferQueue)
    {
        // The allocation isn't expected to fail, but if it does then this loop has to finish
        // to ensure that all pointers are either valid or nullptr.
        transfer.overlapped.hEvent = CreateEvent (nullptr, false, false, nullptr);
        clearAllExceptHandle (transfer.overlapped);
        transfer.first = nullptr;
    }
    for (auto &transfer : m_transferQueue)
    {
        if (!transfer.overlapped.hEvent)
        {
            throw RuntimeError ("CreateEvent failed");
        }
    }

    m_deviceHandle->ControlEndPt->TimeOut = AMUNDSEN_TIMEOUT;
    m_deviceHandle->EndPoints[DATA_IN]->TimeOut = AMUNDSEN_DATA_TIMEOUT;

    // Send a CLEAR_FEATURE (endpoint halt) to put the firmware in a known state
    {
        auto ept = m_deviceHandle->ControlEndPt;
        ept->Target =  TGT_ENDPT;
        ept->ReqType = REQ_STD;
        ept->Direction = DIR_TO_DEVICE;
        ept->ReqCode = AMUNDSEN_REQ_SET_CUR;
        ept->Value = 0; // wValue, endpoint halt
        ept->Index = 0x83; // wIndex, CX3_EP_BULK_VIDEO

        std::vector<uint8_t> emptyBuffer;
        auto bufLen = static_cast<LONG> (emptyBuffer.size());
        auto ret = ept->XferData (emptyBuffer.data(), bufLen);
        checkUsbError (ret, 0, bufLen, ept);
    }

    // Send a SET_INTERFACE to start the streaming
    {
        // First set the interface to alternate setting 1.  On Windows/CyAPI the SetAltIntfc request
        // is sent to the device if, and only if, the setAltIntfc() call would change to a different
        // interface.
        //
        // On both Windows and Linux, the call will fail if the firmware does not have the requested
        // alternate setting for that interface.  For UVC devices, alternate setting 1 is the
        // isochoronous data mode, on Amundsen it's just a dummy mode to ensure that the USB stack
        // sends the next request to set it back to alternate setting zero.
        //
        // The command still returns true, even if it wasn't sent by the USB stack.
        auto ret = m_deviceHandle->SetAltIntfc (1);

        // Request alternate setting 0, which is Amundsen's bulk data setting.
        if (ret)
        {
            ret = m_deviceHandle->SetAltIntfc (0);
        }

        // On some USB host controllers, the SetAltIntfc calls above fail. Use the other way to
        // start streaming, which is supported on arctic_v0.16.1 and later.
        if (!ret)
        {
            LOG (WARN) << "SET_INTERFACE failed, falling back to COMMIT_CONTROL";
            auto ept = m_deviceHandle->ControlEndPt;
            ept->Target = TGT_INTFC;
            ept->ReqType = REQ_CLASS;
            ept->Direction = DIR_TO_DEVICE;
            ept->ReqCode = AMUNDSEN_REQ_SET_CUR;
            ept->Value = AMUNDSEN_WVALUE_COMMIT_CONTROL;
            ept->Index = AMUNDSEN_STREAMING_INTERFACE_NUMBER;

            auto &data = AMUNDSEN_DATA_COMMIT_CONTROL;
            auto bufLen = static_cast<LONG> (data.size());
            ret = ept->XferData (const_cast<uint8_t *> (data.data()), bufLen);

            // On arctic_v0.15.x and before, the firmware will send back a stall, because it's an
            // unrecognised command. Handle that with a specific error message, before calling
            // checkUsbError (which will throw with a generic error message).
            //
            // Unlike most of the write commands, this one can detect a stall, because it happens
            // before the data phase of the transaction.
            if (!ret)
            {
                throw RuntimeError ("SET_INTERFACE failed, and fallback to COMMIT_CONTROL also failed");
            }
            checkUsbError (ret, data.size(), bufLen, ept);
        }
    }

    m_isConnected = true;
}

void BridgeAmundsenCyApi::closeConnection()
{
    stopCapture();

    for (auto &transfer : m_transferQueue)
    {
        CloseHandle (transfer.overlapped.hEvent);
    }
    m_transferQueue.clear();

    if (m_deviceHandle)
    {
        m_deviceHandle->Close();
        m_deviceHandle.reset();
    }

    m_isConnected = false;
}

bool BridgeAmundsenCyApi::isConnected() const
{
    return m_isConnected && (m_deviceHandle != nullptr);
}

BridgeAmundsenCommon::PrepareStatus BridgeAmundsenCyApi::prepareToReceivePayload (uint8_t *first)
{
    if (m_countQueued == m_transferQueue.size())
    {
        return PrepareStatus::ALL_BUFFERS_ALREADY_PREPARED;
    }

    auto ept = m_deviceHandle->EndPoints[CyEndpoint::DATA_IN];

    const auto index = circularAdd (m_firstQueued, m_countQueued, m_transferQueue.size());
    auto &asyncTransfer = m_transferQueue.at (index);
    clearAllExceptHandle (asyncTransfer.overlapped);
    asyncTransfer.first = first;
    asyncTransfer.context = ept->BeginDataXfer (asyncTransfer.first, CAST_AMUNDSEN_STRIDE_SIZE, &asyncTransfer.overlapped);
    m_countQueued++;
    return PrepareStatus::SUCCESS;
}

void BridgeAmundsenCyApi::cancelPendingPayloads()
{
    auto ept = m_deviceHandle->EndPoints[CyEndpoint::DATA_IN];
    ept->Abort();

    while (m_countQueued)
    {
        long transferred = CAST_AMUNDSEN_STRIDE_SIZE;
        auto &asyncTransfer = m_transferQueue.at (m_firstQueued);
        ept->WaitForXfer (&asyncTransfer.overlapped, AMUNDSEN_DATA_TIMEOUT);
        ept->FinishDataXfer (asyncTransfer.first, transferred, &asyncTransfer.overlapped, asyncTransfer.context);
        circularIncrement (m_firstQueued, m_transferQueue.size());
        m_countQueued--;
    }
}

std::size_t BridgeAmundsenCyApi::receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout)
{
    long transferred = CAST_AMUNDSEN_STRIDE_SIZE;
    auto ept = m_deviceHandle->EndPoints[CyEndpoint::DATA_IN];

    if (!m_countQueued)
    {
        throw LogicError ("no prepared buffers");
    }
    auto &asyncTransfer = m_transferQueue.at (m_firstQueued);
    if (first != asyncTransfer.first)
    {
        throw LogicError ("circular buffer pointers mixed up");
    }
    while (!ept->WaitForXfer (&asyncTransfer.overlapped, AMUNDSEN_DATA_TIMEOUT))
    {
        if (!retryOnTimeout)
        {
            ept->Abort();
            break;
        }
    }
    auto usbRet = ept->FinishDataXfer (first, transferred, &asyncTransfer.overlapped, asyncTransfer.context);
    asyncTransfer.first = nullptr;
    circularIncrement (m_firstQueued, m_transferQueue.size());
    m_countQueued--;

    // Errors here are a subset of the ones in checkUsbError()
    ULONG usbdStatus = ept->UsbdStatus;
    ULONG ntStatus = ept->NtStatus;
    if (usbdStatus == 0xC0000011 && (ntStatus == 0xC0000001 || ntStatus == 0xC000009D))
    {
        // generic UNSUCCESSFUL (0xC0000001) or DEVICE_NOT_CONNECTED (0xC000009D)
        throw Disconnected ();
    }
    else if (usbdStatus == 0xC0010000 && ntStatus == 0xc0000120)
    {
        LOG (DEBUG) << "receivePayload read timeout";
        return 0;
    }
    else if (usbdStatus != 0)
    {
        std::vector<char> s;
        s.resize (USB_STRING_MAXLEN);
        m_deviceHandle->UsbdStatusString (usbdStatus, s.data());
        LOG (WARN) << "receivePayload error " << std::string (s.data());
    }
    else if (!usbRet)
    {
        // \todo, what is the error that's getting here when using the async functions? throw RuntimeError ("Unknown USB IO error");
        return 0;
    }
    // transferred can be less than AMUNDSEN_STRIDE_SIZE, and it's expected to be less that it for
    // every payload, with the final payload being smaller than the others.

    return transferred;
}

royale::usb::config::UvcExtensionType BridgeAmundsenCyApi::getVendorExtensionType() const
{
    return AMUNDSEN_VENDOR_EXTENSION_TYPE;
}

std::unique_lock<std::recursive_mutex> BridgeAmundsenCyApi::lockVendorExtension()
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    return lock;
}

bool BridgeAmundsenCyApi::onlySupportsFixedSize() const
{
    return false;
}

void BridgeAmundsenCyApi::vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data)
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    auto ept = m_deviceHandle->ControlEndPt;
    ept->Target = TGT_INTFC;
    ept->ReqType = REQ_CLASS;
    ept->Direction = DIR_FROM_DEVICE;
    ept->ReqCode = AMUNDSEN_REQ_GET_CUR;
    ept->Value = narrow_cast<uint16_t> (id << 8);
    ept->Index = AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER;

    ZeroMemory (data.data(), data.size());
    auto bufLen = static_cast<LONG> (data.size());
    auto ret = ept->XferData (data.data(), bufLen);
    checkUsbError (ret, data.size(), bufLen, ept);
}

void BridgeAmundsenCyApi::vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data)
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    auto ept = m_deviceHandle->ControlEndPt;
    ept->Target = TGT_INTFC;
    ept->ReqType = REQ_CLASS;
    ept->Direction = DIR_TO_DEVICE;
    ept->ReqCode = AMUNDSEN_REQ_SET_CUR;
    ept->Value = narrow_cast<uint16_t> (id << 8);
    ept->Index = AMUNDSEN_VENDOR_UNIT_ID_AND_INTERFACE_NUMBER;

    auto bufLen = static_cast<LONG> (data.size());
    auto ret = ept->XferData (const_cast<uint8_t *> (data.data()), bufLen);
    checkUsbError (ret, data.size(), bufLen, ept);
}
