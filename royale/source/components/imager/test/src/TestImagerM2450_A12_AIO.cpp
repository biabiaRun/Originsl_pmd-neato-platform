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
#include <StubBridgeImager.hpp>
#include <imager/ImagerM2450_A12_AIO.hpp>
#include <SimImagerM2450_A12_AIO.hpp>
#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter_AIO.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/NarrowCast.hpp>
#include <TestImagerCommon.hpp>

#include <cmath>

using namespace royale::imager::M2450_A12;

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 26000000;

    class UseCaseAlignment : public UseCaseForM2450
    {
    public:
        explicit UseCaseAlignment (bool stopAlignGrayScale = false) : UseCaseForM2450 (5u, 4, 1000u, 29500000u, 0u, 134u)
        {
            if (stopAlignGrayScale)
            {
                m_rawFrames.back().alignment = ImagerRawFrame::ImagerAlignment::STOP_ALIGNED;
            }
        }
    };

    class UseCasePicoFlexxDC : public ImagerUseCaseFourPhase
    {
    public:
        explicit UseCasePicoFlexxDC (bool alternateDC, uint16_t targetRate = 45u) : ImagerUseCaseFourPhase (targetRate, 4, 1000u, 29500000)
        {
            m_imageColumns = 352;
            m_imageRows = 287;
            m_roiCMin = 0;
            m_roiRMin = 0;

            if (alternateDC)
            {
                for (auto &rf : m_rawFrames)
                {
                    rf.dutyCycle = alternateDC ? ImagerRawFrame::ImagerDutyCycle::DC_25 : ImagerRawFrame::ImagerDutyCycle::DC_25_DEPRECATED;
                }
            }
        }
    };

    /**
    * A standard 4+4 use case, but second raw frame set should start at 1/5 s
    */
    class UseCaseMasterClockTicks : public UseCaseForM2450
    {
    public:
        explicit UseCaseMasterClockTicks (bool useTwoMasterClockTicks) : UseCaseForM2450 (10u, 8, 1000u, 30000000, 60000000, 134u)
        {
            if (useTwoMasterClockTicks)
            {
                m_targetRate = static_cast<uint16_t> (m_targetRate * 2u);
                m_rawFrames.at (4).alignment = ImagerRawFrame::ImagerAlignment::CLOCK_ALIGNED;
            }
        }
    };

    const uint16_t AIO_NR_CFGCNT_S00_PLLSET = 0xC323;
    const uint16_t AIO_SR_NR_FRAMERATE = 0xC321;
    const uint16_t AIO_NR_CFGCNT_PLLCFG1_LUT1 = 0xC3A0;
    const uint16_t AIO_SSC_PLLCFG4_LUTx = 0xC24C;
    const uint16_t AIO_SSC_PLLCFG5_LUTx = 0xC24D;
    const uint16_t AIO_SSC_PLLCFG6_LUTx = 0xC24E;
    const uint16_t AIO_SSC_PLLCFG7_LUTx = 0xC24F;
    const uint16_t AIO_SSC_PLLCFG8_LUTx = 0xC250;
    const uint16_t AIO_SSC_PLLCFG_LUT_OFFSET = 5;
}

class TestImagerM2450_A12_AIO: public ::testing::Test
{
protected:
    explicit TestImagerM2450_A12_AIO()
    {

    }

    ~TestImagerM2450_A12_AIO() override
    {

    }

    virtual void SetUp() override
    {
        m_simImager.reset (new SimImagerM2450_A12_AIO());

        ASSERT_NO_THROW (m_bridge.reset (new StubBridgeImager (std::move (m_simImager))));

        createImager();
    }

    virtual void TearDown() override
    {
        m_simImager.reset();
        m_bridge.reset();
        m_imager.reset();
    }

    void initImager()
    {
        m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

        ASSERT_NO_THROW (m_imager->initialize());
    }

    void createImager (uint32_t sysfreq = SYSFREQ, ImgImageDataTransferType interfaceType = ImgImageDataTransferType::PIF,
                       bool useSuperFrame = false)
    {
        {
            ImagerParameters params{ m_bridge, nullptr, useSuperFrame, false,
                                     ImgTrigger::GPIO13, interfaceType, 0., {},
                                     sysfreq, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                     ImgIlluminationPad::SE_P, 100000000, false };

            ASSERT_NO_THROW (m_imager.reset (new ImagerM2450_A12_AIO (params)));
        }

        ASSERT_NO_THROW (m_imager->sleep());
        ASSERT_NO_THROW (m_imager->wake());
    }

    void setImageDataTransferType (ImgImageDataTransferType interfaceType)
    {
        createImager (SYSFREQ, interfaceType);
    }

    void setSystemFrequency (uint32_t systemFrequency)
    {
        createImager (systemFrequency);
    }

    std::unique_ptr <SimImagerM2450_A12_AIO> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2450_A12_AIO> m_imager;
};


TEST_F (TestImagerM2450_A12_AIO, CreateImagerDirectly)
{
    ImagerParameters params1{ nullptr, nullptr, false, false, ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {}, 0u, ImagerRawFrame::ImagerDutyCycle::DC_0, ImgIlluminationPad::SE_P, 0u, false };

    ASSERT_THROW (new ImagerM2450_A12_AIO (params1), LogicError);
}

TEST_F (TestImagerM2450_A12_AIO, UnsupportedTransmissionMode)
{
    ImagerParameters params{ m_bridge, nullptr, true, false,
                             ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0., {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 100000000, false };

    ASSERT_THROW (std::unique_ptr<royale::imager::ImagerM2450_A12_AIO> (new ImagerM2450_A12_AIO (params)), InvalidValue);
}

TEST_F (TestImagerM2450_A12_AIO, InitImager)
{
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0FFF);

    ASSERT_THROW (m_imager->initialize(), Exception);

    initImager();
}

TEST_F (TestImagerM2450_A12_AIO, DoNotSupportDirectWrite)
{
    ASSERT_THROW (m_imager->writeRegisters ({}, {}), NotImplemented);
}


TEST_F (TestImagerM2450_A12_AIO, CheckStates)
{
    UseCaseCustomFreq_M2450 ucd;
    uint16_t idx;

    ASSERT_NO_THROW (m_imager->sleep());

    //Power Down State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucd), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucd, idx), WrongState);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));
    ASSERT_NO_THROW (m_imager->setExternalTrigger (false));

    ASSERT_THROW (m_imager->initialize(), WrongState);

    ASSERT_NO_THROW (m_imager->wake());

    //Power Up State active
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucd), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucd, idx), WrongState);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));
    ASSERT_NO_THROW (m_imager->setExternalTrigger (false));

    ASSERT_NO_THROW (m_imager->initialize());

    //Ready State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->stopCapture(), WrongState);
    ASSERT_THROW (m_imager->reconfigure (ucd, idx), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (false), WrongState);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucd));

    //Again in Ready State active
    ASSERT_NO_THROW (m_imager->startCapture());

    //Capturing State active
    ASSERT_THROW (m_imager->initialize(), WrongState);
    ASSERT_THROW (m_imager->sleep(), WrongState);
    ASSERT_THROW (m_imager->executeUseCase (ucd), WrongState);
    ASSERT_THROW (m_imager->startCapture(), WrongState);
    ASSERT_THROW (m_imager->setExternalTrigger (true), WrongState);

    ASSERT_NO_THROW (m_imager->reconfigure (ucd, idx));

    //Again in Capturing State
    ASSERT_NO_THROW (m_imager->stopCapture());

    //Ready State active
    ASSERT_NO_THROW (m_imager->sleep());
}

