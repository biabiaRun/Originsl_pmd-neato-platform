/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <StubBridgeImager.hpp>
#include <imager/ImagerM2452_B1x_AIO.hpp>
#include <SimImagerM2452.hpp>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/WrongState.hpp>
#include <TestImagerCommon.hpp>
#include <imager/M2452/PseudoDataInterpreter.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>

using namespace royale::imager::M2452;

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 24000000;
    const uint16_t AIO_CFGCNT_PLLCFG1_LUT1 = 0xb334;

    class UseCaseMixedSSCDisabled : public UseCaseCustomMM_M2452
    {
    public:
        UseCaseMixedSSCDisabled() :
            UseCaseCustomMM_M2452 (6u, 2u, 135u, 135u, 80320000u, 80320000u, 80320000u, 0u, false)
        {
            m_imageColumns = 32;
            m_imageRows = 32;
        }
    };

    class UseCaseMixedSSCEnabled : public UseCaseCustomMM_M2452
    {
    public:
        UseCaseMixedSSCEnabled() :
            UseCaseCustomMM_M2452 (6u, 2u, 135u, 135u, 80320000u, 60240000, 80320000u, 0u, false,
                                   true, 10000., 0.5, 0.0125, 0.0166)
        {
            m_imageColumns = 32;
            m_imageRows = 32;
        }
    };

    class UseCaseNormalSSCDisabled : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseNormalSSCDisabled() : ImagerUseCaseFourPhase (5u, 8, 2000u, 80320000u, 60240000u)
        {
            m_imageColumns = 32;
            m_imageRows = 32;
        }
    };

    class UseCaseNormalSSCEnabled : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseNormalSSCEnabled() :
            ImagerUseCaseFourPhase (5u, 8, 2000u, 80320000u, 60240000u, 0u, true, true, 10000., 0.5, 0.0125, 0.0166)
        {
            m_imageColumns = 32;
            m_imageRows = 32;
        }
    };

    const uint16_t AIO_NR_CFGCNT_S00_PLLSET = 0xB303;
    const uint16_t AIO_NR_CFGCNT_PLLCFG1_LUT1 = 0xb334;
    const uint16_t PLLCFG4_SSC_LUTx = 0xb398;
    const uint16_t PLLCFG5_SSC_LUTx = 0xb399;
    const uint16_t PLLCFG6_SSC_LUTx = 0xb39a;
    const uint16_t PLLCFG7_SSC_LUTx = 0xb39b;
    const uint16_t PLLCFG8_SSC_LUTx = 0xb39c;
}

class TestImagerM2452_B1x_AIO : public ::testing::Test
{
protected:
    TestImagerM2452_B1x_AIO()
    {

    }

    virtual ~TestImagerM2452_B1x_AIO()
    {

    }

    virtual void SetUp()
    {
        const ::testing::TestInfo *const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();

        std::stringstream s;
        s << test_info->test_case_name() << "_" << test_info->name() << ".log";

        m_logFile.open (s.str());

        m_simImager.reset (new SimImagerM2452 (0xB11));
        m_bridge.reset (new StubBridgeImager (std::move (m_simImager), m_logFile));
        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";

        {
            ImagerParameters params{ m_bridge, nullptr, true, false,
                                     ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.0000006f, {},
                                     SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                     ImgIlluminationPad::SE_P, 90000000, false };

            m_imager.reset (new ImagerM2452_B1x_AIO (params));
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
        m_logFile.close();
    }

    void initImager()
    {
        m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0B12);

        ASSERT_NO_THROW (m_imager->initialize());
    }

    void testExpoTime (float modFreq, float expoTime, uint16_t regExpo)
    {
        const uint16_t preScaleMap[] = { 1, 8, 32, 128 };

        //check by inverse calculation (devspec MiraLotte 1.4 / 8.6.4.5)
        //tillu = texpo x pre_scale x (1/clk_m0)
        const uint16_t preScale = preScaleMap[static_cast<uint16_t> (regExpo >> 14)];
        const uint16_t tExpo = static_cast<uint16_t> (regExpo & 0x3FFF);
        const float tIllu = (float) tExpo * (float) preScale * (1e6f / modFreq);

        ASSERT_NEAR ( (float) expoTime, tIllu, 2.0f);
    }

    void checkTargetFrameRate (const ImagerUseCaseDefinition &ucs, const uint16_t lpfsmFrameRate, const uint16_t regCounterReload, const uint16_t regInserts)
    {
        const double targetFrameRateExpected = static_cast<double> (ucs.getTargetRate());

        m_bridge->clearRegisters();
        ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

        const auto regChanged = m_bridge->getWrittenRegisters();

        ASSERT_TRUE (regChanged.count (regCounterReload) > 0);
        ASSERT_TRUE (regChanged.count (regInserts) > 0);

        const double lpfsmcountertime = regChanged.at (lpfsmFrameRate) / (SYSFREQ / 128.);
        const double lowfrcountertime = regChanged.at (regCounterReload) / (SYSFREQ / 64.);
        const double targetFrameRate = 1. / ( (1. + regChanged.at (regInserts) + 2.) * lowfrcountertime);

        ASSERT_NEAR (targetFrameRateExpected, targetFrameRate, 0.0001);
        ASSERT_NEAR (3., 1. / (targetFrameRateExpected * lpfsmcountertime), 0.0001);
    }

    std::ofstream m_logFile;
    std::unique_ptr <SimImagerM2452> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2452_B1x_AIO> m_imager;
};

TEST_F (TestImagerM2452_B1x_AIO, CreateImagerDirectly)
{
    ImagerParameters params1{ nullptr, nullptr, false, false, ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {}, 0u, ImagerRawFrame::ImagerDutyCycle::DC_0, ImgIlluminationPad::SE_P, 0u, false };

    ASSERT_THROW (new ImagerM2452_B1x_AIO (params1), LogicError);
}

TEST_F (TestImagerM2452_B1x_AIO, UnsupportedTransmissionMode)
{
    ImagerParameters params{ m_bridge, nullptr, false, false,
                             ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.0000006f, {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 90000000, false };

    ASSERT_THROW (std::unique_ptr<royale::imager::ImagerM2452_B1x_AIO> (new ImagerM2452_B1x_AIO (params)), InvalidValue);
}

TEST_F (TestImagerM2452_B1x_AIO, InitImager)
{
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0FFF);

    ASSERT_THROW (m_imager->initialize(), Exception);

    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0B12);

    ASSERT_NO_THROW (m_imager->initialize());

    //verify if the initialization has downloaded the firmware
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ISM_FWSTARTADDRESS) > 0);

    //verify that the imager prevents double initialization
    ASSERT_THROW (m_imager->initialize(), WrongState);

}

