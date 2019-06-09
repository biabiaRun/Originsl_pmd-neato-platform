/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/ArcticProtocolConstants.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/RuntimeError.hpp>

#include <common/exceptions/DeviceDetectedError.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <algorithm>
#include <cstdint>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::bridge::arctic;

namespace
{
    void checkMaximumSize (const std::size_t size)
    {
        if (size > arctic::MAXIMUM_DATA_SIZE)
        {
            throw LogicError ("Buffer too large for Arctic protocol");
        }
    }

    struct SmallestControl
    {
        VendorControlId id;
        uint16_t size;
    };

    SmallestControl calculateSmallestControl (const std::size_t size, bool onlyUseFixedSize)
    {
        checkMaximumSize (size);
        if (!onlyUseFixedSize)
        {
            return {VendorControlId::CONTROL_VARIABLE_SIZE, static_cast<uint16_t> (size) };
        }
        if (size <= 64)
        {
            return {VendorControlId::CONTROL_64, 64};
        }
        if (size <= 512)
        {
            return {VendorControlId::CONTROL_512, 512};
        }
        return {VendorControlId::CONTROL_4096, 4096};
    };
    static_assert (arctic::MAXIMUM_DATA_SIZE == 4096u, "Assumptions for calculateSmallestControl() are wrong");

    std::vector<uint8_t> createSetup (VendorReqType type, VendorRequest request, uint16_t a, uint16_t b, size_t dataSize)
    {
        std::vector<uint8_t> setup;
        setup.reserve (8);
        setup.push_back (type);
        setup.push_back (request);
        pushBackBe16 (setup, a);
        pushBackBe16 (setup, b);
        pushBackBe16 (setup, narrow_cast<uint16_t> (dataSize));

        return setup;
    }

    std::vector<uint8_t> createReadSetup (VendorRequest request, uint16_t a, uint16_t b, size_t dataSize)
    {
        return createSetup (VendorReqType::READ, request, a, b, dataSize);
    }

    std::vector<uint8_t> createWriteSetup (VendorRequest request, uint16_t a, uint16_t b, size_t dataSize)
    {
        return createSetup (VendorReqType::WRITE, request, a, b, dataSize);
    }
}

UvcExtensionArctic::UvcExtensionArctic (std::shared_ptr<IUvcExtensionAccess> access) :
    m_access {access},
    m_onlyUseFixedSize {access->onlySupportsFixedSize() }
{
    // Read the version number.  This a a 64-byte read, because it's read with the smallest
    // fixed-size read (64 bytes).
    std::vector<uint8_t> result;
    result.resize (64); // we read with a CONTROL_64, and the decoding of the data is hardcoded in this function


    auto lock = m_access->lockVendorExtension();
    try
    {
        const std::vector<uint8_t> &setup = createReadSetup (VendorRequest::VERSION_AND_SUPPORT, 0, 0, result.size());
        m_access->vendorExtensionSet (VendorControlId::CONTROL_SETUP, setup);
        m_access->vendorExtensionGet (VendorControlId::CONTROL_64, result);
    }
    catch (const PossiblyUsbStallError &)
    {
        checkError (true);
        throw;
    }

    m_firmwareVersion.major = bufferToHostBe16 (&result[VersionAndSupport::MAJOR]);
    m_firmwareVersion.minor = bufferToHostBe16 (&result[VersionAndSupport::MINOR]);
    m_firmwareVersion.patch = bufferToHostBe32 (&result[VersionAndSupport::PATCH]);

    if (result[VersionAndSupport::FLAGS] & SupportFlags::BOOT_INCOMPLETE)
    {
        // This does not throw, because Royale may be being used to upgrade the firmware. It's only
        // expected with development firmware, and was added in v0.14.1.
        LOG (ERROR) << "The camera's BOOT_INCOMPLETE flag is set, the camera is not expected to work";
    }

    // Although there is a SupportFlags::VARIABLE_LENGTH, for fixed-length this needs to check the
    // version number to check that the fix for MFIRM-65 is present.
    if (m_onlyUseFixedSize && m_firmwareVersion.major == 0 && m_firmwareVersion.minor < 14)
    {
        throw CouldNotOpen ("To use this bridge, an Arctic firmware version that supports fixed-size controls is required, please reflash the device with v0.14.1 or later");
    }

    if (m_firmwareVersion.major == 0 && m_firmwareVersion.minor < 6)
    {
        throw CouldNotOpen ("Unsupported (old) firmware, please reflash the device");
    }
    else if (m_firmwareVersion.major == 0 && m_firmwareVersion.minor == 6)
    {
        m_supportV06 = true;
    }

    // ArcticUsbSpeed was added in v0.11.3, and ArcticUsbTransferFormat in v0.13.1
    //
    // In older firmware, these fields were reserved and always zero, which static_casts to
    // ArcticUsbSpeed::UNKNOWN and ArcticUsbTransferFormat::UNKNOWN.
    m_speed = static_cast<ArcticUsbSpeed> (result[VersionAndSupport::USB_SPEED]);
    m_transferFormat = static_cast<ArcticUsbTransferFormat> (result[VersionAndSupport::TRANSFER_FORMAT]);
}

UvcExtensionArctic::~UvcExtensionArctic ()
{
}

IUvcExtensionAccess *UvcExtensionArctic::getUvcExtensionAccess()
{
    return m_access.get();
}

