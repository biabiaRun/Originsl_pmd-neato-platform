/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
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
             * Enclustra is a type of firmware that can be used in the Pico device family.
             *
             * This uses CyApi's GUIDs combined with USB's standard vendor/product IDs to find
             * connected devices running Enclustra.
             */
            class BusEnumeratorEnclustraCyApi : public BusEnumeratorCyApi
            {
            public:
                ROYALE_API explicit BusEnumeratorEnclustraCyApi (const royale::usb::config::UsbProbeDataList &probeData);
                ROYALE_API ~BusEnumeratorEnclustraCyApi() override;

                std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<CCyUSBDevice> device) const override;
            };
        }
    }
}
