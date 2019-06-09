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

#include <hal/ICapturedBuffer.hpp>

#include <buffer/BufferDataFormat.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/IntegerMath.hpp>

#include <algorithm>
#include <cstddef>

namespace royale
{
    namespace buffer
    {
        /**
         * Data handling functions that can be shared between Bridge implementations.  All of these
         * convert a buffer to the format expected for IBufferCaptureListener (native-endian
         * uint16_t, with the 12-bit data left-padded with zeros to 16 bits).
         */
        class BufferUtils
        {
        public:
            /**
             * Returns the number of bytes, in the given format, which would normalize to the given
             * number of pixels.
             *
             * This throws if called with BufferDataFormat::UNKNOWN.
             */
            inline static std::size_t expectedRawSize (std::size_t pixels, BufferDataFormat format)
            {
                switch (format)
                {
                    case BufferDataFormat::RAW16:
                    case BufferDataFormat::S32V234:
                        return pixels * 2;
                    case BufferDataFormat::RAW12:
                        return (pixels * 3) / 2;
                    default:
                        throw royale::common::LogicError ("expectedRawSize called with data format UNKNOWN");
                }
            }

            /**
             * Similar to expectedRawSize, but if called with BufferDataFormat::UNKNOWN it will
             * return the largest size that could be required for any supported format.
             */
            inline static std::size_t maxRawSize (std::size_t pixels, BufferDataFormat format)
            {
                switch (format)
                {
                    case BufferDataFormat::RAW12:
                        return (pixels * 3) / 2;
                    case BufferDataFormat::RAW16:
                    case BufferDataFormat::S32V234:
                    default:
                        return pixels * 2;
                }
            }

            /**
             * The inverse of expectedRawSize, given the size of a block of raw data, return the
             * number of pixels that it is expected to normalize to.
             */
            inline static std::size_t expectedPixelCount (std::size_t rawSize, BufferDataFormat format)
            {
                switch (format)
                {
                    case BufferDataFormat::RAW16:
                    case BufferDataFormat::S32V234:
                        return rawSize / 2;
                    case BufferDataFormat::RAW12:
                        return (rawSize * 2) / 3;
                    default:
                        throw royale::common::LogicError ("expectedPixelCount called with data format UNKNOWN");
                }
            }

            /**
             * Convert data received in RAW12, storing it in the area pointed to by pixelData.
             * The caller must ensure that both pointers are valid for pixelCount pixels.
             *
             * The algorithm supports src pointing to the same location as pixelData.
             */
            inline static void copyOrInPlaceRaw12 (const uint8_t *srcData, std::size_t pixelCount, uint16_t *pixelData)
            {
                // The two branches contain the same logic, one is just a manual unroll
                const auto pixelUnroll = pixelCount & ~std::size_t (0x7);
                for (std::size_t iPlusOne = (pixelCount / 2); iPlusOne > (pixelUnroll / 2); iPlusOne--)
                {
                    const auto src = &srcData [3 * (iPlusOne - 1)];
                    uint16_t pixelA = static_cast<uint16_t> ( (src [1] << 4) | (src [2] >> 4));
                    uint16_t pixelB = static_cast<uint16_t> ( (src [0] << 4) | (src [2] & 0xf));
                    const auto dest = &pixelData [2 * (iPlusOne - 1)];
                    dest[1] = pixelA;
                    dest[0] = pixelB;
                }
                for (std::size_t iPlusFour = (pixelUnroll / 2); iPlusFour > 0; iPlusFour -= 4)
                {
                    const auto src = &srcData [3 * (iPlusFour - 4)];
                    uint16_t pixelA = static_cast<uint16_t> ( (src [1] << 4) | (src [2] >> 4));
                    uint16_t pixelB = static_cast<uint16_t> ( (src [0] << 4) | (src [2] & 0xf));
                    uint16_t pixelC = static_cast<uint16_t> ( (src [4] << 4) | (src [5] >> 4));
                    uint16_t pixelD = static_cast<uint16_t> ( (src [3] << 4) | (src [5] & 0xf));
                    uint16_t pixelE = static_cast<uint16_t> ( (src [7] << 4) | (src [8] >> 4));
                    uint16_t pixelF = static_cast<uint16_t> ( (src [6] << 4) | (src [8] & 0xf));
                    uint16_t pixelG = static_cast<uint16_t> ( (src [10] << 4) | (src [11] >> 4));
                    uint16_t pixelH = static_cast<uint16_t> ( (src [9] << 4) | (src [11] & 0xf));
                    const auto dest = &pixelData [2 * (iPlusFour - 4)];
                    dest[1] = pixelA;
                    dest[0] = pixelB;
                    dest[3] = pixelC;
                    dest[2] = pixelD;
                    dest[5] = pixelE;
                    dest[4] = pixelF;
                    dest[7] = pixelG;
                    dest[6] = pixelH;
                }
            }

