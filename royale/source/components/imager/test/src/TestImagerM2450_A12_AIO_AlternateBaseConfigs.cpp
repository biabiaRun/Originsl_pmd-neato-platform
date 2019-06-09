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
#include <imager/ImagerM2450_A12_AIO.hpp>
#include <TestImagerCommon.hpp>
#include <SimImagerM2450_A12_AIO.hpp>
#include <imager/M2450_A12/ImagerRegisters.hpp>
#include <common/exceptions/WrongState.hpp>

using namespace royale::imager::M2450_A12;

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 26000000;
}

class TestImagerM2450_A12_AIO_AlternateBaseConfigs : public ::testing::Test
{
protected:
    TestImagerM2450_A12_AIO_AlternateBaseConfigs()
    {

    }

    virtual ~TestImagerM2450_A12_AIO_AlternateBaseConfigs()
    {

    }

    virtual void SetUp()
    {
        m_simImager.reset (new SimImagerM2450_A12_AIO());
        m_bridge.reset (new StubBridgeImager (std::move (m_simImager)));

        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";
    }

    void initImagerWithBaseConfig (ImgTrigger trigger)
    {
        {
            ImagerParameters params{ m_bridge, nullptr, false, false,
                                     trigger, ImgImageDataTransferType::PIF, 0., {},
                                     SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                     ImgIlluminationPad::SE_P, 100000000, false };

            m_imager.reset (new ImagerM2450_A12_AIO (params));
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

    std::unique_ptr <SimImagerM2450_A12_AIO> m_simImager;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <ImagerM2450_A12_AIO> m_imager;
};

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StartCaptureExternalTriggerFalse)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO13);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (false));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //now the pad should be set to input
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_FALSE (regChanged.count (ANAIP_GPIOMUX4) > 0);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StartCaptureExternalTriggerGpio13)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO13);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //now the pad should be set to input
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x00a0);

    //should be in capturing state now, no second call to startCapture allowed
    ASSERT_THROW (m_imager->startCapture(), WrongState);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StartCaptureExternalTriggerGpio14)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO14);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //now the pad should be set to input
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x1400);

    //should be in capturing state now, no second call to startCapture allowed
    ASSERT_THROW (m_imager->startCapture(), WrongState);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StopCaptureNoExternalTriggerGpio13)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO13);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //no external trigger happend, stopping should succeed and disable the input pad
    ASSERT_NO_THROW (m_imager->stopCapture());

    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x0180);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StopCaptureNoExternalTriggerGpio14)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO14);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //no external trigger happend, stopping should succeed and disable the input pad
    ASSERT_NO_THROW (m_imager->stopCapture());

    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x3000);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StopCaptureStillExternalTriggerGpio13)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO13);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //simulate start by external trigger
    m_bridge->getImager().startCapturing();

    //external trigger not yet released, imager must force stop
    ASSERT_NO_THROW (m_imager->stopCapture());

    //stopCapture must release the external trigger input (stop will be forced)
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x0180);
}

TEST_F (TestImagerM2450_A12_AIO_AlternateBaseConfigs, StopCaptureStillExternalTriggerGpio14)
{
    initImagerWithBaseConfig (ImgTrigger::GPIO14);
    ASSERT_NO_THROW (m_imager->setExternalTrigger (true));

    ASSERT_NO_THROW (m_imager->initialize());

    ASSERT_NO_THROW (m_imager->executeUseCase (UseCaseForM2450()));
    ASSERT_NO_THROW (m_imager->startCapture());

    //simulate start by external trigger
    m_bridge->getImager().startCapturing();

    //external trigger not yet released, imager must force stop
    ASSERT_NO_THROW (m_imager->stopCapture());

    //stopCapture must release the external trigger input (stop will be forced)
    const auto regChanged = m_bridge->getWrittenRegisters();
    ASSERT_TRUE (regChanged.count (ANAIP_GPIOMUX4) > 0);
    ASSERT_EQ (regChanged.at (ANAIP_GPIOMUX4), 0x3000);
}
