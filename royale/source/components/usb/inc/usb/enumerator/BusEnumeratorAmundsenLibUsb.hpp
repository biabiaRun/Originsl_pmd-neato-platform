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

#include <usb/enumerator/BusEnumeratorLibUsb.hpp>

namespace royale
{
    namespace usb
    {
        namespace enumerator
        {
            /**
             * The "Amundsen" protocol is a vendor-variant of UVC (USB Video Class) that doesn't
             * identify itself as UVC so that Win10's Frame Server doesn't modify the data stream.
             *
             * This uses USB's standard vendor/product IDs to find connected devices running this
             * (or more precisely running the Arctic firmware over Amundsen).
             */
            class BusEnumeratorAmundsenLibUsb : public BusEnumeratorLibUsb
            {
            public:
                explicit BusEnumeratorAmundsenLibUsb (const royale::usb::config::UsbProbeDataList &probeData);
                ~BusEnumeratorAmundsenLibUsb() override;

                std::unique_ptr<royale::factory::IBridgeFactory> createBridgeFactory (
                    std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> device) const override;
            };
        }
    }
}