TEST_F (TestImagerM2452_B1x_AIO, ClonePseudoData)
{
    auto pdi = dynamic_cast<PseudoDataInterpreter *> (m_imager->createPseudoDataInterpreter().get());
    ASSERT_NE (pdi, nullptr);
}

TEST_F (TestImagerM2452_B1x_AIO, StartCapture)
{
    initImager();

    //try start with MTCU busy
    m_bridge->writeImagerRegister (MTCU_STATUS, 0x0000);
    ASSERT_THROW (m_imager->startCapture(), LogicError);

    //try normal start
    m_bridge->writeImagerRegister (MTCU_STATUS, 0x0003);
    ASSERT_NO_THROW (m_imager->startCapture());
}

TEST_F (TestImagerM2452_B1x_AIO, ValidPllFreqRange)
{
    UseCaseCustomFreq_M2452 ucsCustom (true);
    const uint32_t freq1 = 80320000;
    const uint32_t freq2 = 60240000;

    ucsCustom.setFrequency (freq1 - 1);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);

    ucsCustom.setFrequency (freq1);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::SUCCESS);

    ucsCustom.setFrequency (freq1 + 1);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);

    ucsCustom.setFrequency (freq2 - 1);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);

    ucsCustom.setFrequency (freq2);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::SUCCESS);

    ucsCustom.setFrequency (freq2 + 1);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureExposurePositive)
{
    UseCaseCustomFreq_M2452 ucs (true);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with a valid use case
    uint16_t frameNumber = 0;
    ucs.setExposureTime (101);

    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureGrayscaleExposurePositive)
{
    UseCaseForM2452 ucs (true, 100u);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with a valid use case
    auto expoTime = ucs.getRawFrames().at (1).exposureTime;
    ucs.setExposureTime (expoTime);

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureExposureNegative)
{
    UseCaseForM2452 ucs (true);
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with an invalid use case
    UseCaseCustomFreq_M2452 ucsCustom (true);
    const uint32_t freq1 = 80320000;
    uint16_t frameNumber = 0;
    ucsCustom.setFrequency (freq1 - 1);

    ASSERT_THROW (m_imager->reconfigure (ucsCustom, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureSequenceLengthChangeNormalMode)
{
    UseCaseForM2452 ucs (true);
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with an invalid use case
    UseCaseForM2452 ucsCustom (true, 135u);

    uint16_t frameNumber = 0;
    ASSERT_THROW (m_imager->reconfigure (ucsCustom, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureSequenceLengthChangeMixedMode)
{
    UseCaseCustomMM_M2452 ucs (3u, 1u);
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with an invalid use case
    UseCaseCustomMM_M2452 ucsChanged (3u, 3u);
    uint16_t frameNumber = 0;
    ASSERT_THROW (m_imager->reconfigure (ucsChanged, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureDutyCycleChange)
{
    UseCaseForM2452 ucs (true);
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    ucs.setDutyCycle (ImagerRawFrame::ImagerDutyCycle::DC_25); //default is 50%, change it

    uint16_t frameNumber = 0;
    ASSERT_THROW (m_imager->reconfigure (ucs, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2452_B1x_AIO, CheckTargetFrameRate1fpsNormal)
{
    ASSERT_NO_THROW (m_imager->initialize());
    UseCaseNormalSSCEnabled ucs;
    ucs.setTargetRate (1u);

    const uint16_t AIO_LPFSMFRATE = 0xB300;
    const uint16_t AIO_LOWFPS_COUNTERRELOAD = 0xB340;
    const uint16_t AIO_LOWFPS_INSERTS = 0xB341;
    checkTargetFrameRate (ucs, AIO_LPFSMFRATE, AIO_LOWFPS_COUNTERRELOAD, AIO_LOWFPS_INSERTS);
}

TEST_F (TestImagerM2452_B1x_AIO, CheckTargetFrameRate1fpsMixed)
{
    ASSERT_NO_THROW (m_imager->initialize());
    UseCaseMixedSSCEnabled ucs;
    ucs.setTargetRate (1u);

    const uint16_t MM_MB_LPFSMFR = 0xB30F;
    const uint16_t MM_MB_LOWFPS_COUNTERRELOAD = 0xB310;
    const uint16_t MM_MB_LOWFPS_INSERTS = 0xB311;

    checkTargetFrameRate (ucs, MM_MB_LPFSMFR, MM_MB_LOWFPS_COUNTERRELOAD, MM_MB_LOWFPS_INSERTS);
}

TEST_F (TestImagerM2452_B1x_AIO, CheckTargetFrameRate2fpsNormal)
{
    ASSERT_NO_THROW (m_imager->initialize());
    UseCaseNormalSSCEnabled ucs;
    ucs.setTargetRate (2u);

    const uint16_t AIO_LPFSMFRATE = 0xB300;
    const uint16_t AIO_LOWFPS_COUNTERRELOAD = 0xB340;
    const uint16_t AIO_LOWFPS_INSERTS = 0xB341;
    checkTargetFrameRate (ucs, AIO_LPFSMFRATE, AIO_LOWFPS_COUNTERRELOAD, AIO_LOWFPS_INSERTS);
}

TEST_F (TestImagerM2452_B1x_AIO, SSC_ReconfigUseCase)
{
    ImagerUseCaseFourPhase ucs (5u, 8, 1000u, 80320000u, 60240000u, 1000u, true, true, 10000., 0.5, 0.0125, 0.0166);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //only CFGCNT_CTRLSEQ should have been written
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_EQ (1u, regChanged.size());
}

TEST_F (TestImagerM2452_B1x_AIO, PllConfigFor80320kHz)
{
    UseCaseCustomFreq_M2452 ucsCustom (true);
    const uint32_t freq1 = 80320000;
    ucsCustom.setFrequency (freq1);

    m_logFile << "init imager...\n";

    ASSERT_NO_THROW (m_imager->initialize());

    m_logFile << "execute use case...\n";

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ref_clk_i;output_frequency;enable_wavegen;fssc;kspread;delta;anaip_pllcfg1;anaip_pllcfg2;anaip_pllcfg3;anaip_pllcfg4;anaip_pllcfg5;anaip_pllcfg6;anaip_pllcfg7;anaip_pllcfg8
    const std::vector<double> testVector{ 24000000, 321280000, 1, 10000, 0.5000, 0.0125, 25185, 64662, 3810, 21801, 37224, 18381, 22290, 2, };

    //the frequency should be assigned to LUT number 1
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET) & ~0xf9ff, 0); //the LUT assignment to LUT1

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) & 1 << 10, 0); //the enable ssc bit

    ASSERT_TRUE (regChanged.count (PLLCFG4_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG5_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG6_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG7_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG8_SSC_LUTx) > 0);

    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1), testVector.at (6));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1), testVector.at (7));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2), testVector.at (8));
    ASSERT_EQ (regChanged.at (PLLCFG4_SSC_LUTx), testVector.at (9));
    ASSERT_EQ (regChanged.at (PLLCFG5_SSC_LUTx), testVector.at (10));
    ASSERT_EQ (regChanged.at (PLLCFG6_SSC_LUTx), testVector.at (11));
    ASSERT_EQ (regChanged.at (PLLCFG7_SSC_LUTx), testVector.at (12));
    ASSERT_EQ (regChanged.at (PLLCFG8_SSC_LUTx), testVector.at (13));
}

