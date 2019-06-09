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

#include <hal/ICapturedBuffer.hpp>

#include <cstdint>
#include <cstddef>

namespace royale
{
    namespace buffer
    {
        /**
         * An implementation of the common functionality for an ICapturedBuffer.
         *
         * This class expects the Bridge subclass to handle the buffer allocation and deallocation;
         * for an ICapturedBuffer implementation that handles the buffers itself please see
         * SimpleCapturedBuffer.
         *
         * This is effectively a reinterpret_cast<>() of the data received from the hardware.  If the
         * hardware has already converted the data to 16-bit, it can simply be saved to the memory
         * location returned from getUnderlyingBuffer(). If not, this class can still be used as a
         * buffer, but the bridge must do in-place normalisation before passing the ICapturedBuffer
         * to another class.  The BufferUtils class has helper functions for in-place normalisation.
         */
        class OffsetBasedCapturedBuffer : public royale::hal::ICapturedBuffer
        {
        public:
            /**
             * Constructor.  Pixels include everything that the imager sends over the interface,
             * which will include both the image data and the pseudo data; to the users of this
             * class, all that data is just pixels without an interpretation of which parts are
             * which.
             *
             * \param buffer where the data is stored, must have a equal or longer lifespan than this class
             * \param size number of bytes in the complete array
             * \param pixelOffset where in the underlying data buffer the pixel data starts (offset in bytes, not uint16_t)
             * \param pixelCount number of pixels (not bytes) in the normalised representation.
             * \param timestamp the value to return for getTimeMicroseconds
             */
            OffsetBasedCapturedBuffer (uint8_t *buffer, std::size_t size, std::size_t pixelOffset, std::size_t pixelCount, uint64_t timestamp);

            /**
             * Deleted copy constructor - the OffsetBasedCapturedBuffer doesn't track buffer
             * ownership, so can't safely be copied.  Even for the buffer-ownership tracking
             * SimpleCapturedBuffer, allocating buffers for copies would be slow and it's probably
             * better to catch that at compile time.
             */
            OffsetBasedCapturedBuffer (const OffsetBasedCapturedBuffer &) = delete;
            OffsetBasedCapturedBuffer &operator= (const OffsetBasedCapturedBuffer &) = delete;
            ~OffsetBasedCapturedBuffer() = default;

            /**
             * Capturing from hardware or via an API can require other data to be in the buffer,
             * either a header before the data or padding after the data.  The underlying buffer
             * is the full area for writing data to.
             */
            uint8_t *getUnderlyingBuffer();
            std::size_t getUnderlyingBufferSize();

            // From ICapturedBuffer
            ROYALE_API uint16_t *getPixelData() override;
            ROYALE_API std::size_t getPixelCount() override;
            ROYALE_API uint64_t getTimeMicroseconds() override;

            /**
             * Set the value returned by getTimeMicroseconds.
             */
            void setTimeMicroseconds (uint64_t timestamp);

        private:
            uint8_t *m_data;
            std::size_t m_size;

            std::size_t m_pixelOffset;
            std::size_t m_pixelCount;
            uint64_t m_timestamp;
        };
    }
}
