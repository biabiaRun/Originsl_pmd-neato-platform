/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/exceptions/ValidButUnchanged.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/NarrowCast.hpp>
#include <factory/ImagerUseCaseDefinitionAdapter.hpp>
#include <imager/ImagerM2452_B1x_AIO.hpp>
#include <SimImagerM2452.hpp>
#include <StubBridgeImager.hpp>
#include <TestImagerCommon.hpp>
#include <usecase/UseCaseArbitraryPhases.hpp>

#include <cmath>
#include <chrono>
#include <thread>

using namespace royale::imager::M2452;

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;


namespace
{
    const uint32_t SYSFREQ = 24000000;

    // to define an invalid ROI
    class UseCaseROIInvalid : public UseCaseForM2452
    {
    public:
        UseCaseROIInvalid (uint16_t offsetColumn, uint16_t offsetRow, uint16_t sizeColumn, uint16_t sizeRow) : UseCaseForM2452()
        {
            m_imageColumns = 224 + 1;
            m_imageRows = 172 + 1;
        }
    };

    // a valid ROI
    class UseCaseROIValid2 : public UseCaseForM2452
    {
    public:
        UseCaseROIValid2() : UseCaseForM2452()
        {
            m_imageColumns = 128;
            m_imageRows = 96;
        }
    };

    class UseCaseAlterDCs : public UseCaseForM2452
    {
    public:
        UseCaseAlterDCs () : UseCaseForM2452()
        {
            m_rawFrames.at (0).dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_25;
        }
    };

    class UseCaseInvalidDC : public UseCaseForM2452
    {
    public:
        UseCaseInvalidDC() : UseCaseForM2452()
        {
            for (auto &rawFrame : m_rawFrames)
            {
                rawFrame.dutyCycle = ImagerRawFrame::ImagerDutyCycle::DC_75;
            }
        }
    };

    /*
    * We're trying to do basic tests for the M2452, but can't use the ImagerM2452 class directly
    * because it relies on some things to be implemented in derived classes.
    * A possible approach would be to implement a fake M2452-derived imager here, but unfortunately
    * this causes linker errors on MSVC (which would require lots of IMAGER_EXPORT declarations to fix).
    *
    * As a workaround, the B1x implementation is used.
    */
    const uint32_t imagerDesignStep = 0x0B12;
    using ImagerM2452_Type = ImagerM2452_B1x_AIO;

}

class TestImagerM2452 : public ::testing::Test
{
protected:
    TestImagerM2452()
    {

    }

    virtual ~TestImagerM2452()
    {

    }

    virtual void SetUp()
    {
        m_simImager.reset (new SimImagerM2452 (imagerDesignStep));
        m_bridge.reset (new StubBridgeImager (std::move (m_simImager)));

        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";

        createImager();
    }

    void createImager (ImgImageDataTransferType imageDataTransferType = ImgImageDataTransferType::MIPI_2LANE)
    {
        {
            ImagerParameters params{ m_bridge, nullptr, true, false,
                                     ImgTrigger::I2C, imageDataTransferType, 0.0000006f, {},
                                     SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                     ImgIlluminationPad::SE_P, 99900000, false };

            m_imager.reset (new ImagerM2452_Type (params));
        }

        ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";

        ASSERT_NO_THROW (m_imager->sleep());
        ASSERT_NO_THROW (m_imager->wake());
    }

    virtual void TearDown()
    {
        m_simImager.reset();
        m_bridge.reset();
        m_imager.reset();
    }

    void testExposureCalc (ImagerUseCaseFourPhase &ucs)
    {
        const uint16_t preScaleMap[] = { 1, 8, 32, 128 };

        // \todo ROYAL-2318 update tests for mixed mode
        auto exposureUpperLimit = ucs.getRawFrames().front().exposureTime;

        //use 100us steps to keep UT execution below 50ms
        for (uint32_t expoTime = 100u; expoTime < exposureUpperLimit; expoTime += 100)
        {
            ucs.setExposureTime (expoTime);

            //let bridge forget all previous register changes
            m_bridge->clearRegisters();

            //test only expo times supported by the imager
            if (ImagerVerificationStatus::SUCCESS == m_imager->verifyUseCase (ucs))
            {
                //use case will be executed using the mock bridge, no actual harware required
                ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

                //ask the bridge what has been written
                const auto regChanged = m_bridge->getWrittenRegisters();

                ASSERT_TRUE (regChanged.count (RECONFIG_CFGCNT_S00_EXPOTIME) > 0);

                //check by inverse calculation (devspec MiraLotte 1.4 / 8.6.4.5)
                //tillu = texpo x pre_scale x (1/clk_m0)
                const uint16_t preScale = preScaleMap[static_cast<uint16_t> (regChanged.at (RECONFIG_CFGCNT_S00_EXPOTIME) >> 14)];
                const uint16_t tExpo = static_cast<uint16_t> (regChanged.at (RECONFIG_CFGCNT_S00_EXPOTIME) & 0x3FFF);
                const float tIllu = (float) tExpo * (float) preScale * (1e6f / (float) ucs.getRawFrames().begin()->modulationFrequency);

                ASSERT_NEAR ( (float) expoTime, tIllu, 2.0f);
            }
        }
    }

