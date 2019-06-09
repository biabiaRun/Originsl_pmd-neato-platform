/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageSpiImagerM2452.hpp>

#include <common/EndianConversion.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>

#include <MockBridgeImagerSpiStorage.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

using namespace royale::common;
using namespace royale::config;
using namespace royale::storage;
using namespace royale::stub::storage;
using namespace testing;

namespace
{
    /**
     * This overrides the default timings in StorageSpiImagerM2452 and makes the unit test fast.
     */
    const auto testSpiConfig = FlashMemoryConfig
                               {
                                   FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                               }
                               .setImageSize (100 * 1024)
                               .setEraseTime (std::chrono::microseconds (1))
                               .setReadTime (std::chrono::microseconds (1))
                               .setWriteTime (std::chrono::microseconds (1));
}

class TestStorageSpiImagerM2452 : public ::testing::Test
{
protected:
    TestStorageSpiImagerM2452 ()
        : m_routing (static_cast<ImagerAsBridgeType> (0))
    {
    }

    virtual ~TestStorageSpiImagerM2452()
    {
    }

    virtual void SetUp()
    {
        m_bridgeImager = std::make_shared<MockBridgeImagerSpiStorage> (testSpiConfig.imageSize);
        m_routing = MockBridgeImagerSpiStorage::getSimulatedImagerType();
    }

    virtual void TearDown()
    {
        m_bridgeImager.reset();
    }

    std::shared_ptr<MockBridgeImagerSpiStorage> m_bridgeImager;
    ImagerAsBridgeType m_routing;
};

/**
 * Writes a test pattern which large enough that it requires two 4kB sectors to be erased,
 * reads it back and checks the data.
 *
 * If this fails, reducing the number 2100 to a small test pattern of only a few bytes would touch
 * fewer edge-cases, and might be a good test to start debugging.
 */
TEST_F (TestStorageSpiImagerM2452, TestWriteAndReadBack)
{
    std::vector<uint8_t> testPattern;
    for (auto i = 0u; i < 2100; i++)
    {
        pushBackBe16 (testPattern, static_cast<uint16_t> (i));
    }
    auto storage = std::make_shared<StorageSpiImagerM2452> (testSpiConfig, m_bridgeImager, m_routing);
    ASSERT_NO_THROW (storage->writeStorage (testPattern));

    std::vector<uint8_t> data (testPattern.size() + 100, 0);
    ASSERT_NO_THROW (storage->readStorage (0, data));
    for (auto i = 0u; i < testPattern.size(); i++)
    {
        ASSERT_EQ (testPattern.at (i), data.at (i));
    }

    // Check that a read with an odd number of bytes works too
    data.clear ();
    data.resize (257, 0);
    ASSERT_LE (data.size(), testPattern.size()) << "Error in test code, read exceeds testPattern";
    ASSERT_NO_THROW (storage->readStorage (0, data));
    for (auto i = 0u; i < data.size(); i++)
    {
        ASSERT_EQ (testPattern.at (i), data.at (i));
    }

}

/**
 * Test that the storage throws an exception when given a non-zero accessOffset, because
 * the accessOffset is not yet supported.  This test will need to be replaced if support for
 * accessOffset is added.
 */
TEST_F (TestStorageSpiImagerM2452, TestAccessOffsetNotImplemented)
{
    auto config = testSpiConfig;
    config.setAccessOffset (64 * 1024);

    std::shared_ptr<StorageSpiImagerM2452> storage;
    try
    {
        storage = std::make_shared<StorageSpiImagerM2452> (config, m_bridgeImager, m_routing);
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

/**
 * Test that the imager design step is checked, and a mismatch causes an exception to be thrown.
 */
TEST_F (TestStorageSpiImagerM2452, TestDesignStepMismatch)
{
    m_bridgeImager->setDesignStep (0x123);

    std::shared_ptr<StorageSpiImagerM2452> storage;
    try
    {
        storage = std::make_shared<StorageSpiImagerM2452> (testSpiConfig, m_bridgeImager, m_routing);
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

/**
 * Test that the image size limit causes an exception to be thrown.
 *
 * For this test, the storage class is configured with a very small memory, but the simulation is
 * still simulating a larger "physical" device than the storage class believes it has, so that any
 * exceptions are caused by checks in the storage class, not from the simulation.
 */
TEST_F (TestStorageSpiImagerM2452, TestSizeLimit)
{
    auto config = testSpiConfig;
    config.setImageSize (4 * 1024)
    .setPageSize (256)
    .setSectorSize (4 * 1024);

    auto storage = std::make_shared<StorageSpiImagerM2452> (config, m_bridgeImager, m_routing);

    // The size of the test pattern is arbitrary.
    auto testPattern = std::vector<uint8_t> (config.imageSize + 100, 0);
    ASSERT_ANY_THROW (storage->writeStorage (testPattern));
}

/**
 * Sets the imager to never signal that the read is complete, check that the situation is detected
 * and there's a timeout in StorageSpiImagerM2452.
 */
TEST_F (TestStorageSpiImagerM2452, TestReadTimeout)
{
    std::size_t testSize = m_bridgeImager->setWillBrownoutDuringOperation ();
    auto storage = std::make_shared<StorageSpiImagerM2452> (testSpiConfig, m_bridgeImager, m_routing);

    std::vector<uint8_t> data (testSize + 100, 0);
    ASSERT_ANY_THROW (storage->readStorage (0, data));
}
