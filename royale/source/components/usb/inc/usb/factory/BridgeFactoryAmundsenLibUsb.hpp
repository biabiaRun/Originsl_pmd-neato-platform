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

#include <usb/factory/BridgeFactoryArcticCommon.hpp>

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>

namespace royale
{
    namespace factory
    {
        class BridgeFactoryAmundsenLibUsb : public BridgeFactoryArcticCommon
        {
        public:
            explicit BridgeFactoryAmundsenLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> desc);

            BridgeFactoryAmundsenLibUsb (const BridgeFactoryAmundsenLibUsb &) = delete;
            const BridgeFactoryAmundsenLibUsb &operator= (const BridgeFactoryAmundsenLibUsb &) = delete;
            ~BridgeFactoryAmundsenLibUsb() override;

            void initialize () override;

        private:
            std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> m_desc;
        };
    }
}
