/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/ModuleConfigFactoryFixed.hpp>

#include <FactoryTestHelpers.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdint>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::test::utils;
using namespace testing;

namespace
{
    class MockBridgeFactory : public IBridgeFactory
    {
    public:
        MOCK_METHOD0 (initialize, void());
    };
}

/**
 * Test that ModuleConfigFactoryFixed returns the ModuleConfig and does not call the BridgeFactory.
 */
TEST (TestModuleConfigFactoryFixed, FixedConfig)
{
    auto minimalConfig = getMinimalModuleConfig();
    ModuleConfigFactoryFixed confFactory (minimalConfig);
    MockBridgeFactory bridgeFactory;

    std::shared_ptr<const ModuleConfig> probedConfig;
    // Normal callers of probeAndCreate are expected to have only an IModuleConfigFactory pointer,
    // without knowing which concrete class they're accessing; this means that they're using the
    // virtual function table, therefore probeAndCreate doesn't need to be declared as ROYALE_API.
    // The upcast on the next line makes MSVC++ use the virtual function table, even though the
    // exact class is known.
    ASSERT_NO_THROW (probedConfig = static_cast<IModuleConfigFactory *> (&confFactory)->probeAndCreate (bridgeFactory));
    ASSERT_NE (nullptr, probedConfig);
    ASSERT_EQ (minimalConfig.imagerConfig.imagerType, probedConfig->imagerConfig.imagerType);
}