    /**
     * Given a UseCaseDefinition which has already been executed by the imager, check that the
     * expected sequence registers have been set.  Check the all the LUT entries used by the
     * sequence have also been set.
     */
    void checkSequenceRegsAndLuts (const ImagerUseCaseDefinition &ucs, const std::map < uint16_t, uint16_t > &regChanged)
    {
        ASSERT_TRUE (regChanged.count (CFGCNT_CTRLSEQ) > 0);
        ASSERT_EQ (ucs.getRawFrames().size(), static_cast<std::size_t> (1 + (regChanged.at (CFGCNT_CTRLSEQ) & 0x1f)));

        for (std::size_t sequence = 0; sequence < ucs.getRawFrames().size(); sequence++)
        {
            const auto reconfigExpotime = narrow_cast<uint16_t> (RECONFIG_CFGCNT_S00_EXPOTIME + sequence * 3);
            const auto reconfigPs = narrow_cast<uint16_t> (RECONFIG_CFGCNT_S00_PS + sequence * 3);

            ASSERT_TRUE (regChanged.count (reconfigExpotime) > 0);
            ASSERT_TRUE (regChanged.count (reconfigPs) > 0);

            // check that the appropriate LUT has been configured
            const auto pllPreset = (regChanged.at (reconfigPs) >> 9) & 0x3;
            ASSERT_TRUE (regChanged.count (narrow_cast<uint16_t> (RECONFIG_CFGCNT_PLLCFG1_LUT1 + 0 + 3 * pllPreset)) > 0);
            ASSERT_TRUE (regChanged.count (narrow_cast<uint16_t> (RECONFIG_CFGCNT_PLLCFG1_LUT1 + 1 + 3 * pllPreset)) > 0);
            ASSERT_TRUE (regChanged.count (narrow_cast<uint16_t> (RECONFIG_CFGCNT_PLLCFG1_LUT1 + 2 + 3 * pllPreset)) > 0);
        }
    }

    void setImageDataTransferType (ImgImageDataTransferType imageDataTransferType)
    {
        createImager (imageDataTransferType);
    }

    std::unique_ptr <SimImagerM2452> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2452> m_imager;
};

TEST_F (TestImagerM2452, CreateImagerDirectly)
{
    ImagerParameters params1{ nullptr, nullptr, false, false, ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {}, 0u, ImagerRawFrame::ImagerDutyCycle::DC_0, ImgIlluminationPad::SE_P, 0u, false };

    ASSERT_THROW (new ImagerM2452_B1x_AIO (params1), LogicError);
}

TEST_F (TestImagerM2452, InitImager)
{
    ASSERT_NO_THROW (m_imager->initialize());
}

TEST_F (TestImagerM2452, GetSerialNumber)
{
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL1, 0xA001);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL2, 0xB002);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL3, 0xC003);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL4, 0xD004);

    ASSERT_NO_THROW (m_imager->getSerialNumber());

    std::string serial = m_imager->getSerialNumber();

    ASSERT_EQ (serial, "A001-B002-C003-D004");
}

TEST_F (TestImagerM2452, DoNotSupportDirectWrite)
{
    ASSERT_THROW (m_imager->writeRegisters ({}, {}), NotImplemented);
}

