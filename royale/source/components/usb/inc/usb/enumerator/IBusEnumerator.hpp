/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <usb/factory/IBridgeFactory.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <functional>
#include <memory>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            class IBusEnumerator
            {
            public:
                virtual ~IBusEnumerator() = default;

#if defined(TARGET_PLATFORM_ANDROID)
                virtual void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback, uint32_t androidUsbDeviceFD,
                                               uint32_t androidUsbDeviceVid,
                                               uint32_t androidUsbDevicePid) = 0;
#else
                virtual void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback) = 0;
#endif

            };
        }
    }
}
