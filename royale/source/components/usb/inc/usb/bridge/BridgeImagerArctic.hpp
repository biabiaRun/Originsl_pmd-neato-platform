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

#include <hal/IBridgeImager.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Supports the UVC CX3 and UVC FX3 bridges.  Both of these have a
             * common command set, and are accessed via a platform-specific bridge.
             * All I2C commands are tunnelled through the UVC extension.
             */
            class BridgeImagerArctic : public royale::hal::IBridgeImager
            {
            public:
                ROYALE_API explicit BridgeImagerArctic (std::shared_ptr<UvcExtensionArctic> extension);
                ROYALE_API ~BridgeImagerArctic();

                // IBridgeImager
                void setImagerReset (bool state) override;
                void readImagerRegister (uint16_t regAddr, uint16_t &value) override;
                void writeImagerRegister (uint16_t regAddr, uint16_t value) override;
                void readImagerBurst (uint16_t firstRegAddr, std::vector<uint16_t> &values) override;
                void writeImagerBurst (uint16_t firstRegAddr, const std::vector<uint16_t> &values) override;
                void sleepFor (std::chrono::microseconds sleepDuration) override;

            private:
                /**
                 * The platform-specific low-level part of the control channel implementation.
                 */
                std::shared_ptr<UvcExtensionArctic> m_extension;
            };
        }
    }
}
