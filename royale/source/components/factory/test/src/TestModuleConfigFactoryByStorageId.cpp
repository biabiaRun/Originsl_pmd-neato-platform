/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/ModuleConfigFactoryByStorageId.hpp>

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigSpi.hpp>
#include <storage/NonVolatileStorageShadow.hpp>
#include <FactoryTestHelpers.hpp>
#include <LenaTestUtils.hpp>
#include <ZwetschgeTestUtils.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdint>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::storage;
using namespace royale::test::utils;
using namespace testing;

namespace
{
    class MockBridgeFactory : public IBridgeFactory
    {
        MOCK_METHOD0 (initialize, void());
    };

    struct RouteConfig
    {
        std::shared_ptr<royale::common::ISensorRoutingConfig> route;
        royale::config::FlashMemoryConfig config;
    };

    /**
     * Returns parameters which, when given to NonVolatileStorageFactory, should return a storage
     * where INonVolatileStorage::getModuleIdentifier() returns the identifier.
     */
    RouteConfig getRouteConfigWithIdentifier (const royale::Vector<uint8_t> &identifier,
            const royale::Vector<uint8_t> &calibration = {0, 1, 2, 3})
    {
        const royale::String suffix ("simulation");
        const royale::String serial ("1111-2222-3333-4444");
        RouteConfig routeConfig
        {
            nullptr,
            FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::FIXED)
        };
        routeConfig.config.nonVolatileStorageFixed = std::make_shared<NonVolatileStorageShadow> (std::move (calibration), std::move (identifier), std::move (suffix), std::move (serial));
        return routeConfig;
    }
}

/**
 * This test simulates a device on which ModuleConfigFactoryByStorageId can successfully call
 * getModuleIdentifier(), and then checks that it returns the expected configuration.
 *
 * This test is based on two simpler tests: TestNonVolatileStorageFactory.FixedStorageWithV7Items
 * and TestModuleConfigFactoryFixed.FixedConfig.  If either of those are failing then debugging
 * should start with them, not this one.
 *
 * The static_cast<IModuleConfigFactory *> upcasts are explained in TestModuleConfigFactoryFixed.
 */
TEST (TestModuleConfigFactoryByStorageId, ByStorageIdConfig)
{
    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    auto minimalConfig5678 = getMinimalModuleConfig();
    minimalConfig5678.coreConfigData.cameraName = "Camera5678";

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { {0x03, 0x04}, minimalConfig34 },
        { {0x05, 0x06, 0x07, 0x08}, minimalConfig5678 }
    };

    MockBridgeFactory bridgeFactory;

    // Test that a known identifier is recognised
    {
        auto storage = getRouteConfigWithIdentifier ({0x03, 0x04});
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
        ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    }

    // Test that an unknown identifier returns nullptr if there's no defaultId
    {
        auto storage = getRouteConfigWithIdentifier ({0x01, 0x04});
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_EQ (nullptr, probedConfig);
    }

    // Test that an unknown identifier returns the default config if there's a defaultId
    {
        auto storage = getRouteConfigWithIdentifier ({0x01, 0x04});
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig, {0x03, 0x04});

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
        ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    }

    // Test that an empty identifier returns nullptr (assuming that there is no empty identifier in
    // the list), and does not cause an exception or crash
    {
        auto storage = getRouteConfigWithIdentifier ({});
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_EQ (nullptr, probedConfig);
    }

    // Test that a empty identifier is recognised, if the list includes an empty identifier
    {
        auto storage = getRouteConfigWithIdentifier ({});
        const auto probeConfigWithEmpty = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
        {
            { {0x01, 0x02}, minimalConfig12 },
            { {0x03, 0x04}, minimalConfig34 },
            { {}, minimalConfig5678 }
        };
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfigWithEmpty);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (minimalConfig5678.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
        ASSERT_EQ (royale::String ("Camera5678"), probedConfig->coreConfigData.cameraName);
    }
}

/**
 * Test that a failure in the bridge layer returns nullptr, and does not propagate the exception.
 */