            /**
             * Convert data received in RAW12 format, unpacking it in-place in the same buffer.
             *
             * In this RAW12 format, 2 pixels are packed in 3 bytes, with the most-significant 8
             * bits of each pixel in their own byte, and the least-significant 4 bits of each pixel
             * stored in the third byte.
             *
             * This is the format used by the Arctic firmware (from v0.8 onwards, on CX3 devices),
             *
             * The caller must ensure that the buffer has the correct pixel count, and the buffer is
             * large enough to store that many pixels in normalized format.
             */
            inline static void normalizeRaw12 (royale::hal::ICapturedBuffer &buffer)
            {
                auto pixelData = buffer.getPixelData ();
                auto src = reinterpret_cast<uint8_t *> (pixelData);
                copyOrInPlaceRaw12 (src, buffer.getPixelCount(), pixelData);
            }

            /**
             * Convert data received in RAW16, storing it in the area pointed to by pixelData.
             * The caller must ensure that both pointers are valid for pixelCount pixels.
             *
             * The algorithm supports src pointing to the same location as pixelData.
             */
            inline static void copyOrInPlaceRaw16 (const uint8_t *srcData, std::size_t pixelCount, uint16_t *pixelData)
            {
                // \todo ROYAL-2447 This always does the extra overhead for converting Enclustra
                // format, as described in the note in normalizeRaw16.

                // The two branches contain the same logic, one is just a manual unroll
                const auto pixelUnroll = pixelCount & ~std::size_t (0x7);
                for (std::size_t iPlusOne = pixelCount; iPlusOne > pixelUnroll; iPlusOne--)
                {
                    const auto src = &srcData [2 * (iPlusOne - 1)];
                    const auto pixelA = static_cast<uint16_t> ( (src [0] | (src [1] << 8)) & 0x0fff);
                    pixelData[iPlusOne - 1] = pixelA;
                }
                for (std::size_t iPlusEight = pixelUnroll; iPlusEight > 0; iPlusEight -= 8)
                {
                    const auto src = &srcData[2 * (iPlusEight - 8)];
                    const auto pixelA = static_cast<uint16_t> ( (src [0] | (src [1] << 8)) & 0x0fff);
                    const auto pixelB = static_cast<uint16_t> ( (src [2] | (src [3] << 8)) & 0x0fff);
                    const auto pixelC = static_cast<uint16_t> ( (src [4] | (src [5] << 8)) & 0x0fff);
                    const auto pixelD = static_cast<uint16_t> ( (src [6] | (src [7] << 8)) & 0x0fff);
                    const auto pixelE = static_cast<uint16_t> ( (src [8] | (src [9] << 8)) & 0x0fff);
                    const auto pixelF = static_cast<uint16_t> ( (src [10] | (src [11] << 8)) & 0x0fff);
                    const auto pixelG = static_cast<uint16_t> ( (src [12] | (src [13] << 8)) & 0x0fff);
                    const auto pixelH = static_cast<uint16_t> ( (src [14] | (src [15] << 8)) & 0x0fff);
                    const auto dest = &pixelData[iPlusEight - 8];
                    dest[0] = pixelA;
                    dest[1] = pixelB;
                    dest[2] = pixelC;
                    dest[3] = pixelD;
                    dest[4] = pixelE;
                    dest[5] = pixelF;
                    dest[6] = pixelG;
                    dest[7] = pixelH;
                }
            }

