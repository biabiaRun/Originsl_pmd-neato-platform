/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <buffer/BufferUtils.hpp>
#include <buffer/SimpleCapturedBuffer.hpp>

#include <common/EndianConversion.hpp>
#include <common/exceptions/LogicError.hpp>

#include <gtest/gtest.h>

#include <vector>

using namespace royale::buffer;
using namespace royale::common;

namespace
{
    /**
     * A small (22 pixel) test frame. To test both the unrolled and single-pixel loops, the number
     * of pixels isn't a multiple of 4.
     */
    const auto testFramePixels = std::vector<uint16_t>
                                 {
                                     0x0102, 0x0304, 0x0567, 0x0fed, 0x0234, 0x0abc, 0x0357, 0x09bd,
                                     0x0000, 0x0111, 0x0222, 0x0333, 0x0444, 0x0555,
                                     0x0888, 0x0999, 0x0aaa, 0x0bbb, 0x0ccc, 0x0ddd, 0x0eee, 0x0fff
                                 };

    /**
     * The testFramePixels, encoded with RAW12 encoding.
     *
     * The first two bytes of the raw data are the high bits of pixel[0] and the high bits of
     * pixel[1]. Byte 3 is the low nibble of pixel[0] and the low nibble of pixel[1]; in the order
     * that, if the data is sent as a bit stream with the least significant bit first, the pixel[0]
     * nibble is sent before the pixel[1] nibble.
     */
    const auto testDataRaw12 = std::vector<uint8_t>
                               {
                                   0x10, 0x30, 0x42, 0x56, 0xfe, 0xd7,
                                   0x23, 0xab, 0xc4, 0x35, 0x9b, 0xd7,
                                   0x00, 0x11, 0x10, 0x22, 0x33, 0x32,
                                   0x44, 0x55, 0x54, 0x88, 0x99, 0x98,
                                   0xaa, 0xbb, 0xba, 0xcc, 0xdd, 0xdc,
                                   0xee, 0xff, 0xfe
                               };

    /**
     * The testFramePixels, encoded with RAW16 / Enclustra encoding.
     *
     * A few pixels also have the high nibble set, to test Enclustra encoding.
     */
    const auto testDataRaw16 = std::vector<uint8_t>
                               {
                                   0x02, 0xf1, 0x04, 0x03, 0x67, 0x05, 0xed, 0x0f,
                                   0x34, 0x02, 0xbc, 0x0a, 0x57, 0x03, 0xbd, 0x09,
                                   0x00, 0xf0, 0x11, 0xf1, 0x22, 0x02, 0x33, 0x03,
                                   0x44, 0x04, 0x55, 0x05, 0x88, 0x08, 0x99, 0x09,
                                   0xaa, 0x0a, 0xbb, 0x0b, 0xcc, 0x0c, 0xdd, 0x0d,
                                   0xee, 0x0e, 0xff, 0x0f
                               };

    std::vector<uint8_t> createStrideData (const std::vector<uint8_t> &src, std::size_t elementSize, std::size_t strideSize)
    {
        const auto PAD = uint8_t (0);
        if (elementSize >= strideSize)
        {
            throw LogicError();
        }
        const auto padLength = strideSize - elementSize;

        std::vector<uint8_t> dest;
        for (auto i = 0u; i < src.size(); i++)
        {
            if ( (i != 0) && (i % elementSize == 0))
            {
                for (auto j = 0u; j < padLength; j++)
                {
                    dest.push_back (PAD);
                }
            }
            dest.push_back (src.at (i));
        }

        return dest;
    }
}

TEST (TestBufferUtils, CopyAndNormalizeRaw16)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    BufferUtils::copyAndNormalize (dest, testDataRaw16.data(), testDataRaw16.size(), BufferDataFormat::RAW16);

    const auto pixelData = dest.getPixelData();
    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), pixelData[i]);
    }
}

TEST (TestBufferUtils, CopyAndNormalizeRaw12)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    BufferUtils::copyAndNormalize (dest, testDataRaw12.data(), testDataRaw12.size(), BufferDataFormat::RAW12);

    const auto pixelData = dest.getPixelData();
    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), pixelData[i]);
    }
}

/**
 * Test the conversion of RAW16 data, with the source buffer having 6 bytes per element,
 * padded to 8 bytes per stride.
 */
TEST (TestBufferUtils, Raw16WithStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    const auto src = createStrideData (testDataRaw16, elementSize, strideSize);

    BufferUtils::copyAndNormalizeStrides (dest, src.data(), src.data() + src.size(), elementSize, strideSize, BufferDataFormat::RAW16);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}

