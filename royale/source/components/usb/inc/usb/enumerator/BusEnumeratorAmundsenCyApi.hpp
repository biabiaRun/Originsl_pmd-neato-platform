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

#include <usb/enumerator/BusEnumeratorCyApi.hpp>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * The "Amundsen" protocol is a vendor-variant of UVC (USB Video Class) that doesn't
             * identify itself as UVC so that operating-system provided UVC drivers don't alter
             * the data stream.
             *
             * This uses CyApi's GUIDs combined with USB's standard vendor/product IDs to find
             * connected devices running Amundsen (or more precisely running the Arctic firmware
             * over Amundsen).
             */
            class BusEnumeratorAmundsenCyApi : public BusEnumeratorCyApi
            {
            public:
                ROYALE_API explicit BusEnumeratorAmundsenCyApi (const royale::usb::config::UsbProbeDataList &probeData);
                ROYALE_API ~BusEnumeratorAmundsenCyApi() override;

                std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<CCyUSBDevice> device) const override;
            };
        }
    }
}