TEST_F (TestImagerM2452, CheckStates)
{
    UseCaseForM2452 ucs;
    uint16_t idx;

    ASSERT_NO_THROW (m_imager->sleep());

    //Power Down State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucs), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucs, idx), WrongState);
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), InvalidValue);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (false));

    ASSERT_NO_THROW (m_imager->wake());

    //Power Up State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucs), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucs, idx), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), InvalidValue);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (false));

    ASSERT_NO_THROW (m_imager->initialize());


    //Ready State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucs, idx), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (false), WrongState);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //Again in Ready State active
    ASSERT_NO_THROW (m_imager->startCapture());

    //Capturing State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->sleep(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucs), WrongState);
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), WrongState);

    ASSERT_NO_THROW (m_imager->reconfigure (ucs, idx));

    //Again in Capturing State
    ASSERT_NO_THROW (m_imager->stopCapture());

    //Ready State active
    ASSERT_NO_THROW (m_imager->sleep());
}

TEST_F (TestImagerM2452, InitAndExecuteForCSI2_MIPI1Lane)
{
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, imagerDesignStep);

    setImageDataTransferType (ImgImageDataTransferType::MIPI_1LANE);
    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0112);

    UseCaseForM2452 ucs;
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2452, InitAndExecuteForCSI2_MIPI2Lane)
{
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, imagerDesignStep);

    setImageDataTransferType (ImgImageDataTransferType::MIPI_2LANE);
    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0192);

    UseCaseForM2452 ucs;
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2452, StartCapture)
{
    ASSERT_NO_THROW (m_imager->initialize());

    //try start with MTCU busy
    m_bridge->writeImagerRegister (MTCU_STATUS, 0x0000);
    ASSERT_THROW (m_imager->startCapture(), LogicError);

    //try normal start
    m_bridge->writeImagerRegister (MTCU_STATUS, 0x0003);
    ASSERT_NO_THROW (m_imager->startCapture());
}

TEST_F (TestImagerM2452, StopCapture)
{
    m_bridge->writeImagerRegister (MTCU_STATUS, 0x0003);
    ASSERT_THROW (m_imager->stopCapture(), LogicError);
}

TEST_F (TestImagerM2452, ReconfigureExposurePositive)
{
    UseCaseForM2452 ucs;

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with a valid use case
    uint16_t frameNumber = 0;
    ucs.setExposureTime (101);

    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));
}

TEST_F (TestImagerM2452, ReconfigureExposureNegative)
{
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with an invalid use case
    UseCaseROIInvalid ucs (0, 0, 0, 0);
    uint16_t frameNumber = 0;
    ASSERT_THROW (m_imager->reconfigure (ucs, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2452, UseCaseMixedModeSupported)
{
    UseCaseCustomMM_M2452 ucs;
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseVerifyROIModPositive)
{
    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should be able to accept.
    */
    UseCaseROIValid2 ucs;

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseVerifyROIModNegative)
{
    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalid ucs (0, 0, 1, 1);

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseVerifyROIColumn)
{
    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalid ucs (31, 31, 17, 17);

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseVerifyROIExceedMaxImageSize)
{
    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalid ucs (0, 0, 224 + 1, 172 + 1);

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseInvalidRawFrameCount)
{
    ImagerUseCaseFourPhase ucLower (50u, 0);
    ASSERT_NE (m_imager->verifyUseCase (ucLower), ImagerVerificationStatus::SUCCESS);

    ImagerUseCaseFourPhase ucUpper (50u, 18);
    ASSERT_NE (m_imager->verifyUseCase (ucUpper), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, ValidPllFreqRange)
{
    /*
    * Create an instance of a ImagerM2450_A12
    * and try to set a modfreq above or below
    * the allowed freqrange.
    */
    UseCaseCustomFreq_M2452 ucsCustom;

    //test 3MHz
    ucsCustom.setFrequency (3000000);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);

    //test 101MHz
    ucsCustom.setFrequency (101000000);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);
}

TEST_F (TestImagerM2452, CustomModPllFreq)
{
    ASSERT_NO_THROW (m_imager->initialize());

    /*
    * Create an instance of a ImagerM2452 and execute
    * an use-case with one modified modfreq. The only registers
    * that are written during execution should be the 3 pllcfg registers for the new freq.
    */
    UseCaseCustomFreq_M2452 ucs;
    UseCaseCustomFreq_M2452 ucsCustom;
    ucsCustom.setFrequency (60240000);

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //should be 1 x CFGCNT_CTRLSEQ + 3 x pllfreq + 4 x expotime
    //note: target frame rate register not depends on expotime/pllfreq when using LPFSM
    EXPECT_EQ (8, static_cast<int> (regChanged.size()));
}

TEST_F (TestImagerM2452, InvalidDC)
{
    ASSERT_NO_THROW (m_imager->initialize());

    /*
    * Create an instance of a ImagerM2452 and test
    * if an invalid duty cycle is not accepted.
    */
    UseCaseInvalidDC ucsInvalid;

    ASSERT_EQ (m_imager->verifyUseCase (ucsInvalid), ImagerVerificationStatus::DUTYCYCLE);
}

TEST_F (TestImagerM2452, CustomDCs)
{
    ASSERT_NO_THROW (m_imager->initialize());

    /*
    * Create an instance of a ImagerM2452 and execute
    * an use-case with modified duty cycle.
    */
    UseCaseForM2452 ucs;
    UseCaseAlterDCs ucsCustom25;

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom25), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom25));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_NE (0u, regChanged.size());
}

