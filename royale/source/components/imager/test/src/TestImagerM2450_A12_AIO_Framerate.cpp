/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
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
#include <imager/M2450_A12/ConstantsAIOFirmware.hpp>
#include <TestImagerCommon.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <factory/ImagerUseCaseDefinitionAdapter.hpp>

using namespace royale::imager::M2450_A12;
using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 24000000;  //Excel-Sheet, Standard Configuration, C10
}

class ImagerWrapper : public ImagerM2450_A12_AIO
{
public:
    explicit ImagerWrapper (const ImagerParameters &params)
        : ImagerM2450_A12_AIO (params)
    {}

    bool publicCalcRawFrameRateTime (const ImagerUseCaseDefinition &useCase, uint32_t expoTime, uint32_t modFreq, bool isFirstRawFrame, double &rawFrameTime) const
    {
        return calcRawFrameRateTime (useCase, expoTime, modFreq, isFirstRawFrame, rawFrameTime);
    }
};

class TestImagerM2450_A12_AIO_Framerate: public ::testing::Test
{
protected:
    explicit TestImagerM2450_A12_AIO_Framerate()
    {

    }

    ~TestImagerM2450_A12_AIO_Framerate() override
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

    void createImager (uint32_t sysfreq = SYSFREQ, ImgImageDataTransferType interfaceType = ImgImageDataTransferType::MIPI_2LANE,
                       bool useSuperFrame = false)
    {
        {
            ImagerParameters params{ m_bridge, nullptr, useSuperFrame,
                                     royale::config::ImConnectedTemperatureSensor::NONE,
                                     ImgTrigger::GPIO13, interfaceType, 3.46e-7, {},
                                     sysfreq, ImagerRawFrame::ImagerDutyCycle::DC_37_5,
                                     ImgIlluminationPad::SE_P, 100000000, false };

            ASSERT_NO_THROW (m_imager.reset (new ImagerM2450_A12_AIO (params)));
        }

        ASSERT_NO_THROW (m_imager->sleep());
        ASSERT_NO_THROW (m_imager->wake());
    }

    std::unique_ptr <SimImagerM2450_A12_AIO> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2450_A12_AIO> m_imager;
};

std::shared_ptr<ImagerUseCaseDefinition> setupUsecase()
{
    //Some definitions used for this use case
    static const bool   ssc_enable = true;
    static const double ssc_freq = 10000.;
    static const double ssc_kspread = 0.5;
    static const double ssc_delta_80320kHz = 0.0125;
    static const uint16_t imageColumns = 320;
    static const uint16_t imageRows = 240;
    static const uint16_t roiCMin = 0;
    static const uint16_t roiRMin = 0;
    static const uint16_t flowControlRate = 0;
    static const uint32_t exposureMod = 27u;
    static const uint32_t exposureGrey = 27u;

    const uint32_t targetFrameRate = 50u;

    //special use case used by customer

    royale::usecase::UseCaseFourPhase uc4p (
        targetFrameRate, 80320000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 650u },
        exposureMod, exposureGrey, royale::usecase::ExposureGray::On,
        royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
        ssc_enable, ssc_freq, ssc_kspread, ssc_delta_80320kHz
    );
    uc4p.setImage (imageColumns, imageRows);

    return std::make_shared<royale::factory::ImagerUseCaseDefinitionAdapter> (uc4p, roiCMin, roiRMin, flowControlRate);
}

// This test is used to check if the correct register values are derived for a special use case.
// It checks if the correct framerate will result and was introduced due to a framerate mismatch issue (ROYALSUP-15)
TEST_F (TestImagerM2450_A12_AIO_Framerate, FramerateMismatchTest)
{
    initImager();
    auto usecase = setupUsecase();

    ASSERT_NO_THROW (m_imager->executeUseCase (*usecase));

    //ask the bridge what has been written
    const auto regChanged = m_bridge->getWrittenRegisters();

    //Check exposure time registers
    ASSERT_EQ (regChanged.at (0xC320), 0x010F);
    ASSERT_EQ (regChanged.at (0xC324), 0x010F);
    ASSERT_EQ (regChanged.at (0xC328), 0x010F);
    ASSERT_EQ (regChanged.at (0xC32C), 0x010F);
    ASSERT_EQ (regChanged.at (0xC330), 0x010F);

    //Check framerate registers
    ASSERT_EQ (regChanged.at (0xC321), 0x0000);
    ASSERT_EQ (regChanged.at (0xC325), 0x0000);
    ASSERT_EQ (regChanged.at (0xC329), 0x0000);
    ASSERT_EQ (regChanged.at (0xC32D), 0x0000);
    ASSERT_EQ (regChanged.at (0xC331), 0x0751);

}
