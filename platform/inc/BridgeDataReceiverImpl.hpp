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

#include <hal/IBridgeDataReceiver.hpp>
#include <hal/IBufferCaptureListener.hpp>
#include <mutex>

namespace platform
{
    class BridgeDataReceiverImpl : public royale::hal::IBridgeDataReceiver
    {
    public:
        BridgeDataReceiverImpl();
        ~BridgeDataReceiverImpl();

        void setBufferCaptureListener (royale::hal::IBufferCaptureListener *collector) override;
        std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;
        void startCapture() override;
        void stopCapture() override;
        float getPeakTransferSpeed() override;
        bool isConnected() const override;
        royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;
        void setEventListener (royale::IEventListener *listener) override;

        void queueBuffer (royale::hal::ICapturedBuffer *buffer) override;

    private:
        royale::hal::IBufferCaptureListener *m_bufferCaptureListener;
        std::mutex m_changeListenerLock;
    };
}
