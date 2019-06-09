/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/events/EventDeviceDisconnected.hpp>

using namespace royale::event;

EventDeviceDisconnected::EventDeviceDisconnected ()
{
}

royale::EventSeverity EventDeviceDisconnected::severity() const
{
    return royale::EventSeverity::ROYALE_FATAL;
}

const royale::String EventDeviceDisconnected::describe() const
{
    return "USB Device disconnected.";
}

royale::EventType EventDeviceDisconnected::type() const
{
    return royale::EventType::ROYALE_DEVICE_DISCONNECTED;
}
