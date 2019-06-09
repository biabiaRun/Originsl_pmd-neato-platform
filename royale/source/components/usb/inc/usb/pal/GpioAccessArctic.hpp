/***************************************************************************
**                                                                        **
**  Copyright (C) 2016 Infineon Technologies                              **
**  All rights reserved.                                                  **
**                                                                        **
****************************************************************************/

#pragma once

#include <usb/bridge/ArcticProtocolConstants.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>
#include <pal/IGpioAccess.hpp>

namespace royale
{
    namespace usb
    {
        namespace pal
        {
            namespace arctic
            {
                /**
                 * Access to the CX3's and FX3's GPIOs, using Arctic Protocol's
                 * portable identifiers for each GPIO.
                 */
                class GpioAccessArctic : public royale::pal::IGpioAccess
                {
                public:
                    explicit GpioAccessArctic (std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> extension);
                    ~GpioAccessArctic() override = default;

                    void setGpio (uint32_t id, GpioState state) override;

                private:
                    std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> m_extension;
                };
            }
        }
    }
}
