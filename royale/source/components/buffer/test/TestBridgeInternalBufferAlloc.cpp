/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <buffer/BridgeInternalBufferAlloc.hpp>

#include <EndianConversion.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/MakeUnique.hpp>
#include <common/NarrowCast.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <memory>

using namespace royale::buffer;
using namespace royale::common;
using namespace royale::hal;

namespace
{
    /**
     * The BridgeInternalBufferAlloc only handles buffers, and needs the subclass to implement most
     * of the IBridgeDataReceiver interface.
     */
    class MockBridgeIBA : public BridgeInternalBufferAlloc
    {
    public:
        bool isConnected() const override
        {
            return true;
        }

        float getPeakTransferSpeed() override
        {
            // USB3 SuperSpeed
            return 5000e6f / 16;
        }

        std::size_t executeUseCase (int width, int height, std::size_t preferredBufferCount) override
        {
            auto size = narrow_cast<std::size_t> (width * height);
            createAndQueueInternalBuffers (preferredBufferCount, size, 0, 0);
            return preferredBufferCount;
        }

        void startCapture() override
        {
        }

        void stopCapture() override
        {
        }

        royale::Vector<royale::Pair<royale::String, royale::String>> getBridgeInfo() override
        {
            royale::Vector<royale::Pair<royale::String, royale::String>> bridgeInformation;
            return bridgeInformation;
        }

        void setEventListener (royale::IEventListener *listener) override
        {
        }

        /**
         * Make dequeueInternalBuffer public
         *
         * This isn't declared "override", because the base method is non-virtual.
         */
        OffsetBasedCapturedBuffer *dequeueInternalBuffer()
        {
            return BridgeInternalBufferAlloc::dequeueInternalBuffer();
        }

        /**
         * Make queueBuffer public
         */
        void queueBuffer (ICapturedBuffer *buffer) override
        {
            BridgeInternalBufferAlloc::queueBuffer (buffer);
        }

        bool shouldBlockForDequeue() override
        {
            m_queryShouldBlock++;
            return false;
        }

        /**
         * Returns the number of calls that have been made to shouldBlockForDequeue().
         */
        unsigned int getShouldBlockCounter() const
        {
            return m_queryShouldBlock;
        }

    private:
        std::atomic<unsigned int> m_queryShouldBlock {0};
    };
}

class TestBridgeInternalBufferAlloc : public ::testing::Test
{
protected:
    TestBridgeInternalBufferAlloc() = default;
    ~TestBridgeInternalBufferAlloc() = default;

    void SetUp() override
    {
        m_bridge = makeUnique<MockBridgeIBA> ();
    }

    void TearDown() override
    {
        for (auto buffer : m_buffers)
        {
            m_bridge->queueBuffer (buffer);
        }
        m_buffers.clear();

        m_bridge.reset();
    }

    void requeueABuffer()
    {
        ASSERT_FALSE (m_buffers.empty());
        auto buffer = m_buffers.back();
        m_buffers.pop_back();
        ASSERT_NO_THROW (m_bridge->queueBuffer (buffer));
    }

    std::unique_ptr<MockBridgeIBA> m_bridge;
    /** These buffers will be released in the TearDown() */
    std::vector<ICapturedBuffer *> m_buffers;
};

TEST_F (TestBridgeInternalBufferAlloc, CheckBufferAllocation)
{
    const std::size_t BUFFER_COUNT = 4;

    // The magic numbers are arbitrary width and height
    ASSERT_NO_THROW (m_bridge->executeUseCase (100, 5, BUFFER_COUNT));

    // That number of buffers should be available without blocking
    for (std::size_t i = 0; i < BUFFER_COUNT; i++)
    {
        m_buffers.push_back (m_bridge->dequeueInternalBuffer());
        ASSERT_NE (nullptr, m_buffers.back());
    }
    ASSERT_EQ (0u, m_bridge->getShouldBlockCounter());

    // All buffers are allocated, so this should return nullptr
    ASSERT_EQ (nullptr, m_bridge->dequeueInternalBuffer());
    auto counter = m_bridge->getShouldBlockCounter();
    ASSERT_NE (0u, counter);

    requeueABuffer();

    // It should now be possible to get another buffer, this shouldn't call shouldBlockForDequeue
    m_buffers.push_back (m_bridge->dequeueInternalBuffer());
    ASSERT_NE (nullptr, m_buffers.back());
    ASSERT_EQ (counter, m_bridge->getShouldBlockCounter());
}