TEST_F (TestImagerM2450_A12_AIO, InitWithSysFreqMod)
{
    setSystemFrequency (24000000);

    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG1) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG2) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG3) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG4) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG5) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG6) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG7) > 0);
    ASSERT_TRUE (regChanged.count (ANAIP_DPHYPLLCFG8) > 0);
}

TEST_F (TestImagerM2450_A12_AIO, InitAndVerifyCustomReadoutSettings)
{
    UseCaseForM2450 ucs (45u, 4, 1000u, 30000000u, 0u, 100u);

    initImager();
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    //switch to SOCD=8cycles and ODDD=34cycles delay
    {
        ImagerParameters params{ m_bridge, nullptr, false, false,
        ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0.0000078f, { { CFGCNT_ROS1, 0xF806 } },
        SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
        ImgIlluminationPad::SE_P, 100000000, false };

        m_imager.reset (new ImagerM2450_A12_AIO (params));
    }

    ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";
    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());

    //due to longer readout time the same use case is not feasible anymore...
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, ClonePseudoData)
{
    const auto pdi = m_imager->createPseudoDataInterpreter();
    ASSERT_NE (dynamic_cast<M2450_A12::PseudoDataInterpreter_AIO *> (pdi.get()), nullptr);
}

TEST_F (TestImagerM2450_A12_AIO, GetSerialNumber)
{
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL1, 0xAAAA);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL2, 0x5555);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL3, 0xAAAA);
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL4, 0x5555);

    ASSERT_NO_THROW (m_imager->getSerialNumber());

    std::string serial = m_imager->getSerialNumber();

    ASSERT_EQ (serial, "0005-2110-0341-1021");
}

TEST_F (TestImagerM2450_A12_AIO, RoiLimit_IRS1010)
{
    UseCaseForM2450 ucs (45u, 4, 1000u, 30000000u, 0u, 100u);

    //test if a fused 19k Mira is rejected
    m_bridge->writeImagerRegister (ANAIP_EFUSEVAL1, 1 << 2);

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForPIF)
{
    setImageDataTransferType (ImgImageDataTransferType::PIF);
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_PIFCCFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_PIFCCFG) == 0x1057);
    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0280);

    UseCaseForM2450 ucs (5u, 4, 1000u, 30000000u, 0u, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForPIFSuperFrame)
{
    ImagerParameters params{ m_bridge, nullptr, true, false,
                             ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0., {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 100000000, false };

    ASSERT_THROW (m_imager.reset (new ImagerM2450_A12_AIO (params)), InvalidValue);
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForCSI2_MIPI1Lane)
{
    setImageDataTransferType (ImgImageDataTransferType::MIPI_1LANE);
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_PIFCCFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_PIFCCFG) == 0x0000);
    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0211);

    UseCaseForM2450 ucs (5u, 4, 1000u, 30000000u, 0u, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForCSI2_MIPI1LaneSuperFrame)
{
    createImager (SYSFREQ, ImgImageDataTransferType::MIPI_1LANE, true);
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_PIFCCFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_PIFCCFG) == 0x0000);
    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0201);

    UseCaseForM2450 ucs (5u, 4, 1000u, 30000000u, 0u, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForCSI2_MIPI2Lane)
{
    setImageDataTransferType (ImgImageDataTransferType::MIPI_2LANE);
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_PIFCCFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_PIFCCFG) == 0x0000);
    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0291);

    UseCaseForM2450 ucs (5u, 4, 1000u, 30000000u, 0u, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, InitAndExecuteForCSI2_MIPI2LaneSuperFrame)
{
    createImager (SYSFREQ, ImgImageDataTransferType::MIPI_2LANE, true);
    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_PIFCCFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_PIFCCFG) == 0x0000);
    ASSERT_TRUE (regChanged.count (CFGCNT_CSICFG) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_CSICFG) == 0x0281);

    UseCaseForM2450 ucs (5u, 4, 1000u, 30000000u, 0u, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, CheckIfInterfaceDelayIsSet)
{
    initImager();

    {
        ImagerParameters params{ m_bridge, nullptr, false, false,
                                 ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0.0000078f, {},
                                 SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                 ImgIlluminationPad::SE_P, 100000000, false };

        m_imager.reset (new ImagerM2450_A12_AIO (params));
    }

    ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";
    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (CFGCNT_IFDEL) > 0);
    ASSERT_TRUE (regChanged.at (CFGCNT_IFDEL) == (0x4000 | 0x0410));
}


TEST_F (TestImagerM2450_A12_AIO, StartCapture)
{
    ASSERT_NO_THROW (m_imager->initialize());

    //try start with MTCU busy
    m_bridge->writeImagerRegister (CFGCNT_STATUS, 0x0000);
    ASSERT_THROW (m_imager->startCapture(), LogicError);

    //try normal start
    m_bridge->writeImagerRegister (CFGCNT_STATUS, 0x8001);
    ASSERT_NO_THROW (m_imager->startCapture());
}

TEST_F (TestImagerM2450_A12_AIO, StopCapture)
{
    m_bridge->writeImagerRegister (CFGCNT_STATUS, 0x0001);
    ASSERT_THROW (m_imager->stopCapture(), LogicError);
}