TEST_F (TestImagerM2452_B1x_AIO, PllConfigFor60240kHz)
{
    UseCaseCustomFreq_M2452 ucsCustom (true);
    const uint32_t freq1 = 60240000;
    ucsCustom.setFrequency (freq1);

    m_logFile << "init imager...\n";

    ASSERT_NO_THROW (m_imager->initialize());

    m_logFile << "execute use case...\n";

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ref_clk_i;output_frequency;enable_wavegen;fssc;kspread;delta;anaip_pllcfg1;anaip_pllcfg2;anaip_pllcfg3;anaip_pllcfg4;anaip_pllcfg5;anaip_pllcfg6;anaip_pllcfg7;anaip_pllcfg8
    const std::vector<double> testVector{ 24000000, 240960000, 1, 10000, 0.5000, 0.0166, 33349, 15729, 1546, 21801, 59442, 13556, 22290, 2, };

    //the frequency should be assigned to LUT number 1, and this LUT must have the SSC enabled
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET) & ~0xf9ff, 0); //the LUT assignment to LUT1

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) & 1 << 10, 0); //the enable ssc bit

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1) > 0);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1) > 0);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG4_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG5_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG6_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG7_SSC_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (PLLCFG8_SSC_LUTx) > 0);

    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1), testVector.at (6));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1), testVector.at (7));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2), testVector.at (8));
    ASSERT_EQ (regChanged.at (PLLCFG4_SSC_LUTx), testVector.at (9));
    ASSERT_EQ (regChanged.at (PLLCFG5_SSC_LUTx), testVector.at (10));
    ASSERT_EQ (regChanged.at (PLLCFG6_SSC_LUTx), testVector.at (11));
    ASSERT_EQ (regChanged.at (PLLCFG7_SSC_LUTx), testVector.at (12));
    ASSERT_EQ (regChanged.at (PLLCFG8_SSC_LUTx), testVector.at (13));
}

TEST_F (TestImagerM2452_B1x_AIO, UseCaseSupportStandardEightPhase)
{
    ImagerUseCaseFourPhase ucs (5u, 8, 1000u, 80320000u, 60240000u, 1000u, true);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //do just some sanity checks to validate that this use case has been configured
    ASSERT_TRUE (regMMConfig.count (CFGCNT_CTRLSEQ) > 0);
    ASSERT_EQ (regMMConfig.at (CFGCNT_CTRLSEQ), 0x8008); //safe reconfig enabled and 9 raw frames

    ASSERT_TRUE (regMMConfig.count (0xB301) > 0); //an exposure register has been set
}

TEST_F (TestImagerM2452_B1x_AIO, UseCaseSupportCalibration)
{
    UseCaseCalibrationForM2452 ucs;
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    initImager();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    ucs.setSecondToLastExpo (10u);

    uint16_t reconfigIdx = 0u;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, UseCaseSupportStandardMixedMode)
{
    //1:1 ratio, one 4+1 and one 8+1 per sequence
    UseCaseCustomMM_M2452 ucs (3u, 1u, 1000u, 1000u, 30000000u, 20200000u, 20600000u, 1000u);

    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //do just some sanity checks to validate that this use case has been configured
    const uint16_t MM_REPEAT_START = 0xB2F8;
    const size_t usedMBs = 3; //the 4+1 and 8+1 raw frames should fit into 3 MBs

    for (size_t adrOffset = 0; adrOffset < 8; adrOffset++)
    {
        const uint16_t adrMBRepeat = static_cast<uint16_t> (MM_REPEAT_START + adrOffset);
        ASSERT_TRUE (regMMConfig.count (adrMBRepeat) > 0);
        ASSERT_EQ (regMMConfig.at (adrMBRepeat), adrOffset < usedMBs ? 1u : 0u);
    }

    const uint16_t safeReconfigFlag = 0x8000;
    ASSERT_TRUE (regMMConfig.count (0xB312) > 0);

    ASSERT_EQ (safeReconfigFlag | 4u, regMMConfig.at (0xB312)); //4 means 5 raw frames, and it should have the safe reconfig bit set

    ASSERT_TRUE (regMMConfig.count (0xB325) > 0);
    ASSERT_EQ (3u, regMMConfig.at (0xB325)); //3 means 4 raw frames, not safe for reconfig (this is the first part of the 8+1)

    ASSERT_TRUE (regMMConfig.count (0xB338) > 0);
    ASSERT_EQ (safeReconfigFlag | 4u, regMMConfig.at (0xB338)); //4 means 5 raw frames, and it should have the safe reconfig bit set
}

TEST_F (TestImagerM2452_B1x_AIO, UseCaseSupportIrregularMixedMode)
{
    //1:1 ratio, one 4+1 and one 8+1 per sequence
    UseCaseCustomMM_M2452 ucs (3u, 1u, 135u, 135u, 80320000u, 60240000, 80320000u, 135u, false,
                               false, 0., 0., 0., 0.0, true);
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //do just some sanity checks to validate that this use case has been configured
    const uint16_t MM_REPEAT_START = 0xB2F8;
    const size_t usedMBs = 3; //the 4+1 and 8+1 raw frames should fit into 3 MBs

    for (size_t adrOffset = 0; adrOffset < 8; adrOffset++)
    {
        const uint16_t adrMBRepeat = static_cast<uint16_t> (MM_REPEAT_START + adrOffset);
        ASSERT_TRUE (regMMConfig.count (adrMBRepeat) > 0);
        ASSERT_EQ (regMMConfig.at (adrMBRepeat), adrOffset < usedMBs ? 1u : 0u);
    }

    const uint16_t safeReconfigFlag = 0x8000;

    ASSERT_TRUE (regMMConfig.count (0xB312) > 0);
    ASSERT_EQ (safeReconfigFlag | 4u, regMMConfig.at (0xB312)); //4 means 5 raw frames, and it should have the safe reconfig bit set

    ASSERT_TRUE (regMMConfig.count (0xB325) > 0);
    ASSERT_EQ (3u, regMMConfig.at (0xB325)); //3 means 4 raw frames, not safe for reconfig (this is the first part of the 8+1)

    ASSERT_TRUE (regMMConfig.count (0xB338) > 0);
    ASSERT_EQ (safeReconfigFlag | 4u, regMMConfig.at (0xB338)); //4 means 5 raw frames, and it should have the safe reconfig bit set
}

