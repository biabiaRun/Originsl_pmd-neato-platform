/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/ModuleConfigFactoryZwetschge.hpp>

#include <common/FileSystem.hpp>
#include <common/SensorRoutingConfigSpi.hpp>
#include <config/SensorRoutingFilename.hpp>
#include <FactoryTestHelpers.hpp>
#include <StorageTestUtils.hpp>
#include <ZwetschgeTestUtils.hpp>

#include <gmock/gmock.h>

#include <cstdint>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::test::utils;
using namespace testing;

namespace
{
    class MockBridgeFactory : public IBridgeFactory
    {
        MOCK_METHOD0 (initialize, void());
    };
}

/**
 * Read Zwetschge from the filesystem, and get the productId from the Zwetschge file.
 *
 * This is a real-world scenario, where an external process (which runs before Royale) reads the
 * storage and creates a cache.
 * - In this scenario, the imager-attached storage can't be read by Royale
 * - Royale trusts the external process to have correctly copied the data (if the file exists,
 *   it matches the flash).
 *
 * This test is similar to TestNonVolatileStorageFactory.ConstructZwetschgeToReadRealFile4Arg.
 */
TEST (TestModuleConfigFactoryZwetschge, ConstructZwetschgeFromFilesystem)
{
    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    auto minimalConfig5678 = getMinimalModuleConfig();
    minimalConfig5678.coreConfigData.cameraName = "Camera5678";

    const auto probeConfig = royale::Vector<ModuleConfigFactoryZwetschge::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { idOfZwetschgeExampleFlashImage(), minimalConfig34 },
        { {0x05, 0x06, 0x07, 0x08}, minimalConfig5678 }
    };

    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    MockBridgeFactory bridgeFactory;
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::ZWETSCHGE);
    const auto refDataFile = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
    const auto routing = std::make_shared<SensorRoutingFilename> (refDataFile.zwetschgeFile);

    ModuleConfigFactoryZwetschge confFactory (routing, config, probeConfig);
    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
    ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    ASSERT_NE (nullptr, probedConfig->imagerConfig.externalImagerConfig);
    ASSERT_FALSE (probedConfig->coreConfigData.useCases.empty());
}

/**
 * This test simulates a device on which ModuleConfigFactoryByStorageIdBase can successfully call
 * getModuleIdentifier(), and then checks that it returns the expected configuration.
 *
 * This test simulates a USB-connected device with a MiraBelle imager, and with the product code
 * and calibration storage connected to the imager's SPI.
 *
 * The static_cast<IModuleConfigFactory *> upcasts are explained in TestModuleConfigFactoryFixed.
 */
TEST (TestModuleConfigFactoryZwetschge, ConstructZwetschgeToReadM2453Spi)
{
    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    auto minimalConfig5678 = getMinimalModuleConfig();
    minimalConfig5678.coreConfigData.cameraName = "Camera5678";

    const auto probeConfig = royale::Vector<ModuleConfigFactoryZwetschge::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { idOfZwetschgeExampleFlashImage(), minimalConfig34 },
        { {0x05, 0x06, 0x07, 0x08}, minimalConfig5678 }
    };

    // Set up the contents of the SPI, and create the simulated hardware and bridge for it
    decltype (createSimBridgeImagerFactoryWithStorage ({})) simHardware;
    {
        const auto refDataFile = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
        royale::Vector<uint8_t> refData;
        readFileToVector (refDataFile.zwetschgeFile, refData);
        ASSERT_FALSE (refData.empty()) << "Error in test code, no Zwetschge data to put in the simulated storage";
        ASSERT_LE (refData.size(), std::numeric_limits<uint32_t>::max());
        simHardware = createSimBridgeImagerFactoryWithStorage (refData.toStdVector());
    }

    // Data for NonVolatileStorageFactory to read that via the SimImager
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::ZWETSCHGE);

    ModuleConfigFactoryZwetschge confFactory (simHardware.routing, config, probeConfig);
    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (* (simHardware.bridgeFactory)));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
    ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    ASSERT_NE (nullptr, probedConfig->imagerConfig.externalImagerConfig);
    ASSERT_FALSE (probedConfig->coreConfigData.useCases.empty());
}

