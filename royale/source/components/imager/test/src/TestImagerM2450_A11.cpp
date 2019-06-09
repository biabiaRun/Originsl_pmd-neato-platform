/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <StubBridgeImager.hpp>
#include <imager/ImagerM2450_A11.hpp>
#include <imager/M2450_A11/ImagerRegisters.hpp>
#include <TestImagerCommon.hpp>
#include <gtest/gtest.h>
#include <common/exceptions/Exception.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/LogicError.hpp>
#include <chrono>
#include <thread>

using namespace royale::imager;
using namespace royale::common;
using namespace royale::stub;

namespace
{
    class SimImagerM2450_A11 : public ISimImager
    {
    public:
        SimImagerM2450_A11()
        {
            m_SimulatedRegisters = std::map<uint16_t, uint16_t>
            {
                { M2450_A11::ANAIP_DESIGNSTEP, 0x0A11 },
            };
        }
        ~SimImagerM2450_A11() override {};
        void writeRegister (uint16_t regAddr, uint16_t value) override
        {
            m_SimulatedRegisters[regAddr] = value;
        };
        uint16_t readCurrentRegisterValue (uint16_t regAddr) override
        {
            return m_SimulatedRegisters[regAddr];
        };
        void startCapturing() override {};
        void stopCapturing() override {};

        void runSimulation (std::chrono::microseconds sleepDuration) override
        {
            // this class has no asynchronous actions
            (void) sleepDuration;
        }

    private:
        std::map<uint16_t, uint16_t> m_SimulatedRegisters;
    };

    // a valid ROI
    class UseCaseROIValid1 : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseROIValid1()
        {
            m_imageColumns = 112;
            m_imageRows = 63;
            m_roiCMin = 112;
            m_roiRMin = 113;
        }
    };

    // a valid ROI
    class UseCaseROIValid2 : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseROIValid2()
        {
            m_imageColumns = 144;
            m_imageRows = 96;
            m_roiCMin = 96;
            m_roiRMin = 96;
        }
    };

    // an invalid ROI
    class UseCaseROIInvalid : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseROIInvalid()
        {
            m_imageColumns = 1;
            m_imageRows = 1;
        }
    };

    // an invalid ROI that exceeds the max image dimensions
    class UseCaseROIInvalidMax : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseROIInvalidMax()
        {
            m_imageColumns = 352 + 32;
            m_imageRows = 120 + 32;
        }
    };

    class UseCaseModPLLChanged : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseModPLLChanged()
        {
            for (auto &rf : m_rawFrames)
            {
                rf.modulationFrequency = 20200000u;
            }
        }

        void switchFrequency()
        {
            //use another frequency available in the default LUT entries
            for (auto &rf : m_rawFrames)
            {
                rf.modulationFrequency = 99500000u;
            }
        }
    };

    class UseCaseInvalidFrequencies : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseInvalidFrequencies()
        {
            uint32_t modfreq = 30000000u;

            for (auto &rf : m_rawFrames)
            {
                rf.modulationFrequency = modfreq;
                modfreq = static_cast<uint32_t> (modfreq + 10000000u);
            }
        }
    };

    class UseCaseExcessiveExposure : public ImagerUseCaseFourPhase
    {
    public:
        UseCaseExcessiveExposure()
        {
            auto &grayFrame = m_rawFrames.at (0);
            /* adjust frame rate and mod freq to avoid exposure register overflow */
            m_targetRate = 100;
            grayFrame.modulationFrequency = 20200000u;
            grayFrame.exposureTime = 1000000 / m_targetRate;
        }
    };

}

class TestImagerM2450_A11 : public ::testing::Test
{
protected:
    TestImagerM2450_A11()
    {

    }

    virtual ~TestImagerM2450_A11()
    {

    }

    virtual void SetUp()
    {
        std::unique_ptr <SimImagerM2450_A11> simImager (new SimImagerM2450_A11());
        m_bridge.reset (new StubBridgeImager (std::move (simImager)));

        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";

        {
            ImagerParameters params{ m_bridge, nullptr, false, false,
                                     ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {},
                                     26000000, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                     ImgIlluminationPad::SE_P, 99500000, false };

            m_imager.reset (new royale::imager::ImagerM2450_A11 (params));
        }

        ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";

        ASSERT_NO_THROW (m_imager->sleep());
    }