TEST_F (TestImagerM2452_B1x_AIO, UseCaseMixedModeMBLimit)
{
    //this requires a special UCD type with alternating RFS to prevent usage of repeats
    UseCaseAlternateRF_M2452 ucsOk (8);
    UseCaseAlternateRF_M2452 ucsNotOk (9);

    initImager();

    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucsOk));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsOk));

    //9 MBs are not feasible and shouldn't be accepted
    ASSERT_EQ (ImagerVerificationStatus::SEQUENCER, m_imager->verifyUseCase (ucsNotOk));
}

//test executes too slow, thus disabled
TEST_F (TestImagerM2452_B1x_AIO, DISABLED_UseCaseMixedModeRepeatsLimit)
{
    //use case that fills all MBs
    UseCaseCustomMM_M2452 ucsNotOk (1u, 255u * 8u);
    initImager();

    ASSERT_EQ (ImagerVerificationStatus::SEQUENCER, m_imager->verifyUseCase (ucsNotOk));
}

TEST_F (TestImagerM2452_B1x_AIO, VerifyExpoCalcForMBs)
{
    initImager();

    const uint32_t expoTestTimeMB1 = 2000u;
    const uint32_t expoTestTimeMB2 = 135u;
    const uint32_t modFreq1 = 80320000;
    const uint32_t modFreq2 = 60240000;

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    UseCaseCustomMM_M2452 ucd (3u, 1u, expoTestTimeMB1, expoTestTimeMB2, modFreq1, modFreq2, modFreq2);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucd));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    {
        const uint16_t MB1_EXPO = 0xB300;
        ASSERT_TRUE (regChanged.count (MB1_EXPO) > 0);
        testExpoTime ( (float) modFreq1, (float) expoTestTimeMB1, regChanged.at (MB1_EXPO));
    }

    {
        const uint16_t MB2_EXPO = 0xB313;
        ASSERT_TRUE (regChanged.count (MB2_EXPO) > 0);
        testExpoTime ( (float) modFreq2, (float) expoTestTimeMB2, regChanged.at (MB2_EXPO));
    }
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigWithNormalMode)
{
    /* The following test procedure is also pre-si verified
    Interleaved disabled
    Start measurement
    Write new configurations
    Have the system reconfigured
    */
    ImagerUseCaseFourPhase ucsNormal (400u, 8, 8u, 80320000u, 60240000u, 8u, true);
    ucsNormal.setImage (32, 32);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsNormal));

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsNormal.setExposureTime (10u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsNormal, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigWithMixedMode)
{
    /* The following test procedure is also pre-si verified
    Interleaved disabled
    Start measurement
    Write new configurations
    Have the system reconfigured
    */
    UseCaseCustomMM_M2452 ucsMM (200u, 6u, 135u, 135u, 80320000u, 80320000u, 80320000u, 135u);
    ucsMM.setImage (32, 32);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsMM));

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsMM.setExposureTime (145);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigMixedModeTwice)
{
    UseCaseCustomMM_M2452 ucsMM (200u, 6u, 135u, 135u, 80320000u, 80320000u, 80320000u, 135u);
    ucsMM.setImage (32, 32);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsMM));

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsMM.setExposureTime (145);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));

    // need to convince the imager that the previous reconfigure was completed...
    const auto AIO_SR_RECONFIGFLAGS = CFGCNT_S15_PS;
    m_bridge->getImager().writeRegister (AIO_SR_RECONFIGFLAGS, 0);

    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, SwitchToNormalMode)
{
    /* The following test procedure is also pre-si verified
    Interleave enabled
    Start measurement
    Write new measurement blocks
    Have the system reconfigured from one of the measurement blocks
    Stop measurement
    Configure Interleaved disabled
    Start measurement
    Write new configurations
    Have the system reconfigured
    */

    UseCaseCustomMM_M2452 ucsMM (200u, 6u, 135u, 135u, 80320000, 60240000u, 20600000u, 135u);
    ucsMM.setImage (32, 32);

    ImagerUseCaseFourPhase ucsNormal (400u, 4, 8u, 80320000u, 60240000u, 8u, true);
    ucsNormal.setImage (32, 32);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsMM));

    const auto regConfigMM = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regConfigMM.at (CFGCNT_PLLCFG1_LUT1) > 0);

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsMM.setExposureTime (145u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));

    ASSERT_NO_THROW (m_imager->stopCapture());

    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsNormal));

    const auto regConfigToNormal = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regConfigToNormal.count (CFGCNT_CTRLSEQ) > 0);
    ASSERT_EQ (regConfigToNormal.at (CFGCNT_CTRLSEQ), 0x8004); //safe reconfig enabled and 5 raw frames

    ASSERT_TRUE (regConfigToNormal.count (AIO_CFGCNT_PLLCFG1_LUT1) > 0);

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsNormal.setExposureTime (10u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsNormal, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, SwitchToMixedMode)
{
    UseCaseCustomMM_M2452 ucsMM (200u, 6u, 135u, 135u, 80320000, 60240000u, 20600000u, 135u);
    ucsMM.setImage (32, 32);

    ImagerUseCaseFourPhase ucsNormal (400u, 4, 8u, 80320000u, 60240000u, 8u, true);
    ucsNormal.setImage (32, 32);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsNormal));

    const auto regConfigToNormal = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regConfigToNormal.count (CFGCNT_CTRLSEQ) > 0);
    ASSERT_EQ (regConfigToNormal.at (CFGCNT_CTRLSEQ), 0x8004); //safe reconfig enabled and 5 raw frames

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsNormal.setExposureTime (10u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsNormal, reconfigIdx));

    ASSERT_NO_THROW (m_imager->stopCapture());

    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsMM));
    ASSERT_NO_THROW (m_imager->startCapture());

    ucsMM.setExposureTime (145u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));
}

