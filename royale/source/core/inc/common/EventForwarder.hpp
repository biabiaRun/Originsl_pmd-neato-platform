/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>
#include <royale/IEventListener.hpp>
#include <mutex>
#include <type_traits>

namespace royale
{
    /**
    * Implementation class for event sources.
    *
    * To be used as member variable. It holds the listener registration
    * (settable via setEventListener(), which the containing class should delegate to)
    * and allows convenient event generation via a template method.
    *
    * Copy construction and assignment is allowed this time.
    *
    */
    class EventForwarder
    {
    public:
        ROYALE_API EventForwarder();
        ROYALE_API ~EventForwarder() = default;
        ROYALE_API EventForwarder (const EventForwarder &src);
        ROYALE_API EventForwarder &operator= (const EventForwarder &src);

        ROYALE_API void setEventListener (royale::IEventListener *listener);

        template<class T, class... ArgTypes> inline
        void event (ArgTypes &&... args) const
        {
            static_assert (std::is_base_of<royale::IEvent, T>::value,
                           "event<T>(...) needs T to be derived from royale::IEvent");
            sendEvent (std::unique_ptr<royale::IEvent> (new T (std::forward<ArgTypes> (args)...)));
        }


    private:
        ROYALE_API void sendEvent (std::unique_ptr<royale::IEvent> &&event) const;
        royale::IEventListener *getEventListener() const;

        // data
        royale::IEventListener *m_listener;
        mutable std::mutex      m_mutex;

    }; // class EventForwarder
}
