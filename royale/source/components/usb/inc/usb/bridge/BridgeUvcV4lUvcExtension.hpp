/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royalev4l/bridge/BridgeV4l.hpp>

#include <usb/bridge/IUvcExtensionAccess.hpp>

#include <mutex>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Support for accessing the extension unit of USB Video Class devices via the Video For
             * Linux (V4L2) framework.
             *
             * The BridgeV4l implements the IBridgeDataReceiver (for all V4L2 devices, not only UVC
             * ones), and the combination of BridgeUvcV4l and this class handle the UVC specific
             * parts.
             */
            class BridgeUvcV4lUvcExtension : public IUvcExtensionAccess
            {
            public:
                explicit BridgeUvcV4lUvcExtension (const std::shared_ptr<royale::v4l::bridge::BridgeV4l> &bridge);
                ~BridgeUvcV4lUvcExtension () override;

                // From IUvcExtensionAccess
                royale::usb::config::UvcExtensionType getVendorExtensionType() const override;
                std::unique_lock<std::recursive_mutex> lockVendorExtension() override;
                bool onlySupportsFixedSize() const override;
                void vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data) override;
                void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data) override;

            private:
                /**
                 * Must be held when sending commands via the vendor extension.
                 */
                std::recursive_mutex m_controlLock;

                std::shared_ptr<royale::v4l::bridge::BridgeV4l> m_bridge;

                /**
                 * bUnitId to access the vendor extension control.
                 */
                uint8_t m_vendorExtensionUnit;

                /** Which protocol the vendor extension supports. */
                royale::usb::config::UvcExtensionType m_vendorExtensionType;
            };
        }
    }
}
