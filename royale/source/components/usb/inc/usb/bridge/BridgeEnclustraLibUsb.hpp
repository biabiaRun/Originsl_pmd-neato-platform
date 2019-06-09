/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <usb/bridge/BridgeEnclustra.hpp>
#include <usb/descriptor/CameraDescriptorLibUsb.hpp>

#include <memory>
#include <mutex>
#include <vector>

#include <libusb.h>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Enclustra is a type of firmware that can be used in the PicoS.  This implements the
             * BridgeEnclustra based on LibUSB.
             *
             * This finds the right USB device to connect to using the standard USB vendor/product pair.
             */
            class BridgeEnclustraLibUsb : public BridgeEnclustra
            {
            public:
                explicit BridgeEnclustraLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> descriptor);
                ~BridgeEnclustraLibUsb() override;

                void openConnection();
                void closeConnection();
                bool isConnected() const override;
                float getPeakTransferSpeed() override;

            protected:
                // From BridgeEnclustra

                void doCommand (EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer,
                                std::vector<uint8_t> *recvBuffer = nullptr,
                                const std::vector<uint8_t> *sendBuffer = nullptr) override;
                size_t getCommandMessageSize () override;

                bool receiveRawFrame (royale::buffer::OffsetBasedCapturedBuffer &frame, std::size_t offset) override;

            private:
                /**
                 * Ref-count for libusb_init()...libusb_exit().
                 */
                std::shared_ptr<libusb_context *> m_usbContext;

#if defined(TARGET_PLATFORM_ANDROID)
                /**
                 * An already-opened device FD that should be passed to libusb.  Neither the
                 * CameraDescriptor nor the Bridge take ownership, ownership is held by the
                 * USB Manager Android system service.
                 */
                uint32_t m_androidUsbDeviceFD;
#else
                /**
                 * The underlying device that m_deviceHandle provides access to. This Bridge holds a
                 * ref-count for it (and continues to hold it as an "unopened" device even while
                 * m_deviceHandle has that device open).
                 */
                libusb_device *m_unopenedDevice;
#endif

                /**
                 * Handle to the USB device, opened in openConnection.
                 */
                libusb_device_handle *m_deviceHandle;

                /**
                 * Between sending an EnclustraCommand and receiving the ACK, a lock to make sure we
                 * don't send another command and mix up the ACK receiving.
                 */
                std::mutex m_commandLock;
                /**
                 * The Enclustra protocol needs USB commands and responses to be a certain size. To
                 * avoid continually reallocating, this buffer is used.  Should only be accessed with
                 * m_commandLock held.
                 */
                std::vector<uint8_t> m_transferBuffer;

                /**
                 * Empty the control channel of previously-sent ACKs and responses.  Used for error
                 * recovery, or to ensure that there's no waiting data after opening the device.
                 */
                void flushInputBuffer();
            };
        }
    }
}