            /**
             * Convert data received in RAW16 format, and convert the data in-place, which can be a
             * no-op on little-endian architectures.  This may be optimised compared to the
             * normalizeEnclustra function, or it may simply be an alias for it.
             */
            inline static void normalizeRaw16 (royale::hal::ICapturedBuffer &buffer)
            {
                // \todo ROYAL-2447 If this is a little-endian system, and there are no bits set in
                // the high nibbles of the data, then this function could be optimised to a no-op.
                // But the number of devices that this would help is limited, because the current
                // CX3 UVC firmware uses RAW12 instead of RAW16, and the older RAW16 firmware sets
                // bits in the high nibble (so it needs the same conversion as Enclustra).
                normalizeEnclustra (buffer);
            }

            /**
             * Convert data received in "Enclustra" format, changing it in-place in the same buffer.
             *
             * In this format, the data is always little-endian and has metadata bits in the high
             * nibble.  This is a combined endian conversion and application of a 12-bit mask.
             */
            inline static void normalizeEnclustra (royale::hal::ICapturedBuffer &buffer)
            {
                auto pixelData = buffer.getPixelData ();
                copyOrInPlaceRaw16 (reinterpret_cast<uint8_t *> (pixelData), buffer.getPixelCount(), pixelData);
            }

             /**
             * Convert data received in S32V234, storing it in the area pointed to by pixelData.
             * The caller must ensure that both pointers are valid for pixelCount pixels.
             *
             * The algorithm supports src pointing to the same location as pixelData.
             */
            inline static void copyOrInPlaceS32V234 (const uint8_t *srcData, std::size_t pixelCount, uint16_t *pixelData)
            {
                // The two branches contain the same logic, one is just a manual unroll
                const auto pixelUnroll = pixelCount & ~std::size_t (0x7);
                for (std::size_t iPlusOne = pixelCount; iPlusOne > pixelUnroll; iPlusOne--)
                {
                    const auto src = &srcData [2 * (iPlusOne - 1)];
                    const auto pixelA = static_cast<uint16_t> ( ((src [0] >> 4) | (src [1] << 4)) & 0x0fff);
                    pixelData[iPlusOne - 1] = pixelA;
                }
                for (std::size_t iPlusEight = pixelUnroll; iPlusEight > 0; iPlusEight -= 8)
                {
                    const auto src = &srcData[2 * (iPlusEight - 8)];
                    const auto pixelA = static_cast<uint16_t> ( ((src [0] >> 4) | (src [1] << 4)) & 0x0fff);
                    const auto pixelB = static_cast<uint16_t> ( ((src [2] >> 4) | (src [3] << 4)) & 0x0fff);
                    const auto pixelC = static_cast<uint16_t> ( ((src [4] >> 4) | (src [5] << 4)) & 0x0fff);
                    const auto pixelD = static_cast<uint16_t> ( ((src [6] >> 4) | (src [7] << 4)) & 0x0fff);
                    const auto pixelE = static_cast<uint16_t> ( ((src [8] >> 4) | (src [9] << 4)) & 0x0fff);
                    const auto pixelF = static_cast<uint16_t> ( ((src [10] >> 4) | (src [11] << 4)) & 0x0fff);
                    const auto pixelG = static_cast<uint16_t> ( ((src [12] >> 4) | (src [13] << 4)) & 0x0fff);
                    const auto pixelH = static_cast<uint16_t> ( ((src [14] >> 4) | (src [15] << 4)) & 0x0fff);
                    const auto dest = &pixelData[iPlusEight - 8];
                    dest[0] = pixelA;
                    dest[1] = pixelB;
                    dest[2] = pixelC;
                    dest[3] = pixelD;
                    dest[4] = pixelE;
                    dest[5] = pixelF;
                    dest[6] = pixelG;
                    dest[7] = pixelH;
                }
            }

