/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <FactoryTestHelpers.hpp>

#include <common/MakeUnique.hpp>
#include <common/FileSystem.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

#include <SimImagerM2453.hpp>
#include <StubBridgeImager.hpp>

#include <gmock/gmock.h>

using namespace royale::config;
using namespace royale::factory;
using namespace royale::hal;
using namespace royale::stub;
using namespace royale::test::utils;
using namespace testing;

ModuleConfig royale::test::utils::getMinimalModuleConfig()
{
    ModuleConfig minimalConfig = { {}, {ImagerType::M2450_A12_AIO, 1, {}}};
    return minimalConfig;
}

namespace
{
    class MockBridgeImager : public IBridgeImager
    {
    public:
        MOCK_METHOD1 (setImagerReset, void (bool));
        MOCK_METHOD2 (readImagerRegister, void (uint16_t, uint16_t &));
        MOCK_METHOD2 (writeImagerRegister, void (uint16_t, uint16_t));
        MOCK_METHOD2 (readImagerBurst, void (uint16_t, std::vector<uint16_t> &));
        MOCK_METHOD2 (writeImagerBurst, void (uint16_t, const std::vector<uint16_t> &));
        MOCK_METHOD1 (sleepFor, void (std::chrono::microseconds));

        void expectNoRegisterAccess ()
        {
            EXPECT_CALL (*this, readImagerRegister (_, _)).Times (0);
            EXPECT_CALL (*this, writeImagerRegister (_, _)).Times (0);
            EXPECT_CALL (*this, readImagerBurst (_, _)).Times (0);
            EXPECT_CALL (*this, writeImagerBurst (_, _)).Times (0);
        }
    };

    class MockBridgeImagerFactory : public IBridgeFactoryImpl<IBridgeImager>
    {
    public:
        explicit MockBridgeImagerFactory (std::shared_ptr<IBridgeImager> bridgeImager) :
            m_bridgeImager {bridgeImager}
        {
        }

        MOCK_METHOD0 (initialize, void());

        void createImpl (std::shared_ptr<IBridgeImager> &bridgeImager) override
        {
            bridgeImager = m_bridgeImager;
        }

        std::shared_ptr<IBridgeImager> m_bridgeImager;
    };
}
StructForCreateSimBridgeImagerFactoryWithStorage royale::test::utils::createSimBridgeImagerFactoryWithStorage (const std::vector<uint8_t> &flashContents)
{
    assert (flashContents.size() < std::numeric_limits<uint32_t>::max());

    auto simImager = std::make_shared<SimImagerM2453> ();
    auto bridgeImager = std::make_shared<StubBridgeImager> (simImager);

    // Set up the contents of the SPI
    StructForCreateSimBridgeImagerFactoryWithStorage result;
    result.simulatedSpiStorage = simImager->getFlashMemorySpace();
    for (auto i = uint32_t (0); i < flashContents.size(); ++i)
    {
        (*result.simulatedSpiStorage) [i] = flashContents[i];
    }
    // The SimImager expects to be able to read data in 16-bit words, and we want to be able
    // to use the storage as random-access. Data doesn't need to be aligned, but there needs
    // to be 1 byte of padding at the end, so that a read that includes the final byte can
    // read a 16-bit area that includes the byte afterwards.
    (*result.simulatedSpiStorage) [static_cast<uint32_t> (flashContents.size())] = 0;

    result.bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);
    result.routing = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2453_SPI);

    return result;
}
