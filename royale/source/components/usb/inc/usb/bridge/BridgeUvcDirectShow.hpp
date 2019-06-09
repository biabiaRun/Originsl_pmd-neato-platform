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

#include <royale/Definitions.hpp>

#include <usb/bridge/BridgeUvcCommon.hpp>

#include <usb/descriptor/CameraDescriptorDirectShow.hpp>
#include <usb/bridge/WrappedComPtr.hpp>
#include <usb/pal/UsbSpeed.hpp>
#include <common/EventForwarder.hpp>

#include <dshow.h>
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
             * Support for USB Video Class via the Windows DirectShow framework.
             *
             * For this Bridge, the hardware capture starts when the bridge is connected, and stops when
             * it's disconnected.  The startCapture and stopCapture methods just control whether frames
             * are passed to the listener, or dropped within the bridge.
             *
             * The peak transfer speed requires setUsbSpeed to be called.
             */
            class BridgeUvcDirectShow : public BridgeUvcCommon
            {
            public:
                ROYALE_API explicit BridgeUvcDirectShow (std::unique_ptr<royale::usb::descriptor::CameraDescriptorDirectShow> descriptor);
                ~BridgeUvcDirectShow() override;

                void openConnection ();
                void closeConnection ();

                // From BridgeInternalBufferAlloc (IBridgeDataReceiver and IBufferCaptureReleaser)
                float getPeakTransferSpeed () override;
                bool isConnected() const override;

                /**
                 * Returns the object which implements the IKsTopologyInfo interface, used for
                 * access to the vendor extension.
                 */
                WrappedComPtr<IUnknown> getMediaSource();

                /**
                 * Sets the value that getPeakTransferSpeed will return. The peak transfer speed
                 * depends on whether the connection is USB High Speed or USB SuperSpeed, but as
                 * this doesn't seem to be exposed by Windows' UVC API, we ask the firmware via the
                 * UVC Extension.
                 *
                 * This does not change the USB connection itself, just the number that
                 * getPeakTransferSpeed returns.
                 */
                void setUsbSpeed (royale::usb::pal::UsbSpeed speed);

            private:
                /**
                 * Ref-counted UVC device.
                 */
                WrappedComPtr<IMoniker> m_unopenedDevice;

                /**
                 * This is the object that BridgeUvcWindowsUvcExtension accesses.
                 */
                WrappedComPtr<IUnknown> m_topologyInfoForMediaSource;

                /**
                 * Ref-counted control for the DirectShow framework, opened in openConnection.
                 */
                WrappedComPtr<IGraphBuilder> m_graphHandle;

                /**
                 * Access to start or stop the OS-layer capture/acquisition mechanism.
                 */
                WrappedComPtr<IMediaControl> m_mediaControl;

                std::atomic<bool> m_isConnected {false};

                /**
                 * For calculating the return value of getPeakTransferSpeed.
                 */
                royale::usb::pal::UsbSpeed m_usbSpeed {royale::usb::pal::UsbSpeed::UNKNOWN};

                class AcquisitionReceiver;
                friend class AcquisitionReceiver;
            };
        }
    }
}
