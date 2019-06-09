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

#include <usb/factory/BridgeFactoryArcticCommon.hpp>

#include <usb/descriptor/CameraDescriptorDirectShow.hpp>

namespace royale
{
    namespace factory
    {
        class BridgeFactoryUvcDirectShow : public BridgeFactoryArcticCommon
        {
        public:
            explicit BridgeFactoryUvcDirectShow (std::unique_ptr<royale::usb::descriptor::CameraDescriptorDirectShow> desc);

            BridgeFactoryUvcDirectShow (const BridgeFactoryUvcDirectShow &) = delete;
            const BridgeFactoryUvcDirectShow &operator= (const BridgeFactoryUvcDirectShow &) = delete;
            ~BridgeFactoryUvcDirectShow() override;

            void initialize () override;

        private:
            std::unique_ptr<royale::usb::descriptor::CameraDescriptorDirectShow> m_desc;
        };
    }
}
