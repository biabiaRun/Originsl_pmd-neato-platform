/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/pal/UsbSpeed.hpp>
#include <limits>

using namespace royale::usb::pal;
using namespace royale::buffer;

float UsbSpeedUtils::calculatePeakTransferSpeed (UsbSpeed speed, BufferDataFormat format,
        bool unknownIsHigh)
{
    const float pixelBits = (format == BufferDataFormat::RAW12) ? 12.f : 16.f;
    if (unknownIsHigh && speed == UsbSpeed::UNKNOWN)
    {
        speed = UsbSpeed::HIGH;
    }

    switch (speed)
    {
        case UsbSpeed::LOW:
            return 1.5e6f / pixelBits;
        case UsbSpeed::FULL:
            return 12e6f / pixelBits;
        case UsbSpeed::HIGH:
            return 480e6f / pixelBits;
        case UsbSpeed::SUPER:
            return 5000e6f / pixelBits;
        case UsbSpeed::UNKNOWN:
        default:
            // The sentinal defined in getPeakTransferSpeed's documentation
            return std::numeric_limits<float>::infinity();
    }
}

royale::String UsbSpeedUtils::getPrintableName (UsbSpeed speed,
        bool unknownIsHigh)
{
    switch (speed)
    {
        case UsbSpeed::LOW:
            return "low";
        case UsbSpeed::FULL:
            return "full (slow, this is only USB 1.1 speed)";
        case UsbSpeed::HIGH:
            return "high (standard USB2 speed)";
        case UsbSpeed::SUPER:
            return "super (standard USB3 speed)";
        case UsbSpeed::UNKNOWN:
        default:
            if (unknownIsHigh)
            {
                return "unknown (assuming high-speed)";
            }
            return "unknown";
    }
}
