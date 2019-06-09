/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/EndianConversion.hpp>
#include <common/exceptions/OutOfBounds.hpp>

#include <cstdint>
#include <vector>

using namespace royale::common;

TEST (TestEndianConversion, PushBackIterable)
{
    const auto original = std::vector<uint8_t> {0, 1, 2};
    std::vector<uint8_t> destination;

    ASSERT_THROW (pushBackPadded (destination, original, 2u), OutOfBounds);
    ASSERT_EQ (destination.size(), 0u);

    ASSERT_NO_THROW (pushBackPadded (destination, original, 4u));
    const auto reference1 = std::vector<uint8_t> {0, 1, 2, 0};
    ASSERT_EQ (reference1.size(), destination.size());
    ASSERT_EQ (reference1, destination);

    ASSERT_NO_THROW (pushBackIterable (destination, original));
    const auto reference2 = std::vector<uint8_t> {0, 1, 2, 0, 0, 1, 2};
    ASSERT_EQ (reference2.size(), destination.size());
    ASSERT_EQ (reference2, destination);
}

TEST (TestEndianConversion, PushBackHighFirst16)
{
    const auto original = std::vector<uint8_t> {0, 1, 2};
    std::vector<uint16_t> destination;

    ASSERT_NO_THROW (pushBackIterableAsHighFirst16 (destination, original));
    const auto reference1 = std::vector<uint16_t> {0x0001, 0x0200};
    ASSERT_EQ (reference1, destination);
}

TEST (TestEndianConversion, ReadVectors)
{
    const auto buffer = std::vector<uint8_t> {0, 1, 2, 3, 4, 5, 6, 7};

    // scope for variable names
    {
        const auto readAs16To16 = bufferToHostVector16<std::vector<uint16_t>> (buffer.data(), buffer.size() / 2);
        const auto readAs16To32 = bufferToHostVector16<std::vector<uint32_t>> (buffer.data(), buffer.size() / 2);
        const auto readAs16ToSizeT = bufferToHostVector16<std::vector<std::size_t>> (buffer.data(), buffer.size() / 2);

        ASSERT_EQ (std::vector<uint16_t> ({0x100, 0x302, 0x504, 0x706}), readAs16To16);
        ASSERT_EQ (std::vector<uint32_t> ({0x100, 0x302, 0x504, 0x706}), readAs16To32);
        ASSERT_EQ (std::vector<std::size_t> ({0x100, 0x302, 0x504, 0x706}), readAs16ToSizeT);
    }

    // scope for variable names
    {
        const auto readAs32To32 = bufferToHostVector32<std::vector<uint32_t>> (buffer.data(), buffer.size() / 4);
        const auto readAs32ToSizeT = bufferToHostVector32<std::vector<std::size_t>> (buffer.data(), buffer.size() / 4);

        ASSERT_EQ (std::vector<uint32_t> ({0x3020100, 0x7060504}), readAs32To32);
        ASSERT_EQ (std::vector<std::size_t> ({0x3020100, 0x7060504}), readAs32ToSizeT);
    }
}
