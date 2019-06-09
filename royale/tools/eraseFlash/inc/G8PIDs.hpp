/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <factory/ModuleConfigFactoryFixed.hpp>
#include <modules/ModuleConfigData.hpp>
#include <modules/UsbProbeDataListRoyale.hpp>

#include <memory>

namespace royale
{
    namespace config
    {
        royale::usb::config::UsbProbeDataList getUsbProbeDataPlusG8 (bool addG8Maxx, bool addG8Monstar)
        {
            using namespace royale::factory;
            using namespace royale::usb::config;

            auto usbList = royale::config::getUsbProbeDataRoyale();

            // Add G8 VID/PIDs
            if (addG8Maxx)
            {
                usbList.push_back ({ 0x1f46, 0xff15, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff16, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff30, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff31, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff32, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff33, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff34, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff35, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
            }

            if (addG8Monstar)
            {
                usbList.push_back ({ 0x1f46, 0xff13, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff14, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff20, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff21, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff22, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff23, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff24, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff25, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xfff0, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstarDefault) });
                usbList.push_back ({ 0x1f46, 0xff66, BridgeType::UVC, std::make_shared<ModuleConfigFactoryFixed> (moduleconfig::PicoMonstar850nmGlass) });
            }

            return usbList;
        }
    }
}
