/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <royale/IEvent.hpp>

namespace royale
{
    namespace event
    {
        /**
         * This class represents the event on which the external imager configuration file was not
         * found.
         */
        class EventImagerConfigNotFound : public royale::IEvent
        {
        public:
            ROYALE_API EventImagerConfigNotFound();

            /**
             * Get the severity of this event.
             * @return the severity of this event.
             */
            royale::EventSeverity severity() const override;

            /**
             * Get the description of this event.
             * @return the description of this event.
             */
            const royale::String describe() const override;

            /**
             * Get the type of this event.
             * @return the type of this event.
             */
            royale::EventType type() const override;

            /**
             * Set the name of the camera for which the configuration file could not be found.
             * @param cameraName the name of the camera.
             */
            void setCameraName (const String &cameraName);

            /**
             * Set the name of the configuration file which could not be found.
             * @param configFileName the name of the configuration file name.
             */
            void setConfigFileName (const String &configFileName);
        private:
            String m_cameraName;
            String m_configFileName;
        };
    }
}
