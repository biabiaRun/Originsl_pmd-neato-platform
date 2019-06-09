/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "inc/JNIEvent.hpp"

namespace jroyale
{
    using namespace royale;

    jobject createJEvent (JNIEnv *env, const std::unique_ptr<royale::IEvent> &event)
    {
        jobject jSeverity;
        switch (event->severity ())
        {
            case EventSeverity::ROYALE_ERROR:
                jSeverity = jEvent$Severity_ERROR;
                break;
            case EventSeverity::ROYALE_FATAL:
                jSeverity = jEvent$Severity_FATAL;
                break;
            case EventSeverity::ROYALE_INFO:
                jSeverity = jEvent$Severity_INFO;
                break;
            case EventSeverity::ROYALE_WARNING:
                jSeverity = jEvent$Severity_WARNING;
                break;
        }

        jobject jType;
        switch (event->type ())
        {
            case EventType::ROYALE_CAPTURE_STREAM:
                jType = jEvent$Type_CAPTURE_STREAM;
                break;
            case EventType::ROYALE_DEVICE_DISCONNECTED:
                jType = jEvent$Type_DEVICE_DISCONNECTED;
                break;
            case EventType::ROYALE_OVER_TEMPERATURE:
                jType = jEvent$Type_OVER_TEMPERATURE;
                break;
            case EventType::ROYALE_RAW_FRAME_STATS:
                jType = jEvent$Type_RAW_FRAME_STATS;
                break;
        }

        auto jDescription = env->NewStringUTF (event->describe ().c_str ());
        auto jEventObj = env->CallStaticObjectMethod (jCameraEvent, jEvent_createEvent, jSeverity, jDescription, jType);

        return jEventObj;
    }
}
