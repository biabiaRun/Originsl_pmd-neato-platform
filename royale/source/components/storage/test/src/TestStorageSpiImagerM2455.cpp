/****************************************************************************\
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/SpiBusMasterM2455.hpp>
#include <storage/SpiGenericFlash.hpp>

#include <common/EndianConversion.hpp>
#include <common/NarrowCast.hpp>

#include <SimImagerM2455.hpp>
#include <StubBridgeImager.hpp>
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
using namespace royale::stub;
using namespace testing;

namespace
{
    const auto testSpiConfig = FlashMemoryConfig
                               {
                                   FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                               }
                               .setImageSize (100 * 1024);
}

/**
 * Base class for testing the simulation of an M2455 with WS25Q attached via the M2455's SPI bus.
 *
 * The history of the name "TestStorageSpiImagerM2453" is documented in the M2453 version of this
 * file. The M2455 tests continue with this name scheme, because what's being tested is the
 * SPI-via-Imager stack in addition to the SpiGenericFlash class.
 */
class TestStorageSpiImagerM2455 : public ::testing::Test
{
protected:
    TestStorageSpiImagerM2455 ()
    {
    }

    virtual ~TestStorageSpiImagerM2455()
    {
    }

    virtual void SetUp()
    {
        m_simImager = std::make_shared<SimImagerM2455> ();
        m_bridge = std::make_shared<StubBridgeImager> (m_simImager);
    }

    virtual void TearDown()
    {
        m_bridge.reset();
        m_simImager.reset();
    }

    const ImagerAsBridgeType m_imagerType {ImagerAsBridgeType::M2455_SPI};
    const SensorRoutingConfigSpi m_busAddress {ONLY_DEVICE_ON_IMAGERS_SPI};

    std::shared_ptr <StubBridgeImager> m_bridge;
    std::shared_ptr <SimImagerM2455> m_simImager;
};

/**
 * Test that reading succeeds, when the data to be read is already in the simulated flash.
 */
TEST_F (TestStorageSpiImagerM2455, TestRead)
{
    std::vector<uint8_t> testPattern;
    auto flash = m_simImager->getFlashMemorySpace();
    for (uint32_t i = 0u; i < 2100; i++)
    {
        testPattern.emplace_back (static_cast<uint8_t> ( (i >> 8) + i));
        (*flash) [i] = testPattern[i];
    }

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (testSpiConfig, spiMaster, m_busAddress);

    std::vector<uint8_t> data (testPattern.size(), 0);
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
 * Test that reading fails when it exceeds the configured image size.
 */
TEST_F (TestStorageSpiImagerM2455, TestOutOfConfigBoundsRead)
{
    const auto limit = narrow_cast<int32_t> (testSpiConfig.imageSize);

    // The simulation has a check for trying to read addresses that haven't been initialized. So put
    // data in to the simulated space.
    auto flash = m_simImager->getFlashMemorySpace();
    for (int32_t i = -100; i < 100; i++)
    {
        (*flash) [narrow_cast<uint32_t> (limit + i)] = 0;
    }

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (testSpiConfig, spiMaster, m_busAddress);

    std::vector<uint8_t> data (10, 0);
    ASSERT_NO_THROW (storage->readStorage (static_cast<uint32_t> (limit - 80), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit - 8), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit + 8), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit + 80), data));
    ASSERT_ANY_THROW (storage->readStorage (std::numeric_limits<uint32_t>::max(), data));
}

/**
 * Test that reading fails when the address would wrap-around when packed in to 24 bits. This is
 * also the configured limit, as the constructor will throw a logic error if the config is
 * impossible (see TestOutOf24BitBoundsCreation).
 */
TEST_F (TestStorageSpiImagerM2455, TestOutOf24BitBoundsRead)
{
    const auto spiConfig = FlashMemoryConfig{testSpiConfig} .setImageSize (1 << 24);
    const auto limit = narrow_cast<int32_t> (spiConfig.imageSize);

    // The simulation has a check for trying to read addresses that haven't been initialized. So put
    // data in to the simulated space.
    auto flash = m_simImager->getFlashMemorySpace();
    for (int32_t i = -100; i < 100; i++)
    {
        (*flash) [narrow_cast<uint32_t> (limit + i)] = 0;
    }

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (spiConfig, spiMaster, m_busAddress);

    std::vector<uint8_t> data (10, 0);
    ASSERT_NO_THROW (storage->readStorage (static_cast<uint32_t> (limit - 80), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit - 8), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit + 8), data));
    ASSERT_ANY_THROW (storage->readStorage (static_cast<uint32_t> (limit + 80), data));
    ASSERT_ANY_THROW (storage->readStorage (std::numeric_limits<uint32_t>::max(), data));
}

