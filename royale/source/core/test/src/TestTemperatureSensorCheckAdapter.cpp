/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <device/TemperatureSensorCheckAdapter.hpp>
#include <StubTemperatureSensor.hpp>
#include <common/MakeUnique.hpp>
#include <gmock/gmock.h>

using namespace royale;
using royale::common::makeUnique;
using testing::_;

class MockTemperatureChecker : public device::ITemperatureAcceptor
{
public:
    MOCK_CONST_METHOD1 (acceptTemperature, void (float temp));
};


class TestTemperatureSensorCheckAdapter : public ::testing::Test
{
protected:
    TestTemperatureSensorCheckAdapter()
        : m_monitor(), m_sensor(), m_checker ()
    {

    }

    virtual ~TestTemperatureSensorCheckAdapter()
    {

    }

    virtual void SetUp() override
    {
        m_sensor  = std::make_shared<stub::hal::StubTemperatureSensor>();
        auto checker = makeUnique<MockTemperatureChecker>();
        m_checker = checker.get();
        m_monitor = std::make_shared<device::TemperatureSensorCheckAdapter> (m_sensor, std::move (checker));
    }

    virtual void TearDown() override
    {
        m_monitor.reset();
        m_sensor.reset();
        m_checker = nullptr;
    }


    std::shared_ptr<device::TemperatureSensorCheckAdapter>       m_monitor;
    std::shared_ptr<stub::hal::StubTemperatureSensor> m_sensor;
    MockTemperatureChecker                           *m_checker;
};

/** Testing whether values are properly routed thru the getter */
TEST_F (TestTemperatureSensorCheckAdapter, getTemperature)
{
    float test_temp = 20.0f; // well below thresholds
    m_sensor->setTemperature (test_temp);
    EXPECT_CALL (*m_checker, acceptTemperature (test_temp))
    .Times (1);

    // Read directly from sensor
    EXPECT_FLOAT_EQ (test_temp, m_sensor->getTemperature());

    // Read via monitor
    EXPECT_FLOAT_EQ (test_temp, m_monitor->getTemperature());

    test_temp = 30.0f; // still below thresholds
    m_sensor->setTemperature (test_temp);
    EXPECT_CALL (*m_checker, acceptTemperature (test_temp))
    .Times (1);

    EXPECT_FLOAT_EQ (test_temp, m_sensor->getTemperature());
    EXPECT_FLOAT_EQ (test_temp, m_monitor->getTemperature());
}

/** Alerts shouldn't be raised when querying the base sensor directly. */
TEST_F (TestTemperatureSensorCheckAdapter, nonAlerts)
{
    EXPECT_CALL (*m_checker, acceptTemperature (_))
    .Times (0);

    // Read directly from sensor should never trigger any check.

    m_sensor->setTemperature (20.0f); // well below tresholds
    (void) m_sensor->getTemperature();

    m_sensor->setTemperature (5930.0f); // boiling tungsten
    (void) m_sensor->getTemperature();
}