TEST_F (TestImagerM2452_B1x_AIO, SSC_FW_EnableDisable_NormalMode)
{
    UseCaseNormalSSCDisabled useCaseSSCDisabled;
    UseCaseNormalSSCEnabled useCaseSSCEnabled;

    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (useCaseSSCEnabled));
    ASSERT_NO_THROW (m_imager->startCapture());

    const auto regStartSSCEnable = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regStartSSCEnable.count (ISM_ISMSTATE) > 0);
    ASSERT_TRUE (static_cast<uint16_t> (regStartSSCEnable.at (ISM_ISMSTATE) & (1 << 6)) > 0u);
    ASSERT_TRUE (regStartSSCEnable.count (AIO_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regStartSSCEnable.at (AIO_CFGCNT_PLLCFG1_LUT1 + 2) & (1 << 10), 0); //the enable ssc bit (set)

    const uint16_t PLLCFG4_SSC_LUT1 = 0xb398;
    const uint16_t PLLCFG5_SSC_LUT1 = 0xb399;
    const uint16_t PLLCFG6_SSC_LUT1 = 0xb39a;
    const uint16_t PLLCFG7_SSC_LUT1 = 0xb39b;
    const uint16_t PLLCFG4_SSC_LUT2 = 0xb39d;
    const uint16_t PLLCFG5_SSC_LUT2 = 0xb39e;
    const uint16_t PLLCFG6_SSC_LUT2 = 0xb39f;
    const uint16_t PLLCFG7_SSC_LUT2 = 0xb3a0;

    const std::vector<double> testVector80320kHz{ 24000000, 321280000, 1, 10000, 0.5000, 0.0125, 25185, 64662, 3810, 21801, 37224, 18381, 22290, 2, };
    const std::vector<double> testVector60240kHz{ 24000000, 240960000, 1, 10000, 0.5000, 0.0166, 33349, 15729, 1546, 21801, 59442, 13556, 22290, 2, };

    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG4_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG5_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG6_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG7_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG4_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG5_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG6_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG7_SSC_LUT2) > 0);
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG4_SSC_LUT1), testVector80320kHz.at (9));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG5_SSC_LUT1), testVector80320kHz.at (10));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG6_SSC_LUT1), testVector80320kHz.at (11));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG7_SSC_LUT1), testVector80320kHz.at (12));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG4_SSC_LUT2), testVector60240kHz.at (9));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG5_SSC_LUT2), testVector60240kHz.at (10));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG6_SSC_LUT2), testVector60240kHz.at (11));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG7_SSC_LUT2), testVector60240kHz.at (12));

    ASSERT_NO_THROW (m_imager->stopCapture());
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (useCaseSSCDisabled));
    ASSERT_NO_THROW (m_imager->startCapture());

    const auto regStartSSCDisable = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regStartSSCDisable.count (ISM_ISMSTATE) > 0);
    ASSERT_TRUE (static_cast<uint16_t> (regStartSSCDisable.at (ISM_ISMSTATE) & (1 << 6)) == 0u);
    ASSERT_TRUE (regStartSSCDisable.count (AIO_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_EQ (regStartSSCDisable.at (AIO_CFGCNT_PLLCFG1_LUT1 + 2) & (1 << 10), 0); //the enable ssc bit (unset)
}

TEST_F (TestImagerM2452_B1x_AIO, SSC_FW_EnableDisable_MixedMode)
{
    UseCaseMixedSSCDisabled useCaseSSCDisabled;
    UseCaseMixedSSCEnabled useCaseSSCEnabled;

    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (useCaseSSCEnabled));
    ASSERT_NO_THROW (m_imager->startCapture());

    const auto regStartSSCEnable = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regStartSSCEnable.count (ISM_ISMSTATE) > 0);
    ASSERT_TRUE (static_cast<uint16_t> (regStartSSCEnable.at (ISM_ISMSTATE) & (1 << 6)) > 0u);
    ASSERT_TRUE (regStartSSCEnable.count (CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regStartSSCEnable.at (CFGCNT_PLLCFG1_LUT1 + 2) & (1 << 10), 0); //the enable ssc bit (set)

    const uint16_t PLLCFG4_SSC_LUT1 = 0xb398;
    const uint16_t PLLCFG5_SSC_LUT1 = 0xb399;
    const uint16_t PLLCFG6_SSC_LUT1 = 0xb39a;
    const uint16_t PLLCFG7_SSC_LUT1 = 0xb39b;
    const uint16_t PLLCFG4_SSC_LUT2 = 0xb39d;
    const uint16_t PLLCFG5_SSC_LUT2 = 0xb39e;
    const uint16_t PLLCFG6_SSC_LUT2 = 0xb39f;
    const uint16_t PLLCFG7_SSC_LUT2 = 0xb3a0;

    const std::vector<double> testVector80320kHz{ 24000000, 321280000, 1, 10000, 0.5000, 0.0125, 25185, 64662, 3810, 21801, 37224, 18381, 22290, 2, };
    const std::vector<double> testVector60240kHz{ 24000000, 240960000, 1, 10000, 0.5000, 0.0166, 33349, 15729, 1546, 21801, 59442, 13556, 22290, 2, };

    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG4_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG5_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG6_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG7_SSC_LUT1) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG4_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG5_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG6_SSC_LUT2) > 0);
    ASSERT_TRUE (regStartSSCEnable.count (PLLCFG7_SSC_LUT2) > 0);
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG4_SSC_LUT1), testVector80320kHz.at (9));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG5_SSC_LUT1), testVector80320kHz.at (10));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG6_SSC_LUT1), testVector80320kHz.at (11));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG7_SSC_LUT1), testVector80320kHz.at (12));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG4_SSC_LUT2), testVector60240kHz.at (9));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG5_SSC_LUT2), testVector60240kHz.at (10));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG6_SSC_LUT2), testVector60240kHz.at (11));
    ASSERT_EQ (regStartSSCEnable.at (PLLCFG7_SSC_LUT2), testVector60240kHz.at (12));

    ASSERT_NO_THROW (m_imager->stopCapture());
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (useCaseSSCDisabled));
    ASSERT_NO_THROW (m_imager->startCapture());

    const auto regStartSSCDisable = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regStartSSCDisable.count (ISM_ISMSTATE) > 0);
    ASSERT_TRUE (static_cast<uint16_t> (regStartSSCDisable.at (ISM_ISMSTATE) & (1 << 6)) == 0u);
    ASSERT_TRUE (regStartSSCDisable.count (CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_EQ (regStartSSCDisable.at (CFGCNT_PLLCFG1_LUT1 + 2) & (1 << 10), 0); //the enable ssc bit (unset)
}

TEST_F (TestImagerM2452_B1x_AIO, RepeatsForHighRatioUseCase)
{
    //5:1 ratio means 5x5-phase + 1x9-phase
    UseCaseCustomMM_M2452 ucs (20u, 5u, 135u, 135u, 80320000, 60240000u, 20600000u, 135u);

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the repeat registers have some meaningful value
    const auto regConfig = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regConfig.count (0xB2F8) > 0);
    ASSERT_TRUE (regConfig.count (0xB2F9) > 0);
    ASSERT_TRUE (regConfig.count (0xB2FA) > 0);
    ASSERT_TRUE (regConfig.count (0xB2FB) > 0);

    ASSERT_EQ (4u, regConfig.at (0xB2F8));
    ASSERT_EQ (1u, regConfig.at (0xB2F9));
    ASSERT_EQ (1u, regConfig.at (0xB2FA));
    ASSERT_EQ (1u, regConfig.at (0xB2FB));

    const auto mbSizes = m_imager->getMeasurementBlockSizes();

    size_t mbCount = 0;

    for (const auto x : mbSizes)
    {
        mbCount += x;
    }

    // 5x5-phase + 1x9-phase => 34 raw frames expected
    ASSERT_EQ (34u, mbCount);
}

