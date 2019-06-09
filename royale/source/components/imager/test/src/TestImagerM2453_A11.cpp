/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <SimImagerM2453.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/NarrowCast.hpp>
#include <StubBridgeImager.hpp>
#include <imager/ImagerM2453_A11.hpp>
#include <imager/ImagerLenaReader.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/M2453/PseudoDataInterpreter.hpp>
#include <imager/M2453/ImagerRegisters.hpp>
#include <imager/WrapperImagerExternalConfig.hpp>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 24000000;

    //this test might be executed using different working directories,
    //thus the external imager configuration files are copied from
    //the well-known Royale runtime output directory to the current
    //working directory to enable testing of this imager
    const std::string runtimePath = IMAGER_TEST_FILE_PATH; //!< directly specify file path to avoid platform dependent code
    const std::string externalImagerConfiguration{ "Salome_M2453.lena" };

    const std::string temporaryImagerConfiguration{ "TestImagerM2453_A11.lena" };

    const uint16_t flashConfigAddressLower{ 0x3456 };
    const uint16_t flashConfigAddressUpper{ 0x12 };
    const uint32_t flashConfigAddress = flashConfigAddressUpper << 16 | flashConfigAddressLower;
    const size_t flashConfigSmallSizeInRegisters{ 1 };
    const size_t flashConfigLargeSizeInRegisters{ 257 };

    const ImagerUseCaseIdentifier flashBasedSmallUseCaseId{ "{92F58B3E-C881-4AA8-A271-DD1E1D919FD7}" };
    const ImagerUseCaseIdentifier flashBasedLargeUseCaseId{ "{F8CE41A1-2AA0-4E79-A3BA-53C61FE164BE}" };

    const std::vector<IImagerExternalConfig::UseCaseData> flashBasedUseCaseData
    {
        {
            flashBasedSmallUseCaseId,
            "FlashBasedUseCaseWithSmallRegisterConfig",
            { 5 },
            {},
            { flashConfigAddress, flashConfigSmallSizeInRegisters * 2, M2453::CFGCNT }
        }, {
            flashBasedLargeUseCaseId,
            "FlashBasedUseCaseWithLargeRegisterConfig",
            { 5 },
            {},
            { flashConfigAddress, flashConfigLargeSizeInRegisters * 2, M2453::CFGCNT }
        }

    };

    /**
     * The flash based storage is simulated by using a map of uint8_t; this function writes a
     * uint16_t to two consecutive addresses, in big-endian format (which is the format that the
     * imager will use to read registers).
     */
    void pushMapBe16 (std::map<uint32_t, uint8_t> &dest, uint32_t offset, uint16_t value)
    {
        dest [offset] = static_cast<uint8_t> ( (value >> 8) & 0xff);
        dest [narrow_cast<uint32_t> (offset + 1)] = static_cast<uint8_t> (value & 0xff);
    }

    const TimedRegisterList emptyTimedRegisterMap;
    const TimedRegisterList fwPage1
    {
        { 0x0000, 0x0001, 1u },
        { 0x0001, 0x0001, 0u },
        { 0x0000, 0x0001, 1u },
        { 0x0002, 0x0001, 1u },
        { 0x0003, 0x0001, 1u },
        { 0x0004, 0x0001, 1u },
    };
    const TimedRegisterList fwPage2
    {
        { 0x0001, 0x0001, 0u },
        { 0x0003, 0x0001, 0u }
    };

    const auto fwAndUseCaseExternalConfig = std::make_shared<WrapperImagerExternalConfig> (emptyTimedRegisterMap, fwPage1, fwPage2, emptyTimedRegisterMap, emptyTimedRegisterMap, emptyTimedRegisterMap, flashBasedUseCaseData);

    /**
     * The expected result m_bridge->registerCalls() after initializing the imager with the maps in
     * fwAndUseCaseExternalConfig.
     */
    const auto expectedRegisterCallsForHardcodedConfig = uint16_t
            {
                10u
            };
}

/**
* This fixture class contains the simulated hardware (SimImager, Bridge and the flash memory),
* but leaves the creation of the software imager to the subclass.
*/
class TestImagerM2453_A11_Uncreated : public ::testing::Test
{
protected:
    TestImagerM2453_A11_Uncreated() = default;
    ~TestImagerM2453_A11_Uncreated() override = default;

    void SetUp() override
    {
        //the concrete imager model is actually unknown as the external
        //configuration could have changed the hardware imager's
        //behavior completely, so all these tests can use is just a dummy
        //implementation of a simulated imager (receives input but ignores it)
        m_simImager = std::make_shared<SimImagerM2453> ();
        m_bridge = std::make_shared<StubBridgeImager> (m_simImager);
    }

