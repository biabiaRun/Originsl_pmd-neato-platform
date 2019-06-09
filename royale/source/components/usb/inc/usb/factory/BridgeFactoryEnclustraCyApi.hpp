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

#include <usb/factory/IBridgeFactory.hpp>
#include <usb/factory/IBridgeFactoryMira.hpp>

#include <config/ICoreConfig.hpp>

#include <usb/bridge/BridgeEnclustraCyApi.hpp>

#include <memory>

class CCyUSBDevice;

namespace royale
{
    namespace factory
    {
        class BridgeFactoryEnclustraCyApi : public
            IBridgeFactoryMira,
            IBridgeFactoryImpl<royale::storage::IBridgeWithPagedFlash>
        {
        public:
            explicit BridgeFactoryEnclustraCyApi (std::unique_ptr<CCyUSBDevice> device);

            BridgeFactoryEnclustraCyApi (const BridgeFactoryEnclustraCyApi &) = delete;
            const BridgeFactoryEnclustraCyApi &operator= (const BridgeFactoryEnclustraCyApi &) = delete;
            ~BridgeFactoryEnclustraCyApi() override;

            void initialize () override;

        private:
            void createImpl (std::shared_ptr<royale::hal::IBridgeImager> &) override;
            void createImpl (std::shared_ptr<royale::hal::IBridgeDataReceiver> &) override;
            void createImpl (std::shared_ptr<royale::pal::II2cBusAccess> &) override;
            void createImpl (std::shared_ptr<royale::storage::IBridgeWithPagedFlash> &) override;

        private:
            std::unique_ptr<CCyUSBDevice>                              m_device;
            std::shared_ptr<royale::usb::bridge::BridgeEnclustraCyApi> m_bridge;
        };
    }
}
