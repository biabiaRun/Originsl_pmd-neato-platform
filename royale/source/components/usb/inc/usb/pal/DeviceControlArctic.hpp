/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <pal/IDeviceControl.hpp>
#include <usb/bridge/ArcticProtocolConstants.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>
#include <usb/pal/IDeviceStatus.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Supports module-level commands for the UVC CX3 and UVC FX3 bridges.
             */
            class DeviceControlArctic : public royale::pal::IDeviceControl,
                public royale::usb::pal::IDeviceStatus
            {
            public:
                ROYALE_API explicit DeviceControlArctic (std::shared_ptr<UvcExtensionArctic> extension);
                ROYALE_API ~DeviceControlArctic();

                // IDeviceControl
                void resetDevice() override;
                void resetDeviceHard() override;

                // IDeviceStatus
                royale::usb::pal::UsbSpeed getUsbSpeed() override;
                royale::buffer::BufferDataFormat getUsbTransferFormat() override;
                royale::String getFirmwareVersion() override;

                void setSpiDisableAndHighZ (bool disable) override;

            private:
                /**
                 * The platform-specific low-level part of the control channel implementation.
                 */
                std::shared_ptr<UvcExtensionArctic> m_extension;

                /**
                 * Common code in the implementation of resetDevice and resetDeviceHard.
                 */
                void resetDeviceCommon (royale::usb::bridge::arctic::DeviceResetType resetType);
            };
        }
    }
}

