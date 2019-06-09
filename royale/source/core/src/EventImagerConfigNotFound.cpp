/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <common/events/EventImagerConfigNotFound.hpp>

namespace royale
{
    namespace event
    {
        EventImagerConfigNotFound::EventImagerConfigNotFound ()
        {
        }

        royale::EventSeverity EventImagerConfigNotFound::severity() const
        {
            return royale::EventSeverity::ROYALE_FATAL;
        }

        const royale::String EventImagerConfigNotFound::describe() const
        {
            String description;

            description += "The external imager configuration file '";
            description += m_configFileName;
            description += "' for the camera '";
            description += m_cameraName;
            description += "' was not found.";

            return description;
        }

        royale::EventType EventImagerConfigNotFound::type() const
        {
            return royale::EventType::ROYALE_UNKNOWN;
        }

        void EventImagerConfigNotFound::setCameraName (const String &cameraName)
        {
            m_cameraName = cameraName;
        }

        void EventImagerConfigNotFound::setConfigFileName (const String &configFileName)
        {
            m_configFileName = configFileName;
        }
    }
}