TEST_F (TestImagerM2450_A12_AIO, RoiSetting)
{
    initImager();
    /*
    * Create an instance of a ImagerM2450_A12 and execute
    * an use-case with one modified modfreq. The only registers
    * that are written during execution should be expotime and
    * pll-lut assigment, plus the 3 pllcfg registers for the new freq.
    */
    UseCaseForM2450 ucs (45u, 4, 1000u, 30000000u, 0u, 1000u);

    //steps of 16 pixels to keep UT < 50ms
    for (auto ucdWidth = 16; ucdWidth <= 176; ucdWidth += 16)
    {
        for (auto ucdHeight = 16; ucdHeight <= 176; ucdHeight += 16)
        {
            auto test = [ = ] (ImagerUseCaseDefinition & ucd, uint16_t ucdColumns, uint16_t ucdRows)
            {
                //use case will be executed using the mock bridge, no actual harware required
                if (ImagerVerificationStatus::SUCCESS == m_imager->verifyUseCase (ucd))
                {
                    ASSERT_NO_THROW (m_imager->executeUseCase (ucd));

                    //ask the bridge what has been written
                    const auto regChanged = m_bridge->getWrittenRegisters();

                    size_t expectedVal = 1;

                    ASSERT_EQ (regChanged.count (CFGCNT_ROICMAXREG), expectedVal);
                    ASSERT_EQ (regChanged.count (CFGCNT_ROICMINREG), expectedVal);
                    ASSERT_EQ (regChanged.count (CFGCNT_ROIRMAXREG), expectedVal);
                    ASSERT_EQ (regChanged.count (CFGCNT_ROIRMINREG), expectedVal);

                    //get roi sizes
                    const uint16_t roiColumns = static_cast<uint16_t> (regChanged.at (CFGCNT_ROICMAXREG) - regChanged.at (CFGCNT_ROICMINREG) + uint16_t (1));
                    const uint16_t roiRows = static_cast<uint16_t> (regChanged.at (CFGCNT_ROIRMAXREG) - regChanged.at (CFGCNT_ROIRMINREG) + uint16_t (1));

                    ASSERT_EQ (roiColumns, ucdColumns);
                    ASSERT_EQ (roiRows - 1, ucdRows); //one line is pseudo data
                }
            };

            uint16_t width = narrow_cast<uint16_t> (ucdWidth);
            uint16_t height = narrow_cast<uint16_t> (ucdHeight);

            ucs.setImage (width, height);
            test (ucs, width, height);

            ucs.setImage (height, width);
            test (ucs, height, width);
        }
    }
}

TEST_F (TestImagerM2450_A12_AIO, ValidPllFreqRange)
{
    UseCaseCustomFreq_M2450 ucsCustom;
    const uint32_t freq1 = 80320000u;
    const uint32_t freq2 = 60240000u;

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

    //test 3MHz
    ucsCustom.setFrequency (3000000);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);

    //test 101MHz
    ucsCustom.setFrequency (101000000);
    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::MODULATION_FREQUENCY);
}

TEST_F (TestImagerM2450_A12_AIO, ReconfigureExposurePositive)
{
    UseCaseForM2450 ucs (10u, 4, 1000u, 80320000u, 0u, 1000u, false, true, 10000., 0.5, 0.0125);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with a valid use case
    uint16_t frameNumber = 0;
    ucs.setExposureTime (101);

    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));
}

TEST_F (TestImagerM2450_A12_AIO, ReconfigureExposureNegative)
{
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with an invalid use case
    UseCaseForM2450 ucs (1u, 4, 1000u, 0u);
    uint16_t frameNumber = 0;
    ASSERT_THROW (m_imager->reconfigure (ucs, frameNumber), RuntimeError);
}

TEST_F (TestImagerM2450_A12_AIO, ReconfigureFrameRatePositive)
{
    UseCaseForM2450 ucs (10u, 4, 1000u, 30000000u, 0u, 1000u);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //reconfigure with a valid use case
    uint16_t frameNumber = 0;
    ucs.setTargetRate (9u);

    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));
    ASSERT_NO_THROW (m_imager->stopCapture());
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyHighExpoNegative)
{
    initImager();
    //for a high modulation frequency only a certain expotime works because of counter limitations
    UseCaseForM2450 ucs (10u, 4, 9000u, 99870000u);

    ASSERT_EQ (ImagerVerificationStatus::EXPOSURE_TIME, m_imager->verifyUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, ConfigureLowestExpoGrayScale)
{
    setSystemFrequency (19200000);
    initImager();

    ImagerUseCaseFourPhase ucs (50u, 0, 0u, 0u, 0u, 134u);
    ucs.setImage (224, 172);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, ConfigureLowestExpo)
{
    setSystemFrequency (19200000);
    initImager();

    UseCaseCalibration_M2450 ucs1;
    ASSERT_EQ (m_imager->verifyUseCase (ucs1), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs1));

    UseCaseCalibration_M2450 ucs2 (27u, 27u);
    ASSERT_EQ (m_imager->verifyUseCase (ucs2), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs2));
}

TEST_F (TestImagerM2450_A12_AIO, ReConfigureLowestExpo)
{
    setSystemFrequency (19200000);
    initImager();

    UseCaseCalibration_M2450 ucs1 (28u, 28u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs1), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs1));
    ASSERT_NO_THROW (m_imager->startCapture());

    UseCaseCalibration_M2450 ucs2 (27u, 27u);
    ASSERT_EQ (m_imager->verifyUseCase (ucs2), ImagerVerificationStatus::SUCCESS);

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs2, frameNumber));
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyRawFrameRateNegative)
{
    initImager();
    const uint32_t targetFrameRate = 41u;

    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 30000000u);

    //4 raw frames with 160Hz are only possible for a target frame rate higher than 40fps
    ucs.setRawFrameRate (static_cast<uint32_t> (4u * (targetFrameRate - 1u)));

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyHighTargetFrameRatePositive)
{
    initImager();

    //note: achievable target frame rate depends on the interface delay set by the mock module config
    const uint32_t targetFrameRate = 46u;

    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 30000000u);

    //set a target frame rate feasible for this UCD
    ucs.setTargetRate (targetFrameRate);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyUnlimitedFrameRateNegative)
{
    initImager();

    UseCasePicoFlexxDC ucs (false, 0u);
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyLowTargetFrameRateNegative)
{
    initImager();

    const uint32_t targetFrameRate = 1u;

    //use a high modulation frequency (worst case for frame rate counter)
    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 99500000);

    //set a target frame rate feasible for this UCD
    ucs.setTargetRate (targetFrameRate);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyLowTargetFrameRatePositive)
{
    initImager();

    const uint32_t targetFrameRate = 2u;

    //use a high modulation frequency (worst case for frame rate counter)
    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 99500000);

    //set a target frame rate feasible for this UCD
    ucs.setTargetRate (targetFrameRate);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseVerifyHighTargetFrameRateNegative)
{
    initImager();

    {
        ImagerParameters params{ m_bridge, nullptr, false, false,
                                 ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0.0000078f, {},
                                 SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                 ImgIlluminationPad::SE_P, 100000000, false };

        m_imager.reset (new ImagerM2450_A12_AIO (params));
    }

    ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";
    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());

    //note: achievable target frame rate depends on the interface delay set by the mock module config
    const uint32_t targetFrameRate = 47u;

    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 30000000);

    //set a target frame rate not achievable for this UCD
    ucs.setTargetRate (targetFrameRate);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, SSC_ReconfigUseCase)
{
    UseCaseForM2450 ucs (5u, 8, 1000u, 80320000, 60240000, 1000u, true, true, 10000., 0.5, 0.0125, 0.0166);
    ucs.setImage (224, 172);

    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //only CFGCNT_CTRLSEQ should have been written
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_EQ (1u, regChanged.size());
}


