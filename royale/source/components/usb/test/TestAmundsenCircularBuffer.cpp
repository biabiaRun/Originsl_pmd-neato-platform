/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/AmundsenCircularBuffer.hpp>

#include <common/IteratorBoundsCheck.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <cassert>
#include <gmock/gmock.h>

#include <algorithm>
#include <chrono>
#include <deque>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

using namespace royale::buffer;
using namespace royale::hal;
using namespace royale::common;
using namespace royale::usb::bridge;

namespace
{
    const auto AMUNDSEN_STRIDE_SIZE = royale::usb::bridge::BridgeAmundsenCommon::AMUNDSEN_STRIDE_SIZE;

    class StrideCalculator
    {
    public:
        explicit StrideCalculator (uint8_t *start) :
            m_start (start)
        {
        }

        /**
         * Return a pointer to the expected starting point of the n-th stride,
         * or to the byte after the end of the final stride.
         */
        const uint8_t *at (std::size_t n) const
        {
            return m_start + n * AMUNDSEN_STRIDE_SIZE;
        }

    private:
        const uint8_t *m_start;
    };
}

TEST (TestAmundsenCircularBuffer, SequentialIterators)
{
    const std::size_t STRIDE_COUNT = 3;
    auto buffer = AmundsenCircularBuffer (STRIDE_COUNT);
    const StrideCalculator strideStart (buffer.start());

    ASSERT_FALSE (buffer.hasPrepared());

    for (auto i = std::size_t (0); i < STRIDE_COUNT; i++)
    {
        ASSERT_TRUE (buffer.hasUnprepared());
        ASSERT_EQ (strideStart.at (i), buffer.firstUnprepared());
        ASSERT_NO_THROW (buffer.setPrepared());
        ASSERT_TRUE (buffer.hasPrepared());
    }
    ASSERT_FALSE (buffer.hasUnprepared());

    for (auto i = std::size_t (0); i < STRIDE_COUNT; i++)
    {
        ASSERT_TRUE (buffer.hasPrepared());
        ASSERT_EQ (strideStart.at (i), buffer.firstPrepared());
        ASSERT_NO_THROW (buffer.setUsed());
        ASSERT_TRUE (buffer.hasUnprepared());
    }

    // Now wrap round, but only use two buffers
    for (auto i = std::size_t (0); i < STRIDE_COUNT - 1; i++)
    {
        ASSERT_TRUE (buffer.hasUnprepared());
        ASSERT_EQ (strideStart.at (i), buffer.firstUnprepared());
        ASSERT_NO_THROW (buffer.setPrepared());
        ASSERT_TRUE (buffer.hasPrepared());
    }
    ASSERT_TRUE (buffer.hasUnprepared());

    for (auto i = std::size_t (0); i < STRIDE_COUNT - 1; i++)
    {
        ASSERT_TRUE (buffer.hasPrepared());
        ASSERT_EQ (strideStart.at (i), buffer.firstPrepared());
        ASSERT_NO_THROW (buffer.setUsed());
        ASSERT_TRUE (buffer.hasUnprepared());
    }
}

/**
 * Simulate image data being received, check that the pointers which will be passed to
 * BufferUtils::copyAndNormalizeStrides point to the expected locations.
 */
TEST (TestAmundsenCircularBuffer, DataCaptureWithWrapround)
{
    const std::size_t STRIDE_COUNT = 3;
    auto buffer = AmundsenCircularBuffer (STRIDE_COUNT);
    const StrideCalculator strideStart (buffer.start());

    ASSERT_FALSE (buffer.hasPrepared());

    // Prepare and use two payloads
    for (auto i = std::size_t (0); i < 2; i++)
    {
        ASSERT_EQ (strideStart.at (i), buffer.firstUnprepared());
        ASSERT_NO_THROW (buffer.setPrepared());
        ASSERT_EQ (strideStart.at (i), buffer.firstPrepared());
        ASSERT_NO_THROW (buffer.setUsed());
    }

    // Discover that the most recently used payload is the start of a frame
    ASSERT_NO_THROW (buffer.setCurrentBufferIsFrameStart());
    ASSERT_EQ (0u, buffer.countUsedStridesExcludingCurrentBuffer());
    ASSERT_EQ (strideStart.at (1), buffer.startOfFrame());
    ASSERT_FALSE (buffer.isWrapped());

    // Capture another payload
    ASSERT_EQ (strideStart.at (2), buffer.firstUnprepared());
    ASSERT_NO_THROW (buffer.setPrepared());
    ASSERT_EQ (strideStart.at (2), buffer.firstPrepared());
    ASSERT_NO_THROW (buffer.setUsed());
    ASSERT_EQ (1u, buffer.countUsedStridesExcludingCurrentBuffer());
    ASSERT_FALSE (buffer.isWrapped());

    // Capture a third payload
    ASSERT_EQ (strideStart.at (0), buffer.firstUnprepared());
    ASSERT_NO_THROW (buffer.setPrepared());
    ASSERT_EQ (strideStart.at (0), buffer.firstPrepared());
    ASSERT_NO_THROW (buffer.setUsed());
    ASSERT_EQ (2u, buffer.countUsedStridesExcludingCurrentBuffer());
    ASSERT_TRUE (buffer.isWrapped());

    // Check the data pointers for passing to BufferUtils::copyAndNormalizeStrides
    ASSERT_EQ (strideStart.at (1), buffer.startOfFrame());
    ASSERT_EQ (strideStart.at (3), buffer.end());
    ASSERT_EQ (strideStart.at (0), buffer.start());
    ASSERT_EQ (strideStart.at (0), buffer.currentBuffer());
}