            /**
             * Convert data received in RAW16 format, and convert the data in-place, which can be a
             * no-op on little-endian architectures.  This may be optimised compared to the
             * normalizeEnclustra function, or it may simply be an alias for it.
             */
            inline static void normalizeS32V234 (royale::hal::ICapturedBuffer &buffer)
            {
                // \todo ROYAL-2447 If this is a little-endian system, and there are no bits set in
                // the high nibbles of the data, then this function could be optimised to a no-op.
                // But the number of devices that this would help is limited, because the current
                // CX3 UVC firmware uses RAW12 instead of RAW16, and the older RAW16 firmware sets
                // bits in the high nibble (so it needs the same conversion as Enclustra).
                auto pixelData = buffer.getPixelData ();
                copyOrInPlaceS32V234 (reinterpret_cast<uint8_t *> (pixelData), buffer.getPixelCount(), pixelData);
            }

            /**
             * Convert data received, changing it in-place in the same buffer.  This is equivalent
             * to calling either normalizeRaw12 or normalizeRaw16, depending on the format argument.
             *
             * \throw LogicError if called with BufferDataFormat::UNKNOWN
             */
            inline static void normalize (royale::hal::ICapturedBuffer &buffer, BufferDataFormat format)
            {
                switch (format)
                {
                    case BufferDataFormat::RAW16:
                        {
                            BufferUtils::normalizeRaw16 (buffer);
                            break;
                        }
                    case BufferDataFormat::RAW12:
                        {
                            BufferUtils::normalizeRaw12 (buffer);
                            break;
                        }
                    case BufferDataFormat::S32V234:
                        {
                            BufferUtils::normalizeS32V234 (buffer);
                            break;
                        }
                    default:
                        throw royale::common::LogicError ("normalize called with data format UNKNOWN");
                        break;
                }
            }

            /**
             * Convert data from the src buffer to the ICapturedBuffer.  The amount of data copied
             * will be limited by both dest.getPixelSize() and srcSize.
             *
             * If srcSize converts to fewer pixels than dest.getPixelSize(), the contents of the
             * unused part of dest are implementation defined.  Mixed-mode's variable-sized
             * superframes are expected to cause this with the "unused" part being a copy of an old
             * image. Given this, the implementation-defined behavior may overwrite data in the
             * "unused" part to aid in debugging, however it may also leave the "unused" part
             * unchanged.
             *
             * If src and dest's buffer overlap then the result is undefined.
             *
             * @param dest the data will be copied to dest.getPixelData()
             * @param src where to read the data from
             * @param srcSize the maximum number of bytes to read from src
             * @param format what type of data src contains
             */
            inline static void copyAndNormalize (royale::hal::ICapturedBuffer &dest, const void *src, std::size_t srcSize, BufferDataFormat format)
            {
                if (format != BufferDataFormat::RAW12 && format != BufferDataFormat::RAW16 && format != BufferDataFormat::S32V234)
                {
                    throw royale::common::LogicError ("copyAndNormalize called with data format UNKNOWN");
                }

                const auto pixelsToConvert = std::min (dest.getPixelCount(), expectedPixelCount (srcSize, format));

                if (format == BufferDataFormat::RAW12)
                {
                    copyOrInPlaceRaw12 (reinterpret_cast<const uint8_t *> (src), pixelsToConvert, dest.getPixelData());
                }
                else if (format == BufferDataFormat::RAW16)
                {
                    copyOrInPlaceRaw16 (reinterpret_cast<const uint8_t *> (src), pixelsToConvert, dest.getPixelData());
                }
                else if (format == BufferDataFormat::S32V234)
                {
                    copyOrInPlaceS32V234 (reinterpret_cast<const uint8_t *> (src), pixelsToConvert, dest.getPixelData());
                }
            }

