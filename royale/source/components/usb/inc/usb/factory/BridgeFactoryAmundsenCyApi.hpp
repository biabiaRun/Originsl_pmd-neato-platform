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

#include <usb/factory/BridgeFactoryArcticCommon.hpp>

class CCyUSBDevice;

namespace royale
{
    namespace factory
    {
        class BridgeFactoryAmundsenCyApi : public BridgeFactoryArcticCommon
        {
        public:
            explicit BridgeFactoryAmundsenCyApi (std::unique_ptr<CCyUSBDevice> device);

            BridgeFactoryAmundsenCyApi (const BridgeFactoryAmundsenCyApi &) = delete;
            const BridgeFactoryAmundsenCyApi &operator= (const BridgeFactoryAmundsenCyApi &) = delete;
            ~BridgeFactoryAmundsenCyApi() override;

            void initialize () override;

        private:
            std::unique_ptr<CCyUSBDevice> m_device;
        };
    }
}