/**
 * This test simulates a device on which ModuleConfigFactoryZwetschge's call to createExternalConfig
 * fails, thus it doesn't get an INonVolatileStorage to pass to ModuleConfigFactoryByStorageIdBase.
 * A defaultId is provided, and so the default moduleConfig should be returned.
 *
 * An example scenario that causes this is writing a Zwetschge file to offset 0x2000 (with an extra
 * 0x2000 bytes of reserved space, and the ToC starting at 0x4000).
 *
 * The static_cast<IModuleConfigFactory *> upcasts are explained in TestModuleConfigFactoryFixed.
 */
TEST (TestModuleConfigFactoryZwetschge, CreateExternalConfigFails)
{
    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    auto minimalConfig5678 = getMinimalModuleConfig();
    minimalConfig5678.coreConfigData.cameraName = "Camera5678";

    const auto defaultId = decltype (ModuleConfigFactoryZwetschge::value_type::first)
    {
        {
            0x05, 0x06, 0x07, 0x08
        }
    };
    const auto probeConfig = royale::Vector<ModuleConfigFactoryZwetschge::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { idOfZwetschgeExampleFlashImage(), minimalConfig34 },
        { defaultId, minimalConfig5678 }
    };

    // Set up the contents of the SPI, and create the simulated hardware and bridge for it
    decltype (createSimBridgeImagerFactoryWithStorage ({})) simHardware;
    {
        const auto refDataFile = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
        royale::Vector<uint8_t> refData;
        readFileToVector (refDataFile.zwetschgeFile, refData);
        ASSERT_FALSE (refData.size() <= 0x2001) << "Error in test code, not enough Zwetschge data to put in the simulated storage";
        refData.insert (refData.begin() + 0x2000, stringlikeMagicNumber ("This is 0x2000 bytes of reserved area, but written to where the ToC should start", 0x2000));
        ASSERT_LE (refData.size(), std::numeric_limits<uint32_t>::max());
        simHardware = createSimBridgeImagerFactoryWithStorage (refData.toStdVector());
    }

    // Data for NonVolatileStorageFactory to read that via the SimImager
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::ZWETSCHGE);

    ModuleConfigFactoryZwetschge confFactory (simHardware.routing, config, probeConfig, defaultId);
    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (* (simHardware.bridgeFactory)));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig5678.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
    ASSERT_EQ (royale::String ("Camera5678"), probedConfig->coreConfigData.cameraName);

    if (!fileexists (ZWETSCHGE_BACKUP_FILE))
    {
        // It's a minimal config, just enough to open it with the ZwetschgeFlashTool
        ASSERT_EQ (nullptr, probedConfig->imagerConfig.externalImagerConfig);
        ASSERT_TRUE (probedConfig->coreConfigData.useCases.empty());
    }
}

/**
 * Test that a failure in the bridge layer returns nullptr, and does not propagate the exception.
 */
TEST (TestModuleConfigFactoryZwetschge, UnsupportedStorage)
{
    const auto minimalConfig12 = getMinimalModuleConfig();
    const auto probeConfig = royale::Vector<ModuleConfigFactoryZwetschge::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
    };

    MockBridgeFactory bridgeFactory;

    auto route = std::make_shared<SensorRoutingConfigSpi> (0x01);
    auto config = FlashMemoryConfig { FlashMemoryConfig::FlashMemoryType::ZWETSCHGE }
                  .setImageSize (128 * 1024)
                  .setPageSize (256);

    ModuleConfigFactoryZwetschge confFactory (std::move (route), std::move (config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_EQ (nullptr, probedConfig);
}