TEST_F (TestImagerM2450_A12_AIO, PllConfigFor80320kHz)
{
    UseCaseCustomFreq_M2450 ucsCustom;
    const uint32_t freq1 = 80320000;
    ucsCustom.setFrequency (freq1);

    ASSERT_NO_THROW (m_imager->initialize());

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ref_clk_i,output_frequency,enable_wavegen,fssc,kspread,delta,anaip_pllcfg1,anaip_pllcfg2,anaip_pllcfg3,anaip_pllcfg4,anaip_pllcfg5,anaip_pllcfg6,anaip_pllcfg7,anaip_pllcfg8,forbidden_seeking,forbidden_modfreq_div4,forbidden_modfreq_div6,forbidden_limit
    const std::vector<double> testVector{ 26000000, 321280000, 1, 10000, 0.5000, 0.0125, 16473, 24400, 3803, 21801, 39402, 37831, 35087, 2, 0, 0, 0, 0 };

    //the frequency should be assigned to LUT number 1
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET), 0); //the LUT assignment to LUT1

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) & 1 << 10, 0); //the enable ssc bit

    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG4_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG5_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG6_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG7_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG8_LUTx) > 0);

    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1), testVector.at (6));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1), testVector.at (7));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2), testVector.at (8));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG4_LUTx), testVector.at (9));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG5_LUTx), testVector.at (10));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG6_LUTx), testVector.at (11));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG7_LUTx), testVector.at (12));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG8_LUTx), testVector.at (13));
}

TEST_F (TestImagerM2450_A12_AIO, PllConfigFor60240kHz)
{
    UseCaseCustomFreq_M2450 ucsCustom;
    const uint32_t freq1 = 60240000;
    ucsCustom.setFrequency (freq1);

    ASSERT_NO_THROW (m_imager->initialize());

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ref_clk_i,output_frequency,enable_wavegen,fssc,kspread,delta,anaip_pllcfg1,anaip_pllcfg2,anaip_pllcfg3,anaip_pllcfg4,anaip_pllcfg5,anaip_pllcfg6,anaip_pllcfg7,anaip_pllcfg8,forbidden_seeking,forbidden_modfreq_div4,forbidden_modfreq_div6,forbidden_limit
    const std::vector<double> testVector{ 26000000, 240960000, 1, 10000, 0.5000, 0.0166, 16449, 34684, 1732, 21801, 54869, 33712, 35087, 2, 0, 0, 0, 0 };

    //the frequency should be assigned to LUT number 1, and this LUT must have the SSC enabled
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET), 0); //the LUT assignment to LUT1

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) & 1 << 10, 0); //the enable ssc bit

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1) > 0);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1) > 0);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG4_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG5_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG6_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG7_LUTx) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG8_LUTx) > 0);

    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1), testVector.at (6));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1), testVector.at (7));
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2), testVector.at (8));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG4_LUTx), testVector.at (9));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG5_LUTx), testVector.at (10));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG6_LUTx), testVector.at (11));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG7_LUTx), testVector.at (12));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG8_LUTx), testVector.at (13));
}

TEST_F (TestImagerM2450_A12_AIO, PllConfigFor60240kHzPlus80320kHz)
{
    const uint32_t modfreq1 = 80320000;
    const uint32_t modfreq2 = 60240000;
    UseCaseForM2450 ucsCustom (5u, 8, 2000u, modfreq1, modfreq2, 200u, true, true, 10000., 0.5, 0.0125, 0.0166);

    ASSERT_NO_THROW (m_imager->initialize());

    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ref_clk_i,output_frequency,enable_wavegen,fssc,kspread,delta,anaip_pllcfg1,anaip_pllcfg2,anaip_pllcfg3,anaip_pllcfg4,anaip_pllcfg5,anaip_pllcfg6,anaip_pllcfg7,anaip_pllcfg8,forbidden_seeking,forbidden_modfreq_div4,forbidden_modfreq_div6,forbidden_limit
    const std::vector<double> testVector80320{ 26000000, 321280000, 1, 10000, 0.5000, 0.0125, 16473, 24400, 3803, 21801, 39402, 37831, 35087, 2, 0, 0, 0, 0 };
    const std::vector<double> testVector60240{ 26000000, 240960000, 1, 10000, 0.5000, 0.0166, 16449, 34684, 1732, 21801, 54869, 33712, 35087, 2, 0, 0, 0, 0 };

    //the frequency should be assigned to LUT number 1
    const auto regChanged = m_bridge->getWrittenRegisters();

    //grayscale at 3152500
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET), 0); //the LUT assignment to LUT1

    //first raw frame set at 80320000Hz
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 4) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 4), 1); //the LUT assignment to LUT2
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 8) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 8), 1); //the LUT assignment to LUT2
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 12) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 12), 1); //the LUT assignment to LUT2
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 16) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 16), 1); //the LUT assignment to LUT2

    //first raw frame set at 60240000Hz
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 20) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 20), 2); //the LUT assignment to LUT3
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 24) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 24), 2); //the LUT assignment to LUT3
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 28) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 28), 2); //the LUT assignment to LUT3
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_S00_PLLSET + 32) > 0);
    ASSERT_EQ (regChanged.at (AIO_NR_CFGCNT_S00_PLLSET + 32), 2); //the LUT assignment to LUT3

    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 5) & 1 << 10, 0); //the enable ssc bit of LUT2
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 5) > 0);
    ASSERT_GT (regChanged.at (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 8) & 1 << 10, 0); //the enable ssc bit of LUT3

    const uint16_t lutOffset2 = AIO_SSC_PLLCFG_LUT_OFFSET;
    const uint16_t lutOffset3 = 2 * AIO_SSC_PLLCFG_LUT_OFFSET;
    const uint16_t RECONFIG_CFGCNT_PLLCFG1_LUT2 = AIO_NR_CFGCNT_PLLCFG1_LUT1 + 3;
    const uint16_t RECONFIG_CFGCNT_PLLCFG1_LUT3 = RECONFIG_CFGCNT_PLLCFG1_LUT2 + 3;

    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG4_LUTx + lutOffset2) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG5_LUTx + lutOffset2) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG6_LUTx + lutOffset2) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG7_LUTx + lutOffset2) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG8_LUTx + lutOffset2) > 0);

    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT2), testVector80320.at (6));
    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT2 + 1), testVector80320.at (7));
    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT2 + 2), testVector80320.at (8));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG4_LUTx + lutOffset2), testVector80320.at (9));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG5_LUTx + lutOffset2), testVector80320.at (10));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG6_LUTx + lutOffset2), testVector80320.at (11));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG7_LUTx + lutOffset2), testVector80320.at (12));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG8_LUTx + lutOffset2), testVector80320.at (13));

    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG4_LUTx + lutOffset3) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG5_LUTx + lutOffset3) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG6_LUTx + lutOffset3) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG7_LUTx + lutOffset3) > 0);
    ASSERT_TRUE (regChanged.count (AIO_SSC_PLLCFG8_LUTx + lutOffset3) > 0);

    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT3), testVector60240.at (6));
    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT3 + 1), testVector60240.at (7));
    ASSERT_EQ (regChanged.at (RECONFIG_CFGCNT_PLLCFG1_LUT3 + 2), testVector60240.at (8));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG4_LUTx + lutOffset3), testVector60240.at (9));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG5_LUTx + lutOffset3), testVector60240.at (10));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG6_LUTx + lutOffset3), testVector60240.at (11));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG7_LUTx + lutOffset3), testVector60240.at (12));
    ASSERT_EQ (regChanged.at (AIO_SSC_PLLCFG8_LUTx + lutOffset3), testVector60240.at (13));
}


