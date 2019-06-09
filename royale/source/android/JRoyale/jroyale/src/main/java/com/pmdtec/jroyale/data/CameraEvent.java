/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.data;

/**
 * Interface for anything to be passed via IEventListener.
 */
public interface CameraEvent
{
    Severity severity();

    /**
     * Returns debugging information intended for developers using the Royale API. The strings
     * returned may change between releases, and are unlikely to be localised, so are neither
     * intended to be parsed automatically, nor intended to be shown to end users.
     */
    String describe();

    Type type();

    static CameraEvent createEvent(Severity eventSeverity, String describe, Type type)
    {
        return new SimpleCameraEvent(eventSeverity, describe, type);
    }

    /**
     * Severity of an IEvent.
     */
    enum Severity
    {
        /**
         * Information only event.
         */
        INFO,

        /**
         * Potential issue detected (e.g. soft overtemperature limit reached).
         */
        WARNING,

        /**
         * Errors occurred during operation.
         * The operation (e.g. recording or stream capture) has failed and was stopped.
         */
        ERROR,

        /**
         * A severe error was detected.
         * The corresponding ICameraDevice is no longer in a usable state.
         */
        FATAL
    }

    /**
     * Type of an IEvent.
     */
    enum Type
    {
        /**
         * The event was detected as part of the mechanism to receive image data.
         * <p>
         * For events of this class, WARNING is likely to indicate dropped frames, and
         * ERROR is likely to indicate that no more frames will be captured until after the
         * next call to ICameraDevice::startCapture().
         */
        CAPTURE_STREAM,

        /**
         * Royale is no longer able to talk to the camera; this is always severity FATAL.
         */
        DEVICE_DISCONNECTED,

        /**
         * The camera has become hot, likely because of the illumination.
         * <p>
         * For events of this class, WARNING indicates that the device is still functioning
         * but is near to the temperature at which it will be shut down for safety, and ERROR
         * indicates that the safety mechanism has been triggered.
         */
        OVER_TEMPERATURE,

        /**
         * This event is sent regularly during capturing.  The trigger for sending this event is
         * implementation defined and varies for different use cases, but the timing is normally
         * around one per second.
         * <p>
         * If all frames were successfully received then it will be INFO, if any were dropped
         * then it will be WARNING.
         */
        RAW_FRAME_STATS
    }

    class SimpleCameraEvent implements CameraEvent
    {
        private final Severity eventSeverity;
        private final String describe;
        private final Type type;

        SimpleCameraEvent(Severity eventSeverity, String describe, Type type)
        {
            this.eventSeverity = eventSeverity;
            this.describe = describe;
            this.type = type;
        }

        @Override
        public Severity severity()
        {
            return eventSeverity;
        }

        @Override
        public String describe()
        {
            return describe;
        }

        @Override
        public Type type()
        {
            return type;
        }

        @Override
        public String toString()
        {
            return "CameraEvent{" +
                    "eventSeverity=[" + eventSeverity +
                    "], describe=[" + describe +
                    "], type=[" + type + "]}";
        }
    }
}
