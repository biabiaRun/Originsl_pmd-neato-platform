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

#include <device/ProbeResultInfo.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <usb/enumerator/IBusEnumerator.hpp>
#include <usb/factory/IBridgeFactory.hpp>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * DirectShow is a Windows subsystem for video devices.
             *
             * This uses the framework to find connected devices.
             */
            class BusEnumeratorUvcDirectShow : public royale::usb::enumerator::IBusEnumerator
            {
            public:
                /**
                 * Create a UVC bus enumerator.
                 * @param probeData This contains data for recognizing those devices to which Royale
                 * can connect.
                 */
                ROYALE_API explicit BusEnumeratorUvcDirectShow (
                    const royale::usb::config::UsbProbeDataList &probeData);

                ROYALE_API ~BusEnumeratorUvcDirectShow();

                /**
                 * Iterate through a list of all connected devices that the associated Bridge may be able to handle.
                 *
                 * This is implemented to run fast (not spending time opening the devices), while still
                 * finding all available devices. Therefore it will prefer false positives over false
                 * negatives, returning devices that will give an error when openConnection is called.
                 *
                 * @param callback This is the function to be called if a device was matched.
                 */
                ROYALE_API void enumerateDevices (
                    std::function<void (const royale::usb::config::UsbProbeData &,
                                        std::unique_ptr<royale::factory::IBridgeFactory>) >
                    callback) override;

                /**
                 * Iterate through a list of all connected devices that the associated Bridge may be able to handle.
                 *
                 * This is implemented to run fast (not spending time opening the devices), while still
                 * finding all available devices. Therefore it will prefer false positives over false
                 * negatives, returning devices that will give an error when openConnection is called.
                 *
                 * @param callback This is the function to be called if a device was matched.
                 * @param probeResultInfo If this object is not null, it stores information gathered
                 * while this enumerator probes for devices.
                 * @todo Replace IBusEnumerator::enumerateDevices() with this method when the
                 * subclasses of IBusEnumerator have implemented it.
                 */
                ROYALE_API void enumerateDevicesWithInfo (
                    std::function<void (const royale::usb::config::UsbProbeData &,
                                        std::unique_ptr<royale::factory::IBridgeFactory>) >
                    callback,
                    royale::device::ProbeResultInfo *probeResultInfo);

            private:
                royale::usb::config::UsbProbeDataList m_probeData;
            };
        }
    }
}
