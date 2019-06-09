/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <device/PsdTemperatureSensorFilter.hpp>
#include <StubTemperatureSensor.hpp>
#include <gmock/gmock.h>

using namespace royale;
using testing::_;
using testing::AtLeast;
using testing::Return;
using testing::FloatEq;

class MockRawFrame : public common::ICapturedRawFrame
{
public:
    MockRawFrame() = default;
    virtual ~MockRawFrame() = default;
    MOCK_METHOD0 (getImageData, uint16_t *());
    MOCK_CONST_METHOD0 (getPseudoData, const uint16_t *());
};

class MockPsdTemperatureSensor : public hal::IPsdTemperatureSensor
{
public:
    MOCK_METHOD1 (calcTemperature, float (const std::vector<common::ICapturedRawFrame *> &frames));
};


class TestPsdTemperatureSensorFilter : public ::testing::Test
{
protected:
    TestPsdTemperatureSensorFilter()
        : m_filter(), m_sensor()
    {

    }

    virtual ~TestPsdTemperatureSensorFilter()
    {

    }

    virtual void SetUp() override
    {
        m_sensor = std::make_shared<MockPsdTemperatureSensor>();
        m_filter = std::make_shared<device::PsdTemperatureSensorFilter> (m_sensor, mAlpha);

        for (auto i = 0u; i < 5u; ++i)
        {
            auto tmpFrame = new MockRawFrame();
            frames.push_back (tmpFrame);
        }
    }

    virtual void TearDown() override
    {
        m_filter.reset();
        m_sensor = nullptr;

        for (auto frame : frames)
        {
            delete frame;
        }
    }


    std::shared_ptr<device::PsdTemperatureSensorFilter> m_filter;
    std::shared_ptr<MockPsdTemperatureSensor> m_sensor;
    std::vector<royale::common::ICapturedRawFrame *> frames;
    float mAlpha = 0.03f;
};

/// Testing whether values are averaged as expected
TEST_F (TestPsdTemperatureSensorFilter, getTemperature)
{
    auto first_temp = 1.0f;
    auto second_temp = 2.0f;
    auto third_temp = 3.0f;
    auto fourth_temp = 4.0f;
    auto beta = 1.0f - mAlpha;
    EXPECT_CALL (*m_sensor, calcTemperature (_))
    .Times (AtLeast (1))
    .WillOnce (Return (first_temp))
    .WillOnce (Return (second_temp))
    .WillOnce (Return (third_temp))
    .WillRepeatedly (Return (fourth_temp));

    // Read via monitor
    EXPECT_THAT (m_filter->calcTemperature (frames), FloatEq (first_temp));
    EXPECT_THAT (m_filter->calcTemperature (frames), FloatEq (mAlpha * second_temp + beta * first_temp));
    EXPECT_THAT (m_filter->calcTemperature (frames), FloatEq (mAlpha * third_temp + beta * (mAlpha * second_temp + beta * first_temp)));
    EXPECT_THAT (m_filter->calcTemperature (frames), FloatEq (mAlpha * fourth_temp + beta * (mAlpha * third_temp + beta * (mAlpha * second_temp + beta * first_temp))));
}

