/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeEnclustra.hpp>

#include <EndianConversion.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <chrono>
#include <memory>

using namespace royale::usb::bridge;
using namespace royale::common;
using namespace royale::buffer;

namespace
{
    /** First register of a block of registers for the read test */
    const uint16_t READ_BASE_REGISTER_ADDRESS = 0xA000;
    /** Number of registers for the read test */
    const uint16_t COUNT_REGISTERS_TO_READ = 500;

    /**
     * This is a mock of the CyApi/LibUsb part of BridgeEnclustraCyApi/BridgeEnclustraLibUsb.
     */
    class MockBridgeEnclustraTransport : public BridgeEnclustra
    {
    public:
        MockBridgeEnclustraTransport () :
            BridgeEnclustra ()
        {
        }

        ~MockBridgeEnclustraTransport ()
        {
            stopCapture();
        }

        bool isConnected() const override
        {
            return true;
        }

        float getPeakTransferSpeed() override
        {
            // USB3 SuperSpeed / bits per pixel
            return 5000e6f / 16;
        }

        void doCommand (EnclustraCommand command, const std::vector<uint8_t> &cmdBuffer,
                        std::vector<uint8_t> *recvBuffer,
                        const std::vector<uint8_t> *sendBuffer) override
        {
            ASSERT_EQ (uint32_t (command), bufferToHost32 (&cmdBuffer[0]));
            switch (command)
            {
                case EnclustraCommand::READ_I2C:
                    {
                        uint16_t firstRegister = bufferToHostBe16 (&cmdBuffer[13]);
                        std::size_t registersToWrite = recvBuffer->size() / 2;

                        ASSERT_GE (firstRegister, READ_BASE_REGISTER_ADDRESS)
                                << "Accessing an unexpected register (too low)";
                        ASSERT_LE (firstRegister + registersToWrite, size_t (READ_BASE_REGISTER_ADDRESS + COUNT_REGISTERS_TO_READ))
                                << "Accessing an unexpected register (too high)";

                        recvBuffer->clear();
                        for (std::size_t i = 0; i < registersToWrite; i++)
                        {
                            pushBackBe16 (*recvBuffer, static_cast<uint16_t> (firstRegister + i));
                        }
                        break;
                    }
                case EnclustraCommand::START_IMAGER:
                case EnclustraCommand::STOP_IMAGER:
                    {
                        break;
                    }
                default:
                    {
                        throw NotImplemented();
                    }
            }
        }

        size_t getCommandMessageSize () override
        {
            // A small number makes the register test test the splitting-in-to-separate-atoms code
            return 40;
        }

        bool receiveRawFrame (OffsetBasedCapturedBuffer &buffer, size_t offset) override
        {
            // simulate some very quick I/O
            std::this_thread::yield();

            // didn't capture a frame
            return false;
        }
    };
}

class TestBridgeEnclustra : public ::testing::Test
{
protected:
    TestBridgeEnclustra()
        : m_bridgeImager (nullptr)
    {

    }

    virtual ~TestBridgeEnclustra()
    {

    }

    virtual void SetUp()
    {
        m_bridge.reset (new MockBridgeEnclustraTransport);

        m_bridgeImager = dynamic_cast<royale::hal::IBridgeImager *> (m_bridge.get());
        ASSERT_NE (m_bridgeImager, nullptr);
    }

    virtual void TearDown()
    {
        m_bridgeImager = nullptr;
        m_bridge.reset();
    }

    std::unique_ptr <BridgeEnclustra> m_bridge;
    royale::hal::IBridgeImager *m_bridgeImager;
};

/**
 * Tests that reading registers in a burst gives the same results as reading them individually.
 */
TEST_F (TestBridgeEnclustra, BurstRegisterRead)
{
    std::vector<uint16_t> values;
    values.resize (COUNT_REGISTERS_TO_READ);

    ASSERT_NO_THROW (m_bridgeImager->readImagerBurst (READ_BASE_REGISTER_ADDRESS, values));

    // Some may be zero, but not all.
    ASSERT_NE (std::count (values.begin(), values.end(), 0), COUNT_REGISTERS_TO_READ);

    for (uint16_t i = 0; i < COUNT_REGISTERS_TO_READ ; i++)
    {
        uint16_t value;
        uint16_t address = static_cast<uint16_t> (READ_BASE_REGISTER_ADDRESS + i);
        ASSERT_NO_THROW (m_bridgeImager->readImagerRegister (address, value));
        ASSERT_EQ (values[i], value);
    }
}

TEST_F (TestBridgeEnclustra, CaptureFrame)
{
    ASSERT_NO_THROW (m_bridge->startCapture());
}

TEST_F (TestBridgeEnclustra, ExecuteUseCase)
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
TEST_F (TestBridgeEnclustra, ExecuteUseCaseWhileRunning)
{
    auto start = std::chrono::steady_clock::now();

    // the magic numbers are arbitrary width, height and buffer count
    ASSERT_NO_THROW (m_bridge->executeUseCase (100, 5, 4));
    ASSERT_NO_THROW (m_bridge->startCapture());
    ASSERT_THROW (m_bridge->executeUseCase (100, 5, 4), NotImplemented);

    auto stop = std::chrono::steady_clock::now();

    ASSERT_LE (std::chrono::duration_cast<std::chrono::milliseconds> (stop - start).count(), 50);
}
