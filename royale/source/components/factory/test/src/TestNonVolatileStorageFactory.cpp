/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/NonVolatileStorageFactory.hpp>

#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/LogicError.hpp>
#include <config/SensorRoutingFilename.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>
#include <hal/IBridgeImager.hpp>
#include <storage/NonVolatileStorageShadow.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <FactoryTestHelpers.hpp>
#include <LenaTestUtils.hpp>
#include <ZwetschgeTestUtils.hpp>

#include <cstdint>
#include <memory>
#include <vector>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::hal;
using namespace royale::imager;
using namespace royale::storage;
using namespace royale::test::utils;

namespace
{
    using namespace testing;

    class MockBridgeFactory : public IBridgeFactory
    {
    public:
        MOCK_METHOD0 (initialize, void());
    };

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

    /**
     * If this file has Unix-style line endings, this is the 'C' character of the second line of
     * this .cpp file. If it has CRLF-style line endings, then it will be the space before that,
     * and the next character will be the 'C'.
     *
     * There's a "space asterisk space" on that line too, but that makes a off-by-one error harder
     * to debug, so this uses the text instead.
     */
    const auto lineTwoOffset = std::size_t (82);
    /** First character of line 2, if it's got unix-style line endings */
    const auto lineTwoExpectedUnixStyle = 'C';
    /**
     * What we expect to read from line 2.
     */
    const auto lineTwoContent = royale::String ("Copyright (C) 20");
}

TEST (TestNonVolatileStorageFactory, NoStorage)
{
    {
        MockBridgeFactory bridgeFactory;
        FlashMemoryConfig config (FlashMemoryConfig::FlashMemoryType::NONE);
        ASSERT_EQ (nullptr, NonVolatileStorageFactory::createFlash (bridgeFactory, config, nullptr));
    }
}

TEST (TestNonVolatileStorageFactory, FixedStorageCalibOnly)
{
    const royale::Vector<uint8_t> calibration {0, 1, 2, 3};
    FlashMemoryConfig config (FlashMemoryConfig::FlashMemoryType::FIXED);
    config.nonVolatileStorageFixed = std::make_shared<NonVolatileStorageShadow> (calibration);
    MockBridgeFactory bridgeFactory;

    std::shared_ptr<INonVolatileStorage> flash;
    ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (bridgeFactory, config, nullptr));
    ASSERT_EQ (calibration, flash->getCalibrationData());
    ASSERT_EQ (royale::Vector<uint8_t> {}, flash->getModuleIdentifier());
    ASSERT_EQ (royale::String (""), flash->getModuleSuffix());
    ASSERT_EQ (royale::String (""), flash->getModuleSerialNumber());
}

TEST (TestNonVolatileStorageFactory, FixedStorageWithV7Items)
{
    const royale::Vector<uint8_t> calibration {0, 1, 2, 3};
    const royale::Vector<uint8_t> identifier {5, 6, 7, 8};
    const royale::String suffix ("9abc");
    const royale::String serial ("defg-hijk-lmno-pqrs");
    FlashMemoryConfig config (FlashMemoryConfig::FlashMemoryType::FIXED);
    config.nonVolatileStorageFixed = std::make_shared<NonVolatileStorageShadow> (calibration, identifier, suffix, serial);
    MockBridgeFactory bridgeFactory;

    std::shared_ptr<INonVolatileStorage> flash;
    ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (bridgeFactory, config, nullptr));
    ASSERT_NE (nullptr, flash);
    ASSERT_EQ (calibration, flash->getCalibrationData());
    ASSERT_EQ (identifier, flash->getModuleIdentifier());
    ASSERT_EQ (suffix, flash->getModuleSuffix());
    ASSERT_EQ (serial, flash->getModuleSerialNumber());
}

/**
 * This doesn't test StorageSpiImager itself, just the constraint that it won't access the imager
 * unless it has exclusive access.
 */
TEST (TestNonVolatileStorageFactory, ConstructingStorageSpiImagerM2452)
{
    using namespace testing;

    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM);
    const auto routing = SensorRoutingImagerAsBridge (ImagerAsBridgeType::M2452_SPI);

    // With NonVolatileStorageFactoryConstraint::NONE, this should not touch the imager
    {
        auto bridgeImager = std::make_shared<MockBridgeImager> ();
        bridgeImager->expectNoRegisterAccess ();
        EXPECT_CALL (*bridgeImager, setImagerReset (_)).Times (0);

        auto bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);

        std::shared_ptr<INonVolatileStorage> flash;
        ASSERT_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing), LogicError);
    }

    // With NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME, this should reset the imager
    // at least once, and may write registers to it.
    //
    // This test will currently fail if StorageSpiImagerM2452 implements RAII and tries to read
    // ANAIP_DESIGNSTEP during its constructor, because the simulation doesn't simulate any
    // registers of the imager; if that happens then the test should be changed.
    //
    // This test currently passes because StorageSpiImagerM2452 resets the imager in its destructor.
    {

        auto bridgeImager = std::make_shared<MockBridgeImager> ();
        EXPECT_CALL (*bridgeImager, setImagerReset (_)).Times (AtLeast (1));

        auto bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);

        std::shared_ptr<INonVolatileStorage> flash;
        ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing, NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME));
        ASSERT_NE (nullptr, flash);
    }
}

