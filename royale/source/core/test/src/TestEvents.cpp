#include <common/events/EventDeviceDisconnected.hpp>
#include <gtest/gtest.h>

TEST (TestEvents, TestEventDeviceDisconnected)
{
    royale::event::EventDeviceDisconnected event;

    EXPECT_EQ(event.severity(), royale::EventSeverity::ROYALE_FATAL);
    EXPECT_EQ(event.type(), royale::EventType::ROYALE_DEVICE_DISCONNECTED);
}
