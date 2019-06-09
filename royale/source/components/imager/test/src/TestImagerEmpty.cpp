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
#include <SimImagerM2452.hpp>
#include <StubBridgeImager.hpp>
#include <common/exceptions/LogicError.hpp>
#include <imager/ImagerEmpty.hpp>
#include <imager/M2450_A12/PseudoDataInterpreter.hpp>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::stub;

class TestImagerEmpty : public ::testing::Test, IImageSensorLoggingListener
{
protected:
    TestImagerEmpty() :
        m_logCount (0)
    {

    }

    virtual ~TestImagerEmpty()
    {

    }

    virtual void SetUp()
    {
        std::unique_ptr <M2450_A12::PseudoDataInterpreter> pdi;
        m_simImager.reset (new SimImagerM2452 (0xA11));
        pdi.reset (new M2450_A12::PseudoDataInterpreter());
        m_bridge.reset (new StubBridgeImager (std::move (m_simImager)));

        ASSERT_NE (m_bridge, nullptr) << "Bridge instance is null.";

        m_imager.reset (new ImagerEmpty (m_bridge, std::move (pdi)));

        ASSERT_NE (m_imager, nullptr) << "Imager instance is null.";

        m_logCount = 0;
        m_imager->setLoggingListener (this);

        ASSERT_NO_THROW (m_imager->sleep());
    }

    void onImagerLogEvent (const std::string &serial, std::chrono::high_resolution_clock::time_point logTime, ImageSensorLogType logType, const std::string &logMessage) override
    {
        m_logCount++;
    }

    virtual void TearDown()
    {
        m_simImager.reset();
        m_bridge.reset();
        m_imager.reset();
    }

    size_t m_logCount;
    std::shared_ptr <StubBridgeImager> m_bridge;
    std::unique_ptr <SimImagerM2452> m_simImager;
    std::unique_ptr <ImagerEmpty> m_imager;
};


TEST_F (TestImagerEmpty, CreateImagerDirectly)
{
    ASSERT_THROW (new ImagerEmpty (nullptr, nullptr), LogicError);
    ASSERT_THROW (new ImagerEmpty (m_bridge, nullptr), LogicError);
}

TEST_F (TestImagerEmpty, InitImager)
{
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_TRUE (m_logCount > 0);
}

TEST_F (TestImagerEmpty, ClonePseudoData)
{
    auto pdi = dynamic_cast<M2450_A12::PseudoDataInterpreter *> (m_imager->createPseudoDataInterpreter().get());
    ASSERT_NE (pdi, nullptr);
}

TEST_F (TestImagerEmpty, SupportDirectWrite)
{
    ASSERT_NO_THROW (m_imager->writeRegisters ({}, {}));
}

TEST_F (TestImagerEmpty, NoSerial)
{
    ASSERT_EQ (m_imager->getSerialNumber(), "0000-0000-0000-0000");
}

TEST_F (TestImagerEmpty, NoMeasurementBlocks)
{
    ASSERT_EQ (m_imager->getMeasurementBlockSizes(), std::vector<std::size_t> {});
}

TEST_F (TestImagerEmpty, SupportAllUseCases)
{
    ASSERT_EQ (m_imager->verifyUseCase (ImagerUseCaseDefinition()), ImagerVerificationStatus::SUCCESS);
}

TEST_F (TestImagerEmpty, CheckStateless)
{
    uint16_t idx;
    ASSERT_NO_THROW (m_imager->startCapture());
    ASSERT_NO_THROW (m_imager->stopCapture());
    ASSERT_NO_THROW (m_imager->executeUseCase (ImagerUseCaseDefinition()));
    ASSERT_NO_THROW (m_imager->reconfigureExposureTimes (std::vector<uint32_t>(), idx));
    ASSERT_NO_THROW (m_imager->reconfigureTargetFrameRate (std::numeric_limits<uint16_t>::max(), idx));
    ASSERT_NO_THROW (m_imager->initialize());
    ASSERT_NO_THROW (m_imager->sleep());
}

TEST_F (TestImagerEmpty, DirectReadWriteLengthMismatch)
{
    ASSERT_THROW (m_imager->writeRegisters ({ 0x1234 }, { 1u, 2u }), LogicError);

    std::vector<uint16_t> readReg{ 0x0000 };
    ASSERT_THROW (m_imager->readRegisters ({ 0x1234, 0x2345 }, readReg), LogicError);
}