    /**
    * Convenience function which may be called from the subclass
    */
    void createImager (std::shared_ptr<const royale::imager::IImagerExternalConfig> externalConfig)
    {
        ImagerParameters params{ m_bridge, std::move (externalConfig), true, false,
                                 ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.f, {},
                                 SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                 ImgIlluminationPad::SE_P, 90000000, false };

        m_imager.reset (new ImagerM2453_A11 (params));

        ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";

        ASSERT_NO_THROW (m_imager->sleep());

        m_bridge->writeImagerRegister (M2453::ANAIP_DESIGNSTEP, 0x0A11);
    }

    void TearDown() override
    {
        m_imager.reset();
        m_bridge.reset();
        m_simImager.reset();
    }

    std::shared_ptr <SimImagerM2453> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2453_A11> m_imager;
};

/**
* Fixture with creates a flash-defined imager using the configuration in the namespace at the top of
* this file.
*/
class TestImagerM2453_A11_Hardcoded : public TestImagerM2453_A11_Uncreated
{
protected:
    TestImagerM2453_A11_Hardcoded() = default;
    ~TestImagerM2453_A11_Hardcoded() override = default;

    void SetUp() override
    {
        TestImagerM2453_A11_Uncreated::SetUp();
        createImager (fwAndUseCaseExternalConfig);
    }

    void TearDown() override
    {
        TestImagerM2453_A11_Uncreated::TearDown();
    }
};

//Please note that ROYAL-2728 defines to have static register sets for this imager type.
//However, testing this imager currently covers only a single static register set, and
// the test will use a real life configuration. This seems to be suitable for the bring-up
// work. Later there might be a general test for the M2453 imager using a configuration stub for the
// external configuration and for each supported external configuration a separate test class.
/**
* Fixture which creates a flash-defined imager using the external configuration from the
* externalImagerConfiguration file. This matches the ROYAL-2728 implementation stage of having a
* configuration shipped as part of Royale, instead of having it in the imager's flash storage.
*/
class TestImagerM2453_A11_Lena : public TestImagerM2453_A11_Uncreated
{
protected:
    TestImagerM2453_A11_Lena() = default;
    ~TestImagerM2453_A11_Lena() override = default;

    void SetUp() override
    {
        TestImagerM2453_A11_Uncreated::SetUp();

        //note that this tests also the existance of the external
        //configurtion file that is mandatory to have when using this imager
        std::ifstream in{ runtimePath + "/" + externalImagerConfiguration };
        ASSERT_TRUE (in);
        std::ofstream{ temporaryImagerConfiguration } << //need a temporary in case of reading/writing to the same file
                std::string{ (std::istreambuf_iterator<char> (in)), std::istreambuf_iterator<char>() };

        auto externalConfig = ImagerLenaReader::fromFile (temporaryImagerConfiguration);

        m_useCases = externalConfig->getUseCaseList();

        ASSERT_GT (m_useCases.size(), 1u);

        m_extCfgUseCase1 = m_useCases.at (0).guid;
        m_extCfgUseCase2 = m_useCases.at (1).guid;

        createImager (std::move (externalConfig));
    }

    void TearDown() override
    {
        TestImagerM2453_A11_Uncreated::TearDown();
    }

    std::vector<IImagerExternalConfig::UseCaseData> m_useCases;
    ImagerUseCaseIdentifier m_extCfgUseCase1;
    ImagerUseCaseIdentifier m_extCfgUseCase2;

};

