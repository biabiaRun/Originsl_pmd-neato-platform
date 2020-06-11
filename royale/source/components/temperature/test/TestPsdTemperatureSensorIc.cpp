/****************************************************************************\
* Copyright (C) 2020 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <temperature/PsdTemperatureSensorIc.hpp>
#include <common/IPseudoDataInterpreter.hpp>
#include <common/MakeUnique.hpp>
#include <MockRawFrame.hpp>
#include <gmock/gmock.h>
#include <cfloat>

using namespace royale;
using testing::_;
using testing::AtLeast;
using testing::Return;
using testing::FloatEq;
using common::makeUnique;
using sensors::test::MockRawFrame;

class MockPseudoDataInterpreter : public common::IPseudoDataInterpreter
{
    // IPseudoDataInterpreter interface
public:
    MockPseudoDataInterpreter() = default;
    MOCK_METHOD(common::IPseudoDataInterpreter *, clone, (), (override));
    MOCK_METHOD( uint16_t, getFrameNumber, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint16_t, getReconfigIndex, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint16_t, getFollowingFrameNumber, (uint16_t, uint16_t), (const, override));
    MOCK_METHOD( bool, isGreaterFrame, (uint16_t , uint16_t ), (const, override));
    MOCK_METHOD( uint16_t, frameNumberFwdDistance, (uint16_t , uint16_t ), (const, override));
    MOCK_METHOD( uint16_t, getSequenceIndex, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint8_t, getBinning, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint16_t, getHorizontalSize, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint16_t, getVerticalSize, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( float, getImagerTemperature, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( std::vector<uint16_t>, getTemperatureRawValues, (const common::ICapturedRawFrame &), (const, override));
    MOCK_METHOD( uint16_t, getRequiredImageWidth, (), (const, override));
    MOCK_METHOD( void, getEyeSafetyError, (const common::ICapturedRawFrame &, uint32_t &), (const, override));
    MOCK_METHOD( bool, supportsExposureFromPseudoData, (), (const, override));
    MOCK_METHOD( uint32_t, getExposureTime, (const common::ICapturedRawFrame &, uint32_t ), (const, override));
};



class TestPsdTemperatureSensorIc : public ::testing::Test
{
protected:
    TestPsdTemperatureSensorIc()
    {

    }

    virtual ~TestPsdTemperatureSensorIc()
    {

    }

    virtual void SetUp() override
    {

        for (auto i = 0u; i < 5u; ++i)
        {
            auto tmpFrame = new MockRawFrame();
            frames.push_back (tmpFrame);
        }
    }

    virtual void TearDown() override
    {
        for (auto frame : frames)
        {
            delete frame;
        }
    }

    std::vector<royale::common::ICapturedRawFrame *> frames;
    float mAlpha = 0.03f;
};

/// Testing whether values are averaged as expected
TEST_F (TestPsdTemperatureSensorIc, getTemperature)
{
    auto psdi = makeUnique<MockPseudoDataInterpreter>();
    auto first_temp = std::vector<uint16_t>(1,uint16_t(296u));
    auto second_temp = std::vector<uint16_t>(1,uint16_t(0xffffu));
    auto third_temp = std::vector<uint16_t>(1,uint16_t(360u));
    auto fourth_temp = std::vector<uint16_t>(1,uint16_t(161u));
    auto fifth_temp = std::vector<uint16_t>(1,uint16_t(0u));
    EXPECT_CALL (*psdi, getTemperatureRawValues (_))
    .Times (AtLeast (1))
    .WillOnce (Return (first_temp))
    .WillOnce (Return (second_temp))
    .WillOnce (Return (third_temp))
    .WillOnce (Return (fourth_temp))
    .WillRepeatedly (Return (fifth_temp));
    sensors::PsdTemperatureSensorIc sensor (std::move(psdi));

    // Read via monitor
    EXPECT_THAT (sensor.calcTemperature (frames), FloatEq (25.0f));
    EXPECT_THAT (sensor.calcTemperature (frames), FloatEq (12094.21484375f));
    EXPECT_THAT (sensor.calcTemperature (frames), FloatEq (36.84f));
    EXPECT_THAT (sensor.calcTemperature (frames), FloatEq (0.0249996185302734375f));
    EXPECT_THAT (sensor.calcTemperature (frames), FloatEq (-29.76000213623046875f));
}