/**
 * Test the conversion of RAW12 data, with the source buffer having 6 bytes per element,
 * padded to 8 bytes per stride.
 */
TEST (TestBufferUtils, Raw12WithStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    const auto src = createStrideData (testDataRaw12, elementSize, strideSize);

    BufferUtils::copyAndNormalizeStrides (dest, src.data(), src.data() + src.size(), elementSize, strideSize, BufferDataFormat::RAW12);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}

/**
 * Test the conversion of RAW16 data, with the source buffer having 6 bytes per element,
 * padded to 8 bytes per stride, and the source being a circular buffer which wraps round
 * between the two strides.
 */
TEST (TestBufferUtils, Raw16WithCircularBufferFullStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    auto src = createStrideData (testDataRaw16, elementSize, strideSize);

    // Convert src to simulate a circular buffer, with data at circularJoin-src.end(), and
    // then from src.start()-circularJoin.  In C++11, the return value of std::rotate should
    // be the same as circularJoin, but with GCC 4.9 std::rotate returns void.
    std::rotate (src.begin(), src.begin() + strideSize, src.end());
    const auto circularJoin = src.begin() + (src.size() - strideSize);

    BufferUtils::copyAndNormalizeStrides (dest, circularJoin, src.end(), src.begin(), circularJoin, elementSize, strideSize, BufferDataFormat::RAW16);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}

/**
 * As Raw16WithCircularBufferFullStride, except that in this test the wrap-round point of the buffer
 * is aligned to the end of the element, and so there's a stride split over the boundary.
 */
TEST (TestBufferUtils, Raw16WithCircularBufferPartStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    auto src = createStrideData (testDataRaw16, elementSize, strideSize);

    // Convert src to simulate a circular buffer, with data at circularJoin-src.end(), and
    // then from src.start()-circularJoin.  In C++11, the return value of std::rotate should
    // be the same as circularJoin, but with GCC 4.9 std::rotate returns void.
    std::rotate (src.begin(), src.begin() + elementSize, src.end());
    const auto circularJoin = src.begin() + (src.size() - elementSize);

    BufferUtils::copyAndNormalizeStrides (dest, circularJoin, src.end(), src.begin() + (strideSize - elementSize), circularJoin, elementSize, strideSize, BufferDataFormat::RAW16);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}

/**
 * Test the conversion of RAW12 data, with the source buffer having 6 bytes per element,
 * padded to 8 bytes per stride, and the source being a circular buffer which wraps round
 * between the two strides.
 *
 * In this test, the wrap-round point of the buffer is aligned to the strides.
 */
TEST (TestBufferUtils, Raw12WithCircularBufferFullStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    auto src = createStrideData (testDataRaw12, elementSize, strideSize);

    // Convert src to simulate a circular buffer, with data at circularJoin-src.end(), and
    // then from src.start()-circularJoin.  In C++11, the return value of std::rotate should
    // be the same as circularJoin, but with GCC 4.9 std::rotate returns void.
    std::rotate (src.begin(), src.begin() + strideSize, src.end());
    const auto circularJoin = src.begin() + (src.size() - strideSize);

    BufferUtils::copyAndNormalizeStrides (dest, circularJoin, src.end(), src.begin(), circularJoin, elementSize, strideSize, BufferDataFormat::RAW12);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}

/**
 * As Raw12WithCircularBufferFullStride, except that in this test the wrap-round point of the buffer
 * is aligned to the end of the element, and so there's a stride split over the boundary.
 */
TEST (TestBufferUtils, Raw12WithCircularBufferPartStride)
{
    const std::size_t pixelCount = testFramePixels.size();
    SimpleCapturedBuffer dest (pixelCount * sizeof (uint16_t), 0, pixelCount);

    const std::size_t elementSize = 6;
    const std::size_t strideSize = 8;
    auto src = createStrideData (testDataRaw12, elementSize, strideSize);

    // Convert src to simulate a circular buffer, with data at circularJoin-src.end(), and
    // then from src.start()-circularJoin.  In C++11, the return value of std::rotate should
    // be the same as circularJoin, but with GCC 4.9 std::rotate returns void.
    std::rotate (src.begin(), src.begin() + elementSize, src.end());
    const auto circularJoin = src.begin() + (src.size() - elementSize);

    BufferUtils::copyAndNormalizeStrides (dest, circularJoin, src.end(), src.begin() + (strideSize - elementSize), circularJoin, elementSize, strideSize, BufferDataFormat::RAW12);

    for (auto i = 0u; i < pixelCount; i++)
    {
        ASSERT_EQ (testFramePixels.at (i), dest.getPixelData() [i]);
    }
}
