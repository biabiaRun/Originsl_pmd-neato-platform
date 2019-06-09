/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royalev4l/bridge/BridgeV4l.hpp>

#include <usb/pal/UsbSpeed.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Support for Arctic UVC cameras via the Video For Linux framework.
             *
             * With V4L's UVC implementation, calling VIDIOC_STREAMOFF causes a UVC CLEAR_FEATURE
             * request to be sent to the device.  On Windows a CLEAR_FEATURE is sent when Royale
             * disconnects from the UVC stack, and so is interpreted by the firmware (Arctic up to
             * and including v0.17.2) as an unexpected exit from Royale.  The firmware will react by
             * resetting the imager to fail-safe by stopping the illumination.
             *
             * This means that the imager will be reset when we call STREAMOFF, which we need to do
             * to change the buffer allocation.  This class is a workaround, which overrides
             * BridgeV4l::executeUseCase to allocate a fixed number of buffers ignoring the UseCase,
             * and not to reallocate them.
             */
            class BridgeUvcV4l : public royale::v4l::bridge::BridgeV4l
            {
            public:
                /**
                 * The filename is expected to be a /dev/video node.
                 */
                explicit BridgeUvcV4l (const std::string &filename);
                ~BridgeUvcV4l() override;

                std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override;
                void startCapture() override;
                void stopCapture() override;
                void closeConnection() override;
                float getPeakTransferSpeed() override;

                /**
                 * @inheritdoc
                 *
                 * The data from setUvcBridgeInfo is added to the data returned by BridgeV4l's
                 * getBridgeInfo().
                 */
                royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override;

                /**
                 * The data passed to this is returned by getBridgeInfo.
                 *
                 * At the time of writing, it's implementation defined whether calling it a second
                 * time merges the two sets of data or simply overwrites all data from previous
                 * calls.
                 */
                void setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info);

                /**
                 * Sets the value that getPeakTransferSpeed will return. The peak transfer speed
                 * depends on whether the connection is USB High Speed or USB SuperSpeed, but as
                 * this isn't available to the application on all platforms, we instead ask the
                 * firmware via the UVC Extension.
                 *
                 * This does not change the USB connection itself, just the number that
                 * getPeakTransferSpeed returns.
                 */
                void setUsbSpeed (royale::usb::pal::UsbSpeed speed);

            private:
                /**
                 * Zero until the first call to executeUseCase, afterwards the number of buffers
                 * that were allocated.
                 */
                std::size_t m_bufferCount;

                /**
                 * Zero until the first call to executeUseCase, afterwards the imageHeight of the
                 * buffers that were allocated.
                 */
                int m_bufferHeight;

                /**
                 * Flag for startCapture() to pass only the first call to the superclass.
                 */
                bool m_captureStarted;

                /**
                 * Extra information to return from getBridgeInfo.
                 */
                royale::Vector<royale::Pair<royale::String, royale::String>> m_uvcBridgeInfo;

                /**
                 * For calculating the return value of getPeakTransferSpeed.
                 */
                royale::usb::pal::UsbSpeed m_usbSpeed {royale::usb::pal::UsbSpeed::UNKNOWN};
            };
        }
    }
}