TEST_F (TestImagerM2452, DISABLED_CustomExpoTimeLowModFreq)
{
    ASSERT_NO_THROW (m_imager->initialize());

    /*
    * Create an instance of a ImagerM2452 and execute
    * an use-case with modified exposure time.
    */

    UseCaseCustomFreq_M2452 ucs;
    ucs.setFrequency (3200000);

    ASSERT_NO_FATAL_FAILURE (testExposureCalc (ucs));
}

TEST_F (TestImagerM2452, DISABLED_CustomExpoTimeHighModFreq)
{
    ASSERT_NO_THROW (m_imager->initialize());

    /*
    * Create an instance of a ImagerM2452 and execute
    * an use-case with modified exposure time.
    */

    UseCaseCustomFreq_M2452 ucs;
    ucs.setFrequency (99900000);

    ASSERT_NO_FATAL_FAILURE (testExposureCalc (ucs));
}

TEST_F (TestImagerM2452, DISABLED_PllsForDefaultModulation)
{
    /**
    * This uses the same frequencies as the use-case that's built in to the Lotte by default.
    * Check that the PLLs are correctly set for these frequencies.
    */
    ImagerUseCaseFourPhase ucs (5u, 8, 1000u, 60240000u, 80320000u, 1000u);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_FATAL_FAILURE (checkSequenceRegsAndLuts (ucs, m_bridge->getWrittenRegisters()));
}

TEST_F (TestImagerM2452, UseCaseVerifyHighExpoNegative)
{
    //for a high modulation frequency only a certain expotime works because of counter limitations

    ImagerUseCaseFourPhase ucs (10u, 4, 24000u, 99900000u, 0u, 0u);
    ucs.setImage (224, 172);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::EXPOSURE_TIME);
}