TEST (TestImagerM2453_A11_Nofixture, CreateImagerDirectly)
{
    ImagerParameters params{ nullptr, nullptr, true, false,
                             ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.f, {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 90000000, false };

    ASSERT_THROW (new ImagerM2453_A11 (params), LogicError);
}

TEST_F (TestImagerM2453_A11_Lena, CheckStates)
{
    uint16_t idx;

    //Power Down State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (m_extCfgUseCase1), WrongState);
    ASSERT_THROW (m_imager->reconfigureExposureTimes (std::vector<uint32_t> { 1u, 1u, 1u, 1u, 1u }, idx), WrongState);
    ASSERT_THROW (m_imager->initialize(), WrongState);

    ASSERT_NO_THROW (m_imager->wake());

    //Power Up State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (m_extCfgUseCase1), WrongState);
    ASSERT_THROW (m_imager->reconfigureExposureTimes (std::vector<uint32_t> { 1u, 1u, 1u, 1u, 1u }, idx), WrongState);

    ASSERT_NO_THROW (m_imager->initialize());

    //Ready State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->reconfigureExposureTimes (std::vector<uint32_t> { 1u, 1u, 1u, 1u, 1u }, idx), WrongState);

    ASSERT_NO_THROW (m_imager->executeUseCase (m_extCfgUseCase1));

    //Again in Ready State active
    ASSERT_NO_THROW (m_imager->startCapture());

    //Capturing State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->sleep(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (m_extCfgUseCase1), WrongState);
    ASSERT_THROW (m_imager->startCapture(), WrongState);

    const auto regConfig = m_bridge->getWrittenRegisters();

    auto blocks = regConfig.at (0x91CE);
    std::vector<uint32_t> expTimes (blocks, 1u);

    ASSERT_NO_THROW (m_imager->reconfigureExposureTimes (expTimes, idx));

    //Again in Capturing State
    ASSERT_NO_THROW (m_imager->stopCapture());

    //Ready State active
    ASSERT_NO_THROW (m_imager->sleep());
}

TEST_F (TestImagerM2453_A11_Hardcoded, InitImager)
{
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());

    const auto regConfig = m_bridge->getWrittenRegisters();

    //test if the firmware has been transferred correctly
    for (const auto mapEntry : fwPage1)
    {
        ASSERT_TRUE (regConfig.count (mapEntry.address) > 0);
        ASSERT_EQ (mapEntry.value, regConfig.at (mapEntry.address));
    }
    for (const auto mapEntry : fwPage2)
    {
        ASSERT_TRUE (regConfig.count (mapEntry.address) > 0);
        ASSERT_EQ (mapEntry.value, regConfig.at (mapEntry.address));
    }

    ASSERT_EQ (expectedRegisterCallsForHardcodedConfig, m_bridge->registerCalls());
}

/**
 * This is TestImagerM2453_A11_Hardcoded.InitImager, but with two extra registers added: 0xffff
 * followed by 0x0000. These addresses would be consecutive if the addresses wrapped round, the
 * test is to ensure that the software imager splits this in to two separate writes.
 */
TEST_F (TestImagerM2453_A11_Uncreated, WraproundIsNotConsecutive)
{
    const auto expectedCalls = narrow_cast<uint16_t> (expectedRegisterCallsForHardcodedConfig + 2);
    TimedRegisterList fwPage2PlusWraparound = fwPage2;
    assert (fwPage2.back().address != 0xfffe);
    assert (fwPage2.back().address != 0xffff);
    fwPage2PlusWraparound.push_back ({0xffff, 0x0001, 0u});
    fwPage2PlusWraparound.push_back ({0x0000, 0x0001, 0u});

    auto externalConfig = std::make_shared<WrapperImagerExternalConfig> (emptyTimedRegisterMap,
                          fwPage1, fwPage2PlusWraparound, emptyTimedRegisterMap, emptyTimedRegisterMap, emptyTimedRegisterMap,
                          flashBasedUseCaseData);
    createImager (std::move (externalConfig));

    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());

    const auto regConfig = m_bridge->getWrittenRegisters();

    //test if the firmware has been transferred correctly
    for (const auto mapEntry : fwPage1)
    {
        ASSERT_TRUE (regConfig.count (mapEntry.address) > 0);
        ASSERT_EQ (mapEntry.value, regConfig.at (mapEntry.address));
    }
    for (const auto mapEntry : fwPage2PlusWraparound)
    {
        ASSERT_TRUE (regConfig.count (mapEntry.address) > 0);
        ASSERT_EQ (mapEntry.value, regConfig.at (mapEntry.address));
    }

    ASSERT_EQ (expectedCalls, m_bridge->registerCalls());
}

TEST_F (TestImagerM2453_A11_Hardcoded, PseudoData)
{
    auto pdi = dynamic_cast<royale::imager::M2453_A11::PseudoDataInterpreter *> (m_imager->createPseudoDataInterpreter().get());
    ASSERT_NE (pdi, nullptr);
}

TEST_F (TestImagerM2453_A11_Hardcoded, NoLoggingListener)
{
    ASSERT_THROW (m_imager->setLoggingListener (nullptr), NotImplemented);
}