TEST (TestModuleConfigFactoryByStorageId, UnsupportedStorage)
{
    const auto minimalConfig12 = getMinimalModuleConfig();
    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
    };

    MockBridgeFactory bridgeFactory;

    auto route = std::make_shared<SensorRoutingConfigSpi> (0x01);
    auto config = FlashMemoryConfig { FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM }
                  .setImageSize (128 * 1024)
                  .setPageSize (256);

    ModuleConfigFactoryByStorageId confFactory (std::move (route), std::move (config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_EQ (nullptr, probedConfig);
}

/**
 * Test that the feature for creating a cached copy of the calibration works.
 */
TEST (TestModuleConfigFactoryByStorageId, CachedCalibration)
{
    const auto calibInProbeConfig = royale::Vector<uint8_t> {'p', 'r', 'o', 'b', 'e'};
    const auto calibInModuleConfig = royale::Vector<uint8_t> {'m', 'o', 'd', 'u', 'l', 'e'};

    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    minimalConfig12.flashMemoryConfig = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::FIXED);
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    minimalConfig34.flashMemoryConfig = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM);
    auto minimalConfig5678 = getMinimalModuleConfig();
    minimalConfig5678.coreConfigData.cameraName = "Camera5678";
    minimalConfig5678.flashMemoryConfig = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::FIXED);
    minimalConfig5678.flashMemoryConfig.useCaching = true;
    minimalConfig5678.flashMemoryConfig.nonVolatileStorageFixed = std::make_shared<NonVolatileStorageShadow> (calibInModuleConfig);

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { {0x03, 0x04}, minimalConfig34 },
        { {0x05, 0x06, 0x07, 0x08}, minimalConfig5678 }
    };

    MockBridgeFactory bridgeFactory;

    {
        auto storage = getRouteConfigWithIdentifier ({0x01, 0x02}, calibInProbeConfig);
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (FlashMemoryConfig::FlashMemoryType::FIXED, probedConfig->flashMemoryConfig.type);
        ASSERT_NE (nullptr, probedConfig->flashMemoryConfig.nonVolatileStorageFixed);
        ASSERT_EQ (calibInProbeConfig, probedConfig->flashMemoryConfig.nonVolatileStorageFixed->getCalibrationData());
    }

    {
        auto storage = getRouteConfigWithIdentifier ({0x03, 0x04}, calibInProbeConfig);
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM, probedConfig->flashMemoryConfig.type);
        ASSERT_EQ (nullptr, probedConfig->flashMemoryConfig.nonVolatileStorageFixed);
    }

    {
        auto storage = getRouteConfigWithIdentifier ({0x05, 0x06, 0x07, 0x08}, calibInProbeConfig);
        ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

        std::shared_ptr<const ModuleConfig> probedConfig;
        ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
        ASSERT_NE (nullptr, probedConfig);
        ASSERT_EQ (FlashMemoryConfig::FlashMemoryType::FIXED, probedConfig->flashMemoryConfig.type);
        ASSERT_NE (nullptr, probedConfig->flashMemoryConfig.nonVolatileStorageFixed);
        ASSERT_EQ (calibInModuleConfig, probedConfig->flashMemoryConfig.nonVolatileStorageFixed->getCalibrationData());
    }
}

/**
 * Test a ModuleConfig that uses Polar in the device, and Zwetschge on the filesystem.
 */
TEST (TestModuleConfigFactoryByStorageId, PolarThenZwetschge)
{
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    minimalConfig34.imagerConfig.externalConfigFileConfig = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x03, 0x04}, minimalConfig34 },
    };

    MockBridgeFactory bridgeFactory;

    auto storage = getRouteConfigWithIdentifier ({0x03, 0x04});
    ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
    ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    ASSERT_NE (0u, probedConfig->coreConfigData.useCases.size());
    ASSERT_NE (nullptr, probedConfig->imagerConfig.externalImagerConfig);
}

/**
 * Test a ModuleConfig that uses Polar in the device, and Lena on the filesystem.
 */
TEST (TestModuleConfigFactoryByStorageId, PolarThenLena)
{
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    minimalConfig34.imagerConfig.externalConfigFileConfig = royale::test::utils::getNvsfGetterForLenaFile();

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x03, 0x04}, minimalConfig34 },
    };

    MockBridgeFactory bridgeFactory;

    auto storage = getRouteConfigWithIdentifier ({0x03, 0x04});
    ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig34.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
    ASSERT_EQ (royale::String ("Camera34"), probedConfig->coreConfigData.cameraName);
    // Lena only has the externalImagerConfig, so we expect coreConfigData.useCases to be empty
    ASSERT_EQ (0u, probedConfig->coreConfigData.useCases.size());
    ASSERT_NE (nullptr, probedConfig->imagerConfig.externalImagerConfig);
}

/**
 * Test a ModuleConfig that uses Polar in the device, and is configured for Zwetschge on the filesystem,
 * but the file contains Lena data instead.
 */
TEST (TestModuleConfigFactoryByStorageId, LenaMisconfiguredAsZwetschge)
{
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    const auto refFile = royale::test::utils::getNvsfGetterForLenaFile();
    minimalConfig34.imagerConfig.externalConfigFileConfig = ExternalConfigFileConfig::fromZwetschgeFile (refFile.lenaFile);

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x03, 0x04}, minimalConfig34 },
    };

    MockBridgeFactory bridgeFactory;

    auto storage = getRouteConfigWithIdentifier ({0x03, 0x04});
    ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_EQ (nullptr, probedConfig);
}

/**
 * Test a ModuleConfig that uses Polar in the device, and is configured for Lena on the filesystem,
 * but the file contains Zwetschge data instead.
 */
TEST (TestModuleConfigFactoryByStorageId, ZwetschgeMisconfiguredAsLena)
{
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";
    const auto refFile = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
    minimalConfig34.imagerConfig.externalConfigFileConfig = ExternalConfigFileConfig::fromLenaFile (refFile.zwetschgeFile);

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
    {
        { {0x03, 0x04}, minimalConfig34 },
    };

    MockBridgeFactory bridgeFactory;

    auto storage = getRouteConfigWithIdentifier ({0x03, 0x04});
    ModuleConfigFactoryByStorageId confFactory (std::move (storage.route), std::move (storage.config), probeConfig);

    std::shared_ptr<const ModuleConfig> probedConfig;
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_EQ (nullptr, probedConfig);
}
