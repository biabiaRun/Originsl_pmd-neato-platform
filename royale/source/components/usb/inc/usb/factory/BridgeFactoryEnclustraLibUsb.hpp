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

#include <usb/factory/IBridgeFactory.hpp>
#include <usb/factory/IBridgeFactoryMira.hpp>

#include <config/ICoreConfig.hpp>

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/bridge/BridgeEnclustraLibUsb.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        class BridgeFactoryEnclustraLibUsb : public
            IBridgeFactoryMira,
            IBridgeFactoryImpl<royale::storage::IBridgeWithPagedFlash>
        {
        public:
            explicit BridgeFactoryEnclustraLibUsb (std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> desc);

            BridgeFactoryEnclustraLibUsb (const BridgeFactoryEnclustraLibUsb &) = delete;
            const BridgeFactoryEnclustraLibUsb &operator= (const BridgeFactoryEnclustraLibUsb &) = delete;
            ~BridgeFactoryEnclustraLibUsb() override;

            void initialize () override;

        private:
            // impl
            void createImpl (std::shared_ptr<royale::hal::IBridgeImager> &) override;
            void createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &) override;
            void createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &) override;
            void createImpl (std::shared_ptr<royale::storage::IBridgeWithPagedFlash> &) override;

            // data
            std::unique_ptr<royale::usb::descriptor::CameraDescriptorLibUsb> m_desc;
            std::shared_ptr<royale::usb::bridge::BridgeEnclustraLibUsb>  m_bridge;
        };
    }
}