TEST_F (TestImagerM2450_A12_AIO, VerifyPllLUTReassignment)
{
    initImager();

    const uint32_t modFreqLUT1Default = 30000000;
    const uint32_t modFreqLUT2Default = 60000000;
    const uint32_t modFreq1 = 20200000;
    const uint32_t modFreq2 = 40160000;
    const uint32_t modFreq3 = 60240000;
    const uint32_t modFreq4 = 80320000;

    //assign and execute several non-default modulation frequencies
    UseCaseForM2450 ucd1a (5u, 8, 100u, modFreq1, modFreq2, 200u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucd1a));

    UseCaseForM2450 ucd1b (5u, 8, 100u, modFreq3, modFreq4, 200u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucd1b));

    //re-initialize the imager (this must reset LUT assignments to the default frequencies)
    m_imager->sleep();
    ASSERT_NO_THROW (m_imager->wake());
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    UseCaseForM2450 ucd2 (5u, 8, 400u, modFreqLUT1Default, modFreqLUT2Default, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucd2));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    // LUT1 registers must have been changed
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1) == 1);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 1) == 1);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 2) == 1);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 3) == 1);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 4) == 1);
    ASSERT_TRUE (regChanged.count (AIO_NR_CFGCNT_PLLCFG1_LUT1 + 5) == 1);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseExecuteRawFrameRateEqualTargetFrameRate)
{
    initImager();
    //test the mixed usage of raw frame rates and a target frame rate.
    //by defining a target frame rate the last raw frame delay counter should contain
    //a value different to the other raw frame delay counters, but the sum of
    //all raw frame timings must match the target frame rate.
    const float FSYSCLK = 133333333.3f; //this test is only for imagers with this sysclock
    const uint32_t targetFrameRate = 35u;

    //a four phase use case without a grayscale image

    UseCaseForM2450 ucs (targetFrameRate, 4, 1000u, 99870000u, 0u);
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //now set a raw frame rate and verify changed registers
    //note: achievable raw frame rate depends on the interface delay set by the mock module config
    ucs.setRawFrameRate (180);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //should be the four frame rate registers and the CFGCNT_CTRLSEQ
    ASSERT_EQ (static_cast<int> (regChanged.size()), 5);

    auto tSequence = 0.f;

    for (auto reg : regChanged)
    {
        tSequence += (static_cast<float> (1024u * reg.second) / FSYSCLK);
    }

    auto fSequence = 1.f / tSequence;

    //check for match with targetFrameRate (allow just a small deviation - quantization)
    ASSERT_LT (std::fabs (fSequence - static_cast<float> (targetFrameRate)), 0.02);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseExecuteRawFrameRateEqualTargetFrameRateHighExpoNegative)
{
    initImager();
    const uint32_t targetFrameRate = 10u; //use low value - this UT is only about raw frame rates

    //a four phase use case without a grayscale image
    UseCaseForM2450 ucs (targetFrameRate, 4, 5000u, 99870000u, 0u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //now set a raw frame rate (4.9ms) that shouldn't be possible with 5ms expo time
    ucs.setRawFrameRate (201);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseExecuteEightPhaseWithGrayScale)
{
    initImager();
    UseCaseForM2450 ucs (5u, 8, 1000u, 30000000, 20200000, 1000u);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, CustomModPllFreq)
{
    initImager();
    /*
    * Create an instance of the imager and execute
    * an use-case with one modified modfreq. The only registers
    * that are written during execution should be expotime/framerate and
    * pll-lut assigment, the CFGCNT_CTRLSEQ, the 5 pll-ssc registers
    * plus the 3 pllcfg registers for the new freq.
    */
    UseCaseCustomFreq_M2450 ucs (false, 0u);
    UseCaseCustomFreq_M2450 ucsCustom (true, 0u);
    ucsCustom.setFrequency (60240000);

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //should be CFGCNT_CTRLSEQ + 5 x pll-ssc + 4 x expo-time + 1x raw-framerate + 3 x pllfreq (LUT change for UCD-FourPhase)
    ASSERT_EQ (static_cast<int> (regChanged.size()), 14);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseSupportStandardEightPhase)
{
    UseCaseForM2450 ucs (5u, 8, 1000u, 30000000, 20200000, 1000u);

    initImager();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //sanity-check - is the SR-container touch and the sequence length set?
    const uint16_t SR_START = 0xC320;
    ASSERT_TRUE (regMMConfig.count (SR_START) > 0);
    ASSERT_TRUE (regMMConfig.count (CFGCNT_CTRLSEQ) > 0);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseExecuteFourPhaseWithGrayScale)
{
    initImager();
    UseCaseForM2450 ucs (35u, 4, 1000u, 30000000, 20200000, 1000u);

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
}

TEST_F (TestImagerM2450_A12_AIO, PicoFlexxDC)
{
    initImager();
    UseCasePicoFlexxDC ucs (true);

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    UseCasePicoFlexxDC ucsCustom (false);

    ASSERT_EQ (m_imager->verifyUseCase (ucsCustom), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsCustom));
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseMixedModeStopAlignedRawFrameRate)
{
    //test for invariant: sum of all idle/delay times must be equal
    //no matter what alignment the non-master-clock RFS has
    // 1) create use-case non mixed-mode (all start-aligned)
    // 2) execute use-case and get raw-frame-rate register of last raw frame
    // 3) create use-case with equal raw frames, but stop align one
    // 4) execute use case and scan for the raw frame rate register
    // 5) check if the raw frame rate register values are equal
    initImager();
    UseCaseAlignment ucsStandard (false);
    UseCaseAlignment ucsStopAligned (true);

    //generate register settings for ucsStandard
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsStandard));
    const auto regStandard = m_bridge->getWrittenRegisters();

    //reset imager clears tracked register list (force full download of new use case)
    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());

    //generate register settings for ucsStopAligned
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsStopAligned));
    const auto regStopAligned = m_bridge->getWrittenRegisters();

    uint32_t sumRawFrameRatesUcsStandard = 0;
    uint32_t sumRawFrameRatesUcsStopAligned = 0;

    for (auto regAdr = AIO_SR_NR_FRAMERATE; regAdr <= static_cast<uint16_t> (AIO_SR_NR_FRAMERATE + 20u); regAdr = static_cast<uint16_t> (regAdr + 4u))
    {
        if (regStandard.count (regAdr))
        {
            sumRawFrameRatesUcsStandard += regStandard.at (regAdr);
        }
        if (regStopAligned.count (regAdr))
        {
            sumRawFrameRatesUcsStopAligned += regStopAligned.at (regAdr);
        }
    }

    ASSERT_EQ (regStandard.size(), regStopAligned.size());
    ASSERT_EQ (sumRawFrameRatesUcsStandard, sumRawFrameRatesUcsStopAligned);
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseMixedModeMBLimit)
{
    //this requires a special UCD type with alternating raw frames to prevent usage of repeats
    UseCaseAlternateRF_M2450 ucsOk (8);
    UseCaseAlternateRF_M2450 ucsNotOk (9);

    initImager();

    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucsOk));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsOk));

    //9 MBs are not feasible and shouldn't be accepted
    ASSERT_EQ (ImagerVerificationStatus::SEQUENCER, m_imager->verifyUseCase (ucsNotOk));
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseMixedModeLowPowerNotEnabled)
{
    UseCaseCustomMM_M2450 ucs;

    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //verify if the low power counter registers are set
    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //3 MB are in use (1 for the 4+1 measurement and 2 for the 8+1 measurement)
    ASSERT_TRUE (regMMConfig.count (0xC334) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC335) > 0);
    ASSERT_EQ (0x0, regMMConfig.at (0xC334));
    ASSERT_EQ (0x0, regMMConfig.at (0xC335));
    ASSERT_TRUE (regMMConfig.count (0xC34B) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC34C) > 0);
    ASSERT_EQ (0x0, regMMConfig.at (0xC34B));
    ASSERT_EQ (0x0, regMMConfig.at (0xC34C));

    // only the last MB is adding a delay at its end and only it should have the low power registers set
    ASSERT_TRUE (regMMConfig.count (0xC362) > 0);
    ASSERT_EQ (0u, regMMConfig.at (0xC362)); //this would be 0x003e if low power support would be enabled
    ASSERT_TRUE (regMMConfig.count (0xC363) > 0);
    ASSERT_EQ (0u, regMMConfig.at (0xC363)); //this would be 0x1484 if low power support would be enabled
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseInterleavedModeLowPowerNotEnabled)
{
    UseCaseCustomInterleaved_M2450 ucs (5u, 2u, 1000u, 30000000u, 30000000u, 30000000u);
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_EQ (ImagerVerificationStatus::SUCCESS, m_imager->verifyUseCase (ucs));
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //verify if the low power counter registers are set
    const auto regMMConfig = m_bridge->getWrittenRegisters();

    //4 MBs are in use (1 for a 4+1 measurement, 3 for an interleaved 8+1 measurement and its enclosed 4+1 one)

    //the first 4+1 should enter the low power mode (long delay till the next MB starts)
    ASSERT_TRUE (regMMConfig.count (0xC334) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC335) > 0);
    ASSERT_EQ (0u, regMMConfig.at (0xC334)); //this would be 0x0044 if low power support would be enabled
    ASSERT_EQ (0u, regMMConfig.at (0xC335)); //this would be 0x379D if low power support would be enabled

    //the first part of the interleaved must not enter low power mode (the enclosed 4+1 measurement will start right after it)
    ASSERT_TRUE (regMMConfig.count (0xC34B) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC34C) > 0);
    ASSERT_EQ (0x0, regMMConfig.at (0xC34B));
    ASSERT_EQ (0x0, regMMConfig.at (0xC34C));

    //this should be the MB containing the enclosed 4+1 measurement, right after it the second part
    //of the 8+1 measurement should start
    ASSERT_TRUE (regMMConfig.count (0xC362) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC363) > 0);
    ASSERT_EQ (0x0, regMMConfig.at (0xC362));
    ASSERT_EQ (0x0, regMMConfig.at (0xC363));

    //the second part of the 8+1 measurement is the end of the complete sequence and should enter the low power mode...
    ASSERT_TRUE (regMMConfig.count (0xC379) > 0);
    ASSERT_TRUE (regMMConfig.count (0xC37A) > 0);
    ASSERT_EQ (0u, regMMConfig.at (0xC379)); //this would be 0x0042 if low power support would be enabled
    ASSERT_EQ (0u, regMMConfig.at (0xC37A)); //this would be 0xF905 if low power support would be enabled
}

