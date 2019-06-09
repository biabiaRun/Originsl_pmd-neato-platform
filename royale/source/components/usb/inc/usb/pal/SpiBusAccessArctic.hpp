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

#include <pal/ISpiBusAccess.hpp>
#include <usb/bridge/UvcExtensionArctic.hpp>

#include <memory>

namespace royale
{
    namespace usb
    {
        namespace pal
        {
            namespace arctic
            {
                class SpiBusAccessArctic : public royale::pal::ISpiBusAccess
                {
                public:
                    ROYALE_API explicit SpiBusAccessArctic (std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> extension);
                    ~SpiBusAccessArctic () override = default;

                    std::unique_lock<std::recursive_mutex> selectDevice (uint8_t id) override;
                    ReadSize maximumReadSize () override;
                    std::size_t maximumWriteSize () override;
                    void readSpi (const std::vector<uint8_t> &transmit, std::vector<uint8_t> &receive) override;
                    void writeSpi (const std::vector<uint8_t> &transmit) override;

                    /**
                     * The ISpiBusAccess interface can not provide access to firmware-specific methods.
                     * The flash-accessing methods of the CX3 are accessed directly here.
                     */
                    royale::usb::bridge::UvcExtensionArctic &getExtension();

                private:
                    std::shared_ptr<royale::usb::bridge::UvcExtensionArctic> m_extension;
                };
            }
        }
    }
}
