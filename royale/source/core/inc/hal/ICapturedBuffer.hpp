/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
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
#include <royale/Definitions.hpp>

namespace royale
{
    namespace hal
    {
        /**
         * This contains the data from one capture by the data-receiving hardware.
         *
         * If the hardware is receiving individual frames, there will be a one-to-one ratio between
         * ICapturedBuffer and ICapturedRawFrame instances.  When using superframes, one
         * ICapturedBuffer will be mapped to multiple ICapturedRawFrames.
         *
         * Although this has a virtual destructor, it should only be released by calling
         * IBridgeDataReceiver::queueBuffer().  It is expected to be subclassed by the
         * IBridgeDataReceiver subclasses, to fit with the memory-management of the framework that
         * is used for data capture.
         */
        class ICapturedBuffer
        {
        public:
            virtual ~ICapturedBuffer() = default;

            /**
             * The data received by IBridgeDataReceiver, in normalised 16-bit, native-endian format.
             *
             * The pointer is valid until IBridgeDataReceiver::queueBuffer() is called.
             */
            virtual uint16_t *getPixelData() = 0;

            /**
             * The number of pixels in the array returned by getPixelData().  This can be calculated
             * from the width and height in the UseCaseDefinition, but is included here to avoid
             * buffer overruns when the UseCaseDefinition changes.
             */
            virtual std::size_t getPixelCount() = 0;

            /**
             * If a hardware-generated timestamp was available when the image was captured, the time
             * given as microseconds after the 1970 epoch.  If a timestamp was not available, zero.
             *
             * The Royale Core and Spectre will add a timestamp to the depthframes, calculated from
             * the timestamps from all of the buffers that are used to generate that depth frame.
             */
            virtual uint64_t getTimeMicroseconds() = 0;
        };
    }
}
