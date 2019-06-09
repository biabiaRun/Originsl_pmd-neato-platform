#include <device/PsdTemperatureSensorCheckAdapter.hpp>
#include <StubTemperatureSensor.hpp>
#include <gmock/gmock.h>

using namespace royale;
using testing::_;
using testing::AtLeast;
using testing::Return;
using testing::Eq;

class MockTemperatureChecker : public device::ITemperatureAcceptor
{
public:
    MOCK_CONST_METHOD1 (acceptTemperature, void (float temp));
};

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


class TestPsdTemperatureSensorCheckAdapter : public ::testing::Test
{
protected:
    TestPsdTemperatureSensorCheckAdapter()
        : m_monitor(), m_sensor(), m_checker ()
    {

    }

    virtual ~TestPsdTemperatureSensorCheckAdapter()
    {

    }

    virtual void SetUp() override
    {
        m_sensor = std::make_shared<MockPsdTemperatureSensor>();
        m_checker = std::make_shared<MockTemperatureChecker>();
        m_monitor = std::make_shared<device::PsdTemperatureSensorCheckAdapter> (m_sensor, m_checker);

        for (auto i = 0u; i < 5u; ++i)
        {
            auto tmpFrame = new MockRawFrame();
            frames.push_back (tmpFrame);
        }
    }

    virtual void TearDown() override
    {
        m_monitor.reset();
        m_sensor = nullptr;
        m_checker = nullptr;

        for (auto frame : frames)
        {
            delete frame;
        }
    }


    std::shared_ptr<device::PsdTemperatureSensorCheckAdapter> m_monitor;
    std::shared_ptr<MockPsdTemperatureSensor> m_sensor;
    std::shared_ptr<MockTemperatureChecker> m_checker;
    std::vector<royale::common::ICapturedRawFrame *> frames;
};

/** Testing whether values are properly routed thru the getter */
TEST_F (TestPsdTemperatureSensorCheckAdapter, calcTemperature)
{
    float test_temp = 20.0f; // well below thresholds
    EXPECT_CALL (*m_sensor, calcTemperature (_))
    .Times (AtLeast (1))
    .WillRepeatedly (Return (test_temp));
    EXPECT_CALL (*m_checker, acceptTemperature (test_temp))
    .Times (1);

    // Read directly from sensor
    EXPECT_THAT (m_sensor->calcTemperature (frames), Eq (test_temp));

    // Read via monitor
    EXPECT_THAT (m_monitor->calcTemperature (frames), Eq (test_temp));
}

/** Alerts shouldn't be raised when querying the base sensor directly. */
TEST_F (TestPsdTemperatureSensorCheckAdapter, nonAlerts)
{
    auto temp1 = 20.0f; // well below thresholds
    auto temp2 = 5930.0f; // boiling tungsten

    EXPECT_CALL (*m_sensor, calcTemperature (_))
    .WillOnce (Return (temp1))
    .WillOnce (Return (temp2));

    // Read directly from sensor should never trigger any check.
    EXPECT_CALL (*m_checker, acceptTemperature (_))
    .Times (0);

    // Read directly from sensor should never trigger any check.
    (void) m_sensor->calcTemperature (frames);
    (void) m_sensor->calcTemperature (frames);
}