TEST_F (TestImagerEmpty, HexWrite)
{
    ASSERT_NO_THROW (m_imager->writeRegisters ({ 0x1234 }, { 0xAA55 }));

    ASSERT_GT (m_bridge->getWrittenRegisters().size(), 0u);

    ASSERT_EQ (m_bridge->getWrittenRegisters() [0x1234], 0xAA55);
}

TEST_F (TestImagerEmpty, HexRead)
{
    ASSERT_NO_THROW (m_imager->writeRegisters ({ 0x1234 }, { 0xAA55 }));

    std::vector<uint16_t> readReg{ 0x0000 };
    ASSERT_NO_THROW (m_imager->readRegisters ({ 0x1234 }, readReg));

    ASSERT_EQ (readReg[0], uint16_t{ 0xAA55 });
}

TEST_F (TestImagerEmpty, DecimalWrite)
{
    ASSERT_NO_THROW (m_imager->writeRegisters ({ 12345u }, { 0x55AA }));

    ASSERT_GT (m_bridge->getWrittenRegisters().size(), 0u);

    ASSERT_EQ (m_bridge->getWrittenRegisters() [12345], 0x55AA);
}

TEST_F (TestImagerEmpty, DecimalRead)
{
    ASSERT_NO_THROW (m_imager->writeRegisters ({ 1234u }, { 0xAA55 }));

    std::vector<uint16_t> readReg{ 0x0000 };
    ASSERT_NO_THROW (m_imager->readRegisters ({ 1234u }, readReg));

    ASSERT_EQ (readReg[0], uint16_t{ 0xAA55 });
}

TEST_F (TestImagerEmpty, BurstWrite)
{
    m_bridge->clearRegisters();
    m_bridge->resetRegisterCalls();

    ASSERT_NO_THROW (m_imager->writeRegisters ({ 12345u }, { 0x55AA }));

    ASSERT_EQ (m_bridge->registerCalls(), 1u);
    ASSERT_GT (m_bridge->getWrittenRegisters().size(), 0u);

    ASSERT_EQ (m_bridge->getWrittenRegisters() [12345], 0x55AA);

    m_bridge->clearRegisters();
    m_bridge->resetRegisterCalls();

    ASSERT_NO_THROW (m_imager->writeRegisters ({ 12345, 12346 }, { 0x55AA, 0x55AB }));

    ASSERT_EQ (m_bridge->registerCalls(), 1u);
    ASSERT_EQ (m_bridge->getWrittenRegisters().size(), 2u);

    ASSERT_EQ (m_bridge->getWrittenRegisters() [12345], 0x55AA);
    ASSERT_EQ (m_bridge->getWrittenRegisters() [12346], 0x55AB);

    m_bridge->clearRegisters();
    m_bridge->resetRegisterCalls();

    ASSERT_NO_THROW (m_imager->writeRegisters ({ 12345u, 12347u }, { 0x55AA, 0x55AB }));

    ASSERT_GT (m_bridge->registerCalls(), 1u);
    ASSERT_EQ (m_bridge->getWrittenRegisters().size(), 2u);

    ASSERT_EQ (m_bridge->getWrittenRegisters() [12345], 0x55AA);
    ASSERT_EQ (m_bridge->getWrittenRegisters() [12347], 0x55AB);
}

TEST_F (TestImagerEmpty, BurstRead)
{
    m_bridge->clearRegisters();
    m_bridge->resetRegisterCalls();

    ASSERT_NO_THROW (m_imager->writeRegisters ({ 12345, 12346, 12347 }, { 0x55AA, 0x55AB, 0x55AC }));

    m_bridge->resetRegisterCalls();

    std::vector<uint16_t> readReg{ 0x0000, 0x0000 };
    ASSERT_NO_THROW (m_imager->readRegisters ({ 12345u, 12346u }, readReg));

    ASSERT_EQ (m_bridge->registerCalls(), 1u);

    ASSERT_EQ (readReg[0], 0x55AAu);
    ASSERT_EQ (readReg[1], 0x55ABu);

    m_bridge->resetRegisterCalls();

    std::vector<uint16_t> readReg2{ 0x0000, 0x0000 };
    ASSERT_NO_THROW (m_imager->readRegisters ({ 12345u, 12347u }, readReg2));

    ASSERT_EQ (m_bridge->registerCalls(), 2u);

    ASSERT_EQ (readReg2[0], 0x55AAu);
    ASSERT_EQ (readReg2[1], 0x55ACu);
}