TEST_F (TestImagerM2452_B1x_AIO, NormalModeSafeReconfig)
{
    UseCaseForM2452 ucs (true);

    initImager();
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //in non-mixed-mode there is only a single MB and it is always safe for reconfig
    ASSERT_TRUE (regConfig.count (CFGCNT_CTRLSEQ) > 0);
    ASSERT_EQ (0x8003, regConfig.at (CFGCNT_CTRLSEQ));
}

TEST_F (TestImagerM2452_B1x_AIO, MixedModeSafeReconfig)
{
    //2:1 ratio means 2x(4+1)-measurements + 1x(8+1)-measurement
    UseCaseCustomMM_M2452 ucs (20u, 2u, 135u, 135u, 80320000, 60240000u, 20600000u, 135u);

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //Expected are 3 safe reconfig points (3 out of 4 MB have the safe reconfig flag set)
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);
    ASSERT_TRUE (regConfig.count (0xB338) > 0);
    ASSERT_TRUE (regConfig.count (0xB34B) > 0);

    const uint16_t safeReconfigFlag = 0x8000;

    //two (4+1) measurements
    ASSERT_EQ (safeReconfigFlag | 4, regConfig.at (0xB312));
    ASSERT_EQ (safeReconfigFlag | 4, regConfig.at (0xB325));

    //one (8+1) measurement
    ASSERT_EQ (3, regConfig.at (0xB338));
    ASSERT_EQ (safeReconfigFlag | 4, regConfig.at (0xB34B));
}

TEST_F (TestImagerM2452_B1x_AIO, MultiFrameGroupAssignmentSafeReconfig)
{
    UseCaseTwoFGsSharedRawFrame_M2452 ucs;

    ASSERT_EQ (9u, ucs.getRawFrames().size());

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //expected is 1 safe reconfig point (at the last MB)
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);

    //the FG1 contains a shared raw frame (not safe for reconfiguration)
    EXPECT_EQ (0x0004, regConfig.at (0xB312));

    //the FG2 contains a raw frame of FG1 (safe for reconfiguration)
    EXPECT_EQ (0x8003, regConfig.at (0xB325));
}

TEST_F (TestImagerM2452_B1x_AIO, MultiFrameGroupAssignmentSharedFrameAtEndSafeReconfig)
{
    UseCaseTwoFGsSharedRawFrame_M2452 ucs (true);

    ASSERT_EQ (9u, ucs.getRawFrames().size());

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //expected is 1 safe reconfig point (at the last MB)
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);

    //the FG1 contains a shared raw frame (not safe for reconfiguration)
    ASSERT_EQ (0x0003, regConfig.at (0xB312));

    //the FG2 contains a raw frame of FG1 (safe for reconfiguration)
    ASSERT_EQ (0x8004, regConfig.at (0xB325));
}

TEST_F (TestImagerM2452_B1x_AIO, MultiFrameGroupAssignmentOnlyGrayscaleForFG2SafeReconfig)
{
    UseCaseTwoFGsSharedRawFrame_M2452 ucs (true, true);

    ASSERT_EQ (5u, ucs.getRawFrames().size());

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //expected is 1 safe reconfig point
    ASSERT_TRUE (regConfig.count (0xB312) > 0);

    //FG2 contains only a shared raw frame (it is part of FG1 and safe for reconfiguration)
    ASSERT_EQ (0x8004, regConfig.at (0xB312));
}

TEST_F (TestImagerM2452_B1x_AIO, MultiFrameGroupAssignmentFourFGsTwoSharedRawFramesSafeReconfig)
{
    UseCaseFourFGsTwoSharedRawFrames_M2452 ucs;

    ASSERT_EQ (18u, ucs.getRawFrames().size());

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //expected are 2 safe reconfig points
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);
    ASSERT_TRUE (regConfig.count (0xB338) > 0);
    ASSERT_TRUE (regConfig.count (0xB34B) > 0);

    ASSERT_EQ (0x0003, regConfig.at (0xB312));
    ASSERT_EQ (0x8004, regConfig.at (0xB325));
    ASSERT_EQ (0x0003, regConfig.at (0xB338));
    ASSERT_EQ (0x8004, regConfig.at (0xB34B));
}

TEST_F (TestImagerM2452_B1x_AIO, OddRFStoMBAssignmentSafeReconfig)
{
    UseCaseTwoGrayscaleFGs_M2452 ucs;

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //expected is 1 safe reconfig point (at the last MB)
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);

    //the FG1 and the first two raw frames of FG2 (not safe for reconfiguration)
    ASSERT_EQ (0x0004, regConfig.at (0xB312));

    //the remaining raw frame of FG2 (safe for reconfiguration)
    ASSERT_EQ (0x8000, regConfig.at (0xB325));
}

TEST_F (TestImagerM2452_B1x_AIO, InterleavedModeSafeReconfig)
{
    //2:1 ratio means 2x(4+1)-measurements + 1x(8+1)-measurement
    UseCaseCustomInterleaved_M2452 ucs (20u, 2u);

    //Use case format (timeline):
    //CA=>ClockAligned, SA=>StartAligned, EA=>StopAligned
    //HT=>Handtracking, ES1/ES2=>first/second part of environmental scanning
    //GS=>A grayscale raw frame, M4=>4 modulated raw frames
    //SR=>Safe reconfig point, MBx=>Measurement block with index x
    //
    //CA-SA----EA-EA-CA-SA-SA
    //--HT------ES1---HT---ES2
    //--MB0-----MB1---MB2--MB3
    //GS-M4----GS-M4-GS-M4-M4
    //-----SR---------------SR

    initImager();
    m_bridge->clearRegisters();
    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //now check if the safe reconfig flags are set correctly
    const auto regConfig = m_bridge->getWrittenRegisters();

    //Expected are 2 safe reconfig points (2 out of 4 MB have the safe reconfig flag set)
    ASSERT_TRUE (regConfig.count (0xB312) > 0);
    ASSERT_TRUE (regConfig.count (0xB325) > 0);
    ASSERT_TRUE (regConfig.count (0xB338) > 0);
    ASSERT_TRUE (regConfig.count (0xB34B) > 0);

    const uint16_t safeReconfigFlag = 0x8000;

    //HT-MB0
    ASSERT_EQ (safeReconfigFlag | 4, regConfig.at (0xB312));

    //ES1-MB1
    ASSERT_EQ (4, regConfig.at (0xB325));

    //HT-MB2
    ASSERT_EQ (4, regConfig.at (0xB338));

    //ES2-MB3
    ASSERT_EQ (safeReconfigFlag | 3, regConfig.at (0xB34B));
}

