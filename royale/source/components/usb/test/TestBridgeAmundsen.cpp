/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeAmundsenCommon.hpp>

#include <common/IteratorBoundsCheck.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <cassert>
#include <gmock/gmock.h>
#include <ThreadedAssertSupport.hpp>

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
    /**
     * A test image, and the corresponding data in RAW12 and RAW16 formats.
     */
    struct TestData
    {
        std::vector<uint16_t> pixels;
        std::vector<uint8_t> raw12;
        std::vector<uint8_t> raw16;
    };

    /**
     * A small (22 pixel) test frame, matching the one that's tested in TestBufferUtils.
     * (this is a cut & paste from that test).
     *
     * The BridgeAmundsenCommon assumes normal devices, splitting the data in to large payloads,
     * with approximately 12kbytes per payload and small overheads for each payload, and it seems
     * reasonable to optimize for these devices. Small images such as this one must be supported,
     * but the test should expect the bridge to allocate at most two payloads for such a small
     * image.
     */
    const auto twentyTwoPixelImage = TestData
                                     {
                                         {
                                             0x0102, 0x0304, 0x0567, 0x0fed, 0x0234, 0x0abc, 0x0357, 0x09bd,
                                             0x0000, 0x0111, 0x0222, 0x0333, 0x0444, 0x0555,
                                             0x0888, 0x0999, 0x0aaa, 0x0bbb, 0x0ccc, 0x0ddd, 0x0eee, 0x0fff
                                         },
    {
        0x10, 0x30, 0x42, 0x56, 0xfe, 0xd7,
        0x23, 0xab, 0xc4, 0x35, 0x9b, 0xd7,
        0x00, 0x11, 0x10, 0x22, 0x33, 0x32,
        0x44, 0x55, 0x54, 0x88, 0x99, 0x98,
        0xaa, 0xbb, 0xba, 0xcc, 0xdd, 0xdc,
        0xee, 0xff, 0xfe
    },
    {
        0x02, 0xf1, 0x04, 0x03, 0x67, 0x05, 0xed, 0x0f,
        0x34, 0x02, 0xbc, 0x0a, 0x57, 0x03, 0xbd, 0x09,
        0x00, 0xf0, 0x11, 0xf1, 0x22, 0x02, 0x33, 0x03,
        0x44, 0x04, 0x55, 0x05, 0x88, 0x08, 0x99, 0x09,
        0xaa, 0x0a, 0xbb, 0x0b, 0xcc, 0x0c, 0xdd, 0x0d,
        0xee, 0x0e, 0xff, 0x0f
    }
                                     };

    /**
     * A tiny (8 pixel) test frame, cut & pasted from an old version of TestBufferUtils.
     */
    const auto eightPixelImage = TestData
                                 {
                                     {
                                         0x0102, 0x0304, 0x0567, 0x0fed, 0x0234, 0x0abc, 0x0357, 0x09bd
                                     },
    {
        0x10, 0x30, 0x42, 0x56, 0xfe, 0xd7,
        0x23, 0xab, 0xc4, 0x35, 0x9b, 0xd7
    },
    {
        0x02, 0xf1, 0x04, 0x03, 0x67, 0x05, 0xed, 0x0f,
        0x34, 0x02, 0xbc, 0x0a, 0x57, 0x03, 0xbd, 0x09,
    }
                                 };

    template <typename T>
    std::vector<T> vectorMultiplier (std::size_t multiplier, const std::vector<T> &base)
    {
        std::vector<T> result;
        for (std::size_t i = 0; i < multiplier; i++)
        {
            for (T t : base)
            {
                result.emplace_back (t);
            }
        }
        return result;
    }

    /**
     * The ratio of (maximum image supported by the M2450) / twentyTwoPixelImage.pixels.size().
     * It is an integer ratio, as 352 is a multiple of 22.
     */
    const auto monstarRatio = std::size_t (352 * 287) / twentyTwoPixelImage.pixels.size();

    /**
     * Test image that's the size expected from the M2450 at maximum resolution, as used by the pico
     * maxx and pico monstar.
     */
    const auto monstarImage = TestData
                              {
                                  vectorMultiplier (monstarRatio, twentyTwoPixelImage.pixels),
                                  vectorMultiplier (monstarRatio, twentyTwoPixelImage.raw12),
                                  vectorMultiplier (monstarRatio, twentyTwoPixelImage.raw16)
                              };

    /**
     * Returns the header which the Amundsen or UVC protocol includes in each data payload.
     *
     * It doesn't include the size of the data inself, as that's given by the amount of data
     * transferred over USB.
     */
    std::vector<uint8_t> getUvcHeader (bool frameId, bool endOfFrame)
    {
        auto bmHeaderInfo = static_cast<uint8_t> ( (endOfFrame ? 2 : 0) | (frameId ? 1 : 0));
        return std::vector<uint8_t> {12, bmHeaderInfo, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    /**
     * This is a mock of the CyApi/LibUsb part of BridgeAmundsenCyApi/BridgeAmundsenLibUsb.
     *
     * It can simulate capturing data, with the data to return supplied to setPayloads. Once data
     * has been set, receivePayload will treat m_payloads as a circular buffer, with each call to
     * receivePayload returning the next entry in m_payloads.
     */
    class MockBridgeAmundsenTransport : public BridgeAmundsenCommon, public royaletest::ThreadedAssertSupport
    {
    public:
        MockBridgeAmundsenTransport () :
            BridgeAmundsenCommon ()
        {
        }

        ~MockBridgeAmundsenTransport ()
        {
            stopCapture();
        }

        bool isConnected() const override
        {
            return true;
        }

        PrepareStatus prepareToReceivePayload (uint8_t *first) override
        {
            assertEq (std::ptrdiff_t (0), std::count (m_preparedBuffers.cbegin(), m_preparedBuffers.cend(), first), "address queued twice in prepareToReceivePayloads");
            if (m_preparedBuffers.size() >= m_maxPreparedBuffers)
            {
                return PrepareStatus::ALL_BUFFERS_ALREADY_PREPARED;
            }

            m_preparedBuffers.push_back (first);
            return PrepareStatus::SUCCESS;
        }

        // From BridgeAmundsenCommon
        std::size_t receivePayload (uint8_t *first, std::atomic<bool> &retryOnTimeout) override
        {
            // simulate some very quick I/O
            std::this_thread::yield();

            assertTrue (!m_preparedBuffers.empty(), "receivePayload called with no buffers prepared");
            assertEq (m_preparedBuffers.front(), first, "receivePayload called in unexpected sequence");
            m_preparedBuffers.pop_front();

            std::lock_guard<std::mutex> lock (m_mutex);
            if (m_payloads.empty())
            {
                // nothing received
                return 0;
            }

            const auto &src = m_payloads.at (m_nextPayload);
            m_nextPayload = (m_nextPayload + 1) % m_payloads.size();
            auto destIter = iteratorBoundsCheck (first, first + AMUNDSEN_STRIDE_SIZE, src.size());
            std::copy (src.cbegin(), src.cend(), destIter);
            return src.size();
        }

        void cancelPendingPayloads() override
        {
            m_preparedBuffers.clear();
        }

        /**
         * Set the limit on how many buffers prepareToReceivePayload will queue.
         */
        void setMaxPreparedBuffers (std::size_t maxPreparedBuffers)
        {
            m_maxPreparedBuffers = maxPreparedBuffers;
        }

        /**
         * Set the data that receivePayload will return. Because of the toggling frameId, the number
         * of frames passed must be even.
         *
         * Each frame will be split in to payloads of (header + bytesPerPayload) bytes.
         *
         * Note that splitting a small amount of data in to multiple payloads may cause the data
         * receiving to fail. BridgeAmundsenCommon's must allocate enough resources to capture data
         * from any normal device, and calculates appropriate numbers for both the total size and
         * the number of payloads in executeUseCase. When using test data of only 8 or 22 pixels,
         * it's not reasonable to assume that executeUseCase will allocate more than two payloads.
         *
         * There's also a requirement for each payload (excluding header) to be a multiple of 3
         * bytes for RAW12, or 2 bytes for RAW16.
         */
        void setPayloads (const std::vector<std::vector<uint8_t>> &frames, std::size_t bytesPerPayload)
        {
            assert (frames.size() % 2 == 0);

            std::lock_guard<std::mutex> lock (m_mutex);
            m_payloads.clear();
            m_nextPayload = 0u;
            auto frameId = true;
            for (auto &frame : frames)
            {
                std::size_t i = 0;
                while (i + bytesPerPayload + 1 < frame.size())
                {
                    auto payload = getUvcHeader (frameId, false);
                    for (auto x = i ; x < i + bytesPerPayload ; x++)
                    {
                        payload.push_back (frame.at (x));
                    }
                    m_payloads.push_back (std::move (payload));
                    i += bytesPerPayload;
                }

                // last payload for this frame
                auto payload = getUvcHeader (frameId, true);
                for (auto x = i ; x < frame.size() ; x++)
                {
                    payload.push_back (frame.at (x));
                }
                m_payloads.push_back (std::move (payload));

                // prepare for next frame
                frameId = !frameId;
            }
        }

    private:
        /** Synchronization between receivePayload and setPayloads */
        std::mutex m_mutex;
        /** Data being sent from the simulated device, accessed with m_mutex held */
        std::vector<std::vector<uint8_t>> m_payloads;
        /** Index in to m_payloads which will be used for the next receive, accessed with m_mutex held */
        std::size_t m_nextPayload = 0u;
        /**
         * Tracking prepareToReceivePayloads vs receivePayload.  Each call to receivePayload should
         * match pointer at the front of this buffer.
         *
         * Uses std::deque instead af std::queue because prepareToReceivePayloads needs to access
         * all elements to check that an address hasn't been added twice.
         */
        std::deque<uint8_t *> m_preparedBuffers;
        /**
         * The size of m_preparedBuffers at which prepareToReceivePayload returns
         * ALL_BUFFERS_ALREADY_PREPARED
         */
        std::size_t m_maxPreparedBuffers = std::numeric_limits<std::size_t>::max();
    };

    class MockBufferCaptureListener : public IBufferCaptureListener
    {
    public:
        explicit MockBufferCaptureListener (std::shared_ptr<IBufferCaptureReleaser> releaser) :
            m_releaser (releaser)
        {
        }

        void bufferCallback (royale::hal::ICapturedBuffer *buffer) override
        {
            auto vec = std::vector<uint16_t> (buffer->getPixelData(), buffer->getPixelData() + buffer->getPixelCount());
            checkBuffer (vec);
            m_releaser->queueBuffer (buffer);

            {
                std::lock_guard<std::mutex> lock (m_mutex);
                m_counterCallbacks++;
            }
            m_cv.notify_one();
        }

        /**
         * Wrapped method for comparing the received data to the expected data.
         */
        MOCK_METHOD1 (checkBuffer, void (const std::vector<uint16_t> &));

        void releaseAllBuffers () override
        {
            // they're already released synchronously by bufferCallback
        }

        /**
         * Wait until bufferCallback has been called expectation times, or maxDelay expires.
         *
         * \return the number of times that bufferCallback has been called.
         */
        size_t getCounterCallbacks (size_t expectation, std::chrono::milliseconds maxDelay)
        {
            std::unique_lock<std::mutex> lock {m_mutex};
            m_cv.wait_for (lock, maxDelay, [this, expectation] { return m_counterCallbacks >= expectation; });
            return m_counterCallbacks;
        }

    private:
        std::shared_ptr<IBufferCaptureReleaser> m_releaser;
        unsigned int m_counterCallbacks = 0u;
        std::mutex m_mutex;
        std::condition_variable m_cv;
    };
}

class TestBridgeAmundsen : public ::testing::Test
{
protected:
    TestBridgeAmundsen()
    {

    }

    virtual ~TestBridgeAmundsen()
    {

    }

    virtual void SetUp()
    {
        m_bridge.reset (new MockBridgeAmundsenTransport);

        // calls normally made by the BridgeFactoryAmundsenCyApi or BridgeFactoryAmundsenLibUsb
        m_bridge->setUsbSpeed (royale::usb::pal::UsbSpeed::HIGH);
        m_bridge->setTransferFormat (royale::buffer::BufferDataFormat::RAW12);
        m_bridge->setUvcBridgeInfo ({{"UVC_FIRMWARE_VERSION", "simulated"}});
        // the setTransferFormat can be overridden by calling it again at the start of a test

        m_listener = std::make_shared<MockBufferCaptureListener> (m_bridge);
        ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (m_listener.get()));
    }

    virtual void TearDown()
    {
        if (m_listener)
        {
            m_bridge->setBufferCaptureListener (nullptr);
        }
        m_listener.reset();
        m_bridge.reset();
    }

    /**
     * A holder with a lifetime that lasts until the setBufferCaptureListener (nullptr) call.
     * Because setBufferCaptureListener takes a bare pointer, and because the capture will be
     * running in the bridge's acquisition thread, we need to ensure that any listener is still
     * valid even if the test exits because of a failing assertion.
     *
     * If set, TearDown() will unregister the listener.
     */
    std::shared_ptr<MockBufferCaptureListener> m_listener;
    std::shared_ptr<MockBridgeAmundsenTransport> m_bridge;
};

