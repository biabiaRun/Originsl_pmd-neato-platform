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
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace royale
{
    /**
    * An asynchronous FIFO for events.
    *
    * Threadsafe, but not reentrant. In particular, this means event listener code
    * is not supposed to call setEventListener() or cause the destructor to run.
    */
    class EventQueue : public royale::IEventListener
    {
    public:
        ROYALE_API EventQueue();
        ROYALE_API ~EventQueue();
        // No copies allowed.
        ROYALE_API EventQueue (const EventQueue &) = delete;
        EventQueue &operator= (const EventQueue &) = delete;

        ROYALE_API void setEventListener (IEventListener *listener);

        // queue input side
        ROYALE_API void onEvent (std::unique_ptr<royale::IEvent> &&event) override;

        // wait until queue is empty and callbacks are finished.
        ROYALE_API void sync();

    private:
        // queue output side worker thread function
        void eventNotifier();

    private:
        // data
        std::deque<std::unique_ptr<royale::IEvent>> m_queue;
        bool                                        m_callbackActive;
        std::mutex                                  m_queueMutex;
        std::condition_variable                     m_queueCond;    // signals queue change (with m_queueMutex)
        std::condition_variable                     m_callbackCond; // signals callback activity change (with m_queueMutex)
        std::unique_ptr<std::thread>                m_eventNotifier;
        std::mutex                                  m_listenerMutex;
        royale::IEventListener                     *m_listener;

    }; // class EventQueue
}