/**
 * This doesn't test StorageSpiImager itself, just the constraint that it won't access the imager
 * unless it has exclusive access.
 */
TEST (TestNonVolatileStorageFactory, ConstructingStorageSpiImagerM2453)
{
    using namespace testing;

    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM);
    const auto routing = SensorRoutingImagerAsBridge (ImagerAsBridgeType::M2453_SPI);

    // With NonVolatileStorageFactoryConstraint::NONE, this should not touch the imager
    {
        auto bridgeImager = std::make_shared<MockBridgeImager> ();
        bridgeImager->expectNoRegisterAccess ();
        EXPECT_CALL (*bridgeImager, setImagerReset (_)).Times (0);

        auto bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);

        std::shared_ptr<INonVolatileStorage> flash;
        ASSERT_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing), LogicError);
    }

    // With NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME, this should reset the imager
    // at least once, and may write registers to it.
    //
    // This test currently passes because StorageSpiImagerM2453 resets the imager in its destructor,
    // even if it doesn't reset the imager in its initialization.
    {

        auto bridgeImager = std::make_shared<MockBridgeImager> ();
        EXPECT_CALL (*bridgeImager, setImagerReset (_)).Times (AtLeast (1));

        auto bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);

        std::shared_ptr<INonVolatileStorage> flash;
        ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing, NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME));
        ASSERT_NE (nullptr, flash);
    }
}

/**
 * Read line 2 of this file via createStorageReadRandom, see if it says what we expect.
 */
TEST (TestNonVolatileStorageFactory, StorageFileReadsTheCopyrightOnThisTest)
{
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::JUST_CALIBRATION);
    const auto routing = SensorRoutingFilename {TEST_FILE_FOR_TESTNONVOLATILESTORAGEFACTORY_CONSTRUCTINGSTORAGEFILE};
    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    auto bridgeFactory = std::make_shared<MockBridgeFactory> ();

    std::shared_ptr<royale::pal::IStorageReadRandom> reader;
    ASSERT_NO_THROW (reader = NonVolatileStorageFactory::createStorageReadRandom (*bridgeFactory, config, &routing));

    auto testData = std::vector<uint8_t> (lineTwoContent.size() + 1);
    ASSERT_NO_THROW (reader->readStorage (lineTwoOffset, testData));

    // Strip either the first or last character, depending on the line-endings
    auto offset = 0u;
    if (testData.at (offset) == ' ')
    {
        ++offset;
    }
    auto testString = royale::String (reinterpret_cast<char *> (&testData[offset]), lineTwoContent.size());

    ASSERT_EQ (lineTwoContent, testString);
}

/**
 * Read line 2 of this file via createFlash, see if it says what we expect.
 */
TEST (TestNonVolatileStorageFactory, PersistentReadsTheCopyrightOnThisTest)
{
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::JUST_CALIBRATION);
    const auto routing = SensorRoutingFilename {TEST_FILE_FOR_TESTNONVOLATILESTORAGEFACTORY_CONSTRUCTINGSTORAGEFILE};
    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    auto bridgeFactory = std::make_shared<MockBridgeFactory> ();

    std::shared_ptr<INonVolatileStorage> flash;
    ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing));
    ASSERT_NE (nullptr, flash);

    auto calibration = royale::Vector<uint8_t> ();
    ASSERT_NO_THROW (calibration = flash->getCalibrationData());

    // What we've just read is not calibration, it's the contents of this .cpp file
    ASSERT_GE (calibration.size(), lineTwoOffset + lineTwoContent.size() + 1);

    // Read from lineTwoOffset or one character later, depending on the line endings
    auto offset = lineTwoOffset;
    if (calibration.at (offset) != lineTwoExpectedUnixStyle)
    {
        ++offset;
    }
    auto testString = royale::String (reinterpret_cast<char *> (&calibration[offset]), lineTwoContent.size());

    ASSERT_EQ (lineTwoContent, testString);
}

/**
 * Create an instance of StorageFormatPolar, reading from this file. This is expected to fail and
 * say that the data is corrupt, the test is checking that NonVolatileStorageFactory creates the
 * StorageFormatPolar instance.
 */
TEST (TestNonVolatileStorageFactory, ConstructPolarToReadThisCppFile)
{
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM);
    const auto routing = SensorRoutingFilename {TEST_FILE_FOR_TESTNONVOLATILESTORAGEFACTORY_CONSTRUCTINGSTORAGEFILE};
    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    auto bridgeFactory = std::make_shared<MockBridgeFactory> ();

    std::shared_ptr<INonVolatileStorage> flash;
    ASSERT_NO_THROW (flash = NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing));
    ASSERT_NE (nullptr, flash);

    ASSERT_ANY_THROW (flash->getCalibrationData());
}

/**
 * Access a Lena file, reading from the file used by ImagerLenaReaders's unit tests.
 */
