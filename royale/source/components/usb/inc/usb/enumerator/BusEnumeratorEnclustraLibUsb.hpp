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

#include <usb/enumerator/BusEnumeratorLibUsb.hpp>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * Enclustra is a type of firmware that can be used in the Pico device family.
             *
             * This uses USB's standard vendor/product IDs to find connected devices running Enclustra.
             */
            class BusEnumeratorEnclustraLibUsb : public BusEnumeratorLibUsb
            {
            public:
                explicit BusEnumeratorEnclustraLibUsb (const royale::usb::config::UsbProbeDataList &probeData);
                ~BusEnumeratorEnclustraLibUsb() override;

                std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> device) const override;
            };
        }
    }
}
