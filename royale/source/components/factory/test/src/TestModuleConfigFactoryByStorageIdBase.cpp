/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/ModuleConfigFactoryByStorageIdBase.hpp>

#include <common/Crc32.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/MakeUnique.hpp>
#include <FactoryTestHelpers.hpp>

#include <gmock/gmock.h>

#include <cstdint>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::hal;
using namespace royale::test::utils;
using namespace testing;

namespace
{
    class MockVolatileStorage : public INonVolatileStorage
    {
    public:
        MOCK_METHOD0 (initialize, void());
        MOCK_METHOD0 (getModuleIdentifier, royale::Vector<uint8_t>());
        MOCK_METHOD0 (getModuleSuffix, royale::String());
        MOCK_METHOD0 (getModuleSerialNumber, royale::String());
        MOCK_METHOD0 (getCalibrationData, royale::Vector<uint8_t>());
        MOCK_METHOD0 (getCalibrationDataChecksum, uint32_t());
        MOCK_METHOD1 (writeCalibrationData, void (const royale::Vector<uint8_t> &));
        MOCK_METHOD4 (writeCalibrationData, void (const royale::Vector<uint8_t> &,
                      const royale::Vector<uint8_t> &identifier,
                      const royale::String &suffix,
                      const royale::String &serialNumber
                                                 ));

    };


    // Create a template mock object, and set up everything except for the calibration data
    std::shared_ptr<MockVolatileStorage> createMockStorage ()
    {
        auto storage = std::make_shared<MockVolatileStorage> ();
        EXPECT_CALL (*storage, getModuleIdentifier()).WillRepeatedly (Return (royale::Vector<uint8_t> {0x01, 0x02}));
        EXPECT_CALL (*storage, getModuleSerialNumber()).WillRepeatedly (Return (royale::String {"no number"}));
        EXPECT_CALL (*storage, getModuleSuffix()).WillRepeatedly (Return (royale::String {}));
        return storage;
    }

    /**
     * ModuleConfigFactoryByStorageIdBase is an abstract class, this provides the missing
     * override.
     */
    class ConfFactory : public ModuleConfigFactoryByStorageIdBase
    {
    public:
        ConfFactory (
            const royale::Vector<value_type> configs,
            const royale::Vector<uint8_t> &defaultId = {}) :
            ModuleConfigFactoryByStorageIdBase (configs, defaultId)
        {
        }

        std::shared_ptr<const royale::config::ModuleConfig>
        probeAndCreate (royale::factory::IBridgeFactory &bridgeFactory) const override
        {
            throw NotImplemented ("The test is expected to call readAndCreate() directly");
        }
    };
}

// The tests in TestModuleConfigFactoryByStorageId also test ModuleConfigFactoryByStorageIdBase

/**
 * Checks that ModuleConfigFactoryByStorageIdBase handles a FlashMemoryPolar or FlashMemoryZwetschge
 * from which the calibration can not be read, although the moduleId can be read.
 */
TEST (TestModuleConfigFactoryByStorageIdBase, NoCalibration)
{
    auto minimalConfig12 = getMinimalModuleConfig();
    minimalConfig12.coreConfigData.cameraName = "Camera12";
    auto minimalConfig34 = getMinimalModuleConfig();
    minimalConfig34.coreConfigData.cameraName = "Camera34";

    const auto probeConfig = royale::Vector<ModuleConfigFactoryByStorageIdBase::value_type>
    {
        { {0x01, 0x02}, minimalConfig12 },
        { {0x03, 0x04}, minimalConfig34 }
    };

    ConfFactory confFactory (probeConfig);
    ASSERT_EQ (2u, confFactory.enumerateConfigs().size());

    // Test what happens when the calibration can be read
    {
        auto calibration = royale::Vector<uint8_t> {0x00, 0xff};
        auto crc = calculateCRC32 (calibration.data(), calibration.size());
        auto storage = createMockStorage();
        EXPECT_CALL (*storage, getCalibrationData()).WillRepeatedly (Return (calibration));
        EXPECT_CALL (*storage, getCalibrationDataChecksum()).WillRepeatedly (Return (crc));

        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, false));
        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, true));
    }
    // Test what happens when the INonVolatileStorage throws DataNotFound
    {
        auto storage = createMockStorage();
        EXPECT_CALL (*storage, getCalibrationData()).WillRepeatedly (Throw (DataNotFound ()));
        EXPECT_CALL (*storage, getCalibrationDataChecksum()).WillRepeatedly (Return (0u));

        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, false));
        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, true));
    }
    // Test what happens when the INonVolatileStorage throws RuntimeError
    {
        auto storage = createMockStorage();
        EXPECT_CALL (*storage, getCalibrationData()).WillRepeatedly (Throw (RuntimeError ()));
        EXPECT_CALL (*storage, getCalibrationDataChecksum()).WillRepeatedly (Return (0u));

        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, false));
        ASSERT_NO_THROW (confFactory.readAndCreate (*storage, true));
    }
}
