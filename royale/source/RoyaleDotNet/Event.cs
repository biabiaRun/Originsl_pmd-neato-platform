/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RoyaleDotNet
{
    public sealed class Event
    {
        /**
        * Severity of an Event.
        */
        public enum EventSeverity
        {
            /** Information only event.
            */
            ROYALE_INFO = 0,
            /** Potential issue detected (e.g. soft overtemperature limit reached).
            */
            ROYALE_WARNING = 1,
            /** Errors occurred during operation.
            * The operation (e.g. recording or stream capture) has failed and was stopped.
            */
            ROYALE_ERROR = 2,
            /** Severe error was detected.
            * The corresponding ICameraDevice is no longer in an usable state.
            */
            ROYALE_FATAL = 3
        };

        /**
        * Type of an IEvent.
        */
        public enum EventType
        {
            ROYALE_CAPTURE_STREAM,
            ROYALE_DEVICE_DISCONNECTED,
            ROYALE_OVER_TEMPERATURE,
            ROYALE_RAW_FRAME_STATS
        };

        private EventSeverity severity;
        private EventType type;
        private string description;

        internal Event (EventType type, EventSeverity severity, string description)
        {
            this.severity = severity;
            this.type = type;
            this.description = description;
        }

        public EventSeverity Severity
        {
            get
            {
                return severity;
            }
        }

        public EventType Type
        {
            get
            {
                return type;
            }
        }

        public string Description
        {
            get
            {
                return description;
            }
        }
    }
}
