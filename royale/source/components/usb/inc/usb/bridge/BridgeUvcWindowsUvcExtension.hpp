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

#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/bridge/WrappedComPtr.hpp>

#include <vidcap.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * The Windows access to the UVC extension is a property of the UVC driver, and is separate to DirectShow.
             * Both DirectShow and Media Foundation can share the same implementation.
             *
             * Behaviour when the device is disconnected or reset:
             *
             * If the device is reset, the first IO call gets an error, the second one may get an
             * error, but eventually the calls to m_vendorExtension::KsProperty block and never
             * return.  To work around this, the m_vendorExtension is released on the first
             * indication that the hardware is disconnected, and this makes all later IO attempts
             * throw a Disconnected exception.
             *
             * If the device is physically unplugged, it seems that even the first call to
             * m_vendorExtension::KsProperty can block and never return, giving no opportunity to
             * detect the error.
             */
            class BridgeUvcWindowsUvcExtension : public IUvcExtensionAccess
            {
            public:
                ROYALE_API explicit BridgeUvcWindowsUvcExtension (WrappedComPtr<IUnknown> device);
                ~BridgeUvcWindowsUvcExtension();

                void openConnection ();
                void closeConnection ();

                // From IUvcExtensionAccess
                royale::usb::config::UvcExtensionType getVendorExtensionType() const override;
                std::unique_lock<std::recursive_mutex> lockVendorExtension() override;
                bool onlySupportsFixedSize() const override;
                void vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data) override;
                void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data) override;

            private:
                /**
                 * Device to search for the extension during openConnection.
                 */
                WrappedComPtr<IUnknown> m_device;

                /**
                 * Must be held when sending commands via the m_vendorExtension.
                 */
                std::recursive_mutex m_controlLock;

                /**
                 * Ref-counted access to the control channel.
                 */
                WrappedComPtr<IKsControl> m_vendorExtension;
                /** Node to access on the vendor extension control. Only valid if m_vendorExtension != nullptr */
                KSP_NODE m_vendorExtensionNode;
                /** Which protocol the m_vendorExtension supports. */
                royale::usb::config::UvcExtensionType m_vendorExtensionType = royale::usb::config::UvcExtensionType::None;
            };
        }
    }
}
