/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/EventQueue.hpp>
#include <royale/IEvent.hpp>
#include <common/RoyaleLogger.hpp>

#include <cassert>

using namespace royale;

EventQueue::EventQueue()
    : m_queue(),
      m_callbackActive (false),
      m_queueMutex(),
      m_queueCond(),
      m_callbackCond(),
      m_eventNotifier(),
      m_listenerMutex(),
      m_listener (nullptr)
{

}

EventQueue::~EventQueue()
{
    setEventListener (nullptr);
}

void EventQueue::setEventListener (IEventListener *listener)
{
    std::unique_lock<std::mutex> queueLock (m_queueMutex);
    std::unique_lock<std::mutex> listenerLock (m_listenerMutex);

    if (m_listener && !listener)
    {
        assert (m_eventNotifier);
        m_listener = nullptr;
        m_queue.clear();
        m_queueCond.notify_all();
        // must unlock before join to allow eventNotifier to run (and terminate)
        listenerLock.unlock();
        queueLock.unlock();
        m_eventNotifier->join();
        m_eventNotifier.reset (nullptr);
        return;
    }

    m_listener = listener;
    if (m_listener && !m_eventNotifier)
    {
        try
        {
            m_eventNotifier.reset (new std::thread ([this] { eventNotifier(); }));
        }
        catch (...)
        {
            m_listener = nullptr;
            m_queue.clear();
            m_queueCond.notify_all();
            throw;
        }
    }
}

void EventQueue::onEvent (std::unique_ptr<royale::IEvent> &&event)
{
    std::unique_lock<std::mutex> lock (m_queueMutex);
    // It makes no sense to push events if there is no listener.
    if (m_listener)
    {
        m_queue.push_back (std::move (event));
        m_queueCond.notify_all();
    }
}


void EventQueue::sync()
{
    std::unique_lock<std::mutex> lock (m_queueMutex);
    m_queueCond.wait (lock, [this] { return m_queue.empty(); });
    m_callbackCond.wait (lock, [this] { return !m_callbackActive; });
}



void EventQueue::eventNotifier()
{
    std::unique_lock<std::mutex> queueLock (m_queueMutex);

    while (m_listener)
    {
        // Wait for events or the end of the world, whichever comes earlier
        while (m_listener && m_queue.empty())
        {
            m_queueCond.wait (queueLock);
        }

        std::unique_lock<std::mutex> listenerLock (m_listenerMutex);

        if (!m_listener)
        {
            return; // listener was unset, so this thread finishes.
        }
        assert (!m_queue.empty());
        assert (m_listener);

        auto event    = std::move (m_queue.front());

        m_queue.pop_front();
        m_queueCond.notify_all();

        // Queue lock is released while user event handler is running,
        // so it won't block royale functions that generate events.
        m_callbackActive = true;
        m_callbackCond.notify_all();
        queueLock.unlock();
        // m_listenerMutex stays locked.

        try
        {
            m_listener->onEvent (std::move (event));
        }
        catch (...)
        {
            LOG (WARN) << "EventQueue: callback failed (exception)";
        }
        listenerLock.unlock();
        queueLock.lock();
        m_callbackActive = false;
        m_callbackCond.notify_all();
    }

}
