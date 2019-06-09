#include <common/EventQueue.hpp>
#include <royale/IEvent.hpp>
#include <royale/IEventListener.hpp>
#include <gtest/gtest.h>

using namespace royale;

/** Testing register/unregister */
TEST (TestEventQueue, RegUnreg)
{
    EventQueue eventQueue;

    // Event handler (counts events)
    class : public royale::IEventListener
    {
    public:
        int nevents;
        void onEvent (std::unique_ptr<royale::IEvent> &&event) override
        {
            ++nevents;
        }
    } handler;


    class TestEvent : public royale::IEvent
    {
        royale::EventSeverity severity() const override
        {
            return royale::EventSeverity::ROYALE_INFO;
        }

        const royale::String describe() const override
        {
            return royale::String();
        }

        royale::EventType type() const override
        {
            return royale::EventType::ROYALE_CAPTURE_STREAM;
        }
    };



    handler.nevents = 0;

    // Handler is not yet registered.
    eventQueue.onEvent (std::unique_ptr<royale::IEvent> (new TestEvent()));
    eventQueue.sync();

    EXPECT_EQ (0, handler.nevents);

    eventQueue.setEventListener (&handler);
    eventQueue.sync();

    EXPECT_EQ (0, handler.nevents);

    eventQueue.onEvent (std::unique_ptr<royale::IEvent> (new TestEvent()));
    eventQueue.sync();

    EXPECT_EQ (1, handler.nevents);

    eventQueue.setEventListener (nullptr);
    eventQueue.sync();

    EXPECT_EQ (1, handler.nevents);

    eventQueue.onEvent (std::unique_ptr<royale::IEvent> (new TestEvent()));
    eventQueue.sync();

    EXPECT_EQ (1, handler.nevents);

}

