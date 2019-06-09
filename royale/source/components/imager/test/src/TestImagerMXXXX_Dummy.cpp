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
#include <SimImagerMXXXX_Dummy.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <StubBridgeImager.hpp>
#include <imager/ImagerMXXXX_Dummy.hpp>
#include <imager/M2452/PseudoDataInterpreter.hpp>
#include <imager/WrapperImagerExternalConfig.hpp>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

namespace
{
    const uint32_t SYSFREQ = 0; //unknown

    const TimedRegisterList rmStart{ { 0x8000, 0x0001, 0x0000 } };
    const TimedRegisterList rmStop{ { 0x8000, 0x0000, 0x0000 } };
    const TimedRegisterList rmInit{ { 0x1234, 0xABCD, 0x0000 } };
    const TimedRegisterList rmFwPage1{ { 0x1234, 0xABCD, 0x0000 } };
    const TimedRegisterList rmFwPage2{ { 0x1234, 0xABCD, 0x0000 } };
    const TimedRegisterList rmFwStart{ { 0x1234, 0xABCD, 0x0000 } };
    const std::vector<IImagerExternalConfig::UseCaseData> rmUseCases
    {
        { ImagerUseCaseIdentifier {"guid"}, "name", { size_t{ 9 } }, {}, TimedRegisterList { { 0xFEED, 0x0A11, 0x0000 } } }
    };

    const auto rmExternalConfig = std::make_shared<WrapperImagerExternalConfig> (rmInit, rmFwPage1, rmFwPage2, rmFwStart, rmStart, rmStop, rmUseCases);
}

class TestImagerMXXXX_Dummy : public ::testing::Test
{
protected:
    TestImagerMXXXX_Dummy()
    {
    }

    virtual ~TestImagerMXXXX_Dummy()
    {

    }

    virtual void SetUp()
    {
        m_simImager.reset (new SimImagerMXXXX_Dummy());
        m_bridge.reset (new StubBridgeImager (std::move (m_simImager)));

        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";

        ImagerParameters params{ m_bridge, m_externalConfig, true, false,
                                 ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.f, {},
                                 SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                                 ImgIlluminationPad::SE_P, 90000000, false };

        m_imager.reset (new ImagerMXXXX_Dummy (params));

        ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";

        ASSERT_NO_THROW (m_imager->sleep());
    }

    virtual void TearDown()
    {
        m_simImager.reset();
        m_bridge.reset();
        m_imager.reset();
    }

    std::shared_ptr<royale::imager::IImagerExternalConfig> m_externalConfig{ rmExternalConfig };
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <SimImagerMXXXX_Dummy> m_simImager;
    std::unique_ptr <ImagerMXXXX_Dummy> m_imager;
};


TEST_F (TestImagerMXXXX_Dummy, CreateImagerDirectly)
{
    ImagerParameters params{ nullptr, nullptr, true, false,
                             ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.f, {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 90000000, false };

    ASSERT_THROW (new ImagerMXXXX_Dummy (params), LogicError);
}

TEST_F (TestImagerMXXXX_Dummy, CreateImagerWithoutConfig)
{
    ImagerParameters params{ m_bridge, nullptr, true, false,
                             ImgTrigger::I2C, ImgImageDataTransferType::MIPI_2LANE, 0.f, {},
                             SYSFREQ, ImagerRawFrame::ImagerDutyCycle::DC_50,
                             ImgIlluminationPad::SE_P, 90000000, false };

    ASSERT_THROW (new ImagerMXXXX_Dummy (params), RuntimeError);
}

TEST_F (TestImagerMXXXX_Dummy, InitImager)
{
    ASSERT_NO_THROW (m_imager->initialize());
}

TEST_F (TestImagerMXXXX_Dummy, PseudoData)
{
    auto pdi = dynamic_cast<royale::imager::M2452::PseudoDataInterpreter *> (m_imager->createPseudoDataInterpreter().get());
    ASSERT_NE (pdi, nullptr);
}

TEST_F (TestImagerMXXXX_Dummy, NoLoggingListener)
{
    ASSERT_THROW (m_imager->setLoggingListener (nullptr), NotImplemented);
}

TEST_F (TestImagerMXXXX_Dummy, NoSerial)
{
    ASSERT_EQ (m_imager->getSerialNumber(), "0000-0000-0000-0000");
}

TEST_F (TestImagerMXXXX_Dummy, MeasurementBlocks)
{
    ASSERT_EQ (m_imager->getMeasurementBlockSizes(), std::vector<std::size_t> { rmUseCases.at (0).imageStreamBlockSizes });
}

TEST_F (TestImagerMXXXX_Dummy, SupportAllUseCases)
{
    ASSERT_EQ (m_imager->verifyUseCase (ImagerUseCaseIdentifier {""}), ImagerVerificationStatus::SUCCESS);
}
