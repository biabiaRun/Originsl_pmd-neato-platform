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

#include <usb/pal/UsbSpeed.hpp>
#include <buffer/BufferDataFormat.hpp>

namespace royale
{
    namespace usb
    {
        namespace pal
        {
            /**
             * Low-level miscellaneous controls, which are not generic enough to add to
             * IDeviceControl.
             */
            class IDeviceStatus
            {
            public:
                virtual ~IDeviceStatus() = default;

                /**
                 * Please use IBridgeDataReceiver::getPeakTransferSpeed instead of this.
                 *
                 * Returns the connection speed of the device, possibly by asking the device if this
                 * information is not available locally.  This is in bits per second, not the
                 * getPeakTransferSpeed() measure of pixels per second.
                 *
                 * This is a workaround used for implementing
                 * IBridgeDataReceiver::getPeakTransferSpeed on platforms where the speed isn't
                 * available from the API that the IBridgeDataReceiver uses.
                 */
                virtual royale::usb::pal::UsbSpeed getUsbSpeed() = 0;

                /**
                 * Returns the data packing that the device uses to send the image pixels, possibly
                 * by asking the device if this information is not available locally.
                 */
                virtual royale::buffer::BufferDataFormat getUsbTransferFormat() = 0;

                /**
                 * For Arctic devices, returns a string such as "arctic_v0.13.1".
                 */
                virtual royale::String getFirmwareVersion() = 0;
            };
        }
    }
}