TEST_F (TestImagerM2452_B1x_AIO, IrregularMixedMode)
{
    // This should test if use cases which won't work
    // with the standard mixed mode implementation will
    // work with the irregular one
    UseCaseCustomMM_M2452 ucsStandard (50u, 10u, 300u, 1500u, 80320000u, 60240000, 80320000u, 200u, true,
                                       true, 10000., 0.5, 0.0125, 0.0166);

    UseCaseCustomMM_M2452 ucsIrregular (50u, 10u, 300u, 1500u, 80320000u, 60240000, 80320000u, 200u, true,
                                        true, 10000., 0.5, 0.0125, 0.0166, true);

    initImager();

    ASSERT_EQ (ImagerVerificationStatus::FRAMERATE, m_imager->verifyUseCase (ucsStandard));

    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucsIrregular));
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureNormalMode)
{
    //in case of reconfiguration (a call to IImager::reconfigure) different registers are used
    ImagerUseCaseFourPhase ucs (5u, 8, 1000u, 30000000, 20200000, 1000u);
    ucs.setImage (224, 172);
    ucs.setROI (0, 0);

    initImager();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //change expo-time of first two MBs
    ucs.setExposureTime (101u);
    m_bridge->clearRegisters();

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));

    //the normal configuration (using startCapture) would use:
    //[0](0x9839, 0x8008) - CTRL_SEQ (this is forced write to CFGCNT, no DMEM access, so it is okay)
    //[2](0xb301, 0x0bd6) - the changed exposures of the first RFS
    //[3](0xb304, 0x0bd6)
    //[4](0xb307, 0x0bd6)
    //[5](0xb30a, 0x0bd6)
    //[6](0xb30b, 0x005c) - the raw frame rate of the last sequence entry of the 1st RFS
    //[7](0xb30d, 0x07f8) - the changed exposures of the second RFS
    //[8](0xb310, 0x07f8)
    //[9](0xb313, 0x07f8)
    //[10](0xb316, 0x07f8)
    //[11](0xb317, 0x005c) - the raw frame rate of the last sequence entry of the 2nd RFS

    //with the Lotte_x/x_normal_fix_expo the new config should be (refer to ROYAL-2119):
    const uint16_t expoMB1 = 0x0bd6;
    const uint16_t expoMB2 = 0x07f8;

    //compare the expectation to what has been reconfigured
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (0x9821) > 0);
    ASSERT_TRUE (regChanged.count (0x9822) > 0);
    ASSERT_TRUE (regChanged.count (0x9823) > 0);
    ASSERT_TRUE (regChanged.count (0x9824) > 0);
    ASSERT_TRUE (regChanged.count (0x9825) > 0);
    ASSERT_TRUE (regChanged.count (0x9826) > 0);
    ASSERT_TRUE (regChanged.count (0x9827) > 0);
    ASSERT_TRUE (regChanged.count (0x9828) > 0);

    ASSERT_EQ (expoMB1, regChanged.at (0x9821));
    ASSERT_EQ (expoMB1, regChanged.at (0x9822));
    ASSERT_EQ (expoMB1, regChanged.at (0x9823));
    ASSERT_EQ (expoMB1, regChanged.at (0x9824));
    ASSERT_EQ (expoMB2, regChanged.at (0x9825));
    ASSERT_EQ (expoMB2, regChanged.at (0x9826));
    ASSERT_EQ (expoMB2, regChanged.at (0x9827));
    ASSERT_EQ (expoMB2, regChanged.at (0x9828));
}

/**
 * Test based on bug ROYAL-3107:
 *
 * * Choose a 9 (or more) phase use case
 * * Start the imager
 * * Change the exposure time (via safe reconfig)
 * * Switch to a 5 phase use case
 * * Switch back to the original use case -> only the first 5 exposure times are correct
 *
 * This checks that the exposures for all of the phases are written.
 */
