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

namespace royale
{
    namespace buffer
    {
        /**
         * This enum is the list of formats that BufferUtils can convert to the normalized format
         * that Royale Core and Processing expect to be in an ICapturedBuffer.
         *
         * These are formats that are used by the current IBridgeDataReceiver implementations.  For
         * example, the data may be packed 12 bits per pixel as it's transmitted over a USB cable.
         */
        enum class BufferDataFormat
        {
            /**
             * Some bridges can receive both 12-bit and 16-bit data, this is a placeholder until the
             * format can be determined by a method known to the Bridge (for example auto-detected).
             */
            UNKNOWN = 0,
            /**
             * The data format that's unpacked by BufferUtils::normalizeRaw12.
             * This has 2 pixels packed in to 3 bytes.
             */
            RAW12,
            /**
             * The data format that's unpacked by BufferUtils::normalizeRaw16. The data is in the
             * low 12 bits, and on big-endian devices it also needs to be byte-swapped.
             *
             * The high nibble may be known to always be zero, or it may have flags in it.
             * Currently these are treated as a single type and normalizeRaw16 always clears the
             * high nibble, even if it could otherwise be a no-op.
             */
            RAW16,
            /**
             * The data format that's unpacked by BufferUtils::normalizeS32V234.
             * The CSI controller on the S32V234 SoC will write RAW12 data in a way
             * where each pixel is on 16 bits with 4 bits of 0-padding.
             * CSI controller pixel: 0xX0 0xYZ (2 bytes)
             * Reconstituted pixel: 0x0YZX (u16)
             */
            S32V234,
        };
    }
}