            /**
             * Convert data from the source buffer to the ICapturedBuffer.  The amount of data copied
             * will be limited by the sizes given for the sources. The size will be checked against
             * dest.getPixelSize(), but the effect of that test failing is implementation-defined
             * (it can either truncate or throw an exception).
             *
             * The data is in a single buffer, but instead of being contiguous it's contained in
             * blocks of elementSize bytes, each starting at a multiple of strideSize bytes (the
             * first element starts at first[0]).  The final block may be smaller.
             *
             * For RAW12 data, the elementSize must be a multiple of 3 bytes.  The size of the final
             * block must be either zero or a multiple of 3 bytes.
             *
             * For RAW16 data, the elementSize must be a multiple of 2 bytes, and elements must be
             * aligned for access as uint16_ts.  The size of the final block must be either zero or
             * a multiple of 2 bytes, and it must also be aligned.
             *
             * The source buffer only needs to contain the data that will be converted to pixels,
             * only bytes in the range (first ... last-1) will  be accessed, regardless of where in
             * the last stride (last-1) is.  Even if the elements aren't at the start of the containing
             * buffer, the argument `first` should point to the first wanted byte.
             *
             * If the data converts to fewer pixels than dest.getPixelSize(), the contents of the
             * unused part of dest are implementation defined.  Mixed-mode's variable-sized
             * superframes are expected to cause this with the "unused" part being a copy of an old
             * image. Given this, the implementation-defined behavior may overwrite data in the
             * "unused" part to aid in debugging, however it may also leave the "unused" part
             * unchanged.
             *
             * If the source and dest's buffer overlap then the result is undefined.
             *
             * @param dest the data will be copied to dest.getPixelData()
             * @param first beginning of the area to read data from
             * @param last one after the end of the area to read data from
             * @param elementSize number of bytes in each complete chunk (last chunk can be smaller)
             * @param strideSize distance between the start of each chunk
             * @param format what type of data the source contains
             */
            inline static void copyAndNormalizeStrides (royale::hal::ICapturedBuffer &dest,
                    const uint8_t *first, const uint8_t *last,
                    const std::size_t elementSize, const std::size_t strideSize,
                    const BufferDataFormat format)
            {
                if (format != BufferDataFormat::RAW12 && format != BufferDataFormat::RAW16 && format != BufferDataFormat::S32V234)
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides called with data format UNKNOWN");
                }

                if (elementSize > strideSize)
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides called with elements larger than strides");
                }

                const auto completeStrides = size_t (last - first) / strideSize;
                const auto finalSize = std::min (elementSize, size_t (last - first) % strideSize);

                if (completeStrides * elementSize + finalSize > expectedRawSize (dest.getPixelCount(), format))
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides would overflow destination");
                }