void UvcExtensionArctic::uncheckedGet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, std::vector<uint8_t> &data)
{
    auto control = calculateSmallestControl (data.size(), m_onlyUseFixedSize);

    const std::vector<uint8_t> &setup = createReadSetup (req, wValue, wIndex, data.size());
    if (control.size == data.size())
    {
        m_access->vendorExtensionSet (VendorControlId::CONTROL_SETUP, setup);
        m_access->vendorExtensionGet (control.id, data);
    }
    else
    {
        auto expandedBuffer = std::vector<uint8_t> (control.size);
        m_access->vendorExtensionSet (VendorControlId::CONTROL_SETUP, setup);
        m_access->vendorExtensionGet (control.id, expandedBuffer);
        std::copy_n (expandedBuffer.cbegin(), data.size(), data.begin());
    }
}

void UvcExtensionArctic::checkedGet (VendorRequest req, uint16_t a, uint16_t b, std::vector<uint8_t> &data)
{
    checkMaximumSize (data.size());

    auto lock = m_access->lockVendorExtension();
    try
    {
        uncheckedGet (req, a, b, data);
    }
    catch (const PossiblyUsbStallError &)
    {
        checkError (true);
    }
}

void UvcExtensionArctic::checkedSetGet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, const std::vector<uint8_t> &dataSet, std::vector<uint8_t> &dataGet)
{
    checkMaximumSize (dataGet.size());
    checkMaximumSize (dataSet.size());

    auto lock = m_access->lockVendorExtension();
    uncheckedSet (req, wValue, wIndex, dataSet);
    checkError (false);
    try
    {
        uncheckedGet (req, wValue, wIndex, dataGet);
    }
    catch (const PossiblyUsbStallError &)
    {
        checkError (true);
    }
}

void UvcExtensionArctic::uncheckedSet (VendorRequest req, uint16_t a, uint16_t b)
{
    const std::vector<uint8_t> &setup = createWriteSetup (req, a, b, 0);
    m_access->vendorExtensionSet (VendorControlId::CONTROL_SETUP, setup);
}

void UvcExtensionArctic::uncheckedSet (VendorRequest req, uint16_t a, uint16_t b, const std::vector<uint8_t> &data)
{
    auto control = calculateSmallestControl (data.size(), m_onlyUseFixedSize);

    const std::vector<uint8_t> &setup = createWriteSetup (req, a, b, data.size());

    m_access->vendorExtensionSet (VendorControlId::CONTROL_SETUP, setup);
    if (control.size == data.size())
    {
        m_access->vendorExtensionSet (control.id, data);
    }
    else
    {
        std::vector<uint8_t> expandedBuffer {data};
        expandedBuffer.resize (control.size);
        m_access->vendorExtensionSet (control.id, expandedBuffer);
    }
}


void UvcExtensionArctic::checkedSet (VendorRequest req, uint16_t a, uint16_t b)
{
    auto lock = m_access->lockVendorExtension();
    uncheckedSet (req, a, b);
    checkError (false);
}

void UvcExtensionArctic::checkedSet (VendorRequest req, uint16_t a, uint16_t b, const std::vector<uint8_t> &data)
{
    checkMaximumSize (data.size());
    auto lock = m_access->lockVendorExtension();
    uncheckedSet (req, a, b, data);
    checkError (false);
}

void UvcExtensionArctic::checkError (bool assumeError)
{
    std::vector<uint8_t> recvBuffer (8, 0);

    // With v0.6, there is no STALL_INDICATOR flag, and we have to read the ERROR_COUNTER instead.
    if (! (assumeError || m_supportV06))
    {
        try
        {
            m_access->vendorExtensionGet (VendorControlId::CONTROL_SETUP, recvBuffer);
        }
        catch (const PossiblyUsbStallError &)
        {
            // \todo ROYAL-2990 This particular read is failing intermittently while loading the
            // imager firmware. This is a temporary workaround.
            LOG (WARN) << "Stalled during UvcExtensionArctic::checkError, retrying";
            try
            {
                m_access->vendorExtensionGet (VendorControlId::CONTROL_SETUP, recvBuffer);
            }
            catch (const PossiblyUsbStallError &)
            {
                LOG (WARN) << "Double-stalled during UvcExtensionArctic::checkError, retrying a final time";
                m_access->vendorExtensionGet (VendorControlId::CONTROL_SETUP, recvBuffer);
            }
        }
        auto stallIndicator = bool (recvBuffer[0] & STALL_INDICATOR);
        if (! stallIndicator)
        {
            return;
        }
    }

    // recvBuffer will receive the 3 uint32_t items shown in the bufferToHost lines below
    recvBuffer.clear();
    recvBuffer.resize (3 * sizeof (uint32_t), 0);

    uncheckedGet (VendorRequest::ERROR_DETAILS, 0, 0, recvBuffer);

    auto statusEnum = bufferToHostBe32 (&recvBuffer[0]);

    // if needed, clear the status
    if (m_supportV06 && (statusEnum != 0))
    {
        uncheckedSet (VendorRequest::ERROR_DETAILS, 0, 0);
    }

    switch (statusEnum)
    {
        case 0:
            // no details of an error, but the stallIndicator or assumeError was set
            if (m_supportV06 && ! assumeError)
            {
                // this means no error
                return;
            }
            throw RuntimeError ("UVC error, but no details available, maybe a USB error");
        case 1:
            {
                auto firstError = bufferToHostBe32 (&recvBuffer[4]);
                auto secondary = bufferToHostBe32 (&recvBuffer[8]);
                throw DeviceDetectedError ("Firmware detected error", firstError, secondary);
            }
        default:
            throw RuntimeError ("Unknown error status detected in device");
    }
}

ArcticUsbSpeed UvcExtensionArctic::getSpeed () const
{
    return m_speed;
}

ArcticUsbTransferFormat UvcExtensionArctic::getTransferFormat () const
{
    return m_transferFormat;
}

ArcticVersionNumber UvcExtensionArctic::getFirmwareVersion() const
{
    return m_firmwareVersion;
}
