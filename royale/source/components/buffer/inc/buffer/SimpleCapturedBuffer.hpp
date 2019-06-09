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

#include <buffer/OffsetBasedCapturedBuffer.hpp>

#include <cstdint>
#include <cstddef>

namespace royale
{
    namespace buffer
    {
        /**
         * An implementation of ICapturedBuffer that uses a single byte array as the data buffer. This
         * is intended to be used as the byte buffer that all data from the module is read in to by the
         * IBridgeDataReceiver.
         *
         * This is effectively a reinterpret_cast<>() of the data received from the hardware.  If the
         * hardware has already converted the data to 16-bit, it can simply be saved to the memory
         * location returned from getUnderlyingBuffer(). If not, this class can still be used as a
         * buffer, but the bridge must do in-place normalisation before passing the CapturedRawFrame
         * to another class.
         */
        class SimpleCapturedBuffer : public OffsetBasedCapturedBuffer
        {
        public:
            /**
             * Constructor.  Pixels include both the image data and the pseudo data.
             *
             * \param size number of bytes in the complete array
             * \param pixelOffset where in the underlying data buffer the pixel data starts (offset in bytes, not uint16_t)
             * \param pixelCount number of pixels (not bytes) in the normalised representation.
             */
            ROYALE_API SimpleCapturedBuffer (std::size_t size, std::size_t pixelOffset, std::size_t pixelCount);

            SimpleCapturedBuffer (const SimpleCapturedBuffer &) = delete;
            SimpleCapturedBuffer &operator= (const SimpleCapturedBuffer &) = delete;
            ROYALE_API ~SimpleCapturedBuffer();
        };
    }
}
