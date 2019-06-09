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
 
#pragma once

#include <usb/enumerator/IBusEnumerator.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/IBridgeFactory.hpp>

#include <set>

#include <Windows.h>
#include <CyAPI.h>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * This uses CyApi's GUIDs and USB's standard vendor/product IDs to find connected
             * devices running the firmware that is appropriate to the subclass of this abstract
             * class.
             */
            class BusEnumeratorCyApi : public royale::usb::enumerator::IBusEnumerator
            {
            public:
                ROYALE_API explicit BusEnumeratorCyApi (const royale::usb::config::UsbProbeDataList &probeData);
                ROYALE_API ~BusEnumeratorCyApi() override;

                /**
                 * Iterates through all connected devices that the associated Bridge may be able to handle.
                 *
                 * This is implemented to run fast (not spending time opening the devices), while still
                 * finding all available devices. Therefore it will prefer false positives over false
                 * negatives, returning devices that will give an error when openConnection is called.
                 *
                 * \param callback gets called for every device found.
                 *
                 */
                void enumerateDevices (std::function<void (const royale::usb::config::UsbProbeData &,
                                       std::unique_ptr<royale::factory::IBridgeFactory>) > callback) override;

                /**
                 * Returns the bridge-specific factory based on which subclass is being used. For
                 * example, BusEnumeratorEnclustraCyApi creates a BridgeFactoryEnclustraCyApi.
                 */
                virtual std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<CCyUSBDevice> device) const = 0;

            private:
                royale::usb::config::UsbProbeDataList m_probeData;
            };
        }
    }
}
