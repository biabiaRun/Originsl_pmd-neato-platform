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

#include <usb/config/BridgeType.hpp>
#include <factory/IModuleConfigFactory.hpp>

#include <cstdint>
#include <royale/Vector.hpp>

namespace royale
{
    namespace usb
    {
        namespace config
        {
            /**
            * Data for recognising USB-connected devices by the USB vendor/product ids.
            *
            * The IBusEnumerator classes use vid and pid to identify a camera module type
            * while the remaining members (in conjunction with the bridge) allow construction
            * of a suitable CameraCore instance.
            */
            struct UsbProbeData
            {
                uint16_t vid; ///< USB Vendor ID
                uint16_t pid; ///< USB Product ID
                royale::usb::config::BridgeType bridgeType; ///< Bridge type
                std::shared_ptr<royale::factory::IModuleConfigFactory> moduleConfigFactory; ///< Module config
            };

            using UsbProbeDataList = royale::Vector<UsbProbeData>;

            UsbProbeDataList filterUsbProbeDataByBridgeType (const UsbProbeDataList &data, BridgeType bridgeType);
        }
    }
}