TEST_F (TestImagerM2450_A12_AIO, VerifyExpoCalcForMBs)
{
    initImager();

    const uint32_t expoTestTimeMB1 = 2000u;
    const uint32_t expoTestTimeMB2 = 100u;
    const uint32_t modFreq1 = 20200000;
    const uint32_t modFreq2 = 30000000;

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    UseCaseCustomMM_M2450 ucd (5u, 1u, expoTestTimeMB1, expoTestTimeMB2, modFreq1, modFreq2, modFreq2);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucd));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    {
        const uint16_t MB1_EXPO = 0xC320;
        ASSERT_TRUE (regChanged.count (MB1_EXPO) > 0);

        const uint16_t tExpo = static_cast<uint16_t> (regChanged.at (MB1_EXPO));
        const float tIllu = (float) tExpo * 8.f * (1e6f / (float) modFreq1);

        ASSERT_NEAR ( (float) expoTestTimeMB1, tIllu, 2.0f);
    }

    {
        const uint16_t MB2_EXPO = 0xC337;
        ASSERT_TRUE (regChanged.count (MB2_EXPO) > 0);

        const uint16_t tExpo = static_cast<uint16_t> (regChanged.at (MB2_EXPO));
        const float tIllu = (float) tExpo * 8.f * (1e6f / (float) modFreq2);

        ASSERT_NEAR ( (float) expoTestTimeMB2, tIllu, 2.0f);
    }
}

TEST_F (TestImagerM2450_A12_AIO, SwitchToNormalMode)
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

    UseCaseCustomMM_M2450 ucsMM (40u, 6u, 134u, 134u, 30000000u, 20200000u, 20600000u, 134u);
    ucsMM.setImage (160, 160);

    UseCaseForM2450 ucsNormal (40u, 4, 40u, 30000000, 0u, 40u, true);
    ucsNormal.setImage (160, 160);

    uint16_t reconfigIdx = 0u;
    initImager();

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsMM));

    const auto regConfigMM = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regConfigMM.count (CFGCNT_PLLCFG1_LUT1) > 0);

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsMM.setExposureTime (144u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsMM, reconfigIdx));

    ASSERT_NO_THROW (m_imager->stopCapture());

    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucsNormal));

    const auto regConfigToNormal = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regConfigToNormal.count (CFGCNT_CTRLSEQ) > 0);
    ASSERT_EQ (regConfigToNormal.at (CFGCNT_CTRLSEQ), 0x0004); //safe reconfig enabled and 5 raw frames

    ASSERT_TRUE (regConfigToNormal.at (0xC3A0) > 0);

    ASSERT_NO_THROW (m_imager->startCapture());

    ucsNormal.setExposureTime (134u);
    ASSERT_NO_THROW (m_imager->reconfigure (ucsNormal, reconfigIdx));
}

