/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
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

#include <usb/bridge/BridgeEnclustra.hpp>

#include <memory>
#include <mutex>
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
             * Enclustra is a type of firmware that can be used in the PicoS.  This implements the
             * BridgeEnclustra based on Cypress' CyAPI, which is the recommended method for Windows.
             *
             */
            class BridgeEnclustraCyApi : public BridgeEnclustra
            {
            public:
                explicit BridgeEnclustraCyApi (std::unique_ptr<CCyUSBDevice> device);
                ~BridgeEnclustraCyApi() override;

                void openConnection();
                void closeConnection();
                bool isConnected() const override;
                float getPeakTransferSpeed() override;

            protected:
                // From BridgeEnclustra

                void doCommand (EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer,
                                std::vector<uint8_t> *recvBuffer = nullptr,
                                const std::vector<uint8_t> *sendBuffer = nullptr) override;
                size_t getCommandMessageSize () override;

                bool receiveRawFrame (royale::buffer::OffsetBasedCapturedBuffer &frame, std::size_t offset) override;

            private:
                /**
                 * Handle to the device (opened during the probe, already open when this Bridge is constructed).
                 */
                std::unique_ptr<CCyUSBDevice> m_deviceHandle;

                /**
                 * Between sending an EnclustraCommand and receiving the ACK, a lock to make sure we
                 * don't send another command and mix up the ACK receiving.
                 */
                std::mutex m_commandLock;
                /**
                 * The Enclustra protocol needs USB commands and responses to be a certain size. To
                 * avoid continually reallocating, this buffer is used.  Should only be accessed with
                 * m_commandLock held.
                 */
                std::vector<uint8_t> m_transferBuffer;

                /**
                 * Empty the control channel of previously-sent ACKs and responses.  Used for error
                 * recovery, or to ensure that there's no waiting data after opening the device.
                 */
                void flushInputBuffer();
            };
        }
    }
}
