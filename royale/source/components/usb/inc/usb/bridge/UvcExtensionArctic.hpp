/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <usb/bridge/ArcticProtocolConstants.hpp>
#include <usb/bridge/IUvcExtensionAccess.hpp>

#include <cstdint>
#include <memory>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            struct ArcticVersionNumber
            {
                uint16_t major;
                uint16_t minor;
                uint32_t patch;
            };

            /**
             * Supports the control channel for the Arctic (both UVC and Amundsen) CX3 and FX3 devices.
             */
            class UvcExtensionArctic
            {
            public:
                /**
                 * The constructor immediately talks to the device, the IUvcExtensionAccess must
                 * already be connected before constructing the UvcExtensionArctic.
                 */
                ROYALE_API explicit UvcExtensionArctic (std::shared_ptr<IUvcExtensionAccess> access);
                ROYALE_API ~UvcExtensionArctic();

                /**
                 * Implementation of error checking around a call to IUvcExtensionAccess::vendorExtensionGet.
                 *
                 * The meanings of wValue and wIndex is specific to the type of CX3VendorRequest.
                 */
                void checkedGet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, std::vector<uint8_t> &data);

                /**
                * Implementation of error checking around a combined call to IUvcExtensionAccess::vendorExtensionSet and IUvcExtensionAccess::vendorExtensionGet.
                */
                void checkedSetGet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, const std::vector<uint8_t> &dataSet, std::vector<uint8_t> &dataGet);

                /**
                 * Implementation of error checking around a call to IUvcExtensionAccess::vendorExtensionSet.
                 */
                void checkedSet (royale::usb::bridge::arctic::VendorRequest, uint16_t wValue, uint16_t wIndex);
                void checkedSet (royale::usb::bridge::arctic::VendorRequest, uint16_t wValue, uint16_t wIndex, const std::vector<uint8_t> &data);

                /**
                 * Check if the last command on the firmware has recorded an error, and throw an
                 * exception with the details of that error.
                 *
                 * If assumeError is false, this first checks the status indicator, and will only throw
                 * if an error has occurred. If assumeError is set then it always throws.
                 *
                 * This assumes the caller has already called lockExtension, to ensure that it is
                 * checking the error from its own command.
                 *
                 * \param assumeError true if the caller knows an error has occurred (for example, it's just caught a USB stall)
                 * \throw FirmwareDetectedError if exactly one error occurred
                 * \throw RuntimeException if an unknown condition is detected (including if multiple errors happened)
                 */
                void checkError (bool assumeError);

                /**
                 * Does not call checkError, the caller must hold the lock and call checkError before releasing it.
                 *
                 * This is added for IDeviceControl::resetDevice(), where a Disconnected exception may be expected
                 * during checkError, but would be an unexpected error during the uncheckedSet itself.
                 */
                void uncheckedSet (royale::usb::bridge::arctic::VendorRequest, uint16_t wValue, uint16_t wIndex);

                /**
                 * Returns the lower-level interface, for implementations that read and write directly.
                 *
                 * Callers are responsible for calling checkError(), to ensure that the device isn't left in an error state.
                 */
                IUvcExtensionAccess *getUvcExtensionAccess();

                /**
                 * From firmware version 0.11.3 onwards, the connection speed.  If unsupported by
                 * the firmware, this will return ArcticUsbSpeed::UNKNOWN.
                 */
                royale::usb::bridge::arctic::ArcticUsbSpeed getSpeed() const;

                /**
                 * From firmware version 0.13.1 onwards, the data format.  If unsupported by
                 * the firmware, this will return ArcticUsbTransferFormat::UNKNOWN.
                 */
                royale::usb::bridge::arctic::ArcticUsbTransferFormat getTransferFormat() const;

                /**
                 * Returns data from VendorRequest::VERSION_AND_SUPPORT.
                 */
                ArcticVersionNumber getFirmwareVersion() const;

            private:
                /**
                 * Does not call checkError, the caller must hold the lock and call checkError before releasing it.
                 *
                 * Used internally to share code.
                 */
                void uncheckedGet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, std::vector<uint8_t> &data);

                /**
                 * Does not call checkError, the caller must hold the lock and call checkError before releasing it.
                 *
                 * Used internally to share code.
                 */
                void uncheckedSet (royale::usb::bridge::arctic::VendorRequest req, uint16_t wValue, uint16_t wIndex, const std::vector<uint8_t> &data);

                /**
                 * The platform-specific low-level part of the control channel implementation.
                 */
                std::shared_ptr<IUvcExtensionAccess> m_access;

                /**
                 * Compatibility flag for v0.6 support, makes checkError() ignore the STALL flag.
                 */
                bool m_supportV06{ false };

                /**
                 * Most UVC stacks allow the CONTROL_VARIABLE_SIZE to change size, but the Linux
                 * kernel's stack doesn't.  This flag indicates that CONTROL_VARIABLE_SIZE must not
                 * be used.
                 *
                 * This will increase the protocol overhead and the minimum version of Arctic.
                 */
                bool m_onlyUseFixedSize { false };

                ArcticVersionNumber m_firmwareVersion;

                royale::usb::bridge::arctic::ArcticUsbSpeed m_speed {royale::usb::bridge::arctic::ArcticUsbSpeed::UNKNOWN};
                royale::usb::bridge::arctic::ArcticUsbTransferFormat m_transferFormat {royale::usb::bridge::arctic::ArcticUsbTransferFormat::UNKNOWN};
            };
        }
    }
}