    virtual void TearDown()
    {
        m_bridge.reset();
        m_imager.reset();
    }

    void initImager()
    {
        ASSERT_NO_THROW (m_imager->wake());

        m_bridge->writeImagerRegister (M2450_A11::ANAIP_DESIGNSTEP, 0x0A11);

        ASSERT_NO_THROW (m_imager->initialize());
    }

    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <royale::imager::ImagerM2450> m_imager;
};

TEST_F (TestImagerM2450_A11, CreateImagerDirectly)
{
    ImagerParameters params1{ nullptr, nullptr, false, false, ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {}, 0u, ImagerRawFrame::ImagerDutyCycle::DC_0, ImgIlluminationPad::SE_P, 0u, false };
    ImagerParameters params2{ m_bridge, nullptr, false, false, ImgTrigger::I2C, ImgImageDataTransferType::PIF, 0., {}, 0u, ImagerRawFrame::ImagerDutyCycle::DC_0, ImgIlluminationPad::SE_P, 0u, false };

    ASSERT_THROW (new royale::imager::ImagerM2450_A11 (params1), LogicError);
    ASSERT_THROW (new royale::imager::ImagerM2450_A11 (params2), LogicError);
}

TEST_F (TestImagerM2450_A11, InitImager)
{
    m_bridge->writeImagerRegister (M2450_A11::ANAIP_DESIGNSTEP, 0x0FFF);

    ASSERT_THROW (m_imager->initialize(), Exception);

    initImager();
}

TEST_F (TestImagerM2450_A11, StartCapture)
{
    initImager();

    //try start with MTCU busy
    m_bridge->writeImagerRegister (M2450_A11::CFGCNT_STATUS, 0x0000);
    ASSERT_THROW (m_imager->startCapture(), LogicError);

    //try start with MTCU idle and stick at idle
    m_bridge->writeImagerRegister (M2450_A11::CFGCNT_STATUS, 0x0001);
    ASSERT_THROW (m_imager->startCapture(), RuntimeError);
}

TEST_F (TestImagerM2450_A11, StopCapture)
{
    m_bridge->writeImagerRegister (M2450_A11::CFGCNT_STATUS, 0x0001);
    ASSERT_THROW (m_imager->stopCapture(), LogicError);
}

TEST_F (TestImagerM2450_A11, GetSerialNumber)
{
    m_bridge->writeImagerRegister (M2450_A11::ANAIP_EFUSEVAL1, 0xAAAA);
    m_bridge->writeImagerRegister (M2450_A11::ANAIP_EFUSEVAL2, 0x5555);
    m_bridge->writeImagerRegister (M2450_A11::ANAIP_EFUSEVAL3, 0xAAAA);
    m_bridge->writeImagerRegister (M2450_A11::ANAIP_EFUSEVAL4, 0x5555);

    ASSERT_NO_THROW (m_imager->getSerialNumber());

    std::string serial = m_imager->getSerialNumber();

    ASSERT_EQ (serial, "0005-2110-0341-1021");
}

TEST_F (TestImagerM2450_A11, UseCaseMixedModeNotSupported)
{
    UseCaseCustomMM_M2450 ucs;
    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::STREAM_COUNT);
}

TEST_F (TestImagerM2450_A11, UseCaseInvalidRawFrameCount)
{
    ImagerUseCaseFourPhase ucLower (50u, 0);
    ASSERT_EQ (ImagerVerificationStatus::SEQUENCER, m_imager->verifyUseCase (ucLower));

    ImagerUseCaseFourPhase ucUpper (50u, 33);
    ASSERT_EQ (ImagerVerificationStatus::SEQUENCER, m_imager->verifyUseCase (ucUpper));
}

TEST_F (TestImagerM2450_A11, UseCaseInvalidFrameRate)
{
    initImager();

    ImagerUseCaseFourPhase ucs (1000u, 8, 1000u, 30000000u, 20200000u, 1000u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::FRAMERATE);
}

TEST_F (TestImagerM2450_A11, UseCaseInvalidModFreqCount)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
    * with the UseCaseInvalidFrequencies use case.
    */
    UseCaseInvalidFrequencies ucs;

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::MODULATION_FREQUENCY);
}