/**
 * Test that SpiGenericFlash's constructor throws if the FlashMemoryConfig's image size could not
 * fit in to 24 bits.
 */
TEST_F (TestStorageSpiImagerM2455, TestOutOf24BitBoundsCreation)
{
    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    const auto spiConfig = FlashMemoryConfig{testSpiConfig} .setImageSize ( (1 << 24) + 1);
    ASSERT_THROW (std::make_shared<SpiGenericFlash> (spiConfig, spiMaster, m_busAddress), LogicError);
}

/**
 * Writes a test pattern which large enough that it requires two 4kB sectors to be erased, reads it
 * back and checks the data.
 *
 * If this fails, reducing the number 5000 to a small test pattern of only a few bytes would touch
 * fewer edge-cases, and might be a good test to start debugging. For debugging, note that the test
 * pattern jumps every 256 entries, a non-sequential number might not be an off-by-one bug.
 */
TEST_F (TestStorageSpiImagerM2455, TestWriteAndReadBack)
{
    // As this is a writing test, it should avoid the M2455's protected area, even though the
    // SimImagerM2455 doesn't yet simulate the protection.
    const auto unprotected = 0x1000;
    const auto spiConfig = FlashMemoryConfig{testSpiConfig} .setAccessOffset (unprotected);

    std::vector<uint8_t> testPattern;
    for (auto i = 0u; i < 5000; i++)
    {
        testPattern.emplace_back (static_cast<uint8_t> ( (i >> 8) + i));
    }

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (spiConfig, spiMaster, m_busAddress);

    ASSERT_NO_THROW (storage->writeSectorBased (0u, testPattern));

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
 * Test that writing fails when it exceeds the configured image size.
 */
TEST_F (TestStorageSpiImagerM2455, TestOutOfConfigBoundsWrite)
{
    const auto limit = narrow_cast<int32_t> (testSpiConfig.imageSize);

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (testSpiConfig, spiMaster, m_busAddress);

    // Writes need to be sector-aligned, so the test data is bigger than the read tests
    std::vector<uint8_t> data (5000, 0);
    ASSERT_GE (limit, 8192) << "Error in test code, will underflow in the (limit - 8192) case";
    ASSERT_NO_THROW (storage->writeSectorBased (static_cast<uint32_t> (limit - 8192), data));
    ASSERT_ANY_THROW (storage->writeSectorBased (static_cast<uint32_t> (limit - 4096), data));
    ASSERT_ANY_THROW (storage->writeSectorBased (static_cast<uint32_t> (limit), data));
    ASSERT_ANY_THROW (storage->writeSectorBased (static_cast<uint32_t> (limit + 4096), data));
    ASSERT_ANY_THROW (storage->writeSectorBased (std::numeric_limits<uint32_t>::max(), data));
}

/**
 * Test that the storage access offset is implemented
 */
TEST_F (TestStorageSpiImagerM2455, TestReadWithAccessOffset)
{
    auto config = testSpiConfig;
    config.setAccessOffset (64 * 1024);

    std::vector<uint8_t> testPattern;
    auto flash = m_simImager->getFlashMemorySpace();
    for (uint32_t i = 0u; i < 2100; i++)
    {
        testPattern.emplace_back (static_cast<uint8_t> ( (i >> 8) + i));
        (*flash) [narrow_cast<uint32_t> (config.accessOffset + i)] = testPattern[i];
    }

    auto spiMaster = std::make_shared<SpiBusMasterM2455> (m_bridge, m_imagerType);
    auto storage = std::make_shared<SpiGenericFlash> (config, spiMaster, m_busAddress);

    std::vector<uint8_t> data (testPattern.size(), 0);
    ASSERT_NO_THROW (storage->readStorage (0, data));
    for (auto i = 0u; i < testPattern.size(); i++)
    {
        ASSERT_EQ (testPattern.at (i), data.at (i));
    }
}
