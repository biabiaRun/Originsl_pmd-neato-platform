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

#include <cstddef>
#include <cstdint>

#include <buffer/BufferDataFormat.hpp>
#include <royale/String.hpp>

namespace royale
{
    namespace usb
    {
        namespace pal
        {
            /** An enum of the speeds specified by the USB standards */
            enum class UsbSpeed
            {
                UNKNOWN,
                LOW,
                FULL,
                HIGH,
                SUPER
            };

            namespace UsbSpeedUtils
            {
#ifdef TARGET_PLATFORM_ANDROID
                const bool CALCULATE_UNKNOWN_AS_HIGH_SPEED = true;
#else
                const bool CALCULATE_UNKNOWN_AS_HIGH_SPEED = false;
#endif
                /**
                 * The value that IBridgeDataReceiver::getPeakTransferSpeed should return.
                 *
                 * If unknownIsHigh is true, then UsbSpeed::UNKNOWN will be treated as
                 * UsbSpeed::HIGH.  Otherwise, calling this with UsbSpeed::UNKNOWN will return the
                 * sentinal value documented in IBridgeDataReceiver::getPeakTransferSpeed.
                 * This is a workaround for ROYAL-2173.
                 *
                 * BufferDataFormat::UNKNOWN will assume that the data is either 12 or 16 bit,
                 * and return the same value as RAW16 would.
                 */
                float calculatePeakTransferSpeed (UsbSpeed speed,
                                                  royale::buffer::BufferDataFormat format,
                                                  bool unknownIsHigh = CALCULATE_UNKNOWN_AS_HIGH_SPEED);

                /**
                 * Converts the speed enumeration to printable strings, should but not just the
                 * name. For example, FULL becomes "full (slow, this is only USB 1.1 speed)".
                 *
                 * There are distinct, different strings for HIGH, (UNKNOWN, false) and (UNKNOWN,
                 * true).
                 */
                royale::String getPrintableName (UsbSpeed speed,
                                                 bool unknownIsHigh = CALCULATE_UNKNOWN_AS_HIGH_SPEED);
            }
        }
    }
}
