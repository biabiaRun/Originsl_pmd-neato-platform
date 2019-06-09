#include <common/EventForwarder.hpp>
#include <common/events/EventCaptureStream.hpp>
#include <common/events/EventOverTemperature.hpp>
#include <royale/IEvent.hpp>
#include <royale/IEventListener.hpp>
#include <gtest/gtest.h>

using namespace royale;


class TestEventForwarder : public ::testing::Test
{
protected:
    TestEventForwarder()
        : m_eventFwd()
    {

    }

    virtual ~TestEventForwarder()
    {

    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    EventForwarder m_eventFwd;
};

/** Testing register/unregister */
TEST_F (TestEventForwarder, RegUnreg)
{
    // Event handler (counts events according to type)
    class : public royale::IEventListener
    {
    public:
        int nevents;
        int novertemp;
        int ncapture;

        void onEvent (std::unique_ptr<royale::IEvent> &&event) override
        {
            ++nevents;
            if (dynamic_cast<event::EventCaptureStream *> (event.get()) != nullptr)
            {
                ncapture++;
            }
            if (dynamic_cast<event::EventOverTemperature *> (event.get()) != nullptr)
            {
                novertemp++;
            }
        }

        void reset()
        {
            nevents = 0;
            novertemp = 0;
            ncapture = 0;
        }

    } handler;

    handler.reset();

    EXPECT_EQ (0, handler.nevents);
    EXPECT_EQ (0, handler.novertemp);
    EXPECT_EQ (0, handler.ncapture);

    // Handler is not yet registered.
    m_eventFwd.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_INFO, "");
    m_eventFwd.event<event::EventOverTemperature> (royale::EventSeverity::ROYALE_INFO, 0.0f, 0.0f);

    EXPECT_EQ (0, handler.nevents);
    EXPECT_EQ (0, handler.novertemp);
    EXPECT_EQ (0, handler.ncapture);

    m_eventFwd.setEventListener (&handler);

    EXPECT_EQ (0, handler.nevents);
    EXPECT_EQ (0, handler.novertemp);
    EXPECT_EQ (0, handler.ncapture);

    m_eventFwd.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_INFO, "");

    EXPECT_EQ (1, handler.nevents);
    EXPECT_EQ (0, handler.novertemp);
    EXPECT_EQ (1, handler.ncapture);

    handler.reset();
    m_eventFwd.event<event::EventOverTemperature> (royale::EventSeverity::ROYALE_INFO, 0.0f, 0.0f);

    EXPECT_EQ (1, handler.nevents);
    EXPECT_EQ (1, handler.novertemp);
    EXPECT_EQ (0, handler.ncapture);

    handler.reset();
    m_eventFwd.setEventListener (nullptr);
    m_eventFwd.event<event::EventCaptureStream> (royale::EventSeverity::ROYALE_INFO, "");
    m_eventFwd.event<event::EventOverTemperature> (royale::EventSeverity::ROYALE_INFO, 0.0f, 0.0f);

    // no change expected
    EXPECT_EQ (0, handler.nevents);
    EXPECT_EQ (0, handler.novertemp);
    EXPECT_EQ (0, handler.ncapture);

}
