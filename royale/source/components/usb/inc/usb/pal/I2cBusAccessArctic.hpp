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

#include <usb/bridge/UvcExtensionArctic.hpp>
#include <pal/II2cBusAccess.hpp>

namespace royale
{
    namespace usb
    {
        namespace pal
        {
            namespace arctic
            {
                /**
                 * Access to the CX3's I2C bus, directly using I2C addresses.
                 */
                class I2cBusAccessArctic : public royale::pal::II2cBusAccess
                {
                public:
                    ROYALE_API explicit I2cBusAccessArctic (std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> extension);
                    ROYALE_API ~I2cBusAccessArctic() override = default;

                    void readI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, std::vector<uint8_t> &buffer) override;
                    void writeI2c (uint8_t devAddr, royale::pal::I2cAddressMode addrMode, uint16_t regAddr, const std::vector<uint8_t> &buffer) override;
                    void setBusSpeed (uint32_t bps) override;

                    std::size_t maximumDataSize() override;

                private:
                    std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> m_extension;
                };
            }
        }
    }
}
