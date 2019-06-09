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

#include <usb/enumerator/IBusEnumerator.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <royale/String.hpp>

#include <set>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * Video For Linux includes an implementation of UVC, but as the name implies is only
             * available on Linux. We normally use LibUvc on all non-Windows platforms, this
             * implementation is for testing edge cases, and may be useful for CSI-2 devices.
             *
             * This BusEnumerator searches for V4L2 devices by using systemd's libudev, and Royale's
             * UsbProbeData
             */
            class BusEnumeratorUvcV4l : public royale::usb::enumerator::IBusEnumerator
            {
            public:
                explicit BusEnumeratorUvcV4l (const royale::usb::config::UsbProbeDataList &probeData);
                ~BusEnumeratorUvcV4l() override;

                /**
                 * Iterate through a list of all connected devices that the associated Bridge may be able to handle.
                 */
                void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback) override;

            private:
                /**
                 * Once enumerateDevices has found a device, this function does further sanity
                 * checks before calling the callback.
                 */
                void probeDevice (std::function<void (const royale::usb::config::UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback, const royale::usb::config::UsbProbeData &pd, const std::string &filename);

                royale::usb::config::UsbProbeDataList m_probeData;
            };
        }
    }
}

