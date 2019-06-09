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

#include <usb/bridge/IUvcExtensionAccess.hpp>
#include <usb/bridge/BridgeAmundsenCommon.hpp>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <guiddef.h>
#include <Windows.h>
#include <CyAPI.h>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Amundsen is a UVC-like but vendor-specific protocol, so that it isn't redirected
             * through any OS-provided camera server, for example Win10's Frame Server.
             *
             * The BridgeAmundsenCyApi can only talk to Amundsen firmware, it can't handle UVC.
             *
             * The Amundsen devices aren't exactly UVC.  The difference is that UVC has two
             * interfaces, so that switching between bulk data and isochronous operation doesn't
             * affect the control interface.  Amundsen doesn't need isochronous operation, and
             * using just a single interface makes the Windows USB support much easier.
             *
             * This implements the BridgeAmundsen based on Cypress' CyAPI.
             */
            class BridgeAmundsenCyApi : public BridgeAmundsenCommon, public IUvcExtensionAccess
            {
            public:
                explicit BridgeAmundsenCyApi (std::unique_ptr<CCyUSBDevice> device);
                ~BridgeAmundsenCyApi() override;

                void openConnection();
                void closeConnection();
                bool isConnected() const override;

                // From IUvcExtensionAccess
                royale::usb::config::UvcExtensionType getVendorExtensionType() const override;
                std::unique_lock<std::recursive_mutex> lockVendorExtension() override;
                bool onlySupportsFixedSize() const override;
                void vendorExtensionGet (uint16_t id, std::vector<uint8_t> &data) override;
                void vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data) override;

            protected:
                // From BridgeAmundsenCommon
                PrepareStatus prepareToReceivePayload (uint8_t *first) override;
                std::size_t receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout) override;
                void cancelPendingPayloads() override;

            private:
                /**
                 * Handle to the device (opened during the probe, already open when this Bridge is constructed).
                 */
                std::shared_ptr<CCyUSBDevice> m_deviceHandle;

                /**
                 * The access control for lockVendorExtension().
                 */
                std::recursive_mutex m_controlLock;

                /**
                 * True if openConnection() succeeds, until closeConnection() is called.
                 */
                bool m_isConnected {false};

                struct UsbTransfer
                {
                    OVERLAPPED overlapped;
                    PUCHAR context;
                    /** The value of first expected to be passed to receivePayload */
                    uint8_t *first;
                };

                /**
                 * For queuing many IO transactions ready to be handled. This is a circular buffer
                 * with the m_countQueued being the number that have been set up in
                 * prepareToReceivePayloads, and m_firstQueued being the index of the one that will
                 * be used by the next call to receivePayload.
                 *
                 * Synchronization: the queue and its m_firstQueued / m_countQueued markers are only
                 * accessed in BridgeAmundsenCommon's acquisition thread.
                 */
                std::vector<UsbTransfer> m_transferQueue;
                std::size_t m_firstQueued {0};
                std::size_t m_countQueued {0};
            };
        }
    }
}
