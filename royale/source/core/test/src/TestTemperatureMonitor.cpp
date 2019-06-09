#include <device/TemperatureMonitor.hpp>
#include <StubTemperatureSensor.hpp>
#include <gtest/gtest.h>

using namespace royale;
using namespace royale;


class TestTemperatureMonitor : public ::testing::Test,
    private IEventListener
{
protected:
    TestTemperatureMonitor()
        : m_monitor(), m_nevents (0)
    {

    }

    virtual ~TestTemperatureMonitor()
    {

    }

    virtual void SetUp() override
    {
        m_monitor = std::make_shared<device::TemperatureMonitor> (60.0f, 65.0f);
        m_monitor->setEventListener (this);
    }

    virtual void TearDown() override
    {
        m_monitor->setEventListener (nullptr);
        m_monitor.reset();
    }

    void onEvent (std::unique_ptr<royale::IEvent> &&event) override
    {
        ++m_nevents;
    }


    std::shared_ptr<device::TemperatureMonitor>       m_monitor;
    size_t                                            m_nevents;
};

/** Test whether the monitor generates events as it should. */
TEST_F (TestTemperatureMonitor, Alerts)
{
    ASSERT_EQ (0u, m_nevents);

    m_monitor->acceptTemperature (20.0f); // well below tresholds
    EXPECT_EQ (0u, m_nevents);
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    m_monitor->acceptTemperature (59.5f); // slightly below soft limit (within hysteresis)
    EXPECT_EQ (0u, m_nevents);
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    m_monitor->acceptTemperature (60.5f); // slightly above soft limit (within hysteresis)
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    m_monitor->acceptTemperature (64.5f); // slightly below hard limit (within hysteresis)
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    m_monitor->acceptTemperature (65.5f); // slightly above hard limit (within hysteresis)
    EXPECT_EQ (2u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

    m_monitor->acceptTemperature (5930.0f); // boiling tungsten
    EXPECT_EQ (2u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

}

/** Test retrigger feature. */
TEST_F (TestTemperatureMonitor, Retrigger)
{
    ASSERT_EQ (0u, m_nevents);

    auto temperature = 20.0f; // well below tresholds
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    temperature = 60.5f; // slightly above soft limit (within hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    // Repeat measure shouldn't generate another event.
    m_nevents = 0;
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    // After retrigger, repeat measure should generate events.
    m_nevents = 0;
    m_monitor->retrigger();
    EXPECT_EQ (0u, m_nevents); // not yet...
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents); // but after next measurement.
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    // Now test with the hard limit.
    temperature = 5930.0f; // boiling tungsten
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (2u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

    // Repeat measure
    m_nevents = 0;
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

    // After retrigger, repeat measure should generate events.
    m_nevents = 0;
    m_monitor->retrigger();
    EXPECT_EQ (0u, m_nevents); // not yet...
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (2u, m_nevents); // but after next measurement.
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

}

/** Test hysteresis. */
TEST_F (TestTemperatureMonitor, Hysteresis)
{
    ASSERT_EQ (0u, m_nevents);

    auto temperature = 20.0f; // well below tresholds
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    // Test: Trip the soft limit, cool (within hysteresis), heat again

    temperature = 60.5f; // slightly above soft limit (within hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    m_nevents = 0;
    temperature = 59.5f; // cool down a bit (within hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());

    // Test: Trip the soft limit, cool below hysteresis, heat again

    m_nevents = 0;
    temperature = 60.5f; // slightly above soft limit (within hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    m_nevents = 0;
    temperature = 58.5f; // cool down a bit (below hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_FALSE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    temperature = 60.5f; // re-heat above soft limit
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());


    // Test: Trip the hard limit, cool (within hysteresis), heat again
    m_nevents = 0;
    temperature = 65.5f; // slightly above hard limit
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());
    m_nevents = 0;
    temperature = 64.5f; // cool down a bit (within hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());
    temperature = 65.5f; // re-heat above limit
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

    // Test: Trip the hard limit, cool below hysteresis, heat again

    m_nevents = 0;
    temperature = 65.5f; // slightly above hard limit
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents); // no event expected, as we were above limit before.
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());
    m_nevents = 0;
    temperature = 63.5f; // cool down a bit (below hysteresis)
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (0u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_FALSE (m_monitor->hardAlarm());
    temperature = 65.5f; // re-heat above limit
    m_monitor->acceptTemperature (temperature);
    EXPECT_EQ (1u, m_nevents);
    EXPECT_TRUE (m_monitor->softAlarm());
    EXPECT_TRUE (m_monitor->hardAlarm());

}