TEST_F (TestImagerM2450_A12_AIO, ReconfigureNormalMode)
{
    //in case of reconfiguration (a call to IImager::reconfigure) different registers are used
    UseCaseForM2450 ucs (5u, 8, 1000u, 30000000, 20200000, 1000u);

    initImager();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //change expo-time of first two MBs
    ucs.setExposureTime (101u);
    m_bridge->clearRegisters();

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));

    //the normal configuration (using startCapture) would use:
    //[0](0xa88d, 0x8008) - CTRL_SEQ (this is forced write to CFGCNT, no DMEM access, so it is okay)
    //[2](0xc320, 0x017a) - the changed exposures of the first RFS
    //[3](0xc324, 0x017a)
    //[4](0xc328, 0x017a)
    //[5](0xc32c, 0x017a)
    //[6](0xc32d, 0x011b) - the raw frame rate of the last sequence entry of the 1st RFS
    //[7](0xc330, 0x00ff) - the changed exposures of the second RFS
    //[8](0xc334, 0x00ff)
    //[9](0xc338, 0x00ff)
    //[10](0xc33c, 0x00ff)
    //[11](0xc33d, 0x011c) - the raw frame rate of the last sequence entry of the 2nd RFS
    //[12](0xc3ac, 0x0000) - LPFSM FR_1 (if low power would be used this would be 0x0047)
    //[13](0xc3ad, 0x0000) - LPFSM FR_2 (if low power would be used this would be 0x10f5)

    //with the CE_normal_fix the new config should be (refer to ROYAL-2119):
    const uint16_t expoRFS1 = 0x017a; //S00-S03
    const uint16_t frateS03 = 0x011b;
    const uint16_t expoRFS2 = 0x00ff; //S04-S07
    const uint16_t frateS07 = 0x011c;

    //compare the expectation to what has been reconfigured
    auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (0xA850) > 0);
    ASSERT_TRUE (regChanged.count (0xA852) > 0);
    ASSERT_TRUE (regChanged.count (0xA854) > 0);
    ASSERT_TRUE (regChanged.count (0xA856) > 0);
    ASSERT_TRUE (regChanged.count (0xA857) > 0);

    ASSERT_TRUE (regChanged.count (0xA858) > 0);
    ASSERT_TRUE (regChanged.count (0xA85A) > 0);
    ASSERT_TRUE (regChanged.count (0xA85C) > 0);
    ASSERT_TRUE (regChanged.count (0xA85E) > 0);
    ASSERT_TRUE (regChanged.count (0xA85F) > 0);

    ASSERT_EQ (expoRFS1, regChanged[0xA850]);
    ASSERT_EQ (expoRFS1, regChanged[0xA852]);
    ASSERT_EQ (expoRFS1, regChanged[0xA854]);
    ASSERT_EQ (expoRFS1, regChanged[0xA856]);
    ASSERT_EQ (frateS03, regChanged[0xA857]);

    ASSERT_EQ (expoRFS2, regChanged[0xA858]);
    ASSERT_EQ (expoRFS2, regChanged[0xA85A]);
    ASSERT_EQ (expoRFS2, regChanged[0xA85C]);
    ASSERT_EQ (expoRFS2, regChanged[0xA85E]);
    ASSERT_EQ (frateS07, regChanged[0xA85F]);

    //if low power would be used the following register would have changed
    //const uint16_t lpfsm_FR1 = 0x0000;
    //const uint16_t lpfsm_FR2 = 0x0000;
    //ASSERT_TRUE (regChanged.count (0xA878) > 0);
    //ASSERT_TRUE (regChanged.count (0xA879) > 0);
    //ASSERT_EQ (lpfsm_FR1, regChanged[0xA878]);
    //ASSERT_EQ (lpfsm_FR2, regChanged[0xA879]);
}