TEST_F (TestImagerM2450_A11, UseCaseVerify)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
     * with the default use case the imager should be able to accept.
     */
    UseCaseROIValid1 ucs;

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseVerifyROIModPositive)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should be able to accept.
    */
    UseCaseROIValid2 ucs;

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseVerifyROIModNegative)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalid ucs;

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseExecuteGrayScale)
{
    initImager();

    ImagerUseCaseFourPhase ucs (50u, 0, 1000u, 0u, 0u, 1000u);

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //ask the bridge what has been written
    std::map < uint16_t, uint16_t > regChanged = m_bridge->getWrittenRegisters();

    //four ROI register, 2 LUT, sequence count
    ASSERT_EQ (static_cast<int> (regChanged.size()), 12);
}

TEST_F (TestImagerM2450_A11, UseCaseExecuteFourPhaseWithGrayScale)
{
    initImager();

    ImagerUseCaseFourPhase ucs (50u, 4, 1000u, 30000000u, 0u, 1000u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);

}

TEST_F (TestImagerM2450_A11, UseCaseExecuteEightPhaseWithGrayScale)
{
    initImager();

    ImagerUseCaseFourPhase ucs (5u, 8, 1000u, 30000000u, 20200000u, 1000u);

    ASSERT_EQ (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseExecuteROIModNegative)
{
    initImager();

    /* Creates an imager object and calls the executeUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalid ucs;

    ASSERT_THROW (m_imager->executeUseCase (ucs), Exception);
}

TEST_F (TestImagerM2450_A11, UseCaseVerifyROIExceedMaxImageSize)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseROIInvalidMax ucs;

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseVerifyExcessiveExposure)
{
    initImager();

    /* Creates an imager object and calls the verifyUseCase method
    * with a modified use case the imager should not be able to accept.
    */
    UseCaseExcessiveExposure ucs;

    ASSERT_NE (m_imager->verifyUseCase (ucs), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerM2450_A11, UseCaseFastChangeOfRoi)
{
    initImager();

    /* Creates an imager object and calls the executeUseCase method
    * with a modified use case the imager should be able to accept.
    * The use case is then partly modified and executed again.
    * The configuration written to the imager is tracked and evaluated.
    * The written registers should not contain more registers as
    * required to execute the use case change.
    */
    UseCaseROIValid1 ucs1;
    UseCaseROIValid2 ucs2;

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_EQ (m_imager->verifyUseCase (ucs1), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs1));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //change only a part of the use-case definition (move ROI a bit)
    //verify and execute new use case
    ASSERT_EQ (m_imager->verifyUseCase (ucs2), ImagerVerificationStatus::SUCCESS);
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs2));

    //ask the bridge what has been written
    std::map < uint16_t, uint16_t > regChanged = m_bridge->getWrittenRegisters();

    //four ROI register and one framerate register should have changed
    ASSERT_EQ (static_cast<int> (regChanged.size()), 5);
}

TEST_F (TestImagerM2450_A11, UseCaseFastChangeModPllFreq)
{
    initImager();

    /* Creates an imager object and calls the executeUseCase method
    * with a use case the imager should be able to accept.
    * The ModPLL frequency is then changed and executed again.
    * The configuration written to the imager is tracked and evaluated.
    * The written registers should not contain more registers as
    * required to execute the use case change.
    */
    UseCaseModPLLChanged ucs;

    //use case will be executed using the mock bridge, no actual harware required
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //let bridge forget all previous register changes
    m_bridge->clearRegisters();

    //change only the mod. pll frequency
    ucs.switchFrequency();

    //execute new use case
    ASSERT_NO_THROW (m_imager->executeUseCase (ucs));

    //ask the bridge what has been written
    std::map < uint16_t, uint16_t > regChanged = m_bridge->getWrittenRegisters();

    //four registers should have changed to set the new LUT references for the four raw frames
    //four registers should have changed to set the new exposure times for the four raw frames
    //one register should have changed to adapt the framerate counter which depends on the modfreq
    ASSERT_EQ (static_cast<int> (regChanged.size()), 9);
}