TEST_F (TestImagerM2453_A11_Hardcoded, GetSerialNumber)
{
    m_bridge->writeImagerRegister (M2453_A11::ANAIP_EFUSEVAL1, 0xA001);
    m_bridge->writeImagerRegister (M2453_A11::ANAIP_EFUSEVAL2, 0xB002);
    m_bridge->writeImagerRegister (M2453_A11::ANAIP_EFUSEVAL3, 0xC003);
    m_bridge->writeImagerRegister (M2453_A11::ANAIP_EFUSEVAL4, 0xD004);

    ASSERT_NO_THROW (m_imager->getSerialNumber());

    std::string serial = m_imager->getSerialNumber();

    ASSERT_EQ (serial, "A001-B002-C003-D004");
}

TEST_F (TestImagerM2453_A11_Lena, RejectInvalidUseCaseIdentifier)
{
    ASSERT_EQ (ImagerVerificationStatus::USECASE_IDENTIFIER,
               m_imager->verifyUseCase (ImagerUseCaseIdentifier { "not-content-of-externalImagerConfiguration" }));
}

TEST_F (TestImagerM2453_A11_Hardcoded, NotSupportSetExternalTrigger)
{
    ASSERT_THROW (m_imager->setExternalTrigger (true), InvalidValue);
    ASSERT_THROW (m_imager->setExternalTrigger (false), InvalidValue);
}

TEST_F (TestImagerM2453_A11_Lena, NotSupportReconfigureTargetFrameRate)
{
    uint16_t idx;
    ASSERT_THROW (m_imager->reconfigureTargetFrameRate (0u, idx), NotImplemented);
}

TEST_F (TestImagerM2453_A11_Hardcoded, MeasurementBlocksNoExecutingUseCase)
{
    ASSERT_THROW (m_imager->getMeasurementBlockSizes(), LogicError);
}

TEST_F (TestImagerM2453_A11_Lena, ExternalUseCaseConfigApplied)
{
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (m_extCfgUseCase1));

    const auto regConfig = m_bridge->getWrittenRegisters();

    //check first and last data item of the external config,
    //look at the externalImagerConfiguration file for these values
    ASSERT_TRUE (regConfig.count (0xA00C) > 0);
    ASSERT_EQ (0x0115, regConfig.at (0xA00C));
    ASSERT_TRUE (regConfig.count (0x9401) > 0);
    ASSERT_EQ (0x0002, regConfig.at (0x9401));
}

TEST_F (TestImagerM2453_A11_Lena, ExternalMultipleUseCases)
{
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (m_extCfgUseCase1));
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (m_extCfgUseCase2));
    ASSERT_NO_THROW (m_imager->executeUseCase (m_extCfgUseCase1));
    ASSERT_EQ (size_t{ 1 }, m_imager->getMeasurementBlockSizes().size());

    const auto regConfig = m_bridge->getWrittenRegisters();

    auto blocks = regConfig.at (0x91CE);

    ASSERT_EQ (blocks, m_imager->getMeasurementBlockSizes().at (0));

    //it is known that address 0x91CE will contain the sequence length
    ASSERT_TRUE (regConfig.count (0x91CE) > 0);

    //check first and last data item of the external config,
    //look at the externalImagerConfiguration file for these values
    ASSERT_TRUE (regConfig.count (0xA00C) > 0);
    ASSERT_EQ (0x0115, regConfig.at (0xA00C));
    ASSERT_TRUE (regConfig.count (0x9401) > 0);
    ASSERT_EQ (0x0002, regConfig.at (0x9401));

    ASSERT_NO_THROW (m_imager->executeUseCase (m_extCfgUseCase2));
    ASSERT_EQ (size_t{ 1 }, m_imager->getMeasurementBlockSizes().size());

    const auto regConfigUseCase2 = m_bridge->getWrittenRegisters();

    auto blocks2 = regConfigUseCase2.at (0x91CE);
    ASSERT_EQ (blocks2, m_imager->getMeasurementBlockSizes().at (0));

    //it is known that address 0x91CE will contain the sequence length
    ASSERT_TRUE (regConfigUseCase2.count (0x91CE) > 0);
}