TEST_F (TestImagerM2450_A12_AIO, ReconfigureMixedMode)
{
    //in case of reconfiguration (a call to IImager::reconfigure) different registers are used

    //2:1 ratio means 2x(4+1)-measurements + 1x(8+1)-measurement
    UseCaseCustomMM_M2450 ucs (20u, 2u, 1000u, 1000u, 30000000u, 20200000u, 20600000u, 1000u);

    initImager();
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));
    ASSERT_NO_THROW (m_imager->startCapture());

    //change expo-time of first two MBs
    ucs.setExposureTime (108u, 0, 9);
    m_bridge->clearRegisters();

    uint16_t frameNumber = 0;
    ASSERT_NO_THROW (m_imager->reconfigure (ucs, frameNumber));

    //the normal configuration (using startCapture) would use:
    //[1](0xc320, 0x0195) - the changed exposures of the first (4+1) measurement
    //[2](0xc324, 0x0195)
    //[3](0xc328, 0x0195)
    //[4](0xc32c, 0x0195)
    //[5](0xc32d, 0x011c) - the raw frame rate of the last sequence entry of the 1st MB
    //[6](0xc334, 0x0000) - LPFSM FR_1 of 1st MB (if low power would be used this would be 0x000f)
    //[7](0xc335, 0x0000) - LPFSM FR_2 of 1st MB (if low power would be used this would be 0x01D4)
    //[8](0xc337, 0x0195) - the changed exposures of the second (4+1) measurement
    //[9](0xc33b, 0x0195)
    //[10](0xc33f, 0x0195)
    //[11](0xc343, 0x0195)
    //[12](0xc344, 0x011c) - the raw frame rate of the last sequence entry of the 2nd MB
    //[13](0xc379, 0x0000) - LPFSM FR_1 of 4th MB (if low power would be used this would be 0x0003)
    //[14](0xc37a, 0x0000) - LPFSM FR_2 of 4th MB (if low power would be used this would be 0xFC84)

    //with the CE_AIO_mb_fix the new config should be (refer to ROYAL-2119):
    const uint16_t expoMB1 = 0x0195;
    const uint16_t decodeMB1Expo = 0/*1st MB*/ + (0xF << 4) /*seqnum 0-3*/ + (0 << 12) /*expo-reg*/;
    const uint16_t fRateMB1 = 0x011c;
    const uint16_t decodeMB1Frate = 0/*1st MB*/ + (0x8 << 4) /*seqnum 3*/ + (1 << 12) /*fr-reg*/;

    //if low power would be used the following two decodes would be used
    //const uint16_t lpfsmMB1_FR1 = 0x0000;
    //const uint16_t decodeMB1CtrlFR1 = 0/*1nd MB*/ + (1 << 9) /*CTRL*/ + (0 << 12) /*LPFSMFR1*/;
    //const uint16_t lpfsmMB1_FR2 = 0x0000;
    //const uint16_t decodeMB1CtrlFR2 = 0/*1nd MB*/ + (1 << 9) /*CTRL*/ + (1 << 12) /*LPFSMFR2*/;

    const uint16_t expoMB2 = 0x0195;
    const uint16_t decodeMB2Expo = 1/*2nd MB*/ + (0xF << 4) /*seqnum 0-3*/ + (0 << 12) /*expo-reg*/;
    const uint16_t fRateMB2 = 0x011c;
    const uint16_t decodeMB2Frate = 1/*2nd MB*/ + (0x8 << 4) /*seqnum 3*/ + (1 << 12) /*fr-reg*/;

    //if low power would be used the following two decodes would be used
    //const uint16_t lpfsmMB4_FR1 = 0x0000;
    //const uint16_t decodeMB4CtrlFR1 = 3/*4th MB*/ + (1 << 9) /*CTRL*/ + (0 << 12) /*LPFSMFR1*/;
    //const uint16_t lpfsmMB4_FR2 = 0x0000;
    //const uint16_t decodeMB4CtrlFR2 = 3/*4th MB*/ + (1 << 9) /*CTRL*/ + (1 << 12) /*LPFSMFR2*/;

    const uint16_t flags = 3/*full cfg done + partly config done*/ + (6 << 8) /*6 params used*/;

    //compare the expectation to what has been reconfigured
    const auto regChanged = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regChanged.count (0xA814) > 0);
    ASSERT_TRUE (regChanged.count (0xA815) > 0);
    ASSERT_TRUE (regChanged.count (0xA816) > 0);
    ASSERT_TRUE (regChanged.count (0xA817) > 0);

    ASSERT_TRUE (regChanged.count (0xA81C) > 0);
    ASSERT_TRUE (regChanged.count (0xA81D) > 0);
    ASSERT_TRUE (regChanged.count (0xA81E) > 0);
    ASSERT_TRUE (regChanged.count (0xA81F) > 0);
    ASSERT_TRUE (regChanged.count (0xA87A) > 0);

    ASSERT_EQ (expoMB1, regChanged.at (0xA814));
    ASSERT_EQ (decodeMB1Expo, regChanged.at (0xA815));
    ASSERT_EQ (fRateMB1, regChanged.at (0xA816));
    ASSERT_EQ (decodeMB1Frate, regChanged.at (0xA817));

    ASSERT_EQ (expoMB2, regChanged.at (0xA81A));
    ASSERT_EQ (decodeMB2Expo, regChanged.at (0xA81B));
    ASSERT_EQ (fRateMB2, regChanged.at (0xA81C));
    ASSERT_EQ (decodeMB2Frate, regChanged.at (0xA81D));

    ASSERT_EQ (flags, regChanged.at (0xA87A));

    //if low power would be used the following register would have changed
    //ASSERT_TRUE(regChanged.count(0xA818) > 0);
    //ASSERT_TRUE(regChanged.count(0xA819) > 0);
    //ASSERT_TRUE(regChanged.count(0xA81A) > 0);
    //ASSERT_TRUE(regChanged.count(0xA81B) > 0);
    //ASSERT_EQ(lpfsmMB1_FR1, regChanged[0xA818]);
    //ASSERT_EQ(decodeMB1CtrlFR1, regChanged[0xA819]);
    //ASSERT_EQ(lpfsmMB1_FR2, regChanged[0xA81A]);
    //ASSERT_EQ(decodeMB1CtrlFR2, regChanged[0xA81B]);

    //ASSERT_TRUE (regChanged.count (0xA820) > 0);
    //ASSERT_TRUE (regChanged.count (0xA821) > 0);
    //ASSERT_TRUE (regChanged.count (0xA822) > 0);
    //ASSERT_TRUE (regChanged.count (0xA823) > 0);
    //ASSERT_EQ (lpfsmMB4_FR1, regChanged[0xA820]);
    //ASSERT_EQ (decodeMB4CtrlFR1, regChanged[0xA821]);
    //ASSERT_EQ (lpfsmMB4_FR2, regChanged[0xA822]);
    //ASSERT_EQ (decodeMB4CtrlFR2, regChanged[0xA823]);
}

TEST_F (TestImagerM2450_A12_AIO, IlluminationPadLVDS)
{
    m_simImager.reset (new SimImagerM2450_A12_AIO());

    ASSERT_NO_THROW (m_bridge.reset (new StubBridgeImager (std::move (m_simImager))));

    {
        ImagerParameters params{ m_bridge, nullptr, false, false,
                                 ImgTrigger::GPIO13, ImgImageDataTransferType::PIF, 0., {},
                                 SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                 ImgIlluminationPad::LVDS, 100000000, false };

        ASSERT_NO_THROW (m_imager.reset (new ImagerM2450_A12_AIO (params)));
    }

    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());

    m_bridge->writeImagerRegister (ANAIP_DESIGNSTEP, 0x0A12);

    ASSERT_NO_THROW (m_imager->initialize());

    //verify if the LVDS registers are set
    const auto regMMConfig = m_bridge->getWrittenRegisters();

    ASSERT_TRUE (regMMConfig.count (0xA892) > 0);
    ASSERT_TRUE (regMMConfig.count (0xB087) > 0);
    ASSERT_EQ (0x0133u, regMMConfig.at (0xA892));
    ASSERT_EQ (0x0001u, regMMConfig.at (0xB087));
}

TEST_F (TestImagerM2450_A12_AIO, UseCaseMixedModeMasterClockDistribution)
{
    //test for invariant: sum of all idle/delay times must be equal
    //no matter what if the second part of a 4+4 use case is start-aligned
    //or is a clock-aligned one
    initImager();
    UseCaseMasterClockTicks ucsStandard (false);
    UseCaseMasterClockTicks ucsSpread (true);

    //generate register settings for ucsStandard
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsStandard));
    const auto regStandard = m_bridge->getWrittenRegisters();

    //reset imager clears tracked register list (force full download of new use case)
    ASSERT_NO_THROW (m_imager->sleep());
    ASSERT_NO_THROW (m_imager->wake());
    ASSERT_NO_THROW (m_imager->initialize());

    //generate register settings for ucsSpread
    m_bridge->clearRegisters();
    ASSERT_NO_THROW (m_imager->executeUseCase (ucsSpread));
    const auto regSpread = m_bridge->getWrittenRegisters();

    uint32_t sumRawFrameRatesStandard = 0;
    uint32_t sumRawFrameRatesSpread = 0;

    const uint16_t regFramerateStart = 0xC321;
    const uint16_t regFramerateEnd = 0xC341;

    for (auto regAdr = regFramerateStart; regAdr <= regFramerateEnd; regAdr = static_cast<uint16_t> (regAdr + 4u))
    {
        sumRawFrameRatesStandard += regStandard.at (regAdr);
        sumRawFrameRatesSpread += regSpread.at (regAdr);
    }

    ASSERT_EQ (sumRawFrameRatesStandard, sumRawFrameRatesSpread);
}