TEST (TestNonVolatileStorageFactory, ConstructLenaToReadRealFile)
{
    const auto routing = royale::test::utils::getNvsfGetterForLenaFile();
    ExternalConfig externalConfig {};
    ASSERT_NO_THROW (externalConfig = NonVolatileStorageFactory::createExternalConfig (routing));
    ASSERT_NE (nullptr, externalConfig.imagerExternalConfig);
}

/**
 * Create an instance of StorageFormatZwetschge, reading from this file. This is expected to fail and
 * say that the data is corrupt.
 *
 * Note: this test should fail in a different way if the reference file (this
 * TestNonVolatileStorageFactory.cpp file) is less than (8Kb + size of the Zwetschge Toc).
 */
TEST (TestNonVolatileStorageFactory, ConstructZwetschgeToReadThisCppFile)
{
    const auto routing = ExternalConfigFileConfig::fromZwetschgeFile (TEST_FILE_FOR_TESTNONVOLATILESTORAGEFACTORY_CONSTRUCTINGSTORAGEFILE);
    ASSERT_THROW (NonVolatileStorageFactory::createExternalConfig (routing), DataNotFound);
}

/**
 * Create an instance of StorageFormatZwetschge, reading from the file used by
 * StorageFormatZwetschge's own unit tests, via
 * createExternalConfig (ExternalConfigFileConfig)
 */
TEST (TestNonVolatileStorageFactory, ConstructZwetschgeToReadRealFile1Arg)
{
    const auto routing = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
    ExternalConfig externalConfig {};
    ASSERT_NO_THROW (externalConfig = NonVolatileStorageFactory::createExternalConfig (routing));
    ASSERT_NE (nullptr, externalConfig.imagerExternalConfig);
    ASSERT_FALSE (externalConfig.royaleUseCaseList.empty());
}

/**
 * Create an instance of StorageFormatZwetschge, reading from the file used by
 * StorageFormatZwetschge's own unit tests, via
 * createExternalConfig (all the args for createStorageReadRandom)
 *
 * The name is ...4Arg because there's a constraint arg in addition to the three used below.
 */
TEST (TestNonVolatileStorageFactory, ConstructZwetschgeToReadRealFile4Arg)
{
    const auto config = FlashMemoryConfig {FlashMemoryConfig::FlashMemoryType::ZWETSCHGE};
    const auto externalConfigFileConfig = royale::test::utils::getNvsfGetterForZwetschgeExampleDevice();
    const auto routing = SensorRoutingFilename {externalConfigFileConfig.zwetschgeFile};
    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    auto bridgeFactory = std::make_shared<MockBridgeFactory> ();

    ExternalConfig externalConfig {};
    ASSERT_NO_THROW (externalConfig = NonVolatileStorageFactory::createExternalConfig (*bridgeFactory, config, &routing));
    ASSERT_NE (nullptr, externalConfig.imagerExternalConfig);
    ASSERT_FALSE (externalConfig.royaleUseCaseList.empty());
}

#include <common/FileSystem.hpp>

/**
 * Create an instance of StorageFormatZwetschge, reading from the simulated flash storage
 * accessed via the a simulated M2453.
 */
TEST (TestNonVolatileStorageFactory, ConstructZwetschgeToReadM2453Spi)
{
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

    // Now read it via the SimImager
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::ZWETSCHGE);
    ExternalConfig externalConfig {};
    ASSERT_NO_THROW (externalConfig = NonVolatileStorageFactory::createExternalConfig (* (simHardware.bridgeFactory), config, simHardware.routing.get(), NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME));
    ASSERT_NE (nullptr, externalConfig.imagerExternalConfig);
    ASSERT_FALSE (externalConfig.royaleUseCaseList.empty());
}

/**
 * Check that using JUST_CONFIG with a routing that it doesn't support will throw.
 */
TEST (TestNonVolatileStorageFactory, JustConfigWithUnsupportedRouting)
{
    const auto config = FlashMemoryConfig (FlashMemoryConfig::FlashMemoryType::JUST_CALIBRATION);
    // We need to pass a bridge factory, but it doesn't get used so it doesn't matter which factory
    // it is.
    auto bridgeFactory = std::make_shared<MockBridgeFactory> ();

    // null routing
    {
        ASSERT_ANY_THROW (NonVolatileStorageFactory::createFlash (*bridgeFactory, config, nullptr));
    }

    // A combination that could be supported, but is currently unsupported in the factory.
    //
    // That the combination is supportable is tested by the ConstructingStorageSpiImagerM2452 test.
    // If that test breaks (see the comments in the test about future changes which could break it),
    // this one will need corresponding changes.
    {
        auto bridgeImager = std::make_shared<MockBridgeImager> ();
        auto bridgeFactory = std::make_shared<MockBridgeImagerFactory> (bridgeImager);
        const auto routing = SensorRoutingImagerAsBridge (ImagerAsBridgeType::M2452_SPI);
        ASSERT_ANY_THROW (NonVolatileStorageFactory::createFlash (*bridgeFactory, config, &routing, NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME));
    }
}