TEST_F (TestImagerM2452, ConfigureLowestExpoGrayScale)
{
    ASSERT_NO_THROW (m_imager->initialize());

    ImagerUseCaseFourPhase ucs (45u, 0, 0u, 0u, 0u, 8u);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

// \todo ROYAL-1674 this test is not valid anymore and shall be re-implemented for the mixed mode firmware
TEST_F (TestImagerM2452, DISABLED_UseCaseValidFrameRate)
{
    ASSERT_NO_THROW (m_imager->initialize());

    const uint32_t targetFrameRate = 50u;

    UseCaseForM2452 ucs;
    ucs.setTargetRate (targetFrameRate);
    ucs.setExposureTime (100);

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //verify if the target frame rate register is set
    ASSERT_TRUE (regChanged.count (RECONFIG_FRAMERATE) > 0);

    //use calculation from DevSpec 2.1 "LPFSMFRATE" register
    const float frameRate = 1.f / (128.f * regChanged.at (RECONFIG_FRAMERATE) * (1.f / static_cast<float> (SYSFREQ)));

    //check for match with targetFrameRate (allow just a small deviation - quantization)
    ASSERT_LT (::fabs (frameRate - static_cast<float> (targetFrameRate)), 0.02);

    //check that the LUTs have been configured
    ASSERT_NO_FATAL_FAILURE (checkSequenceRegsAndLuts (ucs, m_bridge->getWrittenRegisters()));
}

// \todo ROYAL-1674 this test is not valid anymore and shall be re-implemented for the mixed mode firmware
TEST_F (TestImagerM2452, DISABLED_UseCaseExecuteRawFrameRateLowerThanTargetFrameRate)
{
    ASSERT_NO_THROW (m_imager->initialize());

    //Test the mixed usage of raw frame rates and a target frame rate.
    //It is verified if the single raw frame rates are set and, as for the LPFSM
    //implementation required, the last raw frame rate is not changed to fit the
    //target frame rate. The target frame rate is set via the LPFSM frame rate register.
    //All raw frame rates together must not exceed the target framerate.
    const float FSYSCLK = 133333333.3f; //this test is only for imagers with this sysclock
    const uint32_t targetFrameRate = 35u;
    const uint32_t rawFrameRate = 200u;
    const float rawFrames = 4.f;

    //a four phase use case without a grayscale image
    ImagerUseCaseFourPhase ucs (targetFrameRate, 4, 1000u, 99900000u, 0u, 0u);
    ucs.setImage (224, 172);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //now set a raw frame rate and verify changed registers
    ucs.setRawFrameRate (rawFrameRate);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //should be the four frame rate registers and nothing else
    ASSERT_EQ (static_cast<int> (regChanged.size()), 4);

    // \todo ROYAL-1674 the part below may change with the new mixed mode firmware
    // the test still makes sense the check raw frame rate / target frame rate
    // will change to a check for raw frame rate / raw frame set rate due to the low power feature
    auto tSequence = 0.f;

    for (auto reg : regChanged)
    {
        tSequence += (static_cast<float> (1024u * reg.second) / FSYSCLK);
    }

    auto fSequence = rawFrames / tSequence;

    //check for match with specified raw frame rates (allow just a small deviation - quantization)
    ASSERT_LT (::fabs (fSequence - static_cast<float> (rawFrameRate)), 0.02);

    ASSERT_GT (fSequence / rawFrames, static_cast<float> (targetFrameRate));
}

TEST_F (TestImagerM2452, UseCaseInvalidFrameRate)
{
    UseCaseForM2452 ucs;
    ucs.setTargetRate (1000u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2452, UseCaseValidLowFrameRate)
{
    UseCaseForM2452 ucs;
    ucs.setTargetRate (3u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2452, UseCaseInvalidLowFrameRate)
{
    UseCaseForM2452 ucs;
    ucs.setTargetRate (0u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2452, UseCaseVerifyRawFrameRateNegative)
{
    const uint32_t targetFrameRate = 41u;

    UseCaseForM2452 ucs;
    ucs.setTargetRate (targetFrameRate);

    //4 raw frames with 160Hz are only possible for a target frame rate higher than 40fps
    ucs.setRawFrameRate (static_cast<uint32_t> (4u * (targetFrameRate - 1)));

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2452, UseCaseExecuteRawFrameRateEqualTargetFrameRateHighExpoNegative)
{
    ASSERT_NO_THROW (m_imager->initialize());

    const uint32_t targetFrameRate = 10u; //use low value - this UT is only about raw frame rates

    //a four phase use case without a grayscale image
    ImagerUseCaseFourPhase ucs (targetFrameRate, 4, 5000u, 99900000u, 0u, 0u);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //now set a raw frame rate (4.9ms) that shouldn't be possible with 5ms expo time
    ucs.setRawFrameRate (201);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2452, CheckIfInterfaceDelayIsSet)
{
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, imagerDesignStep);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_ROS) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_ROS) == 0x0500); //this ifdel is required only for Toshiba interface chip
}

TEST_F (TestImagerM2452, UseCaseExecuteRecover)
{
    ASSERT_NO_THROW (m_imager->initialize());

    UseCaseForM2452 ucs;
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //change something to provoke register configuration
    ucs.setTargetRate (20u);

    m_bridge->setCorruptedCommunication ();

    ASSERT_THROW (m_imager->executeUseCase (ucs), ValidButUnchanged);
}

TEST_F (TestImagerM2452, UseCaseArbitraryPhase)
{
    royale::usecase::UseCaseArbitraryPhases simple (10u,
            royale::Vector < royale::usecase::UseCaseArbitraryPhaseSetting >
    {
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::GrayScaleIlluminationOff,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000u}, 100u
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            80320000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u, 10000., 0.5, 0.0125
        },
        royale::usecase::UseCaseArbitraryPhaseSetting{
            royale::usecase::UseCaseArbitraryPhaseSettingType::FourPhase,
            60240000u, royale::Pair < uint32_t, uint32_t > {1u, 2000}, 500u, 10000., 0.5, 0.0125
        }
    },
    true);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_EQ (m_imager->verifyUseCase (royale::factory::ImagerUseCaseDefinitionAdapter (simple, 0, 0, 0)), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (royale::factory::ImagerUseCaseDefinitionAdapter (simple, 0, 0, 0)));
}