TEST_F (TestBridgeAmundsen, ExecuteUseCase)
{
    // the magic numbers are arbitrary width, height and buffer count
    ASSERT_NO_THROW (m_bridge->executeUseCase (100, 5, 4));
}

/**
 * If executeUseCase is called while capturing, it should either succeed or throw.
 *
 * The failure cases here would be waiting for the acquisition thread's I/O read to time out,
 * or deadlocking.  Neither is expected to happen, this is expected to take much less than
 * the allowed 50 milliseconds.
 */
TEST_F (TestBridgeAmundsen, ExecuteUseCaseWhileRunning)
{
    auto start = std::chrono::steady_clock::now();

    // the magic numbers are arbitrary width, height and buffer count
    ASSERT_NO_THROW (m_bridge->executeUseCase (100, 5, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    ASSERT_THROW (m_bridge->executeUseCase (100, 5, 4), NotImplemented);

    auto stop = std::chrono::steady_clock::now();

    ASSERT_LE (std::chrono::duration_cast<std::chrono::milliseconds> (stop - start).count(), 50);
}

TEST_F (TestBridgeAmundsen, CaptureFrameInSinglePayload)
{
    using namespace ::testing;
    const auto &testData = twentyTwoPixelImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    m_bridge->setPayloads ({rawData, rawData}, rawData.size());

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Test receiving data in two payloads of (header + 6 bytes) each.
 *
 * This also tests that the bridge allocates at least 2 strides, even for small images.
 */
TEST_F (TestBridgeAmundsen, TwoPayloadsAllSameSize)
{
    using namespace ::testing;
    const auto testData = eightPixelImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    assert (rawData.size() % 6u == 0u); // Test assumes that sample data is exactly 12 bytes
    m_bridge->setPayloads ({rawData, rawData}, 6u);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Test receiving data in two payloads of (header + 27 bytes) and (header + 6) bytes.
 *
 * This also tests that the bridge allocates at least 2 strides, even for small images.
 */
TEST_F (TestBridgeAmundsen, TwoPayloadsLastDifferentSize)
{
    using namespace ::testing;
    const auto &testData = twentyTwoPixelImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    assert (rawData.size() == 33u); // Test assumes that sample data is exactly 33 bytes
    m_bridge->setPayloads ({rawData, rawData}, 27u);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Test receiving RAW12 data in payloads of (header + about 12kbytes data), the payload size in this
 * test is an arbitrary number that could be used by a real device.
 */
TEST_F (TestBridgeAmundsen, MultiplePayloadsRaw12)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    assert (rawData.size() == (352 * 287 * 3) / 2);
    assert (testPixels.size() == 352 * 287);
    m_bridge->setPayloads ({rawData, rawData}, 0x2c0du);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Test receiving RAW16 data in payloads of (header + about 12kbytes data), the payload size in this
 * test is an arbitrary number that could be used by a real device.
 */
TEST_F (TestBridgeAmundsen, MultiplePayloadsRaw16)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw16;
    const auto &testPixels = testData.pixels;

    assert (rawData.size() == 352 * 287 * 2);
    assert (testPixels.size() == 352 * 287);
    m_bridge->setTransferFormat (royale::buffer::BufferDataFormat::RAW16);
    m_bridge->setPayloads ({rawData, rawData}, 0x2c10u);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Test receiving data in payloads of exactly 16kB (including the header).  The CX3 based Amundsen
 * images don't use such sizes, even in RAW16 mode, but the FX3 code used 16kB - 4 bytes.
 *
 * This test must use RAW16 data, as BufferUtils::copyAndNormalizeStrides requires that the size be
 * a multiple of 3 bytes for RAW12.
 */
TEST_F (TestBridgeAmundsen, SixteenKilobytePayloads)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw16;
    const auto &testPixels = testData.pixels;

    m_bridge->setTransferFormat (royale::buffer::BufferDataFormat::RAW16);
    m_bridge->setPayloads ({rawData, rawData}, 0x4000u - 0x12u);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Run the MultiplePayloadsLastDifferentSize test, but limit the number of buffers that can be
 * prepared to one at a time. This doesn't change the amount of data that can be in the image, it
 * just requires the bridge to prepare more buffers in the middle of capturing an image.
 */
TEST_F (TestBridgeAmundsen, LimitedPreparedBuffers)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    m_bridge->setMaxPreparedBuffers (1);
    m_bridge->setPayloads ({rawData, rawData}, 0x2fcdu);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Run the MultiplePayloadsRaw12 test, but require the bridge to auto-detect the format.
 */
TEST_F (TestBridgeAmundsen, AutoDetectRaw12)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw12;
    const auto &testPixels = testData.pixels;

    m_bridge->setTransferFormat (royale::buffer::BufferDataFormat::UNKNOWN);
    m_bridge->setPayloads ({rawData, rawData}, 0x2fcdu);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::milliseconds (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}

/**
 * Run the MultiplePayloadsRaw16 test, but require the bridge to auto-detect the format.
 */
TEST_F (TestBridgeAmundsen, AutoDetectRaw16)
{
    using namespace ::testing;
    const auto &testData = monstarImage;
    const auto &rawData = testData.raw16;
    const auto &testPixels = testData.pixels;

    m_bridge->setTransferFormat (royale::buffer::BufferDataFormat::UNKNOWN);
    m_bridge->setPayloads ({rawData, rawData}, 0x2fd0u);

    EXPECT_CALL (*m_listener, checkBuffer (Eq (testPixels)))
    .Times (AtLeast (1))
    .WillRepeatedly (Return());

    ASSERT_NO_THROW (m_bridge->executeUseCase (static_cast<int> (testPixels.size()), 1, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    m_listener->getCounterCallbacks (2, std::chrono::hours (10));
    ASSERT_NO_THROW (m_bridge->setBufferCaptureListener (nullptr));
}