TEST_F (TestImagerM2453_A11_Hardcoded, FlashTransferSmallConfigTest)
{
    //put a dummy test configuration into the flash
    static_assert (flashConfigSmallSizeInRegisters == 1, "Config does not match test code");
    const uint16_t flashTestData = 0x1234;
    pushMapBe16 (*m_simImager->getFlashMemorySpace(), flashConfigAddress, flashTestData);

    //execute the flash based use case
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (flashBasedSmallUseCaseId));

    //the configuration must have been copied from the flash into the imager memory space
    const auto imagerRegisters = m_simImager->getImagerRegisters();
    const auto spiEnable = 1u << 14;
    const auto spiClockDiv8 = 2u;
    const auto spiCfg = static_cast<uint16_t> (spiEnable | spiClockDiv8);
    ASSERT_TRUE (imagerRegisters.count (M2453::SPICFG));
    ASSERT_EQ (spiCfg, imagerRegisters.at (M2453::SPICFG));
    ASSERT_TRUE (imagerRegisters.count (M2453::PIXMEM));
    ASSERT_EQ (static_cast<uint16_t> ( (0x03 << 8) | flashConfigAddressUpper), imagerRegisters.at (M2453::PIXMEM));
    ASSERT_TRUE (imagerRegisters.count (M2453::PIXMEM + 1));
    ASSERT_EQ (flashConfigAddressLower, imagerRegisters.at (M2453::PIXMEM + 1));
    ASSERT_TRUE (imagerRegisters.count (M2453::CFGCNT));
    ASSERT_EQ (flashTestData, imagerRegisters.at (M2453::CFGCNT));
}

/**
 * This test is intended to check whether the SequentialRegisterHeader::imagerAddress is used.
 *
 * The current implementation of M2453_A11 simply sanity-checks that the imagerAddress is either
 * zero or the expected number. The test considers this a success, so if an exception is thrown then
 * the test passes; if the exception was thrown for any reason other than the imagerAddress, then
 * this would be shown by the failure of
 * TestImagerM2453_A11_Hardcoded::FlashTransferSmallConfigTest.
 */
TEST_F (TestImagerM2453_A11_Uncreated, FlashTransferNonDefaultLocation)
{
    const auto loadAddress = uint16_t (M2453::CFGCNT + 1);
    auto externalConfig = std::make_shared<WrapperImagerExternalConfig> (*fwAndUseCaseExternalConfig);
    for (auto &ucd : externalConfig->m_useCases)
    {
        ASSERT_EQ (M2453::CFGCNT, ucd.sequentialRegisterHeader.imagerAddress);
        ucd.sequentialRegisterHeader.imagerAddress = loadAddress;
    }
    createImager (std::move (externalConfig));

    //put a dummy test configuration into the flash
    static_assert (flashConfigSmallSizeInRegisters == 1, "Config does not match test code");
    const uint16_t flashTestData = 0x1234;
    pushMapBe16 (*m_simImager->getFlashMemorySpace(), flashConfigAddress, flashTestData);

    //execute the flash based use case
    bool useCaseRejected = false;
    try
    {
        m_imager->wake();
        m_imager->initialize();
        m_imager->executeUseCase (flashBasedSmallUseCaseId);
    }
    catch (...)
    {
        useCaseRejected = true;
        SUCCEED();
    }

    if (!useCaseRejected)
    {
        ADD_FAILURE() << "Test out of date? If M2453_A11 now supports SequentialRegisterHeader::imagerAddress, please update the test";

        // This should copy the gtest asserts from FlashTransferSmallConfigTest, with changes:
        // ASSERT_FALSE (imagerRegisters.count (M2453::CFGCNT)) << "Software imager ignored the imagerAddress";
        // ASSERT_TRUE (imagerRegisters.count (loadAddress)) << "Software imager changed behavior, but not to the imagerAddress";
        // ASSERT_EQ (flashTestData, imagerRegisters.at (loadAddress));
    }
}


TEST_F (TestImagerM2453_A11_Hardcoded, FlashTransferLargeConfigTest)
{
    //put a dummy test configuration into the flash
    auto flash = m_simImager->getFlashMemorySpace();
    for (size_t i = 0; i < flashConfigLargeSizeInRegisters; ++i)
    {
        pushMapBe16 (*flash, narrow_cast<uint32_t> (flashConfigAddress + 2 * i), static_cast<uint16_t> (i));
    }

    //execute the flash based use case
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (flashBasedLargeUseCaseId));

    //the configuration must have been copied from the flash into the imager memory space
    const auto imagerRegisters = m_simImager->getImagerRegisters();

    for (size_t cfgcntAddressOffset = 0; cfgcntAddressOffset < flashConfigLargeSizeInRegisters; ++cfgcntAddressOffset)
    {
        const auto cfgcntAddress = static_cast<uint16_t> (M2453::CFGCNT + cfgcntAddressOffset);
        ASSERT_TRUE (imagerRegisters.count (cfgcntAddress)) << " no entry found for address 0x" << std::hex << cfgcntAddress;
        ASSERT_EQ (static_cast<uint16_t> (cfgcntAddressOffset), imagerRegisters.at (cfgcntAddress));
    }
}
