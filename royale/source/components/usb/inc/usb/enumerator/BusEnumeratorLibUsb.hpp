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

#include <usb/enumerator/IBusEnumerator.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <memory>
#include <set>

#include <libusb.h>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * This uses USB's standard vendor/product IDs to find connected devices running the
             * firmware that is appropriate to the subclass of this abstract class.
             */
            class BusEnumeratorLibUsb : public royale::usb::enumerator::IBusEnumerator
            {
            public:
                explicit BusEnumeratorLibUsb (const royale::usb::config::UsbProbeDataList &probeData);
                ~BusEnumeratorLibUsb() override;

                /**
                 * Iterate through all connected devices that the associated Bridge may be able to handle.
                 *
                 * This is implemented to run fast (not spending time opening the devices), while still
                 * finding all available devices. Therefore it will prefer false positives over false
                 * negatives, returning devices that will give an error when openConnection is called.
                 *
                 */
#if defined(TARGET_PLATFORM_ANDROID)
                void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback, uint32_t androidUsbDeviceFD,
                                       uint32_t androidUsbDeviceVid,
                                       uint32_t androidUsbDevicePid) override;
#else
                void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback) override;
#endif

                /**
                 * Returns the bridge-specific factory based on which subclass is being used. For
                 * example, BusEnumeratorEnclustraLibUsb creates a BridgeFactoryEnclustraLibUsb.
                 */
                virtual std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> device) const = 0;

            private:
                std::shared_ptr<libusb_context *> m_usbContext;
                royale::usb::config::UsbProbeDataList m_probeData;
            };
        }
    }
}