TEST_F (TestImagerM2452_B1x_AIO, ReconfigureNormalModeCalibrationFourCalibration)
{
    const auto originalExpo = uint16_t (200u);
    const auto changedExpo = uint16_t (123u);

    // As support code already has a calibration use case and tests for it, use that use case here
    // instead of a 9-phase one
    UseCaseCalibrationForM2452 ucCalib {};
    ucCalib.setExposureTime (originalExpo);

    UseCaseForM2452 ucSmall {false, 100u};

    ASSERT_EQ (5u, ucSmall.getRawFrames().size()) << "Test is wrong";
    ASSERT_FALSE (ucSmall.getMixedModeEnabled()) << "Test is wrong";

    ASSERT_EQ (11u, ucCalib.getRawFrames().size()) << "Test is wrong";
    ASSERT_FALSE (ucCalib.getMixedModeEnabled()) << "Test is wrong";

    initImager();
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucCalib));

    // Sanity-check for this test itself
    {
        const auto regChanged = m_bridge->getWrittenRegisters();
        const auto START_EXPO = uint16_t (0xb301);
        const auto SEQ_INCREMENT = 3u;

        // Both use cases start with the grayscale, so the modulated frames will be
        // zero-based 1-4 and 5-8
        ASSERT_TRUE (ucSmall.getRawFrames().at (0).grayscale) << "Test is wrong";
        ASSERT_TRUE (ucCalib.getRawFrames().at (0).grayscale) << "Test is wrong";
        const auto rfs1FirstReg = static_cast<uint16_t> (START_EXPO + 1 * SEQ_INCREMENT);
        const auto rfs2FirstReg = static_cast<uint16_t> (START_EXPO + 5 * SEQ_INCREMENT);

        ASSERT_TRUE (regChanged.count (rfs1FirstReg) > 0) << "Test is wrong";
        ASSERT_TRUE (regChanged.count (rfs2FirstReg) > 0) << "Test is wrong";

        const auto expectedExpo1 = regChanged.at (rfs1FirstReg);
        const auto expectedExpo2 = regChanged.at (rfs2FirstReg);

        for (auto i = 1u; i < 3; i++)
        {
            const auto address = static_cast<uint16_t> (rfs1FirstReg + SEQ_INCREMENT * i);
            ASSERT_TRUE (regChanged.count (address) > 0) << "Test is wrong, fail for address " << unsigned (address);
            ASSERT_EQ (expectedExpo1, regChanged.at (address)) << "Test is wrong, fail for address " << unsigned (address);
        }
        for (auto i = 1u; i < 3; i++)
        {
            const auto address = static_cast<uint16_t> (rfs2FirstReg + SEQ_INCREMENT * i);
            ASSERT_TRUE (regChanged.count (address) > 0) << "Test is wrong, fail for address " << unsigned (address);
            ASSERT_EQ (expectedExpo2, regChanged.at (address)) << "Test is wrong, fail for address " << unsigned (address);
        }
    }

    // reconfigure
    ASSERT_NO_THROW (m_imager->startCapture());
    ucCalib.setExposureTime (changedExpo);
    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucCalib, frameNumber));
    ASSERT_NO_THROW (m_imager->stopCapture());

    ASSERT_NO_THROW (m_imager->executeUseCase (ucSmall));

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucCalib));

    // Check that the expected registers were written
    {
        const auto regChanged = m_bridge->getWrittenRegisters();
        const auto START_EXPO = uint16_t (0xb301);
        const auto SEQ_INCREMENT = 3u;

        const auto rfs1FirstReg = static_cast<uint16_t> (START_EXPO + 1 * SEQ_INCREMENT);
        const auto rfs2FirstReg = static_cast<uint16_t> (START_EXPO + 5 * SEQ_INCREMENT);

        ASSERT_TRUE (regChanged.count (rfs1FirstReg) > 0);
        ASSERT_TRUE (regChanged.count (rfs2FirstReg) > 0)
                << "FAIL similar to ROYAL-3107, software imager didn't write the registers for the second modulated set";

        const auto expectedExpo1 = regChanged.at (rfs1FirstReg);
        const auto expectedExpo2 = regChanged.at (rfs2FirstReg);

        for (auto i = 1u; i < 3; i++)
        {
            const auto address = static_cast<uint16_t> (rfs1FirstReg + SEQ_INCREMENT * i);
            ASSERT_TRUE (regChanged.count (address) > 0) << "fail for address " << unsigned (address);
            ASSERT_EQ (expectedExpo1, regChanged.at (address)) << "fail for address " << unsigned (address);
        }
        for (auto i = 1u; i < 3; i++)
        {
            const auto address = static_cast<uint16_t> (rfs2FirstReg + SEQ_INCREMENT * i);
            ASSERT_TRUE (regChanged.count (address) > 0) << "fail for address " << unsigned (address);
            ASSERT_EQ (expectedExpo2, regChanged.at (address)) << "fail for address " << unsigned (address);
        }
    }
}

TEST_F (TestImagerM2452_B1x_AIO, ReconfigureMixedMode)
{
    //in case of reconfiguration (a call to IImager::reconfigure) different registers are used

    //2:1 ratio means 2x(4+1)-measurements + 1x(8+1)-measurement
    UseCaseCustomMM_M2452 ucs (20u, 2u, 135u, 135u, 80320000, 60240000u, 20600000u, 135u);

    initImager();
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //change expo-time of first two MBs
    ucs.setExposureTime (141u, 0, 9);
    m_bridge->clearRegisters();

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));

    //the normal configuration (using startCapture) would use:
    //[1](0xb300, 0x2c3d) - the changed exposures of the two (4+1) measurements
    //[2](0xb303, 0x2c3d)
    //[3](0xb306, 0x2c3d)
    //[4](0xb309, 0x2c3d)
    //[5](0xb313, 0x2c3d)
    //[6](0xb316, 0x2c3d)
    //[7](0xb319, 0x2c3d)
    //[8](0xb31c, 0x2c3d)
    //[9](0xb322, 0x02ea)  - LPFSMFR of second MB (after 2x(4+1))
    //[10](0xb348, 0x1f78) - LPFSMFR of last MB (after 1x(8+1))

    //with the Lotte_AIO_mb_param_nhot_fix the new config should be (refer to ROYAL-2119):
    const uint16_t expoMB1 = 0x2c3d;
    const uint16_t decodeMB1Expo = 0/*1st MB*/ + (0xF << 4) /*seqnum 0-3*/ + (0 << 12) /*expo-reg*/;
    const uint16_t expoMB2 = 0x2c3d;
    const uint16_t decodeMB2Expo = 1/*2nd MB*/ + (0xF << 4) /*seqnum 0-3*/ + (0 << 12) /*expo-reg*/;
    const uint16_t lpfsmMB2 = 0x02ea;
    const uint16_t decodeMB2Ctrl = 1/*2nd MB*/ + (1 << 9) /*CTRL*/ + (0 << 12) /*LPFSMFR*/;
    const uint16_t lpfsmMB4 = 0x1f78;
    const uint16_t decodeMB4Ctrl = 3/*4th MB*/ + (1 << 9) /*CTRL*/ + (0 << 12) /*LPFSMFR*/;
    const uint16_t flags = 3/*full cfg done + partly config done*/ + (4 << 8) /*4 params used*/;

    //compare the expectation to what really was reconfigured
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (0x980f) > 0);
    ASSERT_TRUE (regChanged.count (0x9810) > 0);
    ASSERT_TRUE (regChanged.count (0x9811) > 0);
    ASSERT_TRUE (regChanged.count (0x9812) > 0);
    ASSERT_TRUE (regChanged.count (0x9813) > 0);
    ASSERT_TRUE (regChanged.count (0x9814) > 0);
    ASSERT_TRUE (regChanged.count (0x9815) > 0);
    ASSERT_TRUE (regChanged.count (0x9816) > 0);
    ASSERT_TRUE (regChanged.count (0x982f) > 0);

    ASSERT_EQ (expoMB1, regChanged.at (0x980f));
    ASSERT_EQ (decodeMB1Expo, regChanged.at (0x9810));
    ASSERT_EQ (expoMB2, regChanged.at (0x9811));
    ASSERT_EQ (decodeMB2Expo, regChanged.at (0x9812));
    ASSERT_EQ (lpfsmMB2, regChanged.at (0x9813));
    ASSERT_EQ (decodeMB2Ctrl, regChanged.at (0x9814));
    ASSERT_EQ (lpfsmMB4, regChanged.at (0x9815));
    ASSERT_EQ (decodeMB4Ctrl, regChanged.at (0x9816));
    ASSERT_EQ (flags, regChanged.at (0x982f));
}