                // Both copyOrInPlaceRaw12 and copyOrInPlaceRaw16 work from back to front, so this
                // code calls the copyOrInPlace function in the same direction.
                if (format == BufferDataFormat::RAW12)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceRaw12 (&first[completeStrides * strideSize], pixelsInFinalElement,
                                            &pixelData[completeStrides * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStrides; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw12 (&first[i * strideSize], pixelsPerElement,
                                            &pixelData[i * pixelsPerElement]);
                    }
                }
                else if (format == BufferDataFormat::RAW16)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceRaw16 (&first[completeStrides * strideSize], pixelsInFinalElement,
                                            &pixelData[completeStrides * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStrides; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw16 (&first[i * strideSize], pixelsPerElement,
                                            &pixelData[i * pixelsPerElement]);
                    }
                }
                else if (format == BufferDataFormat::S32V234)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceS32V234 (&first[completeStrides * strideSize], pixelsInFinalElement,
                                              &pixelData[completeStrides * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStrides; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceS32V234 (&first[i * strideSize], pixelsPerElement,
                                              &pixelData[i * pixelsPerElement]);
                    }
                }
            }

            /**
             * Version of copyAndNormalizeStrides which works for circular buffers.
             *
             * It supports any situation where the data is split in to exactly two ranges, but
             * circular buffers are the expected usage.  The range first-stop is expected to have
             * a number of complete elements, and the same size elements are expected in the range
             * restart-last.  As with copyAndNormalizeStrides, the final element of restart-last
             * can be a different size.
             *
             * The stride before `stop` (the wrap-round) point must contain a complete element,
             * but it doesn't need to contain a complete stride.  This allows a protocol where each
             * stride includes a header, and `first` points to the first wanted byte (and therefore
             * to an offset in to the stride).
             */
            template <class InputIt>
            inline static void copyAndNormalizeStrides (royale::hal::ICapturedBuffer &dest,
                    const InputIt first, const InputIt stop,
                    const InputIt restart, const InputIt last,
                    const std::size_t elementSize, const std::size_t strideSize,
                    const BufferDataFormat format)
            {
                if (format != BufferDataFormat::RAW12 && format != BufferDataFormat::RAW16 && format != BufferDataFormat::S32V234)
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides called with data format UNKNOWN");
                }

                if (elementSize > strideSize)
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides called with elements larger than strides");
                }

                const auto completeStridesFirstStop = (size_t (stop - first) + strideSize - elementSize) / strideSize;
                const auto completeStridesRestartLast = size_t (last - restart) / strideSize;
                const auto completeStridesTotal = completeStridesFirstStop + completeStridesRestartLast;
                const auto finalSize = std::min (elementSize, size_t (last - restart) % strideSize);

                if (completeStridesTotal * elementSize + finalSize > expectedRawSize (dest.getPixelCount(), format))
                {
                    throw royale::common::LogicError ("copyAndNormalizeStrides would overflow destination");
                }

                // Both copyOrInPlaceRaw12 and copyOrInPlaceRaw16 work from back to front, so this
                // code calls the copyOrInPlace function in the same direction.
                if (format == BufferDataFormat::RAW12)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceRaw12 (&restart[completeStridesRestartLast * strideSize], pixelsInFinalElement,
                                            &pixelData[completeStridesTotal * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesRestartLast; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw12 (&restart[i * strideSize], pixelsPerElement,
                                            &pixelData[ (i + completeStridesFirstStop) * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesFirstStop; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw12 (&first[i * strideSize], pixelsPerElement,
                                            &pixelData[i * pixelsPerElement]);
                    }
                }
                else if (format == BufferDataFormat::RAW16)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceRaw16 (&restart[completeStridesRestartLast * strideSize], pixelsInFinalElement,
                                            &pixelData[completeStridesTotal * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesRestartLast; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw16 (&restart[i * strideSize], pixelsPerElement,
                                            &pixelData[ (i + completeStridesFirstStop) * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesFirstStop; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceRaw16 (&first[i * strideSize], pixelsPerElement, &pixelData[i * pixelsPerElement]);
                    }
                }
                else if (format == BufferDataFormat::S32V234)
                {
                    auto pixelData = dest.getPixelData();
                    const auto pixelsPerElement = expectedPixelCount (elementSize, format);
                    if (finalSize != 0)
                    {
                        const auto pixelsInFinalElement = expectedPixelCount (finalSize, format);
                        copyOrInPlaceS32V234 (&restart[completeStridesRestartLast * strideSize], pixelsInFinalElement,
                                              &pixelData[completeStridesTotal * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesRestartLast; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceS32V234 (&restart[i * strideSize], pixelsPerElement,
                                              &pixelData[ (i + completeStridesFirstStop) * pixelsPerElement]);
                    }
                    for (auto iPlusOne = completeStridesFirstStop; iPlusOne > 0; iPlusOne--)
                    {
                        auto i = iPlusOne - 1;
                        copyOrInPlaceS32V234 (&first[i * strideSize], pixelsPerElement, &pixelData[i * pixelsPerElement]);
                    }
                }
            }
        };
    }
}
