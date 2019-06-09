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

#include <map>
#include <mutex>
#include <stdint.h>

//******************************************************************************
// this will be used in most of the wrapper functions
// checks if an instance exists for the given handle
// returns x in case of failure (this exits the function where this macro is used)
#define GET_INSTANCE_AND_RETURN_VALUE_IF_NULL(x) \
    auto instance = m_instanceManager.GetInstanceFromHandle (handle); \
    do { \
        if (instance == nullptr) \
        { \
            return (x); \
        } \
    } while(0)
//*****************************************************************************

namespace
{
    typedef uint64_t im_handle_type;

    // most significatn 4 bits of im_handle_type are defining the type of instances managed
    // this helps preventing accidental handover of the wrong handle
    enum class InstanceManagerType : im_handle_type
    {
        CAMERA_MANAGER  = 0x01000000000000000,
        CAMERA_DEVICE   = 0x02000000000000000,
        VARIANT         = 0x03000000000000000
    };

    template <class T> class InstanceManager
    {
    private:
        const im_handle_type NO_INSTANCE_HANDLE_NUMBER = 0;
        const im_handle_type FIRST_INSTANCE_HANDLE_NUMBER = 1;

        std::map<im_handle_type, T> m_instanceMap;
        im_handle_type m_nextHandleNumber;
        InstanceManagerType m_type;

        std::mutex m_lock;

    public:
        explicit InstanceManager (InstanceManagerType type);
        ~InstanceManager();

        im_handle_type AddInstance (T instance);
        T GetInstanceFromHandle (im_handle_type handle);
        T DeleteInstance (im_handle_type &handle);
    };

    template <class T> InstanceManager<T>::InstanceManager (InstanceManagerType type)
    {
        m_type = type;
        m_nextHandleNumber = im_handle_type (m_type) | FIRST_INSTANCE_HANDLE_NUMBER;
    }

    template <class T> im_handle_type InstanceManager<T>::AddInstance (T instance)
    {
        std::lock_guard<std::mutex> lock (m_lock);
        im_handle_type ht = (im_handle_type) m_nextHandleNumber;
        m_nextHandleNumber++;
        m_instanceMap[ht] = instance;
        return ht;
    }

    template <class T> InstanceManager<T>::~InstanceManager()
    {
    }

    template <class T> T InstanceManager<T>::GetInstanceFromHandle (im_handle_type handle)
    {
        std::lock_guard<std::mutex> lock (m_lock);
        auto it = m_instanceMap.find (handle);
        if (it == m_instanceMap.end())
        {
            return nullptr;
        }

        return m_instanceMap[handle];
    }

    template <class T> T InstanceManager<T>::DeleteInstance (im_handle_type &handle)
    {
        std::lock_guard<std::mutex> lock (m_lock);
        auto it = m_instanceMap.find (handle);
        if (it != m_instanceMap.end())
        {
            T instance = m_instanceMap[handle];
            m_instanceMap.erase (it);
            return instance;
        }
        handle = (im_handle_type) NO_INSTANCE_HANDLE_NUMBER;
        return nullptr;
    }
}
