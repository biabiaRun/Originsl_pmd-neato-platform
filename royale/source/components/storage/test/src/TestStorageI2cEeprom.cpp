/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageI2cEeprom.hpp>

#include <common/SensorRoutingConfigI2c.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <string.h>

using namespace royale::common;
using namespace royale::config;
using namespace royale::pal;
using namespace royale::storage;

namespace
{
    class MockI2cAccess : public II2cBusAccess
    {
    public:
        MockI2cAccess () = default;
        ~MockI2cAccess() = default;

        void readI2c (uint8_t devAddr,
                      I2cAddressMode addrMode,
                      uint16_t regAddr,
                      std::vector<uint8_t> &buffer) override
        {
            // almost no-op for the current test
            memset (buffer.data(), 0, buffer.size());
        }

        void writeI2c (uint8_t devAddr,
                       I2cAddressMode addrMode,
                       uint16_t regAddr,
                       const std::vector<uint8_t> &buffer) override
        {
            // no-op for the current test
        }

        void setBusSpeed (uint32_t bps) override
        {
        }

        std::size_t maximumDataSize () override
        {
            return 4096;
        }
    };

    /** A 128kB EEPROM (1 bit in the I2C address) */
    class MockEeprom : public MockI2cAccess
    {
        /**
         * I2C address for accessing the lowest bytes of memory.
         */
        uint8_t m_devAddr;

        FlashMemoryConfig m_config;
        std::vector<uint8_t> m_cells;

    public:
        /**
         * The number of address bits which will come from the regAddr,
         * instead of from the I2C address.
         */
        static const unsigned int LOW_BITS = 16;
        static const std::size_t ADDRESS_MASK = 0x1;
        static const std::size_t IMAGE_SIZE = 0x20000;

        explicit MockEeprom (uint8_t addr) :
            m_devAddr {addr},
            m_cells (IMAGE_SIZE, 0u)
        {
        }

        ~MockEeprom() override = default;

        void readI2c (uint8_t devAddr,
                      I2cAddressMode addrMode,
                      uint16_t regAddr,
                      std::vector<uint8_t> &buffer) override
        {
            ASSERT_EQ (m_devAddr, devAddr & ~ADDRESS_MASK);
            std::size_t offset = (devAddr << LOW_BITS) | regAddr;
            offset &= ADDRESS_MASK;

            ASSERT_LE (offset + buffer.size(), m_cells.size());
            std::memcpy (&buffer[0], &m_cells[offset], buffer.size());
        }

        void writeI2c (uint8_t devAddr,
                       I2cAddressMode addrMode,
                       uint16_t regAddr,
                       const std::vector<uint8_t> &buffer) override
        {
            ASSERT_EQ (m_devAddr, devAddr & ~ADDRESS_MASK);
            std::size_t offset = (devAddr << LOW_BITS) | regAddr;
            offset &= ADDRESS_MASK;

            ASSERT_LE (offset + buffer.size(), m_cells.size());
            std::memcpy (&m_cells[offset], &buffer[0], buffer.size());
        }
    };

}

/**
 * Fixture with two separate tools - one for tests that just need I2C access, and one for tests that
 * need the full simulated EEPROM.
 */
class TestStorageI2cEeprom : public ::testing::Test
{
protected:
    TestStorageI2cEeprom ()
    {
    }

    virtual ~TestStorageI2cEeprom()
    {
    }

    virtual void SetUp()
    {
        m_i2cAccess = std::make_shared<MockI2cAccess> ();
        // this does not set up m_eeprom, because the test needs to configure it
    }

    virtual void TearDown()
    {
        m_i2cAccess.reset();
        m_eeprom.reset();
    }

    std::shared_ptr<royale::pal::II2cBusAccess> m_i2cAccess;
    std::shared_ptr<MockEeprom> m_eeprom;
};

/**
 * Test that the I2C EEPROM support throws an exception when given a non-zero accessOffset, because
 * the accessOffset is not yet supported.  This test will need to be replaced, and the
 * MockI2cAccess's simulation improved, when support for the accessOffset is added to the I2C EEPROM
 * implementation.
 *
 * The MockI2cAccess currently returns success for 8-bit I2C accesses, to any I2C address.
 */
TEST_F (TestStorageI2cEeprom, TestI2cFlashAccessOffsetNotImplemented)
{
    auto routing = SensorRoutingConfigI2c {0x48};

    // These don't throw an exception; they're tested here to check that the exception in the second
    // part of the test is caused by the accessOffset
    {
        auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                      .setImageSize (512 * 1024)
                      .setPageSize (256)
                      .setSectorSize (8 * 1024);

        auto storage = std::make_shared<StorageI2cEeprom> (config, m_i2cAccess, routing.getAddress());
        auto testPattern = std::vector<uint8_t> (10000, 0);
        ASSERT_NO_THROW (storage->writeStorage (testPattern));
        ASSERT_NO_THROW (storage->readStorage (0, testPattern));
    }

    // Now check that the exception is throw when an accessOffset is set
    {
        auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                      .setImageSize (512 * 1024)
                      .setPageSize (256)
                      .setSectorSize (8 * 1024)
                      .setAccessOffset (64 * 1024);

        std::shared_ptr<StorageI2cEeprom> storage;
        try
        {
            storage = std::make_shared<StorageI2cEeprom> (config, m_i2cAccess, routing.getAddress());
        }
        catch (const NotImplemented &)
        {
            // successfully threw the exception in the constructor, storage == nullptr
        }

        // if the constructor doesn't throw, both read and write must throw
        if (storage)
        {
            auto testPattern = std::vector<uint8_t> (10000, 0);
            ASSERT_THROW (storage->writeStorage (testPattern), NotImplemented);
            ASSERT_THROW (storage->readStorage (0, testPattern), NotImplemented);
        }
    }
}

/**
 * Checks that writing waits for the writeTime, and that reading doesn't.
 */
TEST_F (TestStorageI2cEeprom, TestI2cWriteTime)
{
    using namespace std::chrono;

    const auto routing = SensorRoutingConfigI2c {0x48};
    m_eeprom = std::make_shared<MockEeprom> (routing.getAddress());

    const auto writeTime = std::chrono::microseconds {100};
    const auto pageCount = 100;
    const auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM}
                        .setImageSize (MockEeprom::IMAGE_SIZE)
                        .setPageSize (256)
                        .setWriteTime (writeTime);

    auto storage = std::make_shared<StorageI2cEeprom> (config, m_eeprom, routing.getAddress());
    auto testPattern = std::vector<uint8_t> (256 * pageCount, 0);
    const auto start = steady_clock::now();
    ASSERT_NO_THROW (storage->writeStorage (testPattern));
    const auto mid = steady_clock::now();
    ASSERT_NO_THROW (storage->readStorage (0, testPattern));
    const auto finish = steady_clock::now();

    ASSERT_GT (duration_cast<microseconds> (mid - start), (pageCount - 1) * writeTime);
    ASSERT_LT (duration_cast<microseconds> (finish - mid), (pageCount - 1) * writeTime);
}
