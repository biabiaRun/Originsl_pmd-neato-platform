/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/bridge/BridgeAmundsenCommon.hpp>

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * The BridgeAmundsenLibUsb supports both UVC and Amundsen firmwares.  In
             * openConnection() it works out which type this device is.
             */
            enum class BridgeAmundsenLibUsbDeviceType
            {
                UNKNOWN,
                UVC,
                AMUNDSEN
            };

            /**
             * Amundsen is a UVC-like but vendor-specific protocol, described in more detail in
             * BridgeAmundsenCommon.
             *
             * The BridgeAmundsenLibUsb can talk to both Amundsen and UVC firmware, but is much
             * simpler than LibUvc because it already knows the details of the connected camera.
             *
             * The Amundsen devices aren't exactly UVC.  The difference is that UVC has two
             * interfaces, so that switching between bulk data and isochronous operation doesn't
             * affect the control interface.  Amundsen doesn't need isochronous operation, and
             * using just a single interface makes the Windows USB support much easier.
             */
            class BridgeAmundsenLibUsb : public BridgeAmundsenCommon, public IUvcExtensionAccess
            {
            public:
                explicit BridgeAmundsenLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> descriptor);
                ~BridgeAmundsenLibUsb() override;

                void openConnection();
                void closeConnection();
                bool isConnected() const override;

                // From IUvcExtensionAccess
                royale::usb::config::UvcExtensionType getVendorExtensionType() const override;
                std::unique_lock<std::recursive_mutex> lockVendorExtension() override;
                bool onlySupportsFixedSize() const override;
                void vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data) override;
                void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data) override;

                // Accessed directly by the BridgeFactory
                /**
                 * Returns UVC if the device is running UVC firmware, AMUNDSEN if it's running Amundsen.
                 *
                 * After openConnection() has returned, this is never expected to return UNKNOWN.
                 */
                BridgeAmundsenLibUsbDeviceType getDeviceType() const;

            protected:
                // From BridgeAmundsenCommon
                PrepareStatus prepareToReceivePayload (uint8_t *first) override;
                std::size_t receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout) override;
                void cancelPendingPayloads() override;

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
                 * The access control for lockVendorExtension().
                 */
                std::recursive_mutex m_controlLock;

                /**
                 * UVC if the device is running UVC firmware, AMUNDSEN if it's running Amundsen.
                 */
                BridgeAmundsenLibUsbDeviceType m_deviceType {BridgeAmundsenLibUsbDeviceType::UNKNOWN};

                /**
                 * True if openConnection() succeeds, until closeConnection() is called.
                 */
                bool m_isConnected {false};

            public:
                struct UsbTransfer
                {
                    /**
                     * LibUSB's own transfer structure. The user_data member will point back to the
                     * UsbTransfer.
                     */
                    libusb_transfer *libusbTransfer;
                    /**
                     * Flag for libusb_handle_events_completed. Set to 0 during preparation of a
                     * transfer, set to 1 in libusb's callback.
                     */
                    int completed;
                    /** The value of first expected to be passed to receivePayload */
                    uint8_t *first;
                };

                /**
                 * For queuing many IO transactions ready to be handled. This is a circular buffer
                 * with the m_countQueued being the number that have been set up in
                 * prepareToReceivePayloads, and m_firstQueued being the index of the one that will
                 * be used by the next call to receivePayload.
                 *
                 * Synchronization: the queue and its m_firstQueued / m_countQueued markers are only
                 * accessed in BridgeAmundsenCommon's acquisition thread.  While I/O is pending, the
                 * contents of each libusb_transfer and the UsbTransfer.completed flag are
                 * controlled by libusb's synchronization, the callback that sets the completed flag
                 * is handled by libusb's synchronization.
                 */
                std::vector<UsbTransfer> m_transferQueue;
                std::size_t m_firstQueued {0};
                std::size_t m_countQueued {0};
            };
        }
    }
}

